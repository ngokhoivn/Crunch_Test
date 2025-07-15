// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "StatsGauge.generated.h"

/**
 * 
 */
UCLASS()
class UStatsGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AttributeText;

	UPROPERTY(EditAnywhere, Category = "Attribute")
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UTexture2D* IconTexture;

	void SetValue(float NewVal);
	FNumberFormattingOptions NumberFormattingOptions;

	void AttributeChanged(const FOnAttributeChangeData& Data);
};
