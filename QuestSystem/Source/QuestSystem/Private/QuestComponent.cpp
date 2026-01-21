// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "QuestComponent.h"
#include "Quest.h"
#include "QuestBranch.h"
#include "QuestState.h"
#include "QuestTask.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
	SetIsReplicatedByDefault(true);
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->IsQuestInProgress())
		{
			TArray<UQuestBranch*> ActiveBranches = Quest->GetActiveBranches();

			for (UQuestBranch* Branch : ActiveBranches)
			{
				if (Branch && Branch->bIsActive)
				{
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

// SIMPLE SAVE/LOAD INTERFACE

FQuestComponentSaveData UQuestComponent::GetSaveData() const
{
	FQuestComponentSaveData SaveData;

	// Save simple tracking data
	SaveData.CompletedDataTasks = CompletedDataTasks;
	SaveData.EnemyKills = EnemyKills;
	SaveData.ObjectInteractions = ObjectInteractions;

	// Save all quest data
	for (UQuest* Quest : ActiveQuests)
	{
		if (!Quest)
		{
			continue;
		}

		FQuestInstanceSaveData QuestSaveData;
		QuestSaveData.QuestPath = Quest->GetPathName();
		QuestSaveData.bIsInProgress = Quest->IsQuestInProgress();
		QuestSaveData.bHasSucceeded = Quest->HasQuestSucceeded();
		QuestSaveData.bHasFailed = Quest->HasQuestFailed();

		// Save current state index
		if (Quest->CurrentState && Quest->QuestStates.Num() > 0)
		{
			QuestSaveData.CurrentStateIndex = Quest->QuestStates.IndexOfByKey(Quest->CurrentState);
		}

		// Save branch data
		for (UQuestBranch* Branch : Quest->QuestBranches)
		{
			FQuestBranchSaveData BranchData;

			if (Branch)
			{
				BranchData.bIsActive = Branch->bIsActive;
				BranchData.bIsComplete = Branch->bIsComplete;

				// Save task data for this branch
				for (UQuestTask* Task : Branch->QuestTasks)
				{
					if (Task)
					{
						BranchData.TaskProgress.Add(Task->GetCurrentProgress());
						BranchData.TaskActiveStates.Add(Task->IsTaskActive() ? 1 : 0);
						BranchData.TaskCompleteStates.Add(Task->IsComplete() ? 1 : 0);
					}
					else
					{
						BranchData.TaskProgress.Add(0);
						BranchData.TaskActiveStates.Add(0);
						BranchData.TaskCompleteStates.Add(0);
					}
				}
			}

			QuestSaveData.Branches.Add(BranchData);
		}

		SaveData.ActiveQuests.Add(QuestSaveData);
	}

	UE_LOG(LogTemp, Log, TEXT("GetSaveData: Packaged %d quests for saving"), SaveData.ActiveQuests.Num());

	return SaveData;
}

void UQuestComponent::LoadSaveData(const FQuestComponentSaveData& SaveData)
{
	UE_LOG(LogTemp, Log, TEXT("LoadSaveData: Loading %d quests"), SaveData.ActiveQuests.Num());

	// Clear existing data
	ClearAllQuestData();

	// Restore simple tracking data
	CompletedDataTasks = SaveData.CompletedDataTasks;
	EnemyKills = SaveData.EnemyKills;
	ObjectInteractions = SaveData.ObjectInteractions;

	// Restore all quests
	for (const FQuestInstanceSaveData& QuestSaveData : SaveData.ActiveQuests)
	{
		// Load the quest asset
		UQuest* QuestAsset = LoadObject<UQuest>(nullptr, *QuestSaveData.QuestPath);

		if (!QuestAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load quest asset: %s"), *QuestSaveData.QuestPath);
			continue;
		}

		// Create runtime instance
		UQuest* RuntimeQuest = DuplicateObject<UQuest>(QuestAsset, this);
		if (!RuntimeQuest)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create runtime instance for: %s"), *QuestSaveData.QuestPath);
			continue;
		}

		// Initialize the quest
		RuntimeQuest->Initialize(this);

		// Restore quest states
		RuntimeQuest->bIsInProgress = QuestSaveData.bIsInProgress;
		RuntimeQuest->bHasSucceeded = QuestSaveData.bHasSucceeded;
		RuntimeQuest->bHasFailed = QuestSaveData.bHasFailed;

		// Restore current state
		if (QuestSaveData.CurrentStateIndex >= 0 && QuestSaveData.CurrentStateIndex < RuntimeQuest->QuestStates.Num())
		{
			RuntimeQuest->CurrentState = RuntimeQuest->QuestStates[QuestSaveData.CurrentStateIndex];
		}

		// Restore branch states
		for (int32 BranchIdx = 0; BranchIdx < QuestSaveData.Branches.Num() && BranchIdx < RuntimeQuest->QuestBranches.Num(); BranchIdx++)
		{
			const FQuestBranchSaveData& BranchData = QuestSaveData.Branches[BranchIdx];
			UQuestBranch* Branch = RuntimeQuest->QuestBranches[BranchIdx];

			if (Branch)
			{
				Branch->bIsActive = BranchData.bIsActive;
				Branch->bIsComplete = BranchData.bIsComplete;

				// Restore task states
				for (int32 TaskIdx = 0; TaskIdx < BranchData.TaskProgress.Num() && TaskIdx < Branch->QuestTasks.Num(); TaskIdx++)
				{
					UQuestTask* Task = Branch->QuestTasks[TaskIdx];
					if (!Task)
					{
						continue;
					}

					// FIXED: Set progress FIRST before calling BeginTask
					Task->CurrentProgress = BranchData.TaskProgress[TaskIdx];

					// Restore active/complete states
					bool bWasActive = (TaskIdx < BranchData.TaskActiveStates.Num()) ? (BranchData.TaskActiveStates[TaskIdx] != 0) : false;
					bool bWasComplete = (TaskIdx < BranchData.TaskCompleteStates.Num()) ? (BranchData.TaskCompleteStates[TaskIdx] != 0) : false;

					if (bWasActive && !bWasComplete)
					{
						// FIXED: Pass false to BeginTask to prevent progress reinitialization from CompletedDataTasks
						Task->BeginTask(RuntimeQuest->OwningComp, false);
					}
					else if (bWasComplete)
					{
						Task->CurrentProgress = Task->RequiredQuantity;
					}
				}
			}
		}

		// Add to active quests
		ActiveQuests.Add(RuntimeQuest);

		// Bind events
		RuntimeQuest->OnQuestStarted.AddDynamic(this, &UQuestComponent::HandleQuestStarted);
		RuntimeQuest->OnQuestSucceeded.AddDynamic(this, &UQuestComponent::HandleQuestSucceeded);
		RuntimeQuest->OnQuestFailed.AddDynamic(this, &UQuestComponent::HandleQuestFailed);
		RuntimeQuest->OnQuestStateChanged.AddDynamic(this, &UQuestComponent::HandleQuestStateChanged);
	}

	OnQuestListUpdated.Broadcast(this);
	UE_LOG(LogTemp, Log, TEXT("LoadSaveData: Successfully loaded %d quests"), ActiveQuests.Num());
}

void UQuestComponent::ClearAllQuestData()
{
	// End all active quests
	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest)
		{
			Quest->ForgetQuest();
		}
	}

	ActiveQuests.Empty();
	CompletedDataTasks.Empty();
	EnemyKills.Empty();
	ObjectInteractions.Empty();

	UE_LOG(LogTemp, Log, TEXT("All quest data cleared"));
}

// QUEST MANAGEMENT

UQuest* UQuestComponent::BeginQuest(UQuest* QuestAsset)
{
	if (!QuestAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginQuest: No quest asset provided!"));
		return nullptr;
	}

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

	UQuest* NewQuest = DuplicateObject<UQuest>(QuestAsset, this);

	if (!NewQuest)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to duplicate quest asset!"));
		return nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("Runtime instance created: %d states, %d branches"),
		NewQuest->QuestStates.Num(), NewQuest->QuestBranches.Num());

	NewQuest->Initialize(this);
	ActiveQuests.Add(NewQuest);

	NewQuest->OnQuestStarted.AddDynamic(this, &UQuestComponent::HandleQuestStarted);
	NewQuest->OnQuestSucceeded.AddDynamic(this, &UQuestComponent::HandleQuestSucceeded);
	NewQuest->OnQuestFailed.AddDynamic(this, &UQuestComponent::HandleQuestFailed);
	NewQuest->OnQuestStateChanged.AddDynamic(this, &UQuestComponent::HandleQuestStateChanged);

	OnQuestListUpdated.Broadcast(this);
	NewQuest->BeginQuest();

	return NewQuest;
}

UQuest* UQuestComponent::BeginQuestByClass(TSubclassOf<UQuest> QuestClass)
{
	if (!QuestClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginQuestByClass: No quest class provided!"));
		return nullptr;
	}

	UQuest* QuestTemplate = Cast<UQuest>(QuestClass->GetDefaultObject());
	if (!QuestTemplate)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginQuestByClass: Failed to get quest template"));
		return nullptr;
	}

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

bool UQuestComponent::IsQuestActiveByClass(TSubclassOf<UQuest> QuestClass) const
{
	if (!QuestClass)
	{
		return false;
	}

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

	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->GetName() == QuestAsset->GetName())
		{
			return Quest;
		}
	}

	return nullptr;
}

UQuest* UQuestComponent::GetQuestByClass(TSubclassOf<UQuest> QuestClass) const
{
	if (!QuestClass)
	{
		return nullptr;
	}

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

	for (UQuest* Quest : ActiveQuests)
	{
		if (Quest && Quest->IsQuestInProgress())
		{
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

void UQuestComponent::RecordEnemyKill(AActor* Enemy, AActor* Killer)
{
	if (!Enemy)
	{
		return;
	}

	OnEnemyKilled.Broadcast(Enemy, Killer);

	TSubclassOf<AActor> EnemyClass = Enemy->GetClass();
	FName EnemyTag = Enemy->Tags.Num() > 0 ? Enemy->Tags[0] : NAME_None;

	bool bFoundExisting = false;

	for (FEnemyKillData& KillData : EnemyKills)
	{
		if (KillData.Matches(EnemyClass, EnemyTag))
		{
			KillData.KillCount++;
			bFoundExisting = true;
			break;
		}
	}

	if (!bFoundExisting)
	{
		FEnemyKillData NewKillData;
		NewKillData.EnemyClass = EnemyClass;
		NewKillData.EnemyTag = EnemyTag;
		NewKillData.KillCount = 1;
		EnemyKills.Add(NewKillData);
	}

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

void UQuestComponent::RecordObjectInteraction(AActor* Object, AActor* Interactor)
{
	if (!Object)
	{
		return;
	}

	OnObjectInteracted.Broadcast(Object, Interactor);

	TSubclassOf<AActor> ObjectClass = Object->GetClass();
	FName ObjectTag = Object->Tags.Num() > 0 ? Object->Tags[0] : NAME_None;

	bool bFoundExisting = false;

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