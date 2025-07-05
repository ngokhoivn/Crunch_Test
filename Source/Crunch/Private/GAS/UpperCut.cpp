// UpperCut.cpp

#include "GAS/UpperCut.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UUpperCut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        K2_EndAbility();
        return;
    }

    if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        UAbilityTask_PlayMontageAndWait* PlayUpperCutMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            UpperCutMontage);

        PlayUpperCutMontageTask->OnBlendOut.AddDynamic(this, &UUpperCut::K2_EndAbility);
        PlayUpperCutMontageTask->OnCancelled.AddDynamic(this, &UUpperCut::K2_EndAbility);
        PlayUpperCutMontageTask->OnInterrupted.AddDynamic(this, &UUpperCut::K2_EndAbility);
        PlayUpperCutMontageTask->OnCompleted.AddDynamic(this, &UUpperCut::K2_EndAbility);
        PlayUpperCutMontageTask->ReadyForActivation();

        UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetUpperCutLaunchTag());
        WaitLaunchEventTask->EventReceived.AddDynamic(this, &UUpperCut::StartLaunching);
        WaitLaunchEventTask->ReadyForActivation();
    }
}

FGameplayTag UUpperCut::GetUpperCutLaunchTag()
{
    return FGameplayTag::RequestGameplayTag("Ability.UpperCut.Launch");
}

void UUpperCut::StartLaunching(FGameplayEventData EventData)
{
    TArray<FHitResult> TargetHitResults = GetHitResultsFromSweepLocationTargetData(
        EventData.TargetData,
        TargetSweepSphereRadius,
        ETeamAttitude::Hostile,
        ShouldDrawDebug());

    
    PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutLaunchSpeed);
        
    if (K2_HasAuthority())
    {
        for (const FHitResult& HitResult : TargetHitResults)
        {
            PushTarget(HitResult.GetActor(), FVector::UpVector * UpperCutLaunchSpeed);
            ApplyGameplayEffectToHitResultActor(HitResult, LaunchDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

        }
    }
}
