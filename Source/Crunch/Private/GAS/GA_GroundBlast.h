//GAGroundBlast.h

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GA_GroundBlast.generated.h"

/**
 * 
 */
UCLASS()
class UGA_GroundBlast : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_GroundBlast();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGenericDamageEffectDef DamageEffectDef;

	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	FGameplayTag BlastGameplayCueTag;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetAreaRadius = 300.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetTraceRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ATargetActor_GroundPick> TargetActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* GroundBlastMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* CastMontage;

	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void TargetCanceled(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
};
