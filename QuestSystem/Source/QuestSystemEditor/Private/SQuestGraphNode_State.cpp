// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "SQuestGraphNode_State.h"
#include "QuestGraphNode_State.h"
#include "QuestState.h"
#include "GraphEditorSettings.h"
#include "SGraphPin.h"
#include "SGraphPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SCommentBubble.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "SQuestGraphNode_State"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SQuestGraphNode_State::Construct(const FArguments& InArgs, UQuestGraphNode_State* InNode)
{
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SQuestGraphNode_State::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed on the node
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	// Build the node widget
	CreateStandardNodeWidget();
}

void SQuestGraphNode_State::CreateStandardNodeWidget()
{
	UQuestGraphNode_State* StateNode = CastChecked<UQuestGraphNode_State>(GraphNode);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			// STRUCTURE: Pins go OUTSIDE the border
			SNew(SHorizontalBox)

				// LEFT side for input pins (outside border)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 10)
				[
					SAssignNew(LeftNodeBox, SVerticalBox)
				]

				// NODE BODY in the middle
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
						.Padding(0)
						.BorderBackgroundColor(this, &SQuestGraphNode_State::GetNodeBodyColor)
						[
							SNew(SBorder)
								.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
								.BorderBackgroundColor(this, &SQuestGraphNode_State::GetNodeTitleColor)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.Padding(6.0f)
								[
									SNew(SHorizontalBox)

										// Icon
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(0, 0, 4, 0)
										[
											SNew(SImage)
												.Image(this, &SQuestGraphNode_State::GetNodeIcon)
										]

										// Title text
										+ SHorizontalBox::Slot()
										.Padding(FMargin(10.0f, 0.0f, 10.0f, 0.0f))
										.VAlign(VAlign_Center)
										.AutoWidth()
										[
											SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													SAssignNew(InlineEditableText, SInlineEditableTextBlock)
														.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
														.Text(this, &SGraphNode::GetEditableNodeTitleAsText)
														.OnVerifyTextChanged(this, &SQuestGraphNode_State::OnVerifyNameTextChanged)
														.OnTextCommitted(this, &SQuestGraphNode_State::OnNameTextCommited)
														.IsReadOnly(this, &SQuestGraphNode_State::IsNameReadOnly)
														.IsSelected(this, &SQuestGraphNode_State::IsSelectedExclusively)
												]
										]
								]
						]
				]

			// RIGHT side for output pins (outside border)
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 10)
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
		];

	// Create pins
	CreatePinWidgets();
}

void SQuestGraphNode_State::CreatePinWidgets()
{
	UQuestGraphNode_State* StateNode = CastChecked<UQuestGraphNode_State>(GraphNode);

	for (UEdGraphPin* Pin : StateNode->Pins)
	{
		if (!Pin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SGraphPin, Pin);
			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SQuestGraphNode_State::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = (PinObj != nullptr) && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
	}

	// Determine which box to add the pin to
	if (PinToAdd->GetDirection() == EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillHeight(1.0f)
			.Padding(5.0f, 2.0f)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
	else // Output pin
	{
		RightNodeBox->AddSlot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.FillHeight(1.0f)
			.Padding(5.0f, 2.0f)
			[
				PinToAdd
			];
		OutputPins.Add(PinToAdd);
	}
}

bool SQuestGraphNode_State::IsNameReadOnly() const
{
	UQuestGraphNode_State* StateNode = CastChecked<UQuestGraphNode_State>(GraphNode);
	return !StateNode->CanRenameNode();
}

bool SQuestGraphNode_State::OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
{
	// Allow any text for now - you can add validation here
	return true;
}

void SQuestGraphNode_State::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	if (UQuestGraphNode_State* StateNode = Cast<UQuestGraphNode_State>(GraphNode))
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Rename Node"));
		StateNode->Modify();
		StateNode->OnRenameNode(InText.ToString());
	}
}

EVisibility SQuestGraphNode_State::GetNodeTitlePlaceholderVisibility() const
{
	return EVisibility::Collapsed;
}

bool SQuestGraphNode_State::IsSelectedExclusively() const
{
	// Simplified - just check if this node is selected
	return GraphNode && GraphNode->IsSelected();
}

FSlateColor SQuestGraphNode_State::GetNodeTitleColor() const
{
	UQuestGraphNode_State* StateNode = CastChecked<UQuestGraphNode_State>(GraphNode);
	if (StateNode && StateNode->QuestState)
	{
		switch (StateNode->QuestState->StateType)
		{
		case EQuestStateType::QuestStart:
			return FLinearColor(0.1f, 0.8f, 0.1f); // Green
		case EQuestStateType::QuestSuccess:
			return FLinearColor(0.1f, 0.4f, 0.9f); // Blue
		case EQuestStateType::QuestFailure:
			return FLinearColor(0.9f, 0.2f, 0.2f); // Red
		default:
			return FLinearColor(0.6f, 0.6f, 0.6f); // Gray
		}
	}
	return FLinearColor::White;
}

FSlateColor SQuestGraphNode_State::GetNodeBodyColor() const
{
	return FLinearColor(0.08f, 0.08f, 0.08f);
}

const FSlateBrush* SQuestGraphNode_State::GetNodeIcon() const
{
	UQuestGraphNode_State* StateNode = CastChecked<UQuestGraphNode_State>(GraphNode);
	if (StateNode && StateNode->QuestState)
	{
		switch (StateNode->QuestState->StateType)
		{
		case EQuestStateType::QuestStart:
			return FAppStyle::GetBrush("Graph.PlayNode.Body");
		case EQuestStateType::QuestSuccess:
			return FAppStyle::GetBrush("Icons.Check");
		case EQuestStateType::QuestFailure:
			return FAppStyle::GetBrush("Icons.X");
		default:
			return FAppStyle::GetBrush("Graph.StateNode.Icon");
		}
	}
	return FAppStyle::GetBrush("Graph.StateNode.Icon");
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE