// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "GraphEditor.h"

class UQuest;
class UQuestGraph;
class SGraphEditor;

/**
 * Quest Asset Editor - The main editor for Quest assets
 */
class FQuestAssetEditor : public FAssetEditorToolkit, public FNotifyHook, public FGCObject
{
public:
	FQuestAssetEditor();
	virtual ~FQuestAssetEditor();

	/** Initialize the editor */
	void InitQuestAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UQuest* InQuest);

	// IToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	// End of IToolkit interface

	// FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FQuestAssetEditor"); }
	// End of FGCObject interface

	// FAssetEditorToolkit interface
	virtual void SaveAsset_Execute() override;
	// End of FAssetEditorToolkit interface

protected:
	/** The quest being edited */
	TObjectPtr<UQuest> Quest;

	/** The graph representing the quest */
	TObjectPtr<UQuestGraph> QuestGraph;

	/** Graph editor widget */
	TSharedPtr<SGraphEditor> GraphEditor;

	/** Create the graph editor widget */
	TSharedRef<SGraphEditor> CreateGraphEditorWidget();

	/** Bind commands for the editor */
	void BindCommands();

	/** Register tabs for the editor */
	void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager);
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager);

	/** Spawn the graph editor tab */
	TSharedRef<SDockTab> SpawnTab_GraphEditor(const FSpawnTabArgs& Args);

	/** Spawn the details panel tab */
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	/** Compile the quest from the graph */
	void CompileQuest();

	/** Selection changed in the graph */
	void OnSelectedNodesChanged(const TSet<UObject*>& NewSelection);

	/** Delete selected nodes */
	void DeleteSelectedNodes();
	bool CanDeleteNodes() const;

	/** Copy selected nodes */
	void CopySelectedNodes();
	bool CanCopyNodes() const;

	/** Cut selected nodes */
	void CutSelectedNodes();
	bool CanCutNodes() const;

	/** Paste nodes */
	void PasteNodes();
	bool CanPasteNodes() const;

	/** Duplicate nodes */
	void DuplicateNodes();
	bool CanDuplicateNodes() const;

private:
	/** Details view */
	TSharedPtr<IDetailsView> DetailsView;

	static const FName GraphEditorTabId;
	static const FName DetailsTabId;
};