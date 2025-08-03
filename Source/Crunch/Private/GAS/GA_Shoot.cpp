#include "GAS/GA_Shoot.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GameplayTagsManager.h"
#include "GAS/ProjectileActor.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

UGA_Shoot::UGA_Shoot()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetCrosshairTag());
}

void UGA_Shoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] Ability Activated."));

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* WaitStartShootingEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
		WaitStartShootingEvent->EventReceived.AddDynamic(this, &UGA_Shoot::StartShooting);
		WaitStartShootingEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitStopShootingEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UCAbilitySystemStatics::GetBasicAttackInputReleasedTag());
		WaitStopShootingEvent->EventReceived.AddDynamic(this, &UGA_Shoot::StopShooting);
		WaitStopShootingEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootProjectileEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetShootTag(), nullptr, false, false);
		WaitShootProjectileEvent->EventReceived.AddDynamic(this, &UGA_Shoot::ShootProjectile);
		WaitShootProjectileEvent->ReadyForActivation();
		UE_LOG(LogTemp, Log, TEXT("[GA_Shoot] Waiting for 'Ability.Shoot' event."));
	}
}

void UGA_Shoot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] Input Released, Ability Ended."));
	K2_EndAbility();
}

void UGA_Shoot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] EndAbility called."));
	if (AimTargetAbilitySystemComponent)
	{
		AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
		AimTargetAbilitySystemComponent = nullptr;
	}
	SendLocalGameplayEvent(UCAbilitySystemStatics::GetTargetUpdatedTag(), FGameplayEventData());

	StopShooting(FGameplayEventData());
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_Shoot::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("ability.shoot");
}

void UGA_Shoot::StartShooting(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] StartShooting triggered."));
	if (HasAuthority(&CurrentActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayShootMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ShootMontage);
		PlayShootMontage->ReadyForActivation();
	}
	else
	{
		PlayMontageLocally(ShootMontage);
	}

	FindAimTarget();
	StartAimTargetCheckTimer();
}

void UGA_Shoot::StopShooting(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] StopShooting triggered."));
	if (ShootMontage)
	{
		StopMontageAfterCurrentSection(ShootMontage);
	}

	StopAimTargetCheckTimer();
}

void UGA_Shoot::ShootProjectile(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] ShootProjectile event received with tag: %s on %s"), *Payload.EventTag.ToString(), K2_HasAuthority() ? TEXT("Server") : TEXT("Client"));

	if (K2_HasAuthority())
	{
		AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
		if (!OwnerAvatarActor)
		{
			UE_LOG(LogTemp, Error, TEXT("[GA_Shoot] OwnerAvatarActor is null."));
			return;
		}

		// 1. Get Socket Location
		FVector SocketLocation = OwnerAvatarActor->GetActorLocation();
		USkeletalMeshComponent* MeshComp = GetOwningComponentFromActorInfo();
		if (MeshComp)
		{
			TArray<FName> OutNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(Payload.EventTag, OutNames);
			FName SocketName = OutNames.Num() > 0 ? OutNames.Last() : NAME_None;
			if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
			{
				SocketLocation = MeshComp->GetSocketLocation(SocketName);
				SocketLocation += ProjectileSpawnOffset; // 🎯 Dùng offset đã khai báo
			}
		}

		// 2. Determine Target Location
		FVector TargetLocation;
		AActor* CurrentAimTarget = GetAimTargetIfValid();

		if (CurrentAimTarget)
		{
			TargetLocation = CurrentAimTarget->GetActorLocation();
		}
		else
		{
			APawn* OwnerPawn = Cast<APawn>(OwnerAvatarActor);
			AController* Controller = OwnerPawn ? OwnerPawn->GetController() : nullptr;

			if (Controller)
			{
				FVector CameraLocation;
				FRotator CameraRotation;
				Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

				FVector TraceStart = CameraLocation;
				FVector TraceEnd = TraceStart + (CameraRotation.Vector() * ShootProjectileRange);

				FHitResult HitResult;
				TArray<AActor*> ActorsToIgnore;
				ActorsToIgnore.Add(OwnerAvatarActor);

				bool bHit = UKismetSystemLibrary::LineTraceSingle(
					this,
					TraceStart,
					TraceEnd,
					UEngineTypes::ConvertToTraceType(ECC_Visibility),
					false,
					ActorsToIgnore,
					EDrawDebugTrace::ForDuration,
					HitResult,
					false
				);

				TargetLocation = bHit ? HitResult.Location : TraceEnd;
			}
			else
			{
				// Fallback if controller is not available (e.g. for AI)
				TargetLocation = OwnerAvatarActor->GetActorLocation() + (OwnerAvatarActor->GetActorForwardVector() * ShootProjectileRange);
			}
		}

		// 3. Calculate rotation from socket to target
		FRotator ProjectileRotation = (TargetLocation - SocketLocation).Rotation();

		// 4. Spawn Projectile
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerAvatarActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AProjectileActor* Projectile = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass, SocketLocation, ProjectileRotation, SpawnParams);
		if (Projectile)
		{
			UE_LOG(LogTemp, Log, TEXT("[GA_Shoot] Projectile successfully spawned at %s, targeting %s"), *SocketLocation.ToString(), *TargetLocation.ToString());
			Projectile->ShootProjectile(ShootProjectileSpeed, ShootProjectileRange, GetAimTargetIfValid(), GetOwnerTeamId(), MakeOutgoingGameplayEffectSpec(ProjectileHitEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[GA_Shoot] Failed to spawn projectile!"));
		}
	}
}


AActor* UGA_Shoot::GetAimTargetIfValid() const
{
	if (HasValidTarget())
		return AimTarget;

	return nullptr;
}

void UGA_Shoot::FindAimTarget()
{
	if (HasValidTarget())
		return;

	if (AimTargetAbilitySystemComponent)
	{
		AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
		AimTargetAbilitySystemComponent = nullptr;
	}

	AimTarget = GetAimTarget(ShootProjectileRange, ETeamAttitude::Hostile);
	if (AimTarget)
	{
		AimTargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AimTarget);
		if (AimTargetAbilitySystemComponent)
		{
			AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &UGA_Shoot::TargetDeadTagUpdated);
		}
	}

	FGameplayEventData EventData;
	EventData.Target = AimTarget;
	SendLocalGameplayEvent(UCAbilitySystemStatics::GetTargetUpdatedTag(), EventData);
}

void UGA_Shoot::StartAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(AimTargetCheckTimerHandle, this, &UGA_Shoot::FindAimTarget, AimTargetCheckTimeInterval, true);
	}
}

void UGA_Shoot::StopAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AimTargetCheckTimerHandle);
	}
}

bool UGA_Shoot::HasValidTarget() const
{
	if (!AimTarget)
		return false;

	if (UCAbilitySystemStatics::IsActorDead(AimTarget))
		return false;

	if (!IsTargetInRange())
		return false;

	return true;
}

bool UGA_Shoot::IsTargetInRange() const
{
	if (!AimTarget)
		return false;

	float Distance = FVector::Distance(AimTarget->GetActorLocation(), GetAvatarActorFromActorInfo()->GetActorLocation());
	return Distance <= ShootProjectileRange;
}

void UGA_Shoot::TargetDeadTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		FindAimTarget();
	}
}