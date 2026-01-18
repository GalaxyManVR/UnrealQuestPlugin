// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetTypeActions_Quest.h"
#include "Quest.h"
#include "QuestAssetEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_Quest"

uint32 FAssetTypeActions_Quest::NarrativeAssetCategory = 0;

FText FAssetTypeActions_Quest::GetName() const
{
	return LOCTEXT("QuestAssetName", "Quest");
}

FColor FAssetTypeActions_Quest::GetTypeColor() const
{
	return FColor(63, 126, 255); // Blue color
}

UClass* FAssetTypeActions_Quest::GetSupportedClass() const
{
	return UQuest::StaticClass();
}

void FAssetTypeActions_Quest::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Obj : InObjects)
	{
		if (UQuest* Quest = Cast<UQuest>(Obj))
		{
			TSharedRef<FQuestAssetEditor> QuestEditor = MakeShared<FQuestAssetEditor>();
			QuestEditor->InitQuestAssetEditor(EToolkitMode::Standalone, EditWithinLevelEditor, Quest);
		}
	}
}

uint32 FAssetTypeActions_Quest::GetCategories()
{
	return NarrativeAssetCategory;
}

#undef LOCTEXT_NAMESPACE
