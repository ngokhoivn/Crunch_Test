//AbilityGauge.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "GameplayAbilityspecHandle.h"
#include "GameplayEffectTypes.h"
#include "AbilityGauge.generated.h"

class UAbilitySystemComponent;
struct FGameplayAbilitySpec;

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
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	void ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CooldownUpdateInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "visual")
	FName IconMaterialParamName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "visual")
	FName CooldownPercentParamname = "Percent";
	
	UPROPERTY(EditDefaultsOnly, Category = "visual")
	FName AbilityLevelParamName = "Level";

	UPROPERTY(EditDefaultsOnly, Category = "visual")
	FName CanCastAbilityParamName = "CanCast";

	UPROPERTY(EditDefaultsOnly, Category = "visual")
	FName UpgradePointAvailableParamName = "Percent";

	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;               // Hiển thị icon kỹ năng

	UPROPERTY(meta = (BindWidget))
	class UImage* LevelGauge;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownCounterText; // Đếm ngược thời gian hồi chiêu

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CooldownDurationText; // Hiển thị tổng thời gian hồi chiêu

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CostText;       // Hiển thị chi phí (năng lượng, mana...)

	UPROPERTY()
	class UGameplayAbility* AbilityCDO;

	void AbilityCommitted(UGameplayAbility* Ability);

	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);

	float CachedCooldownDuration;
	float CachedCooldownTimeRemaining;

	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	FNumberFormattingOptions WholeNumberFormattionOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOption;

	void CooldownFinished();
	void UpdateCooldown();

	const UAbilitySystemComponent* OwnerAbilitySystemComponent;
	FGameplayAbilitySpecHandle CachedAbilitySpecHandle;

	const FGameplayAbilitySpec* GetAbilitySpec();

	bool bIsAbilityLearned = false;

	void AbilitySpecUpdated(const FGameplayAbilitySpec& AbilitySpec);
	void UpdateCanCast();
	void UpgradePointUpdated(const FOnAttributeChangeData& Data);
};
