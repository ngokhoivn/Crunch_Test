// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItem.h"
#include "AbilitySystemComponent.h"
#include "Inventory/PA_ShopItem.h"
#include "GameplayEffect.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CAbilitySystemStatics.h"

FInventoryItemHandle::FInventoryItemHandle()
    : HandleId{ GetInvalidId() }
{
}

FInventoryItemHandle FInventoryItemHandle::InvalidHandle()
{
    static FInventoryItemHandle InvalidHandle = FInventoryItemHandle();
    return InvalidHandle;
}

FInventoryItemHandle::FInventoryItemHandle(uint32 Id)
    : HandleId{ Id }
{
}

bool FInventoryItemHandle::IsValid() const
{
    return HandleId != GetInvalidId();
}

FInventoryItemHandle FInventoryItemHandle::CreateHandle()
{
    return FInventoryItemHandle(GenerateNextId());
}

uint32 FInventoryItemHandle::GenerateNextId()
{
    static uint32 StaticId = 1;
    return StaticId++;
}

uint32 FInventoryItemHandle::GetInvalidId()
{
    return 0;
}

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs)
{
    return Lhs.GetHandleId() == Rhs.GetHandleId();
}

uint32 GetTypeHash(const FInventoryItemHandle& Key)
{
    return Key.GetHandleId();
}

UInventoryItem::UInventoryItem()
    :StackCount{1}
{
}

bool UInventoryItem::IsValid() const
{
    return ShopItem != nullptr;
}

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem)
{
    Handle = NewHandle;
    ShopItem = NewShopItem;
}

void UInventoryItem::ApplyGASModifications(UAbilitySystemComponent* AbilitySystemComponent)
{
    if (!GetShopItem() || !AbilitySystemComponent) return;
    if (!AbilitySystemComponent->GetOwner() || !AbilitySystemComponent->GetOwner()->HasAuthority()) return;

    TSubclassOf<UGameplayEffect> EquipEffect = GetShopItem()->GetEquippedEffect();
    if (EquipEffect)
    {
        AppliedEquipedEffectHandle = AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(
            EquipEffect,
            1, // Level
            AbilitySystemComponent->MakeEffectContext()
        );

    }

    TSubclassOf<UGameplayAbility> GrantedAbility = GetShopItem()->GetGrantedAbility();
    if (GrantedAbility)
    {
        const FGameplayAbilitySpec* FoundSpec = AbilitySystemComponent->FindAbilitySpecFromClass(GrantedAbility);
        if (FoundSpec)
        {
            GrantedAbilitySpecHandle = FoundSpec->Handle;
        }
        else
        {
            GrantedAbilitySpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
        }
    }
}

void UInventoryItem::SetSlot(int NewSlot)
{
    Slot = NewSlot;
}

