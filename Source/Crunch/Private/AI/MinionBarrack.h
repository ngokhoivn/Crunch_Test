//MinionBarrack.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "Minion.h"
#include "MinionBarrack.generated.h"

UCLASS()
class AMinionBarrack : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMinionBarrack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FGenericTeamId BarrackTeamID; // Team ID cho minion

	UPROPERTY(EditAnywhere, Category = "Spawn")
	AActor* Goal; // Mục tiêu mà minion sẽ hướng tới

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.1"))
	float GroupSpawnInterval = 5.0f; // Khoảng thời gian giữa các lần spawn nhóm (giây)

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int MinionPerGroup = 3; // Số lượng minion trong mỗi nhóm spawn

	UPROPERTY()
	TArray<class AMinion*> MinionPool; // Pool chứa các minion đã spawn

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AMinion> MinionClass; // Class của minion sẽ spawn

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<class APlayerStart*> SpawnSpots; // Các điểm spawn cho minion

	int NextSpawnSpotIndex = -1;

	const APlayerStart* GetNextSpawnSpot(); // Lấy điểm spawn tiếp theo	

	void SpawnNewGroup(); // Hàm spawn nhóm minion mới
	void SpawnNewMinions(int Amt);// Hàm spawn minion mới
	AMinion* GetNextAvaliableMinion() const; // Lấy minion tiếp theo có sẵn trong pool

	FTimerHandle SpawnIntervalTimerHandle; // Timer để spawn minion định kỳ


};
