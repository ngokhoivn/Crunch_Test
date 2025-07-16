#include "GAS/GAP_Dead.h"
#include "GAS/CAbilitySystemStatics.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"
#include "Engine/OverlapResult.h"

UGAP_Dead::UGAP_Dead()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    TriggerData.TriggerTag = UCAbilitySystemStatics::GetDeadStatTag();

    AbilityTriggers.Add(TriggerData);
    ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatTag());
}

void UGAP_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!K2_HasAuthority() || !TriggerEventData)
    {
        K2_EndAbility();
        return;
    }

    // 1. Xác định Killer
    AActor* Killer = TriggerEventData->ContextHandle.GetEffectCauser();
    if (!IsValid(Killer) || !UCAbilitySystemStatics::IsHero(Killer))
    {
        Killer = nullptr;
    }

    // 2. Lấy danh sách mục tiêu nhận thưởng
    TArray<AActor*> RewardTargets = GetRewardTargets();

    // 3. Kiểm tra điều kiện kết thúc sớm
    if (RewardTargets.Num() == 0 && !Killer)
    {
        K2_EndAbility();
        return;
    }

    // 4. Thêm Killer vào danh sách nếu chưa có
    if (Killer && !RewardTargets.Contains(Killer))
    {
        RewardTargets.Add(Killer);
    }

    // 5. Tính toán phần thưởng
    bool bFound = false;
    float SelfExperience = 0.f;
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        SelfExperience = ASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(), bFound);
    }

    float TotalExperienceReward = BaseExperienceReward + ExperienceRewardPerExperience * SelfExperience;
    float TotalGoldReward = BaseGoldReward + GoldRewardPerExperience * SelfExperience;

    // 6. Áp dụng phần thưởng cho Killer (nếu có)
    if (Killer)
    {
        float KillerExperienceReward = TotalExperienceReward * KillerRewardPortion;
        float KillerGoldReward = TotalGoldReward * KillerRewardPortion;

        FGameplayEffectSpecHandle KillerEffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
        if (KillerEffectSpec.IsValid())
        {
            KillerEffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetExperienceAttributeTag(), KillerExperienceReward);
            KillerEffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetGoldAttributeTag(), KillerGoldReward);

            ApplyGameplayEffectSpecToTarget(Handle, ActorInfo, ActivationInfo, KillerEffectSpec,
                UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));
        }

        TotalExperienceReward -= KillerExperienceReward;
        TotalGoldReward -= KillerGoldReward;
    }

    // 7. Chia phần thưởng cho các mục tiêu còn lại
    if (RewardTargets.Num() > 0)
    {
        float ExperiencePerTarget = TotalExperienceReward / RewardTargets.Num();
        float GoldPerTarget = TotalGoldReward / RewardTargets.Num();

        FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
        if (EffectSpec.IsValid())
        {
            EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetExperienceAttributeTag(), ExperiencePerTarget);
            EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetGoldAttributeTag(), GoldPerTarget);

            ApplyGameplayEffectSpecToTarget(Handle, ActorInfo, ActivationInfo, EffectSpec,
                UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(RewardTargets, true));
        }
    }

    K2_EndAbility();
}

TArray<AActor*> UGAP_Dead::GetRewardTargets() const
{
    TArray<AActor*> ValidTargets;

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!IsValid(AvatarActor) || !GetWorld())
    {
        return ValidTargets;
    }

    // Thiết lập tham số cho overlap
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

    FCollisionShape CollisionShape;
    CollisionShape.SetSphere(RewardRange);

    TArray<FOverlapResult> OverlapResults;
    GetWorld()->OverlapMultiByObjectType(OverlapResults, AvatarActor->GetActorLocation(),
        FQuat::Identity, ObjectQueryParams, CollisionShape);

    // Lọc các mục tiêu hợp lệ
    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* Target = Result.GetActor();
        if (!IsValid(Target))
        {
            continue;
        }

        // Kiểm tra team và hero status
        const IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(Target);
        if (!TeamInterface || TeamInterface->GetTeamAttitudeTowards(*AvatarActor) != ETeamAttitude::Hostile)
        {
            continue;
        }

        if (UCAbilitySystemStatics::IsHero(Target))
        {
            ValidTargets.Add(Target);
        }
    }

    return ValidTargets;
}