#pragma once

#include "CoreMinimal.h"
#include "AssetData.h"

class FExtender;

namespace DcEditorExtra
{
	
DATACONFIGEDITOREXTRA_API TSharedRef<FExtender> DumpAssetToLogExtender(const TArray<FAssetData>& SelectedAssets);

} // namespace DcEditorExtra


