// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "GA_Shoot.generated.h"

/**
 *
 */
UCLASS()
class UGA_Shoot : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Shoot();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	TSubclassOf<UGameplayEffect> ProjectileHitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ShootProjectileSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ShootProjectileRange = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	TSubclassOf<class AProjectileActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* ShootMontage;

	static FGameplayTag GetShootTag();

	UFUNCTION()
	void StartShooting(FGameplayEventData Payload);

	UFUNCTION()
	void StopShooting(FGameplayEventData Payload);

	UFUNCTION()
	void ShootProjectile(FGameplayEventData Payload);

	AActor* GetAimTargetIfValid() const;

	UPROPERTY()
	AActor* AimTarget;

	UPROPERTY()
	UAbilitySystemComponent* AimTargetAbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	FVector ProjectileSpawnOffset;

	FTimerHandle AimTargetCheckTimerHandle;

	void FindAimTarget();

	UPROPERTY(EditDefaultsOnly, Category = "Target")
	float AimTargetCheckTimeInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Auto Targeting", meta = (AllowPrivateAccess = "true"))
	float AutoTargetRange = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Auto Targeting", meta = (AllowPrivateAccess = "true"))
	float AutoTargetAngle = 70.0f; // Degrees

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ConeSpreadAngle = 10.0f; // Angle in degrees for the cone spread

	// Helper functions
	AActor* GetClosestHostileInFront();
	void GetActorsInRange(const FVector& Center, float Range, TArray<AActor*>& OutActors);

	void StartAimTargetCheckTimer();
	void StopAimTargetCheckTimer();

	bool HasValidTarget() const;
	bool IsTargetInRange() const;

	void TargetDeadTagUpdated(const FGameplayTag Tag, int32 NewCount);
};