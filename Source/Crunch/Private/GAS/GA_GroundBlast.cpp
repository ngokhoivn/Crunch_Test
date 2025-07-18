// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/GA_GroundBlast.h"
#include "AbilitySystemComponent.h"
#include "GAS/CAbilitySystemStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/TargetActor_GroundPick.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GameplayTagsManager.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "TimerManager.h"

UGA_GroundBlast::UGA_GroundBlast()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayGroundBlastAnimTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GroundBlastMontage);
	//PlayGroundBlastAnimTask->OnBlendOut.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	//PlayGroundBlastAnimTask->OnCancelled.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	//PlayGroundBlastAnimTask->OnInterrupted.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	//PlayGroundBlastAnimTask->OnCompleted.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayGroundBlastAnimTask->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &UGA_GroundBlast::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this, &UGA_GroundBlast::TargetCanceled);
	WaitTargetDataTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, TargetActor);

	ATargetActor_GroundPick* GroundPickActor = Cast<ATargetActor_GroundPick>(TargetActor);
	if (GroundPickActor)
	{
		GroundPickActor->SetShouldDrawDebug(ShouldDrawDebug());
		GroundPickActor->SetTargetAreaRadius(TargetAreaRadius);
		GroundPickActor->SetTargetTraceRange(TargetTraceRange);
	}
	WaitTargetDataTask->FinishSpawningActor(this, TargetActor);
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (HasAuthority(&CurrentActivationInfo) && K2_CommitAbility())
	{
		// 1. Gửi GameplayCue để vẽ đường đạn (trail)
		FGameplayCueParameters CueParams;
		// Lấy vị trí socket tay của nhân vật, hoặc một vị trí phù hợp để bắt đầu đường đạn
		if (ACharacter* MyCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			CueParams.Location = MyCharacter->GetMesh()->GetSocketLocation(FName("lowerarm_r")); 
		}
		CueParams.TargetAttachComponent = nullptr;

		// Dùng GameplayCue với Niagara trail (bạn cần tạo tag này trong Project Settings)
		static FGameplayTag TrailCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.GroundBlast.Trail"));
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->ExecuteGameplayCue(TrailCueTag, CueParams);
		}

		// 2. Delay nhỏ rồi spawn explosion + gây damage
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;

		TimerDelegate.BindLambda([this, TargetData]()
			{
				if (TargetData.Num() > 0)
				{
					// 2.1 Explosion Cue
					FGameplayCueParameters ExplosionParams;
					const FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetData, 0);
					if (HitResult.IsValidBlockingHit())
					{
						ExplosionParams.Location = HitResult.ImpactPoint;
						ExplosionParams.Normal = HitResult.ImpactNormal;
						ExplosionParams.RawMagnitude = TargetAreaRadius;

						FGameplayEffectContextHandle ContextHandle = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);
						ContextHandle.AddHitResult(HitResult);
						ExplosionParams.EffectContext = ContextHandle;

						if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
						{
							static FGameplayTag ExplosionCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.GroundBlast.Explosion"));
							ASC->ExecuteGameplayCue(ExplosionCueTag, ExplosionParams);
						}
					}

					// 2.2 Gây Damage và Đẩy lùi
					BP_ApplyGameplayEffectToTarget(TargetData, DamageEffectDef.DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
					FVector PushForce = FVector(0.0f, 0.0f, 300.0f);
					PushTargets(TargetData, PushForce, 0.5f);
				}

				K2_EndAbility();
			});

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.3f, false); // delay 0.3s cho trail bay
	}
	else
	{
		K2_EndAbility();
	}
}

void UGA_GroundBlast::TargetCanceled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	K2_EndAbility();
}


