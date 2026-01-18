

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UQuestGraphNode_State;

/**
 * Custom Slate widget for Quest State nodes
 */
class SQuestGraphNode_State : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SQuestGraphNode_State) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UQuestGraphNode_State* InNode);

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual bool IsNameReadOnly() const override;
	// End of SGraphNode interface

protected:
	// Create the node's visual style
	virtual void CreateStandardNodeWidget();

	// Get the node's color based on state type
	FSlateColor GetNodeTitleColor() const;
	FSlateColor GetNodeBodyColor() const;

	// Get icon for the node
	const FSlateBrush* GetNodeIcon() const;

	// Slate callbacks for text editing
	bool OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage);
	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);
	EVisibility GetNodeTitlePlaceholderVisibility() const;
	bool IsSelectedExclusively() const;
};