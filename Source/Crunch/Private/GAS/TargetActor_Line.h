// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GenericTeamAgentInterface.h"
#include "TargetActor_Line.generated.h"

UCLASS()
class ATargetActor_Line : public AGameplayAbilityTargetActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATargetActor_Line();

	// Gọi từ Ability để cấu hình ban đầu
	void ConfigureTargetSetting(
		float NewTargetRange,
		float NewDetectionCylinderRadius,
		float NewTargetingInterval,
		FGenericTeamId OwnerTeamId,
		bool bShouldDrawDebug
	);

	// Giao diện đội nhóm
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	// Mạng & Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Khởi động chọn mục tiêu
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	// Tick để cập nhật VFX và phát hiện mục tiêu
	virtual void Tick(float DeltaTime) override;

	// Cleanup
	virtual void BeginDestroy() override;

private:
	// Xử lý chọn mục tiêu chính
	void DoTargetCheckAndReport();

	// Cập nhật tia và phát hiện các actor
	void UpdateTargetTrace();

	void SweepAllTargetsAlongLine(TArray<FHitResult>& OutHits) const;

	// Kiểm tra actor có nên là mục tiêu
	bool ShouldReportActorAsTarget(const AActor* ActorToCheck) const;

private:
	// === Tham số ===
	UPROPERTY(Replicated)
	float TargetRange;

	UPROPERTY(Replicated)
	float DetectionCylinderRadius;

	UPROPERTY()
	float TargetingInterval;

	UPROPERTY()
	bool bDrawDebug;

	UPROPERTY(Replicated)
	AActor* AvatarActor;

	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	// === VFX & Component ===
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	FName LazerFXLengthParamName = "Length";

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	class USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	class UNiagaraComponent* LazerVFX;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	class USphereComponent* TargetEndDetectionSphere;

	// Timer cho việc quét mục tiêu định kỳ
	FTimerHandle PeoridicalTargetingTimerHandle;
};
