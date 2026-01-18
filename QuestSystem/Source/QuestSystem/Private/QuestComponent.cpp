

#include "QuestComponent.h"
#include "Quest.h"
#include "QuestBranch.h"
#include "QuestState.h"
#include "QuestTask.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = true; // CHANGED: Enable ticking
	PrimaryComponentTick.TickInterval = 0.1f; // Tick 10 times per second
	SetIsReplicatedByDefault(true);
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

UQuest* UQuestComponent::BeginQuest(UQuest* QuestAsset)
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
	NewQuest->OnQuestStarted.AddDynamic(this, &UQuestComponent::HandleQuestStarted);
	NewQuest->OnQuestSucceeded.AddDynamic(this, &UQuestComponent::HandleQuestSucceeded);
	NewQuest->OnQuestFailed.AddDynamic(this, &UQuestComponent::HandleQuestFailed);
	NewQuest->OnQuestStateChanged.AddDynamic(this, &UQuestComponent::HandleQuestStateChanged);

	OnQuestListUpdated.Broadcast(this);

	// Start the quest
	NewQuest->BeginQuest();

	return NewQuest;
}

// Legacy API support - wraps the new asset-based API
UQuest* UQuestComponent::BeginQuestByClass(TSubclassOf<UQuest> QuestClass)
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

void UQuestComponent::ForgetQuest(UQuest* Quest)
{
	if (Quest && ActiveQuests.Contains(Quest))
	{
		Quest->ForgetQuest();
		ActiveQuests.Remove(Quest);
		OnQuestListUpdated.Broadcast(this);
	}
}

void UQuestComponent::RestartQuest(UQuest* Quest)
{
	if (Quest)
	{
		Quest->RestartQuest();
	}
}

bool UQuestComponent::IsQuestActive(UQuest* QuestAsset) const
{
	return GetQuest(QuestAsset) != nullptr;
}

// Legacy API support
bool UQuestComponent::IsQuestActiveByClass(TSubclassOf<UQuest> QuestClass) const
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

UQuest* UQuestComponent::GetQuest(UQuest* QuestAsset) const
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
UQuest* UQuestComponent::GetQuestByClass(TSubclassOf<UQuest> QuestClass) const
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

TArray<UQuest*> UQuestComponent::GetAllQuests() const
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

TArray<UQuest*> UQuestComponent::GetInProgressQuests() const
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

TArray<UQuest*> UQuestComponent::GetSucceededQuests() const
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

TArray<UQuest*> UQuestComponent::GetFailedQuests() const
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

void UQuestComponent::CompleteDataTask(FName TaskName, const FString& Argument, int32 Quantity)
{
	if (TaskName.IsNone())
	{
		return;
	}

	// Find or create the data task
	bool bFound = false;
	for (FQuestDataTask& DataTask : CompletedDataTasks)
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
		FQuestDataTask NewTask(TaskName, Argument);
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

bool UQuestComponent::HasCompletedDataTask(FName TaskName, const FString& Argument) const
{
	return GetDataTaskCount(TaskName, Argument) > 0;
}

int32 UQuestComponent::GetDataTaskCount(FName TaskName, const FString& Argument) const
{
	for (const FQuestDataTask& DataTask : CompletedDataTasks)
	{
		if (DataTask.TaskName == TaskName && DataTask.Argument == Argument)
		{
			return DataTask.Count;
		}
	}
	return 0;
}

void UQuestComponent::ClearDataTasks()
{
	CompletedDataTasks.Empty();
}

// EVENT HANDLERS

void UQuestComponent::HandleQuestStarted(UQuest* Quest)
{
	OnQuestStarted.Broadcast(Quest);
}

void UQuestComponent::HandleQuestSucceeded(UQuest* Quest)
{
	OnQuestSucceeded.Broadcast(Quest);
}

void UQuestComponent::HandleQuestFailed(UQuest* Quest)
{
	OnQuestFailed.Broadcast(Quest);
}

void UQuestComponent::HandleQuestStateChanged(UQuest* Quest, UQuestState* NewState)
{
	OnQuestStateChanged.Broadcast(Quest, NewState);
}

// ========================================
// ENEMY KILL TRACKING
// ========================================

void UQuestComponent::RecordEnemyKill(AActor* Enemy, AActor* Killer)
{
	if (!Enemy)
	{
		return;
	}

	// Broadcast the event first (so tasks can respond)
	OnEnemyKilled.Broadcast(Enemy, Killer);

	// Determine the enemy's class and tags
	TSubclassOf<AActor> EnemyClass = Enemy->GetClass();
	FName EnemyTag = Enemy->Tags.Num() > 0 ? Enemy->Tags[0] : NAME_None;

	// Update kill tracking data
	bool bFoundExisting = false;

	// Try to find existing entry for this class/tag combo
	for (FEnemyKillData& KillData : EnemyKills)
	{
		if (KillData.Matches(EnemyClass, EnemyTag))
		{
			KillData.KillCount++;
			bFoundExisting = true;
			break;
		}
	}

	// If no existing entry, create a new one
	if (!bFoundExisting)
	{
		FEnemyKillData NewKillData;
		NewKillData.EnemyClass = EnemyClass;
		NewKillData.EnemyTag = EnemyTag;
		NewKillData.KillCount = 1;
		EnemyKills.Add(NewKillData);
	}

	// Also track with no filters (total kills)
	bFoundExisting = false;
	for (FEnemyKillData& KillData : EnemyKills)
	{
		if (KillData.Matches(nullptr, NAME_None))
		{
			KillData.KillCount++;
			bFoundExisting = true;
			break;
		}
	}

	if (!bFoundExisting)
	{
		FEnemyKillData NewKillData;
		NewKillData.KillCount = 1;
		EnemyKills.Add(NewKillData);
	}

	UE_LOG(LogTemp, Log, TEXT("Enemy killed: %s (Total: %d)"),
		*Enemy->GetName(), GetEnemyKillCount(nullptr, NAME_None));
}

int32 UQuestComponent::GetEnemyKillCount(TSubclassOf<AActor> EnemyClass, FName EnemyTag) const
{
	for (const FEnemyKillData& KillData : EnemyKills)
	{
		if (KillData.Matches(EnemyClass, EnemyTag))
		{
			return KillData.KillCount;
		}
	}
	return 0;
}

void UQuestComponent::ClearEnemyKills()
{
	EnemyKills.Empty();
	UE_LOG(LogTemp, Log, TEXT("Enemy kill data cleared"));
}

// ========================================
// OBJECT INTERACTION TRACKING
// ========================================

void UQuestComponent::RecordObjectInteraction(AActor* Object, AActor* Interactor)
{
	if (!Object)
	{
		return;
	}

	// Broadcast the event first (so tasks can respond)
	OnObjectInteracted.Broadcast(Object, Interactor);

	// Determine the object's class and tags
	TSubclassOf<AActor> ObjectClass = Object->GetClass();
	FName ObjectTag = Object->Tags.Num() > 0 ? Object->Tags[0] : NAME_None;

	// Update interaction tracking data
	bool bFoundExisting = false;

	// Track specific actor interactions
	for (FObjectInteractionData& InteractionData : ObjectInteractions)
	{
		if (InteractionData.Matches(nullptr, NAME_None, Object))
		{
			InteractionData.InteractionCount++;
			bFoundExisting = true;
			break;
		}
	}

	if (!bFoundExisting)
	{
		FObjectInteractionData NewData;
		NewData.SpecificActor = Object;
		NewData.InteractionCount = 1;
		ObjectInteractions.Add(NewData);
	}

	// Track by class/tag
	bFoundExisting = false;
	for (FObjectInteractionData& InteractionData : ObjectInteractions)
	{
		if (InteractionData.Matches(ObjectClass, ObjectTag, nullptr))
		{
			InteractionData.InteractionCount++;
			bFoundExisting = true;
			break;
		}
	}

	if (!bFoundExisting)
	{
		FObjectInteractionData NewData;
		NewData.ObjectClass = ObjectClass;
		NewData.ObjectTag = ObjectTag;
		NewData.InteractionCount = 1;
		ObjectInteractions.Add(NewData);
	}

	// Track total interactions (no filters)
	bFoundExisting = false;
	for (FObjectInteractionData& InteractionData : ObjectInteractions)
	{
		if (InteractionData.Matches(nullptr, NAME_None, nullptr) && !InteractionData.SpecificActor.IsValid())
		{
			InteractionData.InteractionCount++;
			bFoundExisting = true;
			break;
		}
	}

	if (!bFoundExisting)
	{
		FObjectInteractionData NewData;
		NewData.InteractionCount = 1;
		ObjectInteractions.Add(NewData);
	}

	UE_LOG(LogTemp, Log, TEXT("Object interacted: %s (Total: %d)"),
		*Object->GetName(), GetObjectInteractionCount(nullptr, NAME_None, nullptr));
}

int32 UQuestComponent::GetObjectInteractionCount(TSubclassOf<AActor> ObjectClass, FName ObjectTag, AActor* SpecificActor) const
{
	for (const FObjectInteractionData& InteractionData : ObjectInteractions)
	{
		if (InteractionData.Matches(ObjectClass, ObjectTag, SpecificActor))
		{
			return InteractionData.InteractionCount;
		}
	}
	return 0;
}

void UQuestComponent::ClearObjectInteractions()
{
	ObjectInteractions.Empty();
	UE_LOG(LogTemp, Log, TEXT("Object interaction data cleared"));
}