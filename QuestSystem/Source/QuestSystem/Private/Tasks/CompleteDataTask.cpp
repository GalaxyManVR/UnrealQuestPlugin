// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n


#include "Tasks/CompleteDataTask.h"
#include "QuestComponent.h"

UCompleteDataTask::UCompleteDataTask()
{
	bRetroactive = false;
	StartingCount = 0;
	TickInterval = 0.5f; // Check every half second
}

void UCompleteDataTask::BeginTask_Implementation()  // Fixed: was CompleteDataTask::
{
	Super::BeginTask_Implementation();

	if (!OwningComp)
	{
		return;
	}

	// Get the current count
	int32 CurrentCount = OwningComp->GetDataTaskCount(DataTask, Argument);

	if (bRetroactive)
	{
		// If retroactive, we count everything that's already been done
		StartingCount = 0;
		CurrentProgress = CurrentCount;

		// Check if we're already done
		if (CurrentProgress >= RequiredQuantity)
		{
			SetProgress(RequiredQuantity);
		}
	}
	else
	{
		// If not retroactive, we only count new completions
		StartingCount = CurrentCount;
		CurrentProgress = 0;
	}
}

void UCompleteDataTask::TaskTick_Implementation()
{
	Super::TaskTick_Implementation();

	if (!OwningComp || IsComplete())
	{
		return;
	}

	// Check current count
	int32 CurrentCount = OwningComp->GetDataTaskCount(DataTask, Argument);
	int32 NewProgress = CurrentCount - StartingCount;

	if (NewProgress > CurrentProgress)
	{
		SetProgress(NewProgress);
	}
}

FText UCompleteDataTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	if (!Argument.IsEmpty())
	{
		return FText::Format(
			FText::FromString(TEXT("{0}: {1}")),
			FText::FromName(DataTask),
			FText::FromString(Argument)
		);
	}

	return FText::FromName(DataTask);
}

FString UCompleteDataTask::GetTaskNodeDescription_Implementation() const
{
	if (!Argument.IsEmpty())
	{
		return FString::Printf(TEXT("%s: %s"), *DataTask.ToString(), *Argument);
	}
	return DataTask.ToString();
}