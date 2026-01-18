#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "QuestGraph.generated.h"

class UQuest;

UCLASS()
class QUESTSYSTEMEDITOR_API UQuestGraph : public UEdGraph  // Add the API macro here!
{
	GENERATED_BODY()

public:
	/** The quest asset this graph represents */
	UPROPERTY()
	TObjectPtr<UQuest> Quest;

	/** Initialize the graph from a quest asset */
	void InitializeGraph(UQuest* InQuest);

	/** Compile the graph back into the quest asset */
	void CompileQuestFromGraph();

	void CompileQuestGraph();
};

