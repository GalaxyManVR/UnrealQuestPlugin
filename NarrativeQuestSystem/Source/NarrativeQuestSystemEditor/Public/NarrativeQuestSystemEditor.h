// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuestEditorUtilities.h"
#include "AssetTypeActions_Quest.h"
#include "QuestGraphNodeFactory.h"
#include "LevelEditor.h"
#include "ToolMenus.h"


class FAssetTypeActions_Quest;
class FQuestGraphNodeFactory;

class FNarrativeQuestSystemEditorModule : public IModuleInterface
{
public:
	/** Adds the dropdown section */
	static uint32 QuestAssetCategoryBit;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register asset type actions */
	void RegisterAssetTypeActions();

	/** Unregister asset type actions */
	void UnregisterAssetTypeActions();

	/** Registered asset type actions */
	TArray<TSharedRef<FAssetTypeActions_Quest>> RegisteredAssetTypeActions;

	/** Custom node factory for Slate widgets */
	TSharedPtr<FQuestGraphNodeFactory> QuestGraphNodeFactory;

	/** Custom category bit for Narrative assets */
	EAssetTypeCategories::Type NarrativeAssetCategoryBit;
};