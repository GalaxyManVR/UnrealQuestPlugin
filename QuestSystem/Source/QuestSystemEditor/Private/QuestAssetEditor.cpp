

#include "QuestAssetEditor.h"
#include "Quest.h"
#include "QuestGraph.h"
#include "QuestGraphSchema.h"
#include "QuestGraphNode_State.h"
#include "QuestGraphNode_Branch.h"
#include "QuestState.h"
#include "QuestBranch.h"
#include "GraphEditorActions.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "EdGraphUtilities.h"
#include "Framework/Commands/UIAction.h"
#include "GraphEditorModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Framework/Commands/GenericCommands.h"

#define LOCTEXT_NAMESPACE "QuestAssetEditor"

const FName FQuestAssetEditor::GraphEditorTabId(TEXT("QuestEditor_GraphEditor"));
const FName FQuestAssetEditor::DetailsTabId(TEXT("QuestEditor_Details"));

FQuestAssetEditor::FQuestAssetEditor()
	: Quest(nullptr)
	, QuestGraph(nullptr)
{
}

FQuestAssetEditor::~FQuestAssetEditor()
{
}

void FQuestAssetEditor::InitQuestAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UQuest* InQuest)
{
	Quest = InQuest;

	//Th graph saves in the .uasset vs editor
	if (!Quest->QuestGraph)
	{
		Quest->Modify();

		Quest->QuestGraph = NewObject<UQuestGraph>(
			Quest,
			UQuestGraph::StaticClass(),
			NAME_None,
			RF_Transactional
		);

		Quest->QuestGraph->Schema = UQuestGraphSchema::StaticClass();
	}

	QuestGraph = Cast<UQuestGraph>(Quest->QuestGraph);

	QuestGraph->Schema = UQuestGraphSchema::StaticClass();

	// Initialize the graph (populate from quest data)
	QuestGraph->InitializeGraph(Quest);

	// Create details view
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(Quest);

	// Bind commands
	BindCommands();

	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_QuestEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.7f)
					->AddTab(GraphEditorTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.3f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, FName("QuestEditorApp"), StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, Quest);
}

void FQuestAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_QuestEditor", "Quest Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(GraphEditorTabId, FOnSpawnTab::CreateSP(this, &FQuestAssetEditor::SpawnTab_GraphEditor))
		.SetDisplayName(LOCTEXT("GraphTab", "Quest Graph"))
		.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FQuestAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef);
}

void FQuestAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(GraphEditorTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
}

TSharedRef<SDockTab> FQuestAssetEditor::SpawnTab_GraphEditor(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == GraphEditorTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("GraphEditorTitle", "Quest Graph"))
		[
			CreateGraphEditorWidget()
		];
}

TSharedRef<SDockTab> FQuestAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DetailsTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTitle", "Details"))
		[
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SGraphEditor> FQuestAssetEditor::CreateGraphEditorWidget()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Quest");

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FQuestAssetEditor::OnSelectedNodesChanged);

	GraphEditor = SNew(SGraphEditor)
		.AdditionalCommands(ToolkitCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(QuestGraph)
		.GraphEvents(InEvents)
		.ShowGraphStateOverlay(false);

	return GraphEditor.ToSharedRef();
};

void FQuestAssetEditor::BindCommands()
{
	// Use the existing member variable instead of creating a new one
	ToolkitCommands = MakeShared<FUICommandList>();

	// Bind standard graph editor commands using GenericCommands
	ToolkitCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &FQuestAssetEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FQuestAssetEditor::CanDeleteNodes));
	ToolkitCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateSP(this, &FQuestAssetEditor::CopySelectedNodes),
		FCanExecuteAction::CreateSP(this, &FQuestAssetEditor::CanCopyNodes));
	ToolkitCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateSP(this, &FQuestAssetEditor::CutSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FQuestAssetEditor::CanCutNodes));
	ToolkitCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateSP(this, &FQuestAssetEditor::PasteNodes),
		FCanExecuteAction::CreateSP(this, &FQuestAssetEditor::CanPasteNodes));
	ToolkitCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateSP(this, &FQuestAssetEditor::DuplicateNodes),
		FCanExecuteAction::CreateSP(this, &FQuestAssetEditor::CanDuplicateNodes));
}

void FQuestAssetEditor::OnSelectedNodesChanged(const TSet<UObject*>& NewSelection)
{
	TArray<UObject*> SelectedObjects;

	for (UObject* Object : NewSelection)
	{
		// If it's a graph node, get the underlying quest data instead
		if (UQuestGraphNode_State* StateNode = Cast<UQuestGraphNode_State>(Object))
		{
			if (StateNode->QuestState)
			{
				SelectedObjects.Add(StateNode->QuestState);
			}
		}
		else if (UQuestGraphNode_Branch* BranchNode = Cast<UQuestGraphNode_Branch>(Object))
		{
			if (BranchNode->QuestBranch)
			{
				SelectedObjects.Add(BranchNode->QuestBranch);
			}
		}
		else
		{
			// For other objects, add them directly
			SelectedObjects.Add(Object);
		}
	}

	if (SelectedObjects.Num() > 0)
	{
		DetailsView->SetObjects(SelectedObjects);
	}
	else
	{
		DetailsView->SetObject(Quest);
	}
}

void FQuestAssetEditor::DeleteSelectedNodes()
{
	if (!GraphEditor)
		return;

	const FScopedTransaction Transaction(FText::FromString("Delete Selected Nodes"));
	GraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
	GraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt))
		{
			if (Node->CanUserDeleteNode())
			{
				Node->Modify();
				Node->DestroyNode();
			}
		}
	}
}

bool FQuestAssetEditor::CanDeleteNodes() const
{
	if (!GraphEditor)
		return false;

	const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt))
		{
			if (Node->CanUserDeleteNode())
			{
				return true;
			}
		}
	}
	return false;
}

void FQuestAssetEditor::CopySelectedNodes()
{
	if (GraphEditor)
	{
		const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
		FString ExportedText;

		for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
		{
			if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
			{
				Node->PrepareForCopying();
			}
		}

		FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
		FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
	}
}

bool FQuestAssetEditor::CanCopyNodes() const
{
	if (GraphEditor)
	{
		const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
		return SelectedNodes.Num() > 0;
	}
	return false;
}

void FQuestAssetEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedNodes();
}

bool FQuestAssetEditor::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FQuestAssetEditor::PasteNodes()
{
	if (GraphEditor)
	{
		const FScopedTransaction Transaction(FText::FromString("Paste Nodes"));
		GraphEditor->GetCurrentGraph()->Modify();

		const FVector2D PasteLocation = GraphEditor->GetPasteLocation();

		FString TextToImport;
		FPlatformApplicationMisc::ClipboardPaste(TextToImport);

		TSet<UEdGraphNode*> PastedNodes;
		FEdGraphUtilities::ImportNodesFromText(GraphEditor->GetCurrentGraph(), TextToImport, PastedNodes);

		FVector2D AvgNodePosition(0.0f, 0.0f);
		for (UEdGraphNode* Node : PastedNodes)
		{
			AvgNodePosition.X += Node->NodePosX;
			AvgNodePosition.Y += Node->NodePosY;
		}

		if (PastedNodes.Num() > 0)
		{
			AvgNodePosition.X /= PastedNodes.Num();
			AvgNodePosition.Y /= PastedNodes.Num();
		}

		for (UEdGraphNode* Node : PastedNodes)
		{
			Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + PasteLocation.X;
			Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + PasteLocation.Y;

			Node->SnapToGrid(16);
			Node->CreateNewGuid();
		}

		GraphEditor->ClearSelectionSet();
		for (UEdGraphNode* Node : PastedNodes)
		{
			GraphEditor->SetNodeSelection(Node, true);
		}

		GraphEditor->NotifyGraphChanged();
	}
}

bool FQuestAssetEditor::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
	return !ClipboardContent.IsEmpty();
}

void FQuestAssetEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FQuestAssetEditor::CanDuplicateNodes() const
{
	return CanCopyNodes();
}

FName FQuestAssetEditor::GetToolkitFName() const
{
	return FName("QuestEditor");
}

FText FQuestAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Quest Editor");
}

FString FQuestAssetEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Quest ").ToString();
}

FLinearColor FQuestAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.5f, 0.8f, 0.5f);
}

void FQuestAssetEditor::SaveAsset_Execute()
{
	// Compile the graph to the quest asset before saving
	if (QuestGraph)
	{
		QuestGraph->CompileQuestFromGraph();
	}

	// Call parent to actually save
	FAssetEditorToolkit::SaveAsset_Execute();
}

void FQuestAssetEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Quest);
	Collector.AddReferencedObject(QuestGraph);
}

#undef LOCTEXT_NAMESPACE