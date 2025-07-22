//CPlayerController.cpp

#include "Player/CPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Player/CPlayerCharacter.h"
#include "Widgets/GameplayWidget.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraShakeBase.h"

void ACPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	CPlayerCharacter = Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ServerSideInit();
		CPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void ACPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	CPlayerCharacter = Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ClientSideInit();
		SpawnGameplayWidget();
	}
}

void ACPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId ACPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (InputSubsystem)
	{
		InputSubsystem->RemoveMappingContext(UInputMapping);
		InputSubsystem->AddMappingContext(UInputMapping, 1);
	}

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(ShopToggleInputAction, ETriggerEvent::Triggered, this, &ACPlayerController::ToggleShop);
		//EnhancedInputComp->BindAction(ToggleGameplayMenuAction, ETriggerEvent::Triggered, this, &ACPlayerController::ToggleGameplayMenu);
	}
}

void ACPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Gọi hàm cơ sở để đảm bảo các thuộc tính được nhân bản đúng cách
	DOREPLIFETIME(ACPlayerController, TeamID); // Nhân bản TeamID để đảm bảo nó được đồng bộ hóa giữa máy chủ và client
}

void ACPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController()) return;

	GameplayWidget = CreateWidget<UGameplayWidget>(this, GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
		GameplayWidget->ConfigureAbilities(CPlayerCharacter->GetAbilities());
	}
	
}

void ACPlayerController::ToggleShop()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleShop();
	}
}