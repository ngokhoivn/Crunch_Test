// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InventoryItemWidget.h"
#include "Inventory/InventoryItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Inventory/PA_ShopItem.h"
#include "Widgets/ItemToolTip.h"
#include "Widgets/InventoryItemDragDropOp.h"

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EmptySlot();
}

bool UInventoryItemWidget::IsEmpty() const
{
	return !InventoryItem || !(InventoryItem->IsValid());
}

void UInventoryItemWidget::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber = NewSlotNumber;
}


void UInventoryItemWidget::UpdateInventoryItem(const UInventoryItem* Item)
{
	InventoryItem = Item;
	if (!InventoryItem || !InventoryItem->IsValid() || InventoryItem->GetStackCount() <= 0)
	{
		EmptySlot();
		return;
	}

	SetIcon(Item->GetShopItem()->GetIcon());
	UItemToolTip* ToolTip = SetToolTipWidget(InventoryItem->GetShopItem());
	if (ToolTip)
	{
		ToolTip->SetPrice(InventoryItem->GetShopItem()->GetSellPrice());
	}

	if (InventoryItem->GetShopItem()->GetIsStackable())
	{
		StackCountText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryItemWidget::EmptySlot()
{
	InventoryItem = nullptr;
	SetIcon(EmptyTexture);
	SetToolTip(nullptr);

	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	ManaCostText->SetVisibility(ESlateVisibility::Hidden);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
}


void UInventoryItemWidget::UpdateStackCount()
{
	if (InventoryItem)
	{
		StackCountText->SetText(FText::AsNumber(InventoryItem->GetStackCount()));
	}
}

UTexture2D* UInventoryItemWidget::GetIconTexture() const
{
	if (InventoryItem && InventoryItem->GetShopItem())
	{
		return InventoryItem->GetShopItem()->GetIcon();
	}
	return nullptr;
}

FInventoryItemHandle UInventoryItemWidget::GetItemHandle() const
{
	if (!IsEmpty())
	{
		return InventoryItem->GetHandle();
	}
	return FInventoryItemHandle::InvalidHandle();
}

void UInventoryItemWidget::RightButtonClicked()
{
	if (!IsEmpty())
		OnRightBttonClicked.Broadcast(GetItemHandle());
}

void UInventoryItemWidget::LeftButtonClicked()
{
	if (!IsEmpty())
		OnLeftBttonClicked.Broadcast(GetItemHandle());
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!IsEmpty() && DragDropOpClass)
	{
		UInventoryItemDragDropOp* DragDropOp = NewObject<UInventoryItemDragDropOp>(this, DragDropOpClass);
		if (DragDropOp)
		{
			DragDropOp->SetDraggedItem(this);
			OutOperation = DragDropOp;
		}
	}
}

bool UInventoryItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UInventoryItemWidget* OtherWidget = Cast<UInventoryItemWidget>(InOperation->Payload))
	{
		if (OtherWidget && !OtherWidget->IsEmpty())
		{
			OnInventoryItemDropped.Broadcast(this, OtherWidget);
			return true;
		}
	}

	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UInventoryItemWidget::StartCooldown(float CooldownDuration, float TimeRemaining)
{
	CooldownTimeRemaining = TimeRemaining;
	CooldownTimeDuration = CooldownDuration;
	GetWorld()->GetTimerManager().SetTimer(CooldownDurationTimerHandle, this, &UInventoryItemWidget::CooldownFinished, CooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownUpdateTimerHandle, this, &UInventoryItemWidget::UpdateCooldown, CooldownUpdateInterval, true);

	CooldownCountText->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryItemWidget::CooldownFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmtDynamicMaterialParamName, 1.f);
	}
}

void UInventoryItemWidget::UpdateCooldown()
{
	CooldownTimeRemaining -= CooldownUpdateInterval;
	float CooldownAmt = 1.f - CooldownTimeRemaining / CooldownTimeDuration;
	CooldownDisplayFormattingOptions.MaximumFractionalDigits = CooldownTimeRemaining > 1.f ? 0 : 2;
	CooldownCountText->SetText(FText::AsNumber(CooldownTimeRemaining, &CooldownDisplayFormattingOptions));
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmtDynamicMaterialParamName, CooldownAmt);
	}
}

void UInventoryItemWidget::ClearCooldown()
{
	CooldownFinished();
}

void UInventoryItemWidget::SetIcon(UTexture2D* IconTexture)
{
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetTextureParameterValue(IconTextureDynamicMaterialParamName, IconTexture);
		return;
	}

	Super::SetIcon(IconTexture);
}
