// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchStatWidget.generated.h"

/**
 *
 */
UCLASS()
class UMatchStatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Match Stat")
	float ProgressUpdateInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Match Stat")
	FName ProgressDynamicMaterialParamName = "Progress";

	UPROPERTY(meta = (BindWidget))
	class UImage* ProgressImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TeamOneCountText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TeamTwoCountText;

	UPROPERTY()
	class AStormCore* StormCore;

	void UpdateTeamInfluence(int TeamOneCount, int TeamTwoCount);

	void MatchFinished(AActor* ViewTarget, int WinningTeam);
	void UpdateProgress();

	FTimerHandle UpdateProgressTimerHandle;
};