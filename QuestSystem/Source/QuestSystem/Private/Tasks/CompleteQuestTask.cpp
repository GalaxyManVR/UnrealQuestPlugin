

#include "Tasks/CompleteQuestTask.h"
#include "Quest.h"
#include "QuestComponent.h"

UCompleteQuestTask::UCompleteQuestTask()
{
}

void UCompleteQuestTask::BeginTask_Implementation()
{
	Super::BeginTask_Implementation();

	if (!OwningComp || !QuestToComplete)
	{
		return;
	}

	// FIXED: Use ByClass version
	TrackedQuest = OwningComp->GetQuestByClass(QuestToComplete);

	if (TrackedQuest)
	{
		// Check if it's already completed
		if (TrackedQuest->HasQuestSucceeded())
		{
			SetProgress(RequiredQuantity);
			return;
		}

		// Bind to the quest's success event
		TrackedQuest->OnQuestSucceeded.AddDynamic(this, &UCompleteQuestTask::OnTargetQuestSucceeded);
	}
	else
	{
		// Quest hasn't been started yet - we'll need to check periodically
		// You could implement a tick here or use a different approach
	}
}

void UCompleteQuestTask::EndTask_Implementation()
{
	Super::EndTask_Implementation();

	// Unbind from the quest
	if (TrackedQuest)
	{
		TrackedQuest->OnQuestSucceeded.RemoveDynamic(this, &UCompleteQuestTask::OnTargetQuestSucceeded);
	}
}

FText UCompleteQuestTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	if (QuestToComplete)
	{
		// Get the quest's default name
		UQuest* QuestCDO = QuestToComplete->GetDefaultObject<UQuest>();
		if (QuestCDO && !QuestCDO->QuestName.IsEmpty())
		{
			return FText::Format(
				FText::FromString(TEXT("Complete: {0}")),
				QuestCDO->QuestName
			);
		}
	}

	return FText::FromString(TEXT("Complete Quest"));
}

FString UCompleteQuestTask::GetTaskNodeDescription_Implementation() const
{
	if (QuestToComplete)
	{
		return FString::Printf(TEXT("Complete Quest: %s"), *QuestToComplete->GetName());
	}
	return TEXT("Complete Quest");
}

void UCompleteQuestTask::OnTargetQuestSucceeded(UQuest* Quest)
{
	if (Quest == TrackedQuest)
	{
		SetProgress(RequiredQuantity);
	}
}