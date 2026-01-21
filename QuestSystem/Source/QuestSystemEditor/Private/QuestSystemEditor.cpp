// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "QuestSystemEditor.h"
#include "AssetToolsModule.h"
#include "AssetTypeActions_Quest.h"
#include "QuestDetails.h"
#include "Quest.h"
#include "QuestEditorUtilities.h"
#include "PropertyEditorModule.h"
#include "QuestGraphNodeFactory.h"
#include "EdGraphUtilities.h"

#define LOCTEXT_NAMESPACE "FQuestSystemEditorModule"

// Initialize the static member - use the correct type!
EAssetTypeCategories::Type FQuestSystemEditorModule::QuestAssetCategoryBit = EAssetTypeCategories::Misc;

void FQuestSystemEditorModule::StartupModule()
{
	// Register custom asset category
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	QuestAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Quest")), LOCTEXT("QuestAssetCategory", "Quest"));

	// Register asset type actions
	RegisterAssetTypeActions();

	// Register detail customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		UQuest::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FQuestDetails::MakeInstance)
	);

	// Register custom node factory for Slate widgets
	QuestGraphNodeFactory = MakeShared<FQuestGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(QuestGraphNodeFactory);
}

void FQuestSystemEditorModule::ShutdownModule()
{
	// Unregister node factory
	if (QuestGraphNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(QuestGraphNodeFactory);
		QuestGraphNodeFactory.Reset();
	}

	// Unregister detail customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UQuest::StaticClass()->GetFName());
	}

	// Unregister asset type actions
	UnregisterAssetTypeActions();
}

void FQuestSystemEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register Quest asset type
	TSharedRef<FAssetTypeActions_Quest> QuestAssetAction = MakeShared<FAssetTypeActions_Quest>();
	AssetTools.RegisterAssetTypeActions(QuestAssetAction);
	RegisteredAssetTypeActions.Add(QuestAssetAction);
}

void FQuestSystemEditorModule::UnregisterAssetTypeActions()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (const TSharedRef<FAssetTypeActions_Quest>& Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
	RegisteredAssetTypeActions.Empty();
}

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FQuestSystemEditorModule, QuestSystemEditor)