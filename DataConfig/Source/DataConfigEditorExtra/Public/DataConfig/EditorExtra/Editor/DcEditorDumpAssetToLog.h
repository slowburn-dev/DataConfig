#pragma once

#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_OLDER_THAN(5, 1, 0)
#include "AssetData.h"
#else
#include "AssetRegistry/AssetData.h"
#endif

class FExtender;

namespace DcEditorExtra
{
	
DATACONFIGEDITOREXTRA_API TSharedRef<FExtender> DumpAssetToLogExtender(const TArray<FAssetData>& SelectedAssets);

} // namespace DcEditorExtra


