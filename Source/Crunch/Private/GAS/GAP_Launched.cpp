//GAP_Launched.cpp

#include "GAS/GAP_Launched.h"
#include "GAS/CAbilitySystemStatics.h"

UGAP_Launched::UGAP_Launched()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = GetLaunchedAbilityActivationTag();

	ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatTag());

	AbilityTriggers.Add(TriggerData);
}

void UGAP_Launched::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (K2_HasAuthority())
	{
		PushSelf(TriggerEventData->TargetData.Get(0)->GetHitResult()->ImpactNormal);
		K2_EndAbility();
	}
}

FGameplayTag UGAP_Launched::GetLaunchedAbilityActivationTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Passive.Launch.Activate");
}
