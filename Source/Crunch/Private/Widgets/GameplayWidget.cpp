//GameplayWidget.cpp

#include "Widgets/GameplayWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "GAS/CAttributeSet.h"

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
