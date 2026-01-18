#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "QuestEditorUtilities.h"
#include "AssetTypeActions_Quest.h"
#include "QuestGraphNodeFactory.h"
#include "LevelEditor.h"
#include "ToolMenus.h"

class FAssetTypeActions_Quest;
class FQuestGraphNodeFactory;

class FQuestSystemEditorModule : public IModuleInterface
{
public:
	/** Custom category bit for Quest assets */
	static EAssetTypeCategories::Type QuestAssetCategoryBit;

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
};