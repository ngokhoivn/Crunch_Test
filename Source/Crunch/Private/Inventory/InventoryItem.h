// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "InventoryItem.generated.h"

class UPA_ShopItem;
class UAbilitySystemComponent;

USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()
public:
	FInventoryItemHandle();
	static FInventoryItemHandle InvalidHandle();
	static FInventoryItemHandle CreateHandle();

	bool IsValid() const;
	uint32 GetHandleId() const { return HandleId; }
private:
	explicit FInventoryItemHandle(uint32 Id);

	UPROPERTY()
	uint32 HandleId;

	static uint32 GenerateNextId();
	static uint32 GetInvalidId();
};

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs);
uint32 GetTypeHash(const FInventoryItemHandle& Key);
/**
 *
 */
UCLASS()
class UInventoryItem : public UObject
{
	GENERATED_BODY()
public:
	// return true is was able to add
	bool AddStackCount();

	// return true if the stack is not empty after reducing
	bool ReduceStackCount();

	// return true if was able to set
	bool SetStackCount(int NewStackCount);

	bool IsStackFull() const;
	bool IsForItem(const UPA_ShopItem* Item) const;

	UInventoryItem();
	bool IsValid() const;

	void InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem);
	const UPA_ShopItem* GetShopItem() const { return ShopItem; }
	FInventoryItemHandle GetHandle() const { return Handle; }
	void ApplyGASModifications(UAbilitySystemComponent* AbilitySystemComponent);
	FORCEINLINE int GetStackCount() const { return StackCount; }
	void SetSlot(int NewSlot);
private:
	UPROPERTY()
	const UPA_ShopItem* ShopItem;
	FInventoryItemHandle Handle;
	
	int StackCount;
	int Slot;

	FActiveGameplayEffectHandle AppliedEquipedEffectHandle;
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;
};