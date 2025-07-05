//CAbilitySystemComponent.cpp


#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	// Đăng ký delegate để lắng nghe sự thay đổi thuộc tính sức khỏe
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this, &UCAbilitySystemComponent::HealthUpdated);
}

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())	return;
	
	for (const TSubclassOf<UGameplayEffect>& EffectClass : InitialEffects)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())	return;

	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, static_cast<int32>(AbilityPair.Key)));
	}

	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, static_cast<int32>(AbilityPair.Key)));
	}

}

void UCAbilitySystemComponent::ApplyFullStatEffects()
{
	AuthApplyGameplayEffect(FullStatEffect, 1); // Áp dụng hiệu ứng đầy đủ thuộc tính nếu có
}



void UCAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// Tạo một hiệu ứng gameplay mới từ GameplayEffect với cấp độ Level
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext());
		// Áp dụng hiệu ứng gameplay này lên chính mình
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{

	if (!GetOwner()) return; // Trả về nếu không có chủ sở hữu

	// Kiểm tra nếu sức khỏe <= 0 và chủ sở hữu có quyền điều khiển
	if (ChangeData.NewValue <= 0.0f && GetOwner()->HasAuthority() && DeathEffect) 
	{
		AuthApplyGameplayEffect(DeathEffect, 1); // Áp dụng hiệu ứng chết
	}
}
