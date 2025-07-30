//CPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "CPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	// chỉ gọi trên máy chủ, khi người chơi điều khiển một Pawn mới
	void OnPossess(APawn* NewPawn) override;
	// chỉ gọi trên client, khi người chơi điều khiển một Pawn mới
	void AcknowledgePossession(APawn* NewPawn) override;

	// Trả về ID team
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	// Gán ID team
	virtual FGenericTeamId GetGenericTeamId() const override;

	// Đăng ký replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupInputComponent();
	void MatchFinished(AActor* ViewTarget, int WiningTeam);

private:
	UFUNCTION(Client, Reliable)
	void Client_MatchFinished(AActor* ViewTarget, int WiningTeam);

	void SpawnGameplayWidget();

	UPROPERTY(EditDefaultsOnly, Category = "View")
	float MatchFinishViewBlendTimeDuration = 2.f;

	UPROPERTY()
	class ACPlayerCharacter* CPlayerCharacter;

	UPROPERTY()
	TArray<UCameraShakeBase*> ActiveShakeInstances;


	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	class UGameplayWidget* GameplayWidget;

	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* UInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ShopToggleInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ToggleGameplayMenuAction;

	UFUNCTION()
	void ToggleShop();

	UFUNCTION()
	void ToggleGameplayMenu();
	void ShowWinLoseState();
};
