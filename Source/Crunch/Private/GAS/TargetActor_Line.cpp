// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/TargetActor_Line.h"
#include "Crunch/Crunch.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/KismetMathLibrary.h"

ATargetActor_Line::ATargetActor_Line()
{
	RootComp = CreateDefaultSubobject<USceneComponent>("Root Comp");
	SetRootComponent(RootComp);

	TargetEndDetectionSphere = CreateDefaultSubobject<USphereComponent>("Target End Detection Sphere");
	TargetEndDetectionSphere->SetupAttachment(GetRootComponent());
	TargetEndDetectionSphere->SetCollisionResponseToChannel(ECC_SpringArm, ECR_Ignore);

	LazerVFX = CreateDefaultSubobject<UNiagaraComponent>("Lazer VFX");
	LazerVFX->SetupAttachment(GetRootComponent());

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	ShouldProduceTargetDataOnServer = true;

	AvatarActor = nullptr;
}

void ATargetActor_Line::ConfigureTargetSetting(float NewTargetRange, float NewDetectionCylinderRadius, float NewTargetingInterval, FGenericTeamId OwnerTeamId, bool bShouldDrawDebug)
{
	TargetRange = NewTargetRange;
	DetectionCylinderRadius = NewDetectionCylinderRadius;
	TargetingInterval = NewTargetingInterval;
	SetGenericTeamId(OwnerTeamId);
	bDrawDebug = bShouldDrawDebug;
}

void ATargetActor_Line::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId = NewTeamID;
}

void ATargetActor_Line::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATargetActor_Line, TeamId);
	DOREPLIFETIME(ATargetActor_Line, TargetRange);
	DOREPLIFETIME(ATargetActor_Line, DetectionCylinderRadius);
	DOREPLIFETIME(ATargetActor_Line, AvatarActor);
}

void ATargetActor_Line::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	if (!OwningAbility)
		return;

	AvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(PeoridicalTargetingTimerHandle, this, &ATargetActor_Line::DoTargetCheckAndReport, TargetingInterval, true);
	}
}

void ATargetActor_Line::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTargetTrace();
}

void ATargetActor_Line::BeginDestroy()
{
	if (GetWorld() && PeoridicalTargetingTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(PeoridicalTargetingTimerHandle);
	}

	Super::BeginDestroy();
}

void ATargetActor_Line::SweepAllTargetsAlongLine(TArray<FHitResult>& OutHits) const
{
	if (!AvatarActor)
		return;

	FVector ViewLocation;
	FRotator ViewRotation;
	AvatarActor->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	FVector Start = ViewLocation;
	FVector End = Start + ViewRotation.Vector() * TargetRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);
	QueryParams.AddIgnoredActor(this);

	GetWorld()->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		ECC_WorldDynamic,
		FCollisionShape::MakeSphere(DetectionCylinderRadius),
		QueryParams
	);
}


void ATargetActor_Line::UpdateTargetTrace()
{
	if (!AvatarActor)
	{
		return;
	}

	TArray<FHitResult> HitResults;
	SweepAllTargetsAlongLine(HitResults);

	FVector ViewLocation;
	FRotator ViewRotation;
	AvatarActor->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	FVector LineEndLocation = ViewLocation + ViewRotation.Vector() * TargetRange;
	float LineLength = TargetRange;

	for (const FHitResult& Hit : HitResults)
	{
		if (ShouldReportActorAsTarget(Hit.GetActor()))
		{
			LineEndLocation = Hit.ImpactPoint;
			LineLength = FVector::Distance(ViewLocation, LineEndLocation);
			break;
		}
	}

	TargetEndDetectionSphere->SetWorldLocation(LineEndLocation);

	if (LazerVFX)
	{
		LazerVFX->SetVariableFloat(LazerFXLengthParamName, LineLength / 100.f);
	}
}


bool ATargetActor_Line::ShouldReportActorAsTarget(const AActor* ActorToCheck) const
{
	if (!ActorToCheck || ActorToCheck == AvatarActor || ActorToCheck == this)
		return false;

	if (auto Attitude = Cast<IGenericTeamAgentInterface>(ActorToCheck))
	{
		return GetTeamAttitudeTowards(*ActorToCheck) == ETeamAttitude::Hostile;
	}

	return false;
}

void ATargetActor_Line::DoTargetCheckAndReport()
{
	if (!HasAuthority())
		return;

	TArray<FHitResult> HitResults;
	SweepAllTargetsAlongLine(HitResults);

	TArray<TWeakObjectPtr<AActor>> HostileTargets;

	for (const FHitResult& Hit : HitResults)
	{
		if (ShouldReportActorAsTarget(Hit.GetActor()))
		{
			HostileTargets.AddUnique(Hit.GetActor());
		}
	}

	FGameplayAbilityTargetDataHandle TargetDataHandle;

	if (HostileTargets.Num() > 0)
	{
		FGameplayAbilityTargetData_ActorArray* ActorArray = new FGameplayAbilityTargetData_ActorArray();
		ActorArray->SetActors(HostileTargets);
		TargetDataHandle.Add(ActorArray);
	}

	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}
