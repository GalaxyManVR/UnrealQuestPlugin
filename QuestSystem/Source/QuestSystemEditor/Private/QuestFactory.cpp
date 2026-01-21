// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n


#include "QuestFactory.h"
#include "Quest.h"
#include "QuestState.h"
#include "QuestSystemEditor.h"
#include "AssetTypeCategories.h"

UQuestFactory::UQuestFactory()
{
    bCreateNew = true;
    bEditAfterNew = true;
    SupportedClass = UQuest::StaticClass();
}

UObject* UQuestFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    UQuest* NewQuest = NewObject<UQuest>(InParent, Class, Name, Flags);

    if (NewQuest)
    {
        // Initialize creates default states
        NewQuest->Initialize(nullptr);

        // Mark the package dirty so the default states are saved
        NewQuest->MarkPackageDirty();
    }

    return NewQuest;
}

bool UQuestFactory::ShouldShowInNewMenu() const
{
    return true;
}

uint32 UQuestFactory::GetMenuCategories() const
{
    return FQuestSystemEditorModule::QuestAssetCategoryBit;
}
