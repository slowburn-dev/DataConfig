#include "DcProjectGame.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogDcProjectGame);

void FDcProjectGame::StartupModule()
{
	UE_LOG(LogDcProjectGame, Log, TEXT("DcProjectGame module starting up"));
}

void FDcProjectGame::ShutdownModule()
{
	UE_LOG(LogDcProjectGame, Log, TEXT("DcProjectGame module shutting down"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FDcProjectGame, DcProjectGame, "DcProjectGame");
