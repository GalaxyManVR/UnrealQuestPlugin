#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Quest.h"
#include "QuestBlueprintLibrary.generated.h"

class UNarrativeComponent;
class AActor;

/**
 * Blueprint helper functions for working with quests
 */
UCLASS()
class NARRATIVEQUESTSYSTEM_API UQuestBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Get the NarrativeComponent from an actor */
	UFUNCTION(BlueprintPure, Category = "Quests")
	static UNarrativeComponent* GetNarrativeComponent(AActor* Actor);

	/** Try to begin a quest on an actor */
	UFUNCTION(BlueprintCallable, Category = "Quests")
	static bool TryBeginQuest(AActor* Actor, TSubclassOf<UQuest> QuestClass);

	/** Check if an actor has an active quest */
	UFUNCTION(BlueprintPure, Category = "Quests")
	static bool HasActiveQuest(AActor* Actor, TSubclassOf<UQuest> QuestClass);

	/** Get a quest from an actor */
	UFUNCTION(BlueprintPure, Category = "Quests")
	static UQuest* GetQuestFromActor(AActor* Actor, TSubclassOf<UQuest> QuestClass);

	/** Complete a data task for the actor */
	UFUNCTION(BlueprintCallable, Category = "Quests")
	static void CompleteDataTask(AActor* Actor, FName TaskName, const FString& Argument, int32 Quantity = 1);

	/** Check if a data task has been completed */
	UFUNCTION(BlueprintPure, Category = "Quests")
	static bool HasCompletedDataTask(AActor* Actor, FName TaskName, const FString& Argument);

	/** Get count of completed data tasks */
	UFUNCTION(BlueprintPure, Category = "Quests")
	static int32 GetDataTaskCount(AActor* Actor, FName TaskName, const FString& Argument);

	/** Check quest status */
	UFUNCTION(BlueprintPure, Category = "Quests")
	static bool HasQuestSucceeded(UQuest* Quest);

	UFUNCTION(BlueprintPure, Category = "Quests")
	static bool HasQuestFailed(UQuest* Quest);

	UFUNCTION(BlueprintPure, Category = "Quests")
	static bool IsQuestInProgress(UQuest* Quest);

	UFUNCTION(BlueprintPure, Category = "Quests")
	static bool IsQuestAtState(UQuest* Quest, FName StateID);
};
