// Copyright Epic Games, Inc. All Rights Reserved.

#include "NarrativeQuestSystemEditor.h"
#include "AssetToolsModule.h"
#include "AssetTypeActions_Quest.h"
#include "QuestDetails.h"
#include "Quest.h"
#include "QuestEditorUtilities.h"
#include "PropertyEditorModule.h"
#include "QuestGraphNodeFactory.h"
#include "EdGraphUtilities.h"  // THIS WAS MISSING - needed for RegisterVisualNodeFactory

#define LOCTEXT_NAMESPACE "FNarrativeQuestSystemEditorModule"

uint32 FNarrativeQuestSystemEditorModule::QuestAssetCategoryBit = 0;

void FNarrativeQuestSystemEditorModule::StartupModule()
{
	// Register custom asset category
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	NarrativeAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Narrative")), LOCTEXT("NarrativeAssetCategory", "Narrative"));

	// Update the static category bit so the factory can use it
	QuestAssetCategoryBit = NarrativeAssetCategoryBit;

	// Set the category in the asset type actions
	FAssetTypeActions_Quest::NarrativeAssetCategory = NarrativeAssetCategoryBit;

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

void FNarrativeQuestSystemEditorModule::ShutdownModule()
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

void FNarrativeQuestSystemEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register Quest asset type
	TSharedRef<FAssetTypeActions_Quest> QuestAssetAction = MakeShared<FAssetTypeActions_Quest>();
	AssetTools.RegisterAssetTypeActions(QuestAssetAction);
	RegisteredAssetTypeActions.Add(QuestAssetAction);
}

void FNarrativeQuestSystemEditorModule::UnregisterAssetTypeActions()
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

IMPLEMENT_MODULE(FNarrativeQuestSystemEditorModule, NarrativeQuestSystemEditor)