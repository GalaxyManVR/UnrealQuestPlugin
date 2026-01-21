// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "QuestGraphSchema.generated.h"

class UQuestTask;

/**
 * Schema for the Quest Graph
 */
UCLASS()
class QUESTSYSTEMEDITOR_API UQuestGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	// UEdGraphSchema interface
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotification) const override;
	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;
	virtual void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	// End of UEdGraphSchema interface

private:
	/** Get all available task classes */
	void GetTaskClasses(TArray<TSubclassOf<UQuestTask>>& OutTaskClasses) const;
};

/** Action to add a new quest state node */
USTRUCT()
struct QUESTSYSTEMEDITOR_API FQuestGraphSchemaAction_NewState : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FQuestGraphSchemaAction_NewState()
		: FEdGraphSchemaAction()
	{}

	FQuestGraphSchemaAction_NewState(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

	// FEdGraphSchemaAction interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	// End of FEdGraphSchemaAction interface
};

/** Action to add a new quest task (creates branch automatically) */
USTRUCT()
struct QUESTSYSTEMEDITOR_API FQuestGraphSchemaAction_NewTask : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FQuestGraphSchemaAction_NewTask()
		: FEdGraphSchemaAction()
	{}

	FQuestGraphSchemaAction_NewTask(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

	/** The task class to create */
	TSubclassOf<UQuestTask> TaskClass;

	// FEdGraphSchemaAction interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	// End of FEdGraphSchemaAction interface
};
