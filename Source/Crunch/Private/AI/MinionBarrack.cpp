//MinionBarrack.cpp

#include "AI/MinionBarrack.h"
#include "AI/Minion.h"
#include "GameFramework/PlayerStart.h"

// Sets default values
AMinionBarrack::AMinionBarrack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMinionBarrack::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) // Chỉ thực hiện trên server
	{
		SpawnNewMinions(5); // Spawn 5 minions khi bắt đầu

		GetWorldTimerManager().SetTimer(
			SpawnIntervalTimerHandle,		// Thiết lập timer để spawn minion định kỳ
			this,							// Hàm sẽ gọi
			&AMinionBarrack::SpawnNewGroup,	// Hàm sẽ gọi để spawn nhóm minion mới
			GroupSpawnInterval,				// Khoảng thời gian giữa các lần spawn nhóm minion mới	
			true // Lặp lại
		);
	}
}

// Called every frame
void AMinionBarrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

const APlayerStart* AMinionBarrack::GetNextSpawnSpot()
{
	if (SpawnSpots.Num() == 0)
	{
		return nullptr; // Không có điểm spawn nào
	}

	++NextSpawnSpotIndex;

	if (NextSpawnSpotIndex >= SpawnSpots.Num())
	{
		NextSpawnSpotIndex = 0; // Quay lại từ đầu nếu đã hết điểm spawn
	}

	return SpawnSpots[NextSpawnSpotIndex];// Trả về điểm spawn tiếp theo
}

void AMinionBarrack::SpawnNewGroup()
{
	int i = MinionPerGroup;

	while (i > 0)
	{
		FTransform SpawnTransform = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnSpot())
		{
			SpawnTransform = NextSpawnSpot->GetActorTransform(); // Lấy vị trí spawn từ điểm spawn tiếp theo
		}

		AMinion* NextAvaliableMinion = GetNextAvaliableMinion();
		if (!NextAvaliableMinion) break;

		NextAvaliableMinion->SetActorTransform(SpawnTransform); // Đặt vị trí spawn cho minion
		NextAvaliableMinion->Activate(); // Kích hoạt minion nếu nó đã "chết"
		--i;

	}

	SpawnNewMinions(i); // Spawn thêm minions nếu cần thiết
}

void AMinionBarrack::SpawnNewMinions(int Amt)
{
	for (int i = 0; i < Amt; ++i)
	{
		FTransform SpawnTransform = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnSpot())
		{
			SpawnTransform = NextSpawnSpot->GetActorTransform(); // Lấy vị trí spawn từ điểm spawn tiếp theo
		}

		AMinion* NewMinion = GetWorld()->SpawnActorDeferred<AMinion>(
			MinionClass,										// Class của minion sẽ spawn
			SpawnTransform,										// Vị trí spawn
			this,												// Chủ sở hữu của minion
			nullptr,											// Instigator (có thể là nullptr nếu không cần)
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn		// Luôn spawn bất kể va chạm	
		);

		NewMinion->SetGenericTeamId(BarrackTeamID); // Thiết lập ID đội cho minion mới
		NewMinion->FinishSpawning(SpawnTransform); // Kết thúc quá trình spawn
		NewMinion->SetGoal(Goal); // Thiết lập mục tiêu cho minion mới
		MinionPool.Add(NewMinion); // Thêm minion mới vào pool
	}
}

AMinion* AMinionBarrack::GetNextAvaliableMinion() const
{
	for (AMinion* Minion : MinionPool)
	{
		if (!Minion->IsActive()) // Kiểm tra minion đã "chết"
		{
			return Minion;
		}
	}
	return nullptr;
}

