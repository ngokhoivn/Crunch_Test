// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Inventory/PA_ShopItem.h"
#include "CAssetManager.generated.h"

class UPA_CharacterDefination;
/**
 *
 */
UCLASS()
class UCAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static UCAssetManager& Get();

	void LoadShopItems(const FStreamableDelegate& LoadFinishedCallback);
	bool GetLoadedShopItems(TArray<const UPA_ShopItem*>& OutItems) const;
	const FItemCollection* GetCombinationForItem(const UPA_ShopItem* Item) const;
	const FItemCollection* GetIngredientForItem(const UPA_ShopItem* Item) const;
private:
	void ShopItemLoadFinished(FStreamableDelegate Callback);
	void BuildItemMaps();
	void AddToCombinationMap(const UPA_ShopItem* Ingredient, const UPA_ShopItem* CombinationItem);

	UPROPERTY()
	TMap<const UPA_ShopItem*, FItemCollection> CombinationMap;
	UPROPERTY()
	TMap<const UPA_ShopItem*, FItemCollection> IngredientMap;
};