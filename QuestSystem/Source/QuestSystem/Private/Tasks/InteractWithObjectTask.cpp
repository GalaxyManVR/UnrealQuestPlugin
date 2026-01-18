#include "Tasks/InteractWithObjectTask.h"
#include "QuestComponent.h"
#include "GameFramework/Actor.h"

UInteractWithObjectTask::UInteractWithObjectTask()
{
	bRetroactive = false;
	bAddWaypoint = true;
	StartingInteractionCount = 0;
	TickInterval = 0.0f; // Event-based, no ticking needed
}

void UInteractWithObjectTask::BeginTask_Implementation()
{
	Super::BeginTask_Implementation();

	if (!OwningComp)
	{
		return;
	}

	// Get current interaction count from the quest component
	int32 CurrentInteractions = OwningComp->GetObjectInteractionCount(ObjectClass, ObjectTag, SpecificActor);

	if (bRetroactive)
	{
		// Count all previous interactions
		StartingInteractionCount = 0;
		int32 Progress = FMath::Min(CurrentInteractions, RequiredQuantity);
		SetProgress(Progress);
	}
	else
	{
		// Only count new interactions from this point
		StartingInteractionCount = CurrentInteractions;
		SetProgress(0);
	}

	// Bind to interaction events
	BindToInteractionEvents();

	// TODO: Add waypoint if bAddWaypoint is true
	// This would integrate with your game's waypoint system
}

void UInteractWithObjectTask::EndTask_Implementation()
{
	Super::EndTask_Implementation();

	// Unbind from events
	UnbindFromInteractionEvents();

	// TODO: Remove waypoint if it was added
}

FText UInteractWithObjectTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	FString ObjectName = TEXT("Objects");
	
	if (SpecificActor)
	{
		ObjectName = SpecificActor->GetName();
	}
	else if (ObjectClass)
	{
		ObjectName = ObjectClass->GetName();
		// Remove common prefixes
		ObjectName.RemoveFromStart(TEXT("BP_"));
		ObjectName.RemoveFromEnd(TEXT("_C"));
	}
	else if (!ObjectTag.IsNone())
	{
		ObjectName = ObjectTag.ToString();
	}

	if (RequiredQuantity > 1)
	{
		return FText::FromString(FString::Printf(
			TEXT("Interact with %d / %d %s"),
			CurrentProgress,
			RequiredQuantity,
			*ObjectName
		));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("Interact with %s"), *ObjectName));
	}
}

FString UInteractWithObjectTask::GetTaskNodeDescription_Implementation() const
{
	FString ObjectName = SpecificActor ? SpecificActor->GetName() :
		(ObjectClass ? ObjectClass->GetName() : 
			(!ObjectTag.IsNone() ? ObjectTag.ToString() : TEXT("Object")));
	
	if (RequiredQuantity > 1)
	{
		return FString::Printf(TEXT("Interact: %d %s"), RequiredQuantity, *ObjectName);
	}
	return FString::Printf(TEXT("Interact: %s"), *ObjectName);
}

void UInteractWithObjectTask::OnObjectInteracted(AActor* InteractedObject, AActor* Interactor)
{
	if (IsComplete() || !InteractedObject)
	{
		return;
	}

	// Check if the interactor is the quest owner
	if (OwningComp && Interactor != OwningComp->GetOwner())
	{
		return;
	}

	// Check if this object matches our filter
	if (!DoesObjectMatchFilter(InteractedObject))
	{
		return;
	}

	// Increment progress
	int32 NewProgress = FMath::Min(CurrentProgress + 1, RequiredQuantity);
	SetProgress(NewProgress);
}

void UInteractWithObjectTask::BindToInteractionEvents()
{
	if (!OwningComp)
	{
		return;
	}

	// Bind to the quest component's object interaction event
	// You'll need to add this delegate to your QuestComponent:
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectInteracted, AActor*, Object, AActor*, Interactor);
	// FOnObjectInteracted OnObjectInteracted;
	
	OwningComp->OnObjectInteracted.AddDynamic(this, &UInteractWithObjectTask::OnObjectInteracted);
}

void UInteractWithObjectTask::UnbindFromInteractionEvents()
{
	if (!OwningComp)
	{
		return;
	}

	OwningComp->OnObjectInteracted.RemoveDynamic(this, &UInteractWithObjectTask::OnObjectInteracted);
}

bool UInteractWithObjectTask::DoesObjectMatchFilter(AActor* Object) const
{
	if (!Object)
	{
		return false;
	}

	// If we have a specific actor, only match that
	if (SpecificActor)
	{
		return Object == SpecificActor;
	}

	// Check class filter
	if (ObjectClass && !Object->IsA(ObjectClass))
	{
		return false;
	}

	// Check tag filter
	if (!ObjectTag.IsNone() && !Object->Tags.Contains(ObjectTag))
	{
		return false;
	}

	return true;
}
