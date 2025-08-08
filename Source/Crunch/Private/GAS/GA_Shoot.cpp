#include "GAS/GA_Shoot.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GameplayTagsManager.h"
#include "GAS/ProjectileActor.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/OverlapResult.h"

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
	}
}

void UGA_Shoot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	K2_EndAbility();
}

void UGA_Shoot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
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
	return FGameplayTag::RequestGameplayTag("Ability.Shoot");
}

void UGA_Shoot::StartShooting(FGameplayEventData Payload)
{
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
	if (ShootMontage)
	{
		StopMontageAfterCurrentSection(ShootMontage);
	}

	StopAimTargetCheckTimer();
}

void UGA_Shoot::ShootProjectile(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
		if (!OwnerAvatarActor)
		{
			return;
		}

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
				SocketLocation += ProjectileSpawnOffset;
			}
		}

		FVector TargetLocation;
		AActor* FinalTarget = GetAimTargetIfValid();

		if (FinalTarget)
		{
			TargetLocation = FinalTarget->GetActorLocation();
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

				UKismetSystemLibrary::LineTraceSingle(this, TraceStart, TraceEnd, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);

				TargetLocation = HitResult.bBlockingHit ? HitResult.Location : TraceEnd;
			}
			else
			{
				TargetLocation = OwnerAvatarActor->GetActorLocation() + (OwnerAvatarActor->GetActorForwardVector() * ShootProjectileRange);
			}
		}

		const FVector CentralDirection = (TargetLocation - SocketLocation).GetSafeNormal();
		const int32 NumberOfProjectiles = 5;

		for (int32 i = 0; i < NumberOfProjectiles; ++i)
		{
			// Calculate the rotation for this projectile
			float Angle;
			if (NumberOfProjectiles > 1)
			{
				// Spread projectiles evenly in the cone
				Angle = (i / (float)(NumberOfProjectiles - 1)) * ConeSpreadAngle - (ConeSpreadAngle / 2.0f);
			}
			else
			{
				Angle = 0.f; // Only one projectile, no spread
			}

			// Rotate the central direction vector by the calculated angle
			// We rotate around the owner's Up vector to spread them horizontally
			FVector RotatedDirection = CentralDirection.RotateAngleAxis(Angle, OwnerAvatarActor->GetActorUpVector());

			FRotator ProjectileRotation = RotatedDirection.Rotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwnerAvatarActor;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AProjectileActor* Projectile = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass, SocketLocation, ProjectileRotation, SpawnParams);
			if (Projectile)
			{
				Projectile->ShootProjectile(ShootProjectileSpeed, ShootProjectileRange, FinalTarget, GetOwnerTeamId(), MakeOutgoingGameplayEffectSpec(ProjectileHitEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)));
			}
		}
	}
}

AActor* UGA_Shoot::GetAimTargetIfValid() const
{
	if (HasValidTarget())
		return AimTarget;

	return nullptr;
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

void UGA_Shoot::FindAimTarget()
{
	AActor* PreviousTarget = AimTarget;
	AActor* AutoTarget = GetClosestHostileInFront();

	if (AutoTarget)
	{
		if (AimTarget != AutoTarget)
		{
			if (AimTargetAbilitySystemComponent)
			{
				AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
				AimTargetAbilitySystemComponent = nullptr;
			}

			AimTarget = AutoTarget;

			AimTargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AimTarget);
			if (AimTargetAbilitySystemComponent)
			{
				AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &UGA_Shoot::TargetDeadTagUpdated);
			}
		}
	}
	else
	{
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
	}

	if (PreviousTarget != AimTarget)
	{
		FGameplayEventData EventData;
		EventData.Target = AimTarget;
		SendLocalGameplayEvent(UCAbilitySystemStatics::GetTargetUpdatedTag(), EventData);
	}
}

AActor* UGA_Shoot::GetClosestHostileInFront()
{
	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (!OwnerActor)
		return nullptr;

	FVector OwnerLocation = OwnerActor->GetActorLocation();
	FVector OwnerForward = OwnerActor->GetActorForwardVector();

	AActor* ClosestHostile = nullptr;
	float ClosestDistance = FLT_MAX;

	const float DotProductThreshold = FMath::Cos(FMath::DegreesToRadians(AutoTargetAngle));

	TArray<AActor*> FoundActors;
	GetActorsInRange(OwnerLocation, AutoTargetRange, FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (!Actor || Actor == OwnerActor)
			continue;

		if (GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
			continue;

		if (UCAbilitySystemStatics::IsActorDead(Actor))
			continue;

		FVector DirectionToTarget = (Actor->GetActorLocation() - OwnerLocation).GetSafeNormal();
		float DotProduct = FVector::DotProduct(OwnerForward, DirectionToTarget);

		if (DotProduct < DotProductThreshold)
			continue;

		float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
		float WeightedDistance = Distance * (2.0f - DotProduct);

		if (WeightedDistance < ClosestDistance)
		{
			ClosestDistance = WeightedDistance;
			ClosestHostile = Actor;
		}
	}

	return ClosestHostile;
}

void UGA_Shoot::GetActorsInRange(const FVector& Center, float Range, TArray<AActor*>& OutActors)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());

	TArray<FOverlapResult> OverlapResults;
	bool bHit = World->OverlapMultiByChannel(
		OverlapResults,
		Center,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Range),
		QueryParams
	);

	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (Result.GetActor())
			{
				OutActors.Add(Result.GetActor());
			}
		}
	}
}