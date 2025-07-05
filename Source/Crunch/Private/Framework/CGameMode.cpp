//CGameMode.cpp

#include "Framework/CGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	// gọi hàm cha để tạo PlayerController
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);

	// ép kiểu PlayerController sang IGenericTeamAgentInterface
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);

	// lấy ID đội cho người chơi mới
	FGenericTeamId TeamID = GetTeamIDForPlayer(NewPlayerController); 

	// Kiểm tra xem PlayerController có hỗ trợ IGenericTeamAgentInterface không
	if (NewPlayerTeamInterface) 
	{
		// Gán ID đội cho người chơi
		NewPlayerTeamInterface->SetGenericTeamId(TeamID); 
	}

	NewPlayerController->StartSpot = FindNextStartSpotForTeam(TeamID); // tìm vị trí bắt đầu cho người chơi
	return NewPlayerController;
}

FGenericTeamId ACGameMode::GetTeamIDForPlayer(const APlayerController* PlayerController) const
{
	static int PlayerCount = 0;
	++PlayerCount;
	return FGenericTeamId(PlayerCount % 2); // Chia đều người chơi vào 2 đội
}

AActor* ACGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);// tìm kiếm tag của đội trong map
	if (!StartSpotTag) // nếu không tìm thấy tag, trả về nullptr
	{
		return nullptr;
	}

	UWorld* World = GetWorld();// lấy thế giới hiện tại

	for (TActorIterator<APlayerStart> It(World); It; ++It) // lặp qua tất cả các PlayerStart trong thế giới
	{
		APlayerStart* PlayerStart = *It;
		if (It->PlayerStartTag == *StartSpotTag) // kiểm tra xem PlayerStart có tag trùng với tag của đội không
		{
			if (It->PlayerStartTag != FName("Taken")) // nếu PlayerStart chưa được sử dụng
			{
				return *It; // trả về PlayerStart phù hợp
			}
		}
	}
	return nullptr; // nếu không tìm thấy PlayerStart phù hợp, trả về nullptr
}
