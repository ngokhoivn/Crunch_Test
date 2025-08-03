// CAbilitySystemStatics.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CAbilitySystemStatics.generated.h"

class UGameplayAbility;
struct FGameplayAbilitySpec;
class UAbilitySystemComponent;

UCLASS()

class UCAbilitySystemStatics : public UBlueprintFunctionLibrary 
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Ability")
    static FGameplayTag GetBasicAttackAbilityTag();
    static FGameplayTag GetBasicAttackInputPressedTag();
    static FGameplayTag GetBasicAttackInputReleasedTag();
	static FGameplayTag GetDeadStatTag(); // Thẻ để xác định trạng thái chết của nhân vật
	static FGameplayTag GetStunStatTag(); // Thẻ để xác định trạng thái Stun 
	static FGameplayTag GetAimStatTag(); 
	static FGameplayTag GetCameraShakeGameplayCueTag(); 
    static FGameplayTag GetHealthFullStatTag();
    static FGameplayTag GetHealthEmptyStatTag();
    static FGameplayTag GetManaFullStatTag();
    static FGameplayTag GetManaEmptyStatTag();
    static FGameplayTag GetHeroRoleTag();
    static FGameplayTag GetExperienceAttributeTag();
    static FGameplayTag GetGoldAttributeTag();
    static FGameplayTag GetCrosshairTag();
    static FGameplayTag GetTargetUpdatedTag();

    static bool IsActorDead(const AActor* ActorToCheck);
    static bool IsHero(const AActor* ActorToCheck);
    static bool ActorHasTag(const AActor* ActorToCheck, const FGameplayTag& Tag);
    static bool IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec);

    static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
    static float GetStaticCostForAbility(const UGameplayAbility* Ability);

    static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& ASC);
    static bool CheckAbilityCostStatic(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);
    static float GetManaCostFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC, int AbilityLevel);
    static float GetCooldownDurationFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC, int AbilityLevel);
    static float GetCooldownRemainingFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);
};