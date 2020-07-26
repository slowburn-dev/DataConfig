#include "DataConfig/DcEnv.h"
#include "Containers/BasicArray.h"

namespace DataConfig
{

//	TODO thread_local
static TBasicArray<FEnv>& GetStaticEnvs()
{
	static TBasicArray<FEnv> Envs;
	if (Envs.Num() == 0)
	{
		Envs.Emplace();
	}

	return Envs;
}

FEnv& DataConfig::Env()
{
	auto& Envs = GetStaticEnvs();
	return Envs[Envs.Num() - 1];
}

FEnv& DataConfig::PushEnv()
{
	auto& Envs = GetStaticEnvs();
	return Envs[Envs.Emplace()];
}

void DataConfig::PopEnv()
{
	auto& Envs = GetStaticEnvs();
	Envs.RemoveAt(Envs.Num() - 1);
}

FScopedEnv::FScopedEnv()
{
	PushEnv();
}

FScopedEnv::~FScopedEnv()
{
	PopEnv();
}


FDiagnostic& FEnv::Diag(uint32 InDiagID)
{
	return Diagnostics[Diagnostics.Emplace(InDiagID)];
}

} // namespace DataConfig

