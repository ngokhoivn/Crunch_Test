//AbilityGauge.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "AbilityGauge.generated.h"

/**
 * 
 */
UCLASS()
class UAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
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
