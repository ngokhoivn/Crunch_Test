//CAIController.cpp

#include "AI/CAIController.h"
#include "Character/CCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/CAbilitySystemStatics.h" 
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ACAIController::ACAIController()
{
	// Thành phần nhận thức AI
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	// Phát hiện bằng thị giác
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	SightConfig->SightRadius = 500.0f; // Bán kính phát hiện
	SightConfig->LoseSightRadius = 1200.f; // Bán kính mất dấu

	SightConfig->SetMaxAge(5.0f); // Thời gian tồn tại của cảm biến
	SightConfig->PeripheralVisionAngleDegrees = 100.0f; // Góc nhìn

	// Cấu hình cảm biến thị giác và đảm bảo nó được thêm vào SensesConfig
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	// Đăng ký sự kiện cập nhật thông tin nhận thức
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACAIController::TargetPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ACAIController::TargetForgotten);

	// Gán BehaviorTree trong constructor (có thể override trong Blueprint)
	// BehaviorTree = LoadObject<UBehaviorTree>(nullptr, TEXT("/Path/To/Your/BehaviorTree"));
}

void ACAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	// Lấy TeamID từ Pawn và gán ngược lại cho Controller (thay vì ép Pawn)
	if (IGenericTeamAgentInterface* PawnTeamInterface = Cast<IGenericTeamAgentInterface>(NewPawn))
	{
		SetGenericTeamId(PawnTeamInterface->GetGenericTeamId());
	}

	// Đăng ký sự kiện GameplayTag "Dead" từ ASC của Pawn
	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewPawn);
	if (PawnASC)
	{
		PawnASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag())
			.AddUObject(this, &ACAIController::PawnDeadTagUpdated);
		
		PawnASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatTag())
			.AddUObject(this, &ACAIController::PawnStunTagUpdated);
	}

	// Chạy Behavior Tree nếu đã thiết lập
	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}

	// Làm mới lại perception sau khi Possess
	ClearAndDisableAllSenses();
	EnableAllSenses();
}


void ACAIController::BeginPlay()
{
	Super::BeginPlay();
	// RunBehaviorTree đã được di chuyển vào OnPossess()
}

void ACAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	// Null check cho AIPerceptionComponent
	if (!AIPerceptionComponent) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		if (!GetCurrentTarget())
		{
			SetCurrentTarget(TargetActor); // Gán mục tiêu mới nếu chưa có
		}
	}
	else if (Stimulus.WasSuccessfullySensed() == false && GetCurrentTarget() == TargetActor)
	{
		SetCurrentTarget(GetNextPerceivedActor()); // Gán mục tiêu mới nếu mục tiêu hiện tại không còn được nhận thức
	}
}

void ACAIController::TargetForgotten(AActor* ForgottenActor)
{
	// Null check cho AIPerceptionComponent
	if (!AIPerceptionComponent) return;
	if (!ForgottenActor) return;

	if (GetCurrentTarget() == ForgottenActor)
	{
		SetCurrentTarget(GetNextPerceivedActor()); // Gán mục tiêu mới nếu mục tiêu hiện tại bị quên
	}
}

const UObject* ACAIController::GetCurrentTarget() const
{
	// Fix: Sửa lại kiểm tra BlackboardComponent bị ngược
	const UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();

	if (BlackboardComponent)
	{
		// Trả về mục tiêu hiện tại từ Blackboard
		return BlackboardComponent->GetValueAsObject(TargetBlackboardKeyName);
	}
	return nullptr; // Trả về nullptr nếu không có BlackboardComponent
}

void ACAIController::SetCurrentTarget(AActor* NewTarget)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent) return;

	if (NewTarget)
	{
		// Gán mục tiêu mới vào Blackboard
		BlackboardComponent->SetValueAsObject(TargetBlackboardKeyName, NewTarget);
	}
	else
	{
		// Xóa mục tiêu hiện tại nếu NewTarget là nullptr
		BlackboardComponent->ClearValue(TargetBlackboardKeyName);
	}
}

AActor* ACAIController::GetNextPerceivedActor() const
{
	// Null check cho AIPerceptionComponent
	if (!AIPerceptionComponent) return nullptr;

	TArray<AActor*> Actors;
	AIPerceptionComponent->GetPerceivedHostileActors(Actors); // Lấy danh sách các actor được nhận thức
	if (Actors.Num() > 0)
	{
		return Actors[0]; // Trả về actor đầu tiên trong danh sách
	}

	return nullptr; // Trả về nullptr nếu không có actor nào được nhận thức
}

void ACAIController::ForgetActorifDead(AActor* ActorToForget)
{
	// Null check cho AIPerceptionComponent
	if (!AIPerceptionComponent) return;
	if (!ActorToForget) return;

	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC) return;

	if (ActorASC->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag()))
	{
		for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter = AIPerceptionComponent->GetPerceptualDataIterator(); Iter; ++Iter)
		{
			if (Iter->Key != ActorToForget) continue;

			// Đặt age của mọi stimulus về max để kích hoạt forgetting
			for (FAIStimulus& Stimuli : Iter->Value.LastSensedStimuli)
			{
				Stimuli.SetStimulusAge(TNumericLimits<float>::Max());
			}
		}
	}
}

void ACAIController::ClearAndDisableAllSenses()
{
	// Null check cho AIPerceptionComponent
	if (!AIPerceptionComponent) return;

	AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());

	// Vô hiệu hóa từng sense cụ thể mà ta biết
	if (SightConfig)
	{
		AIPerceptionComponent->SetSenseEnabled(
			SightConfig->GetSenseImplementation(),
			false
		);
	}

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->ClearValue(TargetBlackboardKeyName);
	}
}

void ACAIController::EnableAllSenses()
{
	// Null check cho AIPerceptionComponent
	if (!AIPerceptionComponent) return;

	// Kích hoạt lại từng sense cụ thể mà ta biết
	if (SightConfig)
	{
		AIPerceptionComponent->SetSenseEnabled(
			SightConfig->GetSenseImplementation(),
			true
		);
	}
}

void ACAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	// Tối ưu hóa logic: Sử dụng bool để rõ ràng hơn
	const bool bIsDead = (NewCount > 0);

	UBrainComponent* Brain = GetBrainComponent();
	if (!Brain) return;

	if (bIsDead)
	{
		// Pawn chết - dừng AI logic và vô hiệu hóa các giác quan
		Brain->StopLogic(TEXT("Pawn is dead"));
		ClearAndDisableAllSenses();
		bIsPawnDead = true;
	}
	else
	{
		// Pawn hồi sinh - khôi phục AI logic và kích hoạt lại các giác quan
		Brain->RestartLogic();
		EnableAllSenses();
		bIsPawnDead = false;
	}
}

void ACAIController::PawnStunTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (bIsPawnDead) return;

	if (NewCount != 0)
	{
		GetBrainComponent()->StopLogic("Stun");

	}
	else
	{
		GetBrainComponent()->StartLogic();
	}
}
