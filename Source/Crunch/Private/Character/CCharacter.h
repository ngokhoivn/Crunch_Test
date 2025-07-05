//CCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "CCharacter.generated.h"

UCLASS()
class ACCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCharacter();
	void ServerSideInit();
	void ClientSideInit();
	bool IsLocallyControlledByPlayer() const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Only Call this function on the server
	virtual void PossessedBy(AController* NewController) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**********************************************/
	/*            GAMEPLAY ABILITY                */
	/**********************************************/

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount); // Xử lý khi thẻ Dead thay đổi
	void StunTagUpdated(const FGameplayTag Tag, int32 NewCount); // Xử lý khi thẻ Stun thay đổi
	void BindGASChangeDelegates();  // Liên kết các delegate thay đổi thuộc tính GAS

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UCAbilitySystemComponent* CAbilitySystemComponent;
	UPROPERTY()
	class UCAttributeSet* CAttributeSet;
	/**********************************************/
	/*                    UI                      */
	/**********************************************/

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ablity")
	class UWidgetComponent* OverHeadWidgetComponent;
	void ConfigureOverHeadWidgetComponent();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityCheckUpdateGap = 1.f; // Thời gian giữa các lần kiểm tra (giây)

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityRangeSquared = 1000000.f; // Khoảng cách tối đa (bình phương) để hiển thị (1000 units = 100^2)

	FTimerHandle HeadStatGaugeVisibilityUpdateTimerHandle;
	void UpdateHeadGaugeVisiility();
	void SetStatusGaugeEnabled(bool bIsEnabled); // Bật hoặc tắt hiển thị của OverHeadWidgetComponent
	/**********************************************/
	/*					Stun			          */
	/**********************************************/
private: 
	UPROPERTY(EditDefaultOnly, Category = "Stun")
	UAnimMontage* StunMontage;

	virtual void OnStun();
	virtual void OnRecoverFromStun();



	/**********************************************/
	/*         Death & Respawn Ability            */
	/**********************************************/

public:
	bool IsDead() const; // Kiểm tra trạng thái
	void RespawnImmediately(); //Logic Respawn

private:
	FTransform MeshRelativeTransform; // Biến lưu trữ vị trí tương đối của mesh khi bắt đầu hoạt ảnh Death

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontageFinishTimeShift = -0.8f; // Thời gian dịch chuyển để phát hoạt ảnh chết (giây)

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	FTimerHandle DeathMontageTimerHandle;// Thời gian để phát hoạt ảnh chết

	void DeathMontageFinished();// Hàm được gọi khi hoạt ảnh death kết thúc

	void PlayDeathAnimation();
	void SetRaddollEnebled(bool bIsEnabled);// Bật hoặc tắt mô phỏng vật lý của nhân vật

	void StartDeathSequence();
	void Respawn();

	virtual void OnDead();// ghi đè sự kiện chết của nhân vật
	virtual void OnRespawn(); // ghi đè sự kiện hồi sinh của nhân vật
	/**********************************************/
	/*                  TeamID                    */
	/**********************************************/
public:

	// Gọi khi người chơi đã đăng nhập thành công
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	// Trả về ID nhóm của người chơi
	virtual FGenericTeamId GetGenericTeamId() const override;
private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID;

	UFUNCTION()
	virtual void OnRep_TeamID() {};	//override in child class
	
	/**********************************************/
	/*                  AI                        */
	/**********************************************/
private:	
	void SetAIPerceptionStimuliSourceComponent(bool bIsEnabled);// Thiết lập thành phần nguồn kích thích nhận thức AI
	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;
};
