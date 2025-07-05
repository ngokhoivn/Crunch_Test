// Minion.cpp


#include "AI/Minion.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"


void AMinion::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	Super::SetGenericTeamId(NewTeamID); // Gọi hàm cha để thiết lập TeamID
    PickSkinBasedOnTeamID();
}

bool AMinion::IsActive() const
{
	return !IsDead();
}

void AMinion::Activate()
{
	RespawnImmediately();
}

void AMinion::SetGoal(AActor* Goal)
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsObject(GoalBlackboardKeyName, Goal); // Thiết lập mục tiêu trong Blackboard
		}
	}
}

void AMinion::PickSkinBasedOnTeamID()
{
    if (USkeletalMesh** Skin = SkinMap.Find(GetGenericTeamId()))
    {
		GetMesh()->SetSkeletalMesh(*Skin); // Đặt skin dựa trên TeamID
    }
}

void AMinion::OnRep_TeamID()
{
	PickSkinBasedOnTeamID();
}
