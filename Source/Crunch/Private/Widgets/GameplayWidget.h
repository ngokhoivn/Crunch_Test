//GameplayWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GameplayWidget.generated.h"

/**
 *
 */
UCLASS()
class UGameplayWidget : public UUserWidget
{
    GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
    void ConfigureAbilities(const TMap<ECAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities);
    void ToggleShop();

private:
    UPROPERTY(meta = (BindWidget))
    class UValueGauge* HealthBar;
    

    UPROPERTY(meta = (BindWidget))
    class UValueGauge* ManaBar;

    UPROPERTY(meta = (BindWidget))
    class UAbilityListView* AbilityListView;

    UPROPERTY(meta = (BindWidget))
    class UStatsGauge* AttackDamageGauge;

    UPROPERTY(meta = (BindWidget))
    class UStatsGauge* ArmorGauge;

    UPROPERTY(meta = (BindWidget))
    class UStatsGauge* MoveSpeedGauge;

    UPROPERTY(meta = (BindWidget))
    class UStatsGauge* IntelligenceGauge;

    UPROPERTY(meta = (BindWidget))
    class UStatsGauge* StrengthGauge;

    UPROPERTY(meta = (BindWidget))
    class UShopWidget* ShopWidget;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* ShopPopUpAnimation;

    void PlayShopPopupAnimation(bool bPlayerForward);
    void SetOwningPawnInputEnabled(bool bPawnInputEnabled);
    void SetShowMouseCursor(bool bShowMouseCursor);
    void SetFocusToGameAndUI();
    void SetFocusToGameOnly();


    UPROPERTY()
	class UAbilitySystemComponent* OwnerAbilitySystemComponent;
};
