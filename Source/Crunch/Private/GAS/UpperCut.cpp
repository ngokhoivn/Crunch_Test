﻿// UpperCut.cpp

#include "GAS/UpperCut.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/GA_Combo.h"
#include "GameplayTagsManager.h"

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

    NextComboName = NAME_None;
}

UUpperCut::UUpperCut()
{
    BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

FGameplayTag UUpperCut::GetUpperCutLaunchTag()
{
    return FGameplayTag::RequestGameplayTag("Ability.UpperCut.Launch");
}

const FGenericDamageEffectDef* UUpperCut::GetDamageEffectDefForCurrentCombo() const
{
    UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
    if (OwnerAnimInstance)
    {
        FName CurrentComboName = OwnerAnimInstance->Montage_GetCurrentSection(UpperCutMontage);
        const FGenericDamageEffectDef* EffectDef = ComboDamageMap.Find(CurrentComboName);
        return EffectDef;
    }

    return nullptr;
}

void UUpperCut::StartLaunching(FGameplayEventData EventData)
{
    if (K2_HasAuthority())
    {
        PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutLaunchSpeed);
        int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);

        for (int i = 0; i < HitResultCount; i++)
        {
            FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, i);
            PushTarget(HitResult.GetActor(), FVector::UpVector * UpperCutLaunchSpeed);
            ApplyGameplayEffectToHitResultActor(HitResult, LaunchDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
        }
    }

    UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboChangedEventTag(), nullptr, false, false);
    WaitComboChangeEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboChangeEvent);
    WaitComboChangeEvent->ReadyForActivation();

    UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
    WaitComboCommitEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboCommitEvent);
    WaitComboCommitEvent->ReadyForActivation();

    UAbilityTask_WaitGameplayEvent* WaitComboDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboTargetEventTag());
    WaitComboDamageEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboDamageEvent);
    WaitComboDamageEvent->ReadyForActivation();
}

void UUpperCut::HandleComboChangeEvent(FGameplayEventData EventData)
{
    FGameplayTag EventTag = EventData.EventTag;
    if (EventTag == UGA_Combo::GetComboChangedEventEndTag())
    {
        NextComboName = NAME_None;
        return;
    }

    TArray<FName> TagNames;
    UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
    NextComboName = TagNames.Last();
}

void UUpperCut::HandleComboCommitEvent(FGameplayEventData EventData)
{
    if (NextComboName == NAME_None)
    {
        return;
    }

    UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
    if (!OwnerAnimInst)
    {
        return;
    }

    OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(UpperCutMontage), NextComboName, UpperCutMontage);
}

void UUpperCut::HandleComboDamageEvent(FGameplayEventData EventData)
{
    if (K2_HasAuthority())
    {
        PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperComboHoldSpeed);
        const FGenericDamageEffectDef* EffectDef = GetDamageEffectDefForCurrentCombo();
        if (!EffectDef)
        {
            return;
        }

        int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);
        for (int i = 0; i < HitResultCount; i++)
        {
            FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, i);
            FVector PushVel = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(EffectDef->PushVelocity);
            PushTarget(HitResult.GetActor(), PushVel);
            ApplyGameplayEffectToHitResultActor(HitResult, EffectDef->DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
        }
    }
}

