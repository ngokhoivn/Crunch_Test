//AbilityGauge.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityGauge.generated.h"

/**
 * 
 */
UCLASS()
class UAbilityGauge : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;               // Hiển thị icon kỹ năng

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownCounterText; // Đếm ngược thời gian hồi chiêu

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CooldownDurationText; // Hiển thị tổng thời gian hồi chiêu

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CostText;       // Hiển thị chi phí (năng lượng, mana...)
};
