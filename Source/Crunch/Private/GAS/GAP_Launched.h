//GAP_Launched.h

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GAP_Launched.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Launched : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGAP_Launched();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	static FGameplayTag GetLaunchedAbilityActivationTag();
};
