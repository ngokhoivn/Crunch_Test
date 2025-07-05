//BTTask_SendInputToAbilitySystem.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "BTTask_SendInputToAbilitySystem.generated.h"

/**
 * 
 */
UCLASS()
class UBTTask_SendInputToAbilitySystem : public UBTTaskNode
{
	GENERATED_BODY()

	// Thông tin về đầu vào sẽ được gửi đến Ability System
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override; 
	UPROPERTY(EditAnywhere, Category = "Ability")
	ECAbilityInputID InputID; // ID của đầu vào sẽ được gửi đến Ability System
	
};
