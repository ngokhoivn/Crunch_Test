// CAbilitySystemStatics.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CAbilitySystemStatics.generated.h"

UCLASS()

class UCAbilitySystemStatics : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Ability")
    static FGameplayTag GetBasicAttackAbilityTag();
	static FGameplayTag GetDeadStatTag(); // Thẻ để xác định trạng thái chết của nhân vật
	static FGameplayTag GetStunStatTag(); // Thẻ để xác định trạng thái Stun 
};