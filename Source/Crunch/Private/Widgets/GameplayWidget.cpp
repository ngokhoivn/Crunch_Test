//GameplayWidget.cpp

#include "Widgets/GameplayWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h" 
#include "Widgets/AbilityListView.h"
#include "Widgets/ShopWidget.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"

void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (OwnerAbilitySystemComponent)
	{
		HealthBar->SetAndBoundToGameplayAttribute(
			OwnerAbilitySystemComponent,
			UCAttributeSet::GetHealthAttribute(),
			UCAttributeSet::GetMaxHealthAttribute()
		);
		ManaBar->SetAndBoundToGameplayAttribute(
			OwnerAbilitySystemComponent, 
			UCAttributeSet::GetManaAttribute(), 
			UCAttributeSet::GetMaxManaAttribute()
		);
	}

}

void UGameplayWidget::ConfigureAbilities(const TMap<ECAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
	AbilityListView->ConfigureAbilities(Abilities);
}

void UGameplayWidget::ToggleShop()
{
	if (ShopWidget->GetVisibility() == ESlateVisibility::HitTestInvisible)
	{
		ShopWidget->SetVisibility(ESlateVisibility::Visible);
		PlayShopPopupAnimation(true);
	}
	else
	{
		ShopWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		PlayShopPopupAnimation(false);

	}
}

void UGameplayWidget::PlayShopPopupAnimation(bool bPlayerForward)
{
	if (bPlayerForward)
	{
		PlayAnimationForward(ShopPopUpAnimation);
	}
	else
	{
		PlayAnimationReverse(ShopPopUpAnimation);
	}
}
