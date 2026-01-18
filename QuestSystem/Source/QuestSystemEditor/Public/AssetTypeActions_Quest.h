

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "Quest.h"

/**
 * Asset type actions for Quest assets
 */
class FAssetTypeActions_Quest : public FAssetTypeActions_Base
{
public:
	// Custom category for Quest assets
	static uint32 QuestAssetCategory;

	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override;
	// End of IAssetTypeActions interface
};