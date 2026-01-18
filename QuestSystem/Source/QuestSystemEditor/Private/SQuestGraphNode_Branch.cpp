

#include "SQuestGraphNode_Branch.h"
#include "QuestGraphNode_Branch.h"
#include "QuestBranch.h"
#include "GraphEditorSettings.h"
#include "SGraphPin.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "SQuestGraphNode_Branch"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SQuestGraphNode_Branch::Construct(const FArguments& InArgs, UQuestGraphNode_Branch* InNode)
{
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SQuestGraphNode_Branch::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			// STRUCTURE: Pins go OUTSIDE the border
			SNew(SHorizontalBox)

				// Input pins on LEFT (outside border)
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
						.BorderImage(FAppStyle::GetBrush("Graph.Node.Body"))
						.Padding(10)
						.BorderBackgroundColor(this, &SQuestGraphNode_Branch::GetBranchColor)
						[
							SNew(SVerticalBox)

								// Icon
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Center)
								.Padding(4.0f)
								[
									SNew(SImage)
										.Image(FAppStyle::GetBrush("Graph.Node.NodeEntryTop"))
										.ColorAndOpacity(FLinearColor(1.0f, 0.7f, 0.2f))
								]

								// Task count
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Center)
								.Padding(5.0f, 2.0f)
								[
									SNew(STextBlock)
										.TextStyle(FAppStyle::Get(), "Graph.Node.NodeTitle")
										.Text(this, &SQuestGraphNode_Branch::GetTaskCountText)
										.ColorAndOpacity(FLinearColor::White)
								]

								// Description
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Center)
								.Padding(4.0f, 2.0f)
								[
									SNew(STextBlock)
										.TextStyle(FAppStyle::Get(), "Graph.Node.NodeTitleExtraLines")
										.Text_Lambda([this]() -> FText
											{
												if (GraphNode)
												{
													return GraphNode->GetNodeTitle(ENodeTitleType::ListView);
												}
												return FText::GetEmpty();
											})
										.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f))
										.WrapTextAt(200.0f)
								]
						]
				]

			// Output pins on RIGHT (outside border)
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 10)
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
		];

	CreatePinWidgets();
}
void SQuestGraphNode_Branch::CreatePinWidgets()
{
	UQuestGraphNode_Branch* BranchNode = CastChecked<UQuestGraphNode_Branch>(GraphNode);

	for (UEdGraphPin* Pin : BranchNode->Pins)
	{
		if (!Pin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SGraphPin, Pin);
			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SQuestGraphNode_Branch::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();

	if (PinToAdd->GetDirection() == EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillHeight(1.0f)
			.Padding(5.0f, 2.0f)  // Horizontal, Vertical padding
			[
				PinToAdd
			],
		InputPins.Add(PinToAdd);
	}
	else // Output
	{
		RightNodeBox->AddSlot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.FillHeight(1.0f)
			.Padding(5.0f, 2.0f)  // Horizontal, Vertical padding
			[
				PinToAdd
			],
		OutputPins.Add(PinToAdd);
	}
}

FSlateColor SQuestGraphNode_Branch::GetBranchColor() const
{
	UQuestGraphNode_Branch* BranchNode = CastChecked<UQuestGraphNode_Branch>(GraphNode);

	if (BranchNode && BranchNode->QuestBranch && BranchNode->QuestBranch->bHidden)
	{
		return FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);
	}

	return FLinearColor(0.12f, 0.09f, 0.05f);
}

FText SQuestGraphNode_Branch::GetTaskCountText() const
{
	UQuestGraphNode_Branch* BranchNode = CastChecked<UQuestGraphNode_Branch>(GraphNode);

	if (BranchNode && BranchNode->QuestBranch)
	{
		int32 TaskCount = BranchNode->QuestBranch->QuestTasks.Num();
		if (TaskCount == 1)
		{
			return LOCTEXT("OneTask", "1 Task");
		}
		return FText::Format(LOCTEXT("TaskCount", "{0} Tasks"), FText::AsNumber(TaskCount));
	}

	return LOCTEXT("NoTasks", "0 Tasks");
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE