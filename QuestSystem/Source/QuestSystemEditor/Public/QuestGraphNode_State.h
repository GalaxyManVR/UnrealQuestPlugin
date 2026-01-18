

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "QuestState.h"
#include "QuestGraphNode_State.generated.h"

class UQuest;

/**
 * Graph node representing a Quest State
 */
UCLASS()
class QUESTSYSTEMEDITOR_API UQuestGraphNode_State : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UQuestGraphNode_State();

	/** The quest state this node represents */
	UPROPERTY()
	TObjectPtr<UQuestState> QuestState;

	// UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	// UEdGraphNode interface
	virtual bool CanUserDeleteNode() const override;
	virtual bool CanDuplicateNode() const override;
	// UE5 version
	virtual bool CanRenameNode() const;
	virtual void OnRenameNode(const FString& NewName);


	// End of UEdGraphNode interface

	/** Compile this node to a quest state */
	void CompileToQuestState(UQuest* Quest);

	/** Initialize from an existing quest state */
	void InitializeFromState(UQuestState* InState);

	/** Find a pin by name */
	UEdGraphPin* FindPin(const FName& PinName) const;
};