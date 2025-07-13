﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/TargetActor_GroundPick.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"
#include "Crunch/Crunch.h"
#include "GenericTeamAgentInterface.h"

void ATargetActor_GroundPick::ConfirmTargetingAndContinue()
{
	TArray<FOverlapResult> OverlapResults;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(TargetAreaRadius);

	GetWorld()->OverlapMultiByObjectType(OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		CollisionShape
	);

	TSet<AActor*> TargetActors;

	IGenericTeamAgentInterface* OwnerTeamInterface = nullptr;
	if (OwningAbility)
	{
		OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	}

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Friendly && !bShouldTargetFriendly)
			continue;

		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Hostile && !bShouldTargetEnemy)
			continue;

		TargetActors.Add(OverlapResult.GetActor());
	}
	FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(), false);

	// Lưu thông tin( vị trí va chạm, normal, actor bị trúng) cho cú đánh đơn lẻ
	FGameplayAbilityTargetData_SingleTargetHit* HitLoc = new FGameplayAbilityTargetData_SingleTargetHit;
	// Thiết lập điểm va chạm ImpactPoint trong Hit Result bằng vị trí hiện tại của Actor
	HitLoc->HitResult.ImpactPoint = GetActorLocation();
	// Thêm data vào FGameplayABilityTargetDataHandle để truyền dữ liệu
	TargetData.Add(HitLoc);

	TargetDataReadyDelegate.Broadcast(TargetData);
}

void ATargetActor_GroundPick::SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy)
{
	bShouldTargetFriendly = bTargetFriendly;
	bShouldTargetEnemy = bTargetEnemy;
}

ATargetActor_GroundPick::ATargetActor_GroundPick()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATargetActor_GroundPick::SetTargetAreaRadius(float NewRadius)
{
	TargetAreaRadius = NewRadius;
}

void ATargetActor_GroundPick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
	{
		SetActorLocation(GetTargetPoint());
	}
}

FVector ATargetActor_GroundPick::GetTargetPoint() const
{
	if (!PrimaryPC || !PrimaryPC->IsLocalPlayerController())
		return GetActorLocation();

	FHitResult TraceResult;

	FVector ViewLoc;
	FRotator ViewRot;

	PrimaryPC->GetPlayerViewPoint(ViewLoc, ViewRot);

	FVector TraceEnd = ViewLoc + ViewRot.Vector() * TargetTraceRange;
	GetWorld()->LineTraceSingleByChannel(TraceResult, ViewLoc, TraceEnd, ECC_Target);

	if (!TraceResult.bBlockingHit)
	{
		GetWorld()->LineTraceSingleByChannel(TraceResult, TraceEnd, TraceEnd + FVector::DownVector * TNumericLimits<float>::Max(), ECC_Target);
	}

	if (bShouldDrawDebug) 
	{
		DrawDebugSphere(GetWorld(), TraceResult.ImpactPoint, TargetAreaRadius, 32, FColor::Red);
	}

	if (!TraceResult.bBlockingHit)
	{
		return GetActorLocation();
	}
	return TraceResult.ImpactPoint;
}
