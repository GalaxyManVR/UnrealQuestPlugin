#include "QuestBlueprintLibrary.h"
#include "NarrativeComponent.h"
#include "GameFramework/Actor.h"

UNarrativeComponent* UQuestBlueprintLibrary::GetNarrativeComponent(AActor* Actor)
{
	if (!Actor) return nullptr;
	return Actor->FindComponentByClass<UNarrativeComponent>();
}

bool UQuestBlueprintLibrary::TryBeginQuest(AActor* Actor, TSubclassOf<UQuest> QuestClass)
{
	UNarrativeComponent* Comp = GetNarrativeComponent(Actor);
	if (Comp && QuestClass)
	{
		return Comp->BeginQuestByClass(QuestClass) != nullptr;
	}
	return false;
}

bool UQuestBlueprintLibrary::HasActiveQuest(AActor* Actor, TSubclassOf<UQuest> QuestClass)
{
	UNarrativeComponent* Comp = GetNarrativeComponent(Actor);
	return Comp ? Comp->IsQuestActiveByClass(QuestClass) : false;
}

UQuest* UQuestBlueprintLibrary::GetQuestFromActor(AActor* Actor, TSubclassOf<UQuest> QuestClass)
{
	UNarrativeComponent* Comp = GetNarrativeComponent(Actor);
	return Comp ? Comp->GetQuestByClass(QuestClass) : nullptr;
}

void UQuestBlueprintLibrary::CompleteDataTask(AActor* Actor, FName TaskName, const FString& Argument, int32 Quantity)
{
	UNarrativeComponent* Comp = GetNarrativeComponent(Actor);
	if (Comp)
	{
		Comp->CompleteNarrativeDataTask(TaskName, Argument, Quantity);
	}
}

bool UQuestBlueprintLibrary::HasCompletedDataTask(AActor* Actor, FName TaskName, const FString& Argument)
{
	UNarrativeComponent* Comp = GetNarrativeComponent(Actor);
	return Comp ? Comp->HasCompletedDataTask(TaskName, Argument) : false;
}

int32 UQuestBlueprintLibrary::GetDataTaskCount(AActor* Actor, FName TaskName, const FString& Argument)
{
	UNarrativeComponent* Comp = GetNarrativeComponent(Actor);
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
