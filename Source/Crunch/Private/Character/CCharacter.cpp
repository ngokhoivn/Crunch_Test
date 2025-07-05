//CCharacter.cpp


#include "Character/CCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/CAbilitySystemStatics.h"
#include "Widgets/OverHeadStatsGauge.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

ACCharacter::ACCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>("CAttribute Set");
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("OverHeadWidgetComponent");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());
	BindGASChangeDelegates(); // Liên kết các delegate thay đổi thuộc tính GAS

	// Tạo component để nhân vật có thể được nhận diện bởi AI
	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("PerceptionStimuliSourceComponent");
}

void ACCharacter::ServerSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this, this);
	CAbilitySystemComponent->ApplyInitialEffects();
	CAbilitySystemComponent->GiveInitialAbilities();
}

void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool ACCharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalController();
}

void ACCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCharacter, TeamID);
}

void ACCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}

// Called when the game starts or when spawned
void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverHeadWidgetComponent();
	MeshRelativeTransform = GetMesh()->GetRelativeTransform(); // Lưu lại transform tương đối của mesh

	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass()); // Đăng ký nhân vật với cảm nhận thị giác của AI
}

// Called every frame
void ACCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}

void ACCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		StartDeathSequence(); // Chỉ cần gọi hàm StartDeathSequence, không cần làm gì khác
	}
	else
	{
		Respawn(); // Chỉ cần gọi hàm Respawn, không cần làm gì khác
	}
}

void ACCharacter::StunTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;

	if (NewCount != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnRecoverFromStun();
		StopAnimMontage(StunMontage);
	}
}

void ACCharacter::BindGASChangeDelegates()
{
	if (CAbilitySystemComponent) // Kiểm tra xem CAbilitySystemComponent có hợp lệ không
	{
		// Đăng ký delegate để lắng nghe sự thay đổi thuộc tính sức khỏe
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &ACCharacter::DeathTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatTag()).AddUObject(this, &ACCharacter::StunTagUpdated);

	}
}

void ACCharacter::ConfigureOverHeadWidgetComponent()
{
	if (!OverHeadWidgetComponent) return;

	if (IsLocallyControlledByPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return;
	}

	UOverHeadStatsGauge* OverHeadStatsGauge = Cast<UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	
	if (OverHeadStatsGauge)
	{
		OverHeadStatsGauge->ConfigureWithASC(GetAbilitySystemComponent());
		OverHeadWidgetComponent->SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdateTimerHandle, this, &ACCharacter::UpdateHeadGaugeVisiility, HeadStatGaugeVisibilityCheckUpdateGap, true);
	
	}
}

void ACCharacter::UpdateHeadGaugeVisiility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalPlayerPawn) {
		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > HeadStatGaugeVisibilityRangeSquared);
	}
}

void ACCharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);// Dừng timer nếu đang chạy

	if (bIsEnabled)
	{
		ConfigureOverHeadWidgetComponent();// Cấu hình lại widget component
	}
	else
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);// Ẩn widget component
	}
}

void ACCharacter::OnStun()
{
}

void ACCharacter::OnRecoverFromStun()
{
}

bool ACCharacter::IsDead() const
{
	// Copy từ IsActive() trong Minion.cpp
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag());

}

void ACCharacter::RespawnImmediately()
{
	if (HasAuthority())
	// Copy từ Activate() trong Minion.cpp
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UCAbilitySystemStatics::GetDeadStatTag()));

}

void ACCharacter::DeathMontageFinished()
{
	if (IsDead())
		SetRaddollEnebled(true); // Bật mô phỏng vật lý sau khi hoạt ảnh chết kết thúc
	GetWorldTimerManager().ClearTimer(DeathMontageTimerHandle); // Xóa timer để tránh gọi lại hàm này nhiều lần
}

void ACCharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		// Lấy thời gian của montage hoạt ảnh chết
		float MontageDuration = PlayAnimMontage(DeathMontage);
		// Đặt timer để gọi lại hàm này sau khi hoạt ảnh kết thúc
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &ACCharacter::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
}

void ACCharacter::SetRaddollEnebled(bool bIsEnabled)
{
	if (bIsEnabled)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform); // Tách mesh khỏi component
		GetMesh()->SetSimulatePhysics(true); // Bật mô phỏng vật lý
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); // Bật va chạm vật lý
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false); // Tắt mô phỏng vật lý
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Tắt va chạm
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform); // Gắn lại mesh vào component gốc
		GetMesh()->SetRelativeTransform(MeshRelativeTransform); // Đặt lại transform tương đối của mesh
	}
}

void ACCharacter::StartDeathSequence()
{
	OnDead(); 

	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->CancelAllAbilities(); // Hủy tất cả các khả năng đang hoạt động
		
	}

	PlayDeathAnimation();
	SetStatusGaugeEnabled(false); // Tắt hiển thị gauge trạng thái
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None); // Dừng chuyển động của nhân vật
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Tắt va chạm của capsule component

	SetAIPerceptionStimuliSourceComponent(false); // Tắt cảm nhận kích thích AI
}

void ACCharacter::Respawn()
{
	OnRespawn(); // Gọi sự kiện hồi sinh của nhân vật

	SetAIPerceptionStimuliSourceComponent(true); // Bật cảm nhận kích thích AI

	SetRaddollEnebled(false); // Tắt mô phỏng vật lý
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // Bật lại va chạm của capsule component
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking); // Đặt lại chế độ di chuyển
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f); // Dừng tất cả các montage hoạt hình
	SetStatusGaugeEnabled(true); // Bật hiển thị gauge trạng thái

	if (HasAuthority() && GetController())
	{
		TWeakObjectPtr<AActor> StartSpot = GetController()->StartSpot;// Lấy vị trí bắt đầu từ controller
		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot->GetActorTransform()); // Đặt lại vị trí của nhân vật
		}
	}

	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->ApplyFullStatEffects(); // Áp dụng lại hiệu ứng đầy đủ thuộc tính
	}
}

void ACCharacter::OnDead()
{

}

void ACCharacter::OnRespawn()
{
}

void ACCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId ACCharacter::GetGenericTeamId() const
{
	return TeamID;
}


void ACCharacter::SetAIPerceptionStimuliSourceComponent(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent)
	{
		return;
	}
	if (bIsEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass()); // Đăng ký cảm nhận thị giác
	}
	else
	{
		PerceptionStimuliSourceComponent->UnregisterFromSense(UAISense_Sight::StaticClass()); // Hủy đăng ký cảm nhận thị giác
	}
}

