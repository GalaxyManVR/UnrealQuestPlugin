// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "Tasks/DefeatEnemiesTask.h"
#include "QuestComponent.h"
#include "GameFramework/Actor.h"

UDefeatEnemiesTask::UDefeatEnemiesTask()
{
	bRetroactive = false;
	StartingKillCount = 0;
	TickInterval = 0.0f; // Event-based, no ticking needed
}

void UDefeatEnemiesTask::BeginTask_Implementation()
{
	Super::BeginTask_Implementation();

	if (!OwningComp)
	{
		return;
	}

	// Get current kill count from the quest component
	int32 CurrentKills = OwningComp->GetEnemyKillCount(EnemyClass, EnemyTag);

	if (bRetroactive)
	{
		// Count all previous kills
		StartingKillCount = 0;
		int32 Progress = FMath::Min(CurrentKills, RequiredQuantity);
		SetProgress(Progress);
	}
	else
	{
		// Only count new kills from this point
		StartingKillCount = CurrentKills;
		SetProgress(0);
	}

	// Bind to enemy death events
	BindToEnemyEvents();
}

void UDefeatEnemiesTask::EndTask_Implementation()
{
	Super::EndTask_Implementation();

	// Unbind from events
	UnbindFromEnemyEvents();
}

FText UDefeatEnemiesTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	FString EnemyName = TEXT("Enemies");
	
	if (EnemyClass)
	{
		EnemyName = EnemyClass->GetName();
		// Remove common prefixes
		EnemyName.RemoveFromStart(TEXT("BP_"));
		EnemyName.RemoveFromEnd(TEXT("_C"));
	}
	else if (!EnemyTag.IsNone())
	{
		EnemyName = EnemyTag.ToString();
	}

	return FText::FromString(FString::Printf(
		TEXT("Defeat %d / %d %s"),
		CurrentProgress,
		RequiredQuantity,
		*EnemyName
	));
}

FString UDefeatEnemiesTask::GetTaskNodeDescription_Implementation() const
{
	FString EnemyName = EnemyClass ? EnemyClass->GetName() : 
		(!EnemyTag.IsNone() ? EnemyTag.ToString() : TEXT("Enemies"));
	
	return FString::Printf(TEXT("Defeat %d %s"), RequiredQuantity, *EnemyName);
}

void UDefeatEnemiesTask::OnEnemyKilled(AActor* KilledEnemy, AActor* Killer)
{
	if (IsComplete() || !KilledEnemy)
	{
		return;
	}

	// Check if this enemy matches our filter
	if (!DoesEnemyMatchFilter(KilledEnemy))
	{
		return;
	}

	// Increment progress
	int32 NewProgress = FMath::Min(CurrentProgress + 1, RequiredQuantity);
	SetProgress(NewProgress);
}

void UDefeatEnemiesTask::BindToEnemyEvents()
{
	if (!OwningComp)
	{
		return;
	}

	
	OwningComp->OnEnemyKilled.AddDynamic(this, &UDefeatEnemiesTask::OnEnemyKilled);
}

void UDefeatEnemiesTask::UnbindFromEnemyEvents()
{
	if (!OwningComp)
	{
		return;
	}

	OwningComp->OnEnemyKilled.RemoveDynamic(this, &UDefeatEnemiesTask::OnEnemyKilled);
}

bool UDefeatEnemiesTask::DoesEnemyMatchFilter(AActor* Enemy) const
{
	if (!Enemy)
	{
		return false;
	}

	// Check class filter
	if (EnemyClass && !Enemy->IsA(EnemyClass))
	{
		return false;
	}

	// Check tag filter
	if (!EnemyTag.IsNone() && !Enemy->Tags.Contains(EnemyTag))
	{
		return false;
	}

	return true;
}
