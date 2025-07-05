//CGameplayAbility.cpp

#include "GAS/CGameplayAbility.h"
#include "GameFramework/Character.h"
#include "GAS/GAP_Launched.h"
#include "GAS/CAbilitySystemStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"

UCGameplayAbility::UCGameplayAbility()
{
    ActivationBlockedTags.AddTag();
}

UAnimInstance* UCGameplayAbility::GetOwnerAnimInstance() const
{
    USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
    if (OwnerSkeletalMeshComp)
    {
        return OwnerSkeletalMeshComp->GetAnimInstance();
    }

    return nullptr;
}

TArray<FHitResult> UCGameplayAbility::GetHitResultsFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, ETeamAttitude::Type TargetTeam, bool bDrawDebug, bool bIgnoreSelf) const
{
    TArray<FHitResult> OutResult;
	TSet<AActor*> HitActors; // Để tránh trùng lặp Actor trong kết quả

    // Lấy giao diện IGenericTeamAgentInterface từ Actor Info
	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());

    // 1. Duyệt qua từng TargetData (mỗi cặp Start/End location)
    for (const TSharedPtr<FGameplayAbilityTargetData>& TargetData : TargetDataHandle.Data)
    {
        // 2. Lấy vị trí bắt đầu/kết thúc từ TargetData
        FVector StartLoc = TargetData->GetOrigin().GetTranslation(); // SourceLocation
        FVector EndLoc = TargetData->GetEndPoint();                 // TargetLocation

        // 3. Thiết lập Object Types (chỉ trace Pawn)
        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
        ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

        // 4. Thiết lập Actors to Ignore (nếu cần)
        TArray<AActor*> ActorsToIgnore;
        if (bIgnoreSelf)
        {
            ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
        }

        // 5. Chế độ Debug (tùy chọn)
        EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

        // 6. Thực hiện Sphere Trace Multi
        TArray<FHitResult> Results;
        UKismetSystemLibrary::SphereTraceMultiForObjects(
            this,              // this thay vì GetWorld() → Sẽ gây crash nếu this không phải AActor
            StartLoc,          // Start location
            EndLoc,            // End location
            SphereSweepRadius, // Bán kính trace
            ObjectTypes,       // Loại object cần trace (Pawn)
            false,             // Trace complex (false = dùng simple collision)
            ActorsToIgnore,    // Actors bỏ qua
            DrawDebugType,     // Vẽ debug trace
            Results,           // Kết quả va chạm
            false              // Ignore bTraceComplex
        );

		for (const FHitResult& Result : Results)
		{
			if (HitActors.Contains(Result.GetActor()))
			{
                continue; // Bỏ qua nếu Actor đã bị trúng đòn trước đó
			}

			if (OwnerTeamInterface)
			{
                // Lấy Attitude của Actor bị trúng đòn
				ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*Result.GetActor());
				if (OtherActorTeamAttitude != TargetTeam)
				{
					continue; // Bỏ qua nếu Actor không cùng team
				}
			}

			HitActors.Add(Result.GetActor()); // Thêm Actor vào HitActors nếu chưa bị trúng
			OutResult.Add(Result); // Thêm kết quả va chạm vào mảng kết quả
		}
    }

    return OutResult;
}

void UCGameplayAbility::PushSelf(const FVector& PushVel)
{
    ACharacter* OwningAvatarCharacter = GetOwningAvatarCharacter();
    if (OwningAvatarCharacter)
    {
        OwningAvatarCharacter->LaunchCharacter(PushVel, true, true);
    }
}

void UCGameplayAbility::PushTarget(AActor* Target, const FVector& PushVel)
{
    if (!Target) return;

    FGameplayEventData EventData;

    FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit;
    FHitResult HitResult;
    HitResult.ImpactNormal = PushVel;
    HitData->HitResult = HitResult;
    EventData.TargetData.Add(HitData);

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, UGAP_Launched::GetLaunchedAbilityActivationTag(), EventData);
}

ACharacter* UCGameplayAbility::GetOwningAvatarCharacter()
{
    if (!AvatarCharacter)
    {
        AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    }

    return AvatarCharacter;
}


void UCGameplayAbility::ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult,
    TSubclassOf<UGameplayEffect> GameplayEffect,
    int Level)
{
    FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, Level);

    FGameplayEffectContextHandle EffectContext = EffectSpecHandle.Data->GetContext();
    EffectContext.AddHitResult(HitResult);
    EffectSpecHandle.Data->SetContext(EffectContext);

    // Tạo TargetData từ Actor bị hit
    FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor());

    ApplyGameplayEffectToTarget(
        GetCurrentAbilitySpecHandle(),
        CurrentActorInfo,
        GetCurrentActivationInfo(),
        TargetDataHandle,
        GameplayEffect,
        Level
    );
}




