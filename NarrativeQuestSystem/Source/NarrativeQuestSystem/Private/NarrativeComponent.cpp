// Copyright Epic Games, Inc. All Rights Reserved.

#include "NarrativeComponent.h"
#include "Quest.h"
#include "QuestBranch.h"
#include "QuestState.h"
#include "QuestTask.h"

UNarrativeComponent::UNarrativeComponent()
{
	PrimaryComponentTick.bCanEverTick = true; // CHANGED: Enable ticking
	PrimaryComponentTick.TickInterval = 0.1f; // Tick 10 times per second
	SetIsReplicatedByDefault(true);
}

void UNarrativeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNarrativeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Tick all active quests and their tasks
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->IsQuestInProgress())
		{
			// Get all active branches
			TArray<UQuestBranch*> ActiveBranches = Quest->GetActiveBranches();

			for (UQuestBranch* Branch : ActiveBranches)
			{
				if (Branch && Branch->bIsActive)
				{
					// Tick all tasks in the branch
					for (UQuestTask* Task : Branch->QuestTasks)
					{
						if (Task && Task->IsTaskActive())
						{
							Task->TickTask();
						}
					}
				}
			}
		}
	}
}

UQuest* UNarrativeComponent::BeginQuest(UQuest* QuestAsset)
{
	if (!QuestAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginQuest: No quest asset provided!"));
		return nullptr;
	}

	// Check if quest is already active (compare by name/path)
	for (UQuest* ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest && ActiveQuest->GetName() == QuestAsset->GetName())
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest '%s' is already active"), *QuestAsset->GetName());
			return ActiveQuest;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=== BeginQuest: Loading quest asset '%s' ==="), *QuestAsset->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Asset has %d states, %d branches"),
		QuestAsset->QuestStates.Num(), QuestAsset->QuestBranches.Num());

	// Create a runtime instance by duplicating the asset
	UQuest* NewQuest = DuplicateObject<UQuest>(QuestAsset, this);

	if (!NewQuest)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to duplicate quest asset!"));
		return nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("Runtime instance created: %d states, %d branches"),
		NewQuest->QuestStates.Num(), NewQuest->QuestBranches.Num());

	// Initialize the quest
	NewQuest->Initialize(this);
	ActiveQuests.Add(NewQuest);

	// Bind quest events
	NewQuest->OnQuestStarted.AddDynamic(this, &UNarrativeComponent::HandleQuestStarted);
	NewQuest->OnQuestSucceeded.AddDynamic(this, &UNarrativeComponent::HandleQuestSucceeded);
	NewQuest->OnQuestFailed.AddDynamic(this, &UNarrativeComponent::HandleQuestFailed);
	NewQuest->OnQuestStateChanged.AddDynamic(this, &UNarrativeComponent::HandleQuestStateChanged);

	OnQuestListUpdated.Broadcast(this);

	// Start the quest
	NewQuest->BeginQuest();

	return NewQuest;
}

// Legacy API support - wraps the new asset-based API
UQuest* UNarrativeComponent::BeginQuestByClass(TSubclassOf<UQuest> QuestClass)
{
	if (!QuestClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginQuestByClass: No quest class provided!"));
		return nullptr;
	}

	// Get the CDO as a template
	UQuest* QuestTemplate = Cast<UQuest>(QuestClass->GetDefaultObject());
	if (!QuestTemplate)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginQuestByClass: Failed to get quest template"));
		return nullptr;
	}

	// Call the asset-based API with the CDO
	return BeginQuest(QuestTemplate);
}

void UNarrativeComponent::ForgetQuest(UQuest* Quest)
{
	if (Quest && ActiveQuests.Contains(Quest))
	{
		Quest->ForgetQuest();
		ActiveQuests.Remove(Quest);
		OnQuestListUpdated.Broadcast(this);
	}
}

void UNarrativeComponent::RestartQuest(UQuest* Quest)
{
	if (Quest)
	{
		Quest->RestartQuest();
	}
}

bool UNarrativeComponent::IsQuestActive(UQuest* QuestAsset) const
{
	return GetQuest(QuestAsset) != nullptr;
}

// Legacy API support
bool UNarrativeComponent::IsQuestActiveByClass(TSubclassOf<UQuest> QuestClass) const
{
	if (!QuestClass)
	{
		return false;
	}

	// Check if any active quest is of this class type
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->IsA(QuestClass))
		{
			return true;
		}
	}

	return false;
}

UQuest* UNarrativeComponent::GetQuest(UQuest* QuestAsset) const
{
	if (!QuestAsset)
	{
		return nullptr;
	}

	// Find active quest that matches the asset name
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->GetName() == QuestAsset->GetName())
		{
			return Quest;
		}
	}

	return nullptr;
}

// Legacy API support
UQuest* UNarrativeComponent::GetQuestByClass(TSubclassOf<UQuest> QuestClass) const
{
	if (!QuestClass)
	{
		return nullptr;
	}

	// Find active quest that is of this class type
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->IsA(QuestClass))
		{
			return Quest;
		}
	}

	return nullptr;
}

TArray<UQuest*> UNarrativeComponent::GetAllQuests() const
{
	TArray<UQuest*> AllQuests;
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest)
		{
			AllQuests.Add(Quest);
		}
	}
	return AllQuests;
}

TArray<UQuest*> UNarrativeComponent::GetInProgressQuests() const
{
	TArray<UQuest*> InProgressQuests;
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->IsQuestInProgress())
		{
			InProgressQuests.Add(Quest);
		}
	}
	return InProgressQuests;
}

TArray<UQuest*> UNarrativeComponent::GetSucceededQuests() const
{
	TArray<UQuest*> SucceededQuests;
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->HasQuestSucceeded())
		{
			SucceededQuests.Add(Quest);
		}
	}
	return SucceededQuests;
}

TArray<UQuest*> UNarrativeComponent::GetFailedQuests() const
{
	TArray<UQuest*> FailedQuests;
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->HasQuestFailed())
		{
			FailedQuests.Add(Quest);
		}
	}
	return FailedQuests;
}

void UNarrativeComponent::CompleteNarrativeDataTask(FName TaskName, const FString& Argument, int32 Quantity)
{
	if (TaskName.IsNone())
	{
		return;
	}

	// Find or create the data task
	bool bFound = false;
	for (FNarrativeDataTask& DataTask : CompletedDataTasks)
	{
		if (DataTask.TaskName == TaskName && DataTask.Argument == Argument)
		{
			DataTask.Count += Quantity;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		FNarrativeDataTask NewTask(TaskName, Argument);
		NewTask.Count = Quantity;
		CompletedDataTasks.Add(NewTask);
	}

	UE_LOG(LogTemp, Log, TEXT("Data task completed: %s - %s (Count: %d)"),
		*TaskName.ToString(),
		*Argument,
		Quantity);

	// AUTO-UPDATE: Check all active quests and update their task progress
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->IsQuestInProgress())
		{
			// Get all active branches and update their tasks
			TArray<UQuestBranch*> ActiveBranches = Quest->GetActiveBranches();
			for (UQuestBranch* Branch : ActiveBranches)
			{
				if (Branch)
				{
					Branch->CheckTaskProgress();
				}
			}
		}
	}
}

bool UNarrativeComponent::HasCompletedDataTask(FName TaskName, const FString& Argument) const
{
	return GetDataTaskCount(TaskName, Argument) > 0;
}

int32 UNarrativeComponent::GetDataTaskCount(FName TaskName, const FString& Argument) const
{
	for (const FNarrativeDataTask& DataTask : CompletedDataTasks)
	{
		if (DataTask.TaskName == TaskName && DataTask.Argument == Argument)
		{
			return DataTask.Count;
		}
	}
	return 0;
}

void UNarrativeComponent::ClearDataTasks()
{
	CompletedDataTasks.Empty();
}

// EVENT HANDLERS

void UNarrativeComponent::HandleQuestStarted(UQuest* Quest)
{
	OnQuestStarted.Broadcast(Quest);
}

void UNarrativeComponent::HandleQuestSucceeded(UQuest* Quest)
{
	OnQuestSucceeded.Broadcast(Quest);
}

void UNarrativeComponent::HandleQuestFailed(UQuest* Quest)
{
	OnQuestFailed.Broadcast(Quest);
}

void UNarrativeComponent::HandleQuestStateChanged(UQuest* Quest, UQuestState* NewState)
{
	OnQuestStateChanged.Broadcast(Quest, NewState);
}