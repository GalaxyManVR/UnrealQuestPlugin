// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuestDetails.h"
#include "Quest.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "QuestDetails"

TSharedRef<IDetailCustomization> FQuestDetails::MakeInstance()
{
	return MakeShareable(new FQuestDetails);
}

void FQuestDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Get the quest being edited
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	TWeakObjectPtr<UQuest> Quest = Cast<UQuest>(ObjectsBeingCustomized[0].Get());
	if (!Quest.IsValid())
	{
		return;
	}

	// Customize the Quest category
	IDetailCategoryBuilder& QuestCategory = DetailBuilder.EditCategory("Quest", FText::GetEmpty(), ECategoryPriority::Important);

	// Add quest name and description at the top
	QuestCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UQuest, QuestName));
	QuestCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UQuest, QuestDescription));
	QuestCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UQuest, bTracked));

	// Add inheritable states
	QuestCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UQuest, InheritableStates));

	// Add a separator
	QuestCategory.AddCustomRow(LOCTEXT("QuestInfoSeparator", "Quest Info"))
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("QuestStructure", "Quest Structure (Edit in Quest Graph)"))
			.Font(IDetailLayoutBuilder::GetDetailFontBold())
		];

	// Show read-only state and branch info
	QuestCategory.AddCustomRow(LOCTEXT("QuestStats", "Quest Statistics"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("StatesLabel", "States"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text_Lambda([Quest]()
			{
				if (Quest.IsValid())
				{
					return FText::AsNumber(Quest->QuestStates.Num());
				}
				return FText::FromString(TEXT("0"));
			})
			.Font(IDetailLayoutBuilder::GetDetailFont())
		];

	QuestCategory.AddCustomRow(LOCTEXT("BranchCount", "Branch Count"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BranchesLabel", "Branches"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text_Lambda([Quest]()
			{
				if (Quest.IsValid())
				{
					return FText::AsNumber(Quest->QuestBranches.Num());
				}
				return FText::FromString(TEXT("0"));
			})
			.Font(IDetailLayoutBuilder::GetDetailFont())
		];
}

#undef LOCTEXT_NAMESPACE
