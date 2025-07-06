//AbilityGauge.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "AbilityGauge.generated.h"

USTRUCT(BlueprintType)
struct FAbilityWidgetData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityClass; // Class kỹ năng

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
};
/**
 * 
 */
UCLASS()
class UAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	void ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "visual")
	FName IconMaterialParamName = "Icon";


	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;               // Hiển thị icon kỹ năng

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownCounterText; // Đếm ngược thời gian hồi chiêu

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CooldownDurationText; // Hiển thị tổng thời gian hồi chiêu

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CostText;       // Hiển thị chi phí (năng lượng, mana...)
};
