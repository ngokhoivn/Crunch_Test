// CAbilitySystemStatics.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CAbilitySystemStatics.generated.h"

class UGameplayAbility;

UCLASS()

class UCAbilitySystemStatics : public UBlueprintFunctionLibrary 
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Ability")
    static FGameplayTag GetBasicAttackAbilityTag();
    static FGameplayTag GetBasicAttackInputPressedTag();
	static FGameplayTag GetDeadStatTag(); // Thẻ để xác định trạng thái chết của nhân vật
	static FGameplayTag GetStunStatTag(); // Thẻ để xác định trạng thái Stun 
	static FGameplayTag GetAimStatTag(); 
	static FGameplayTag GetCameraShakeGameplayCueTag(); 
    static FGameplayTag GetHealthFullStatTag();
    static FGameplayTag GetHealthEmptyStatTag();
    static FGameplayTag GetManaFullStatTag();
    static FGameplayTag GetManaEmptyStatTag();

    

    static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
    static float GetStaticCostForAbility(const UGameplayAbility* Ability);
};