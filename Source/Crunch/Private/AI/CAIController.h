//CAIController.h

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include <Perception/AIPerceptionTypes.h>
#include "GameplayTagContainer.h"
#include "CAIController.generated.h"

class UBehaviorTree;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
/**
 * 
 */
UCLASS()
class ACAIController : public AAIController
{
	GENERATED_BODY()
	
public:	
	ACAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "AI Behavior")
	FName TargetBlackboardKeyName = "Target"; // Tên khóa trong Blackboard để lưu trữ mục tiêu

	UPROPERTY(EditDefaultsOnly, Category = "AI Behavior")
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	class UAIPerceptionComponent* AIPerceptionComponent;
	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus); // Cập nhật thông tin nhận thức của mục tiêu

	UFUNCTION()
	void TargetForgotten(AActor* ForgottenActor); // Xử lý khi mục tiêu bị quên

	const UObject* GetCurrentTarget() const; // Lấy mục tiêu hiện tại từ Blackboard
	void SetCurrentTarget(AActor* NewTarget); // Gán mục tiêu hiện tại

	AActor* GetNextPerceivedActor() const; // Lấy mục tiêu tiếp theo được nhận thức

	void ForgetActorifDead(AActor* ActorToForget); // Quên mục tiêu nếu nó đã chết

	void ClearAndDisableAllSenses(); // Xóa và vô hiệu hóa tất cả các giác quan
	void EnableAllSenses(); // Kích hoạt lại tất cả các giác quan

	void PawnDeadTagUpdated(const FGameplayTag Tag, int32 NewCount); // Cập nhật khi thẻ "Dead" của Pawn thay đổi
	void PawnStunTagUpdated(const FGameplayTag Tag, int32 NewCount); // Cập nhật khi thẻ "Dead" của Pawn thay đổi

	bool bIsPawnDead = false;
};
