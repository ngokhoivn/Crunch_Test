//GA_Combo.cpp

#include "GAS/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "GAS/CAbilitySystemStatics.h"

UGA_Combo::UGA_Combo()
{
	AbilityTags.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ComboMontage);
		PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();	

		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			GetComboChangedEventTag(),
			nullptr,
			false,
			false);
		WaitComboChangeEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangedEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
	}

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetingEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboTargetEventTag());
		WaitTargetingEventTask->EventReceived.AddDynamic(this, &UGA_Combo::DoDamage);
		WaitTargetingEventTask->ReadyForActivation();
	}
	SetupWaitComboInputPress();
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.change"); // Base tag
}

FGameplayTag UGA_Combo::GetComboChangedEventEndTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.change.end");
}

FGameplayTag UGA_Combo::GetComboTargetEventTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.damage");
}

void UGA_Combo::SetupWaitComboInputPress()
{
	UAbilityTask_WaitInputPress * WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &UGA_Combo::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void UGA_Combo::HandleInputPress(float TimeWaited)
{
	// Tiếp tục lắng nghe input cho lần nhấn tiếp theo
	SetupWaitComboInputPress();
	//Thử chuyển sang combo tiếp theo
	TryCommitCombo();
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None)	return;

	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (!OwnerAnimInst) return;

	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(ComboMontage), NextComboName, ComboMontage);
}

TSubclassOf<UGameplayEffect> UGA_Combo::GetDamageEffectForCurrentCombo() const
{
	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (OwnerAnimInstance)
	{
		FName CurrentSectionName = OwnerAnimInstance->Montage_GetCurrentSection(ComboMontage);
		const TSubclassOf<UGameplayEffect>* FoundEffectPtr = DamageEffectMap.Find(CurrentSectionName);
		if (FoundEffectPtr)
		{
			return *FoundEffectPtr;
		}
	}
	return DefaultDamageEffect;
}

void UGA_Combo::ComboChangedEventReceived(FGameplayEventData Data)
{
	FGameplayTag EventTag = Data.EventTag;

	if (EventTag == GetComboChangedEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}

	if (EventTag.MatchesTag(GetComboChangedEventTag()))
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);

		if (TagNames.Num() > 0)
		{
			NextComboName = TagNames.Last();
		}
	}
}

void UGA_Combo::DoDamage(FGameplayEventData Data)
{
	// Dọn timer cũ
	GetWorld()->GetTimerManager().ClearTimer(DamageProcessingTimer);
	PendingHitResults.Empty();
	CurrentDamageIndex = 0;

	// Lưu các mục tiêu
	PendingHitResults = GetHitResultsFromSweepLocationTargetData(Data.TargetData, TargetSweepShereRadius);
	CurrentDamageEffect = GetDamageEffectForCurrentCombo();

	if (PendingHitResults.Num() > 0 && CurrentDamageEffect)
	{
		// Gây damage từng mục tiêu một sau mỗi 0.05 giây
		GetWorld()->GetTimerManager().SetTimer(
			DamageProcessingTimer,
			this,
			&UGA_Combo::ProcessNextDamageTarget,
			0.05f,
			true,
			0.0f
		);
	}
}


void UGA_Combo::ProcessNextDamageTarget()
{
	if (CurrentDamageIndex < PendingHitResults.Num())
	{
		const FHitResult& HitResult = PendingHitResults[CurrentDamageIndex];

		ApplyGameplayEffectToHitResultActor(
			HitResult,
			CurrentDamageEffect,
			GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)
		);

		// Có thể thêm hiệu ứng camera shake, âm thanh, hitstop tại đây

		CurrentDamageIndex++;
	}
	else
	{
		// Xong hết
		GetWorld()->GetTimerManager().ClearTimer(DamageProcessingTimer);
		PendingHitResults.Empty();
		CurrentDamageIndex = 0;
	}
}



