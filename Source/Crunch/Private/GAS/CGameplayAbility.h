//CGameplayAbility.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "CGameplayAbility.generated.h"

/**
 *
 */
UCLASS()
class UCGameplayAbility : public UGameplayAbility, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	UCGameplayAbility();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	class UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultsFromSweepLocationTargetData(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, // đối tượng mục tiêu
	float SphereSweepRadius = 30.0f, 
	ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile, // mặc định là kẻ thù
	bool bDrawDebug = false, bool bIgnoreSelf = true) const; // bIgnoreSelf: bỏ qua chính mình trong kết quả va chạm

	UFUNCTION()
	FORCEINLINE	bool ShouldDrawDebug() const { return bShouldDrawDebug; }

	void PushSelf(const FVector& PushVel);
	void PushTarget(AActor* Target, const FVector& PushVel);
	void PushTargets(const TArray<AActor*>& Targets, const FVector& PushVel);
	void PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& PushForce, float PushDuration);
	ACharacter* GetOwningAvatarCharacter();

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);


private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShouldDrawDebug = false;

	UPROPERTY()
	class ACharacter* AvatarCharacter;
};
