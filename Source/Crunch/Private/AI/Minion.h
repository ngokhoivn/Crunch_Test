//Minion.h

#pragma once

#include "CoreMinimal.h"
#include "Character/CCharacter.h"
#include "Minion.generated.h"

/**
 * 
 */
UCLASS()
class AMinion : public ACCharacter
{
	GENERATED_BODY()
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	bool IsActive() const;
	void Activate();
	void SetGoal(AActor* Goal); // Thiết lập mục tiêu cho Minion

private:
	void PickSkinBasedOnTeamID();

	virtual void OnRep_TeamID() override;


	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TMap<FGenericTeamId, USkeletalMesh*> SkinMap;

	UPROPERTY(EditAnywhere, Category = "AI")
	FName GoalBlackboardKeyName = "Goal"; // Tên khóa trong Blackboard để lưu mục tiêu
};
