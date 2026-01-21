// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "QuestBlueprintLibrary.h"
#include "QuestComponent.h"
#include "GameFramework/Actor.h"

UQuestComponent* UQuestBlueprintLibrary::GetQuestComponent(AActor* Actor)
{
	if (!Actor) return nullptr;
	return Actor->FindComponentByClass<UQuestComponent>();
}

bool UQuestBlueprintLibrary::TryBeginQuest(AActor* Actor, TSubclassOf<UQuest> QuestClass)
{
	UQuestComponent* Comp = GetQuestComponent(Actor);
	if (Comp && QuestClass)
	{
		return Comp->BeginQuestByClass(QuestClass) != nullptr;
	}
	return false;
}

bool UQuestBlueprintLibrary::HasActiveQuest(AActor* Actor, TSubclassOf<UQuest> QuestClass)
{
	UQuestComponent* Comp = GetQuestComponent(Actor);
	return Comp ? Comp->IsQuestActiveByClass(QuestClass) : false;
}

UQuest* UQuestBlueprintLibrary::GetQuestFromActor(AActor* Actor, TSubclassOf<UQuest> QuestClass)
{
	UQuestComponent* Comp = GetQuestComponent(Actor);
	return Comp ? Comp->GetQuestByClass(QuestClass) : nullptr;
}

void UQuestBlueprintLibrary::CompleteDataTask(AActor* Actor, FName TaskName, const FString& Argument, int32 Quantity)
{
	UQuestComponent* Comp = GetQuestComponent(Actor);
	if (Comp)
	{
		Comp->CompleteDataTask(TaskName, Argument, Quantity);
	}
}

bool UQuestBlueprintLibrary::HasCompletedDataTask(AActor* Actor, FName TaskName, const FString& Argument)
{
	UQuestComponent* Comp = GetQuestComponent(Actor);
	return Comp ? Comp->HasCompletedDataTask(TaskName, Argument) : false;
}

int32 UQuestBlueprintLibrary::GetDataTaskCount(AActor* Actor, FName TaskName, const FString& Argument)
{
	UQuestComponent* Comp = GetQuestComponent(Actor);
	return Comp ? Comp->GetDataTaskCount(TaskName, Argument) : 0;
}

bool UQuestBlueprintLibrary::HasQuestSucceeded(UQuest* Quest)
{
	return Quest ? Quest->HasQuestSucceeded() : false;
}

bool UQuestBlueprintLibrary::HasQuestFailed(UQuest* Quest)
{
	return Quest ? Quest->HasQuestFailed() : false;
}

bool UQuestBlueprintLibrary::IsQuestInProgress(UQuest* Quest)
{
	return Quest ? Quest->IsQuestInProgress() : false;
}

bool UQuestBlueprintLibrary::IsQuestAtState(UQuest* Quest, FName StateID)
{
	return Quest ? Quest->IsQuestAtState(StateID) : false;
}
