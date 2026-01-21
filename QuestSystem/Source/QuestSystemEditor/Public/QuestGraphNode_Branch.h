// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "QuestBranch.h"
#include "QuestGraphNode_Branch.generated.h"

class UQuest;

/**
 * Graph node representing a Quest Branch (contains tasks)
 */
UCLASS()
class QUESTSYSTEMEDITOR_API UQuestGraphNode_Branch : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UQuestGraphNode_Branch();

	/** The quest branch this node represents */
	UPROPERTY()
	TObjectPtr<UQuestBranch> QuestBranch;

	// UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	// End of UEdGraphNode interface

	/** Compile this node to a quest branch */
	void CompileToQuestBranch(UQuest* Quest);

	/** Initialize from an existing quest branch */
	void InitializeFromBranch(UQuestBranch* InBranch);

	/** Find a pin by name */
	UEdGraphPin* FindPin(const FName& PinName) const;

	/** Get the input pin */
	UEdGraphPin* GetInputPin() const;

	/** Get the output pin */
	UEdGraphPin* GetOutputPin() const;
};
