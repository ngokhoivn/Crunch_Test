// CPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "Character/CCharacter.h"
#include "InputActionValue.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "CPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ACPlayerCharacter : public ACCharacter
{
	GENERATED_BODY()
public:
	ACPlayerCharacter();
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:
	UPROPERTY (VisibleDefaultsOnly, Category = "View")
	class USpringArmComponent* CameraBoom;

	UPROPERTY (VisibleDefaultsOnly, Category = "View")
	class UCameraComponent* ViewCam;

	FVector GetLookRightDir() const;
	FVector GetLookFwdDir() const;
	FVector GetMoveFwdDir() const;
	/*************************************************/
	/*               Gameplay Ability                */
	/*************************************************/
private:
	virtual void OnAimStateChanged(bool bIsAimming) override;

	UPROPERTY()
	class UCHeroAttributeSet* HeroAttributeSet;

	/*************************************************/
	/*                      Input                    */
	/*************************************************/
private:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LookInputAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* JumpInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LearnAbilityLeaderAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<ECAbilityInputID, class UInputAction*> GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GameplayInputMappingContext;

	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void LearnAbilityLeaderDown(const FInputActionValue& InputActionValue);
	void LearnAbilityLeaderUp(const FInputActionValue& InputActionValue);
	bool bIsLearnAbilityLeaderDown = false;
	void HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID);
	void SetInputEnabledFromPlayerController(bool bEnabled);
	/*************************************************/
	/*                     Stun                      */
	/*************************************************/
	virtual void OnStun() override;
	virtual void OnRecoverFromStun() override;


	/*************************************************/
	/*             Death & Respawn                   */
	/*************************************************/
	virtual void OnDead() override;// Gọi khi nhân vật chết
	virtual void OnRespawn() override; // Gọi khi nhân vật hồi sinh

	/*************************************************/
	/*                  Camera View                  */
	/*************************************************/
private:	
	UPROPERTY(EditDefaultsOnly, Category = "View")
	FVector CameraAimLocalOffset;

	UPROPERTY(EditDefaultsOnly, Category = "View")
	float CameraLerpSpeed = 20.f;

	FTimerHandle CameraLerpTimerHandle;

	void LerpCameraTolocalOffsetLocation(const FVector& Goal);
	void TickCameraLocalOffsetLerp(FVector Goal);

	/*************************************************/
	/*                  Inventory                    */
	/*************************************************/
private:
	class UInventoryComponent* InventoryComponent;
};
