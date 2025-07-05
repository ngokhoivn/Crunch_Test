//BTTask_SendInputToAbilitySystem.cpp

#include "AI/BTTask_SendInputToAbilitySystem.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner(); // Lấy AIController từ Behavior Tree Component
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AIController->GetPawn()); // Lấy Ability System Component từ Pawn của AIController
	if (OwnerASC) {
		OwnerASC->PressInputID(static_cast<int32>(InputID)); // Gửi đầu vào đến Ability System
		return EBTNodeResult::Succeeded; // Trả về kết quả thành công
	}
	return EBTNodeResult::Failed; // Trả về kết quả thất bại nếu không tìm thấy Ability System Component
}
