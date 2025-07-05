//GameplayWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

private:
    UPROPERTY(meta = (BindWidget))
    class UValueGauge* HealthBar;

    UPROPERTY(meta = (BindWidget))
    class UValueGauge* ManaBar;

    UPROPERTY()
	class UAbilitySystemComponent* OwnerAbilitySystemComponent;
};
