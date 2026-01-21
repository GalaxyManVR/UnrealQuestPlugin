// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "Tasks/FinishDialogueTask.h"
#include "QuestComponent.h"

UFinishDialogueTask::UFinishDialogueTask()
{
	bAddWaypointToAvatar = false;
}

void UFinishDialogueTask::BeginTask_Implementation()
{
	Super::BeginTask_Implementation();

	BindToDialogueEvents();

	// TODO: Add waypoint if bAddWaypointToAvatar is true
	// This would integrate with your game's dialogue and waypoint systems
}

void UFinishDialogueTask::EndTask_Implementation()
{
	Super::EndTask_Implementation();

	UnbindFromDialogueEvents();

	// TODO: Remove waypoint if it was added
}

FText UFinishDialogueTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	if (!DialogueName.IsNone())
	{
		return FText::Format(
			FText::FromString(TEXT("Talk to {0}")),
			FText::FromName(DialogueName)
		);
	}

	return FText::FromString(TEXT("Complete Dialogue"));
}

FString UFinishDialogueTask::GetTaskNodeDescription_Implementation() const
{
	if (!DialogueName.IsNone())
	{
		return FString::Printf(TEXT("Dialogue: %s"), *DialogueName.ToString());
	}
	return TEXT("Finish Dialogue");
}

void UFinishDialogueTask::OnDialogueFinished()
{
	SetProgress(RequiredQuantity);
}

//TODO: Future things to add

void UFinishDialogueTask::BindToDialogueEvents()
{
	// TODO: Bind to your dialogue system's events
	// Example:
	// if (Dialogue)
	// {
	//     Dialogue->OnDialogueFinished.AddDynamic(this, &UFinishDialogueTask::OnDialogueFinished);
	// }
}

void UFinishDialogueTask::UnbindFromDialogueEvents()
{
	// TODO: Unbind from your dialogue system's events
	// Example:
	// if (Dialogue)
	// {
	//     Dialogue->OnDialogueFinished.RemoveDynamic(this, &UFinishDialogueTask::OnDialogueFinished);
	// }
}
