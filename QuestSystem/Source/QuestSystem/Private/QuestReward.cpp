// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "QuestReward.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "QuestRewardComponent.h"

// BASE REWARD Implemtation
void UQuestReward::GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest)
{
	// Base implementation - override in subclasses
	bAlreadyGiven = true;
}

FText UQuestReward::GetRewardDescription_Implementation() const
{
	return FText::FromString(TEXT("Reward"));
}

AActor* UQuestReward::GetCustomRewardTarget_Implementation(UQuestComponent* QuestComponent, UQuest* Quest)
{
	// Override this in Blueprint to provide custom targeting logic
	// Default: fallback to quest owner
	return QuestComponent ? QuestComponent->GetOwner() : nullptr;
}

AActor* UQuestReward::GetRewardTarget(UQuestComponent* QuestComponent, UQuest* Quest)
{
	if (!QuestComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetRewardTarget: QuestComponent is null"));
		return nullptr;
	}

	AActor* Target = nullptr;

	switch (TargetType)
	{
	case ERewardTargetType::QuestOwner:
	{
		Target = QuestComponent->GetOwner();
		break;
	}

	case ERewardTargetType::SpecificActor:
	{
		Target = SpecificTargetActor;
		if (!Target)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetRewardTarget: SpecificTargetActor is not set"));
		}
		break;
	}

	case ERewardTargetType::ActorWithTag:
	{
		if (TargetActorTag.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("GetRewardTarget: TargetActorTag is not set"));
			break;
		}

		// Search for actor with the specified tag
		UWorld* World = QuestComponent->GetWorld();
		if (World)
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* Actor = *It;
				if (Actor && Actor->Tags.Contains(TargetActorTag))
				{
					Target = Actor;
					break;
				}
			}

			if (!Target)
			{
				UE_LOG(LogTemp, Warning, TEXT("GetRewardTarget: No actor found with tag '%s'"),
					*TargetActorTag.ToString());
			}
		}
		break;
	}

	case ERewardTargetType::Custom:
	{
		Target = GetCustomRewardTarget(QuestComponent, Quest);
		if (!Target)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetRewardTarget: Custom target resolution returned null"));
		}
		break;
	}

	default:
	{
		UE_LOG(LogTemp, Warning, TEXT("GetRewardTarget: Unknown target type"));
		Target = QuestComponent->GetOwner(); // Fallback
		break;
	}
	}

	return Target;
}

// ========================================
// EXPERIENCE AND REWARDS

void UExperienceReward::GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest)
{
	if (bAlreadyGiven || !QuestComponent)
	{
		return;
	}

	AActor* Target = GetRewardTarget(QuestComponent, Quest);
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Experience Reward: Could not resolve reward target"));
		return;
	}

	bool bRewardGiven = false;

	// Priority 1: QuestRewardComponent (Auto-detects properties!)
	if (UQuestRewardComponent* RewardComponent = Target->FindComponentByClass<UQuestRewardComponent>())
	{
		// Try common XP property names
		if (RewardComponent->ModifyStatByName("Experience", ExperienceAmount) ||
			RewardComponent->ModifyStatByName("XP", ExperienceAmount) ||
			RewardComponent->ModifyStatByName("Exp", ExperienceAmount))
		{
			bRewardGiven = true;
			UE_LOG(LogTemp, Log, TEXT("Experience Reward: Granted %d XP to %s via QuestRewardComponent"),
				ExperienceAmount, *Target->GetName());
		}
	}
	// Priority 2: Check if target implements IRewardReceiver interface
	else if (Target->Implements<URewardReceiver>())
	{
		IRewardReceiver::Execute_ReceiveExperience(Target, ExperienceAmount);
		bRewardGiven = true;
		UE_LOG(LogTemp, Log, TEXT("Experience Reward: Granted %d XP to %s via interface"),
			ExperienceAmount, *Target->GetName());
	}
	// Priority 3: Look for a stats/character component
	// Uncomment and modify based on your component name if all else ends
	/*
	else if (UStatsComponent* StatsComp = Target->FindComponentByClass<UStatsComponent>())
	{
		StatsComp->AddExperience(ExperienceAmount);
		bRewardGiven = true;
		UE_LOG(LogTemp, Log, TEXT("Experience Reward: Granted %d XP to %s via component"),
			ExperienceAmount, *Target->GetName());
	}
	*/

	if (bRewardGiven)
	{
		// Broadcast event for Blueprint handling
		OnRewardGiven.Broadcast(Target, this, Quest);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Experience Reward: Target %s cannot receive experience. Add QuestRewardComponent or implement IRewardReceiver interface."),
			*Target->GetName());
	}

	bAlreadyGiven = true;
}

FText UExperienceReward::GetRewardDescription_Implementation() const
{
	return FText::FromString(FString::Printf(TEXT("%d XP"), ExperienceAmount));
}


// CURRENCY REWARD Implemtation

void UCurrencyReward::GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest)
{
	if (bAlreadyGiven || !QuestComponent)
	{
		return;
	}

	AActor* Target = GetRewardTarget(QuestComponent, Quest);
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Currency Reward: Could not resolve reward target"));
		return;
	}

	bool bRewardGiven = false;

	// Priority 1: QuestRewardComponent
	if (UQuestRewardComponent* RewardComponent = Target->FindComponentByClass<UQuestRewardComponent>())
	{
		// Try to find the currency property by name
		if (RewardComponent->ModifyStatByName(CurrencyType, CurrencyAmount))
		{
			bRewardGiven = true;
			UE_LOG(LogTemp, Log, TEXT("Currency Reward: Granted %d %s to %s via QuestRewardComponent"),
				CurrencyAmount, *CurrencyType.ToString(), *Target->GetName());
		}
	}
	// Priority 2: Interface
	else if (Target->Implements<URewardReceiver>())
	{
		IRewardReceiver::Execute_ReceiveCurrency(Target, CurrencyType, CurrencyAmount);
		bRewardGiven = true;
		UE_LOG(LogTemp, Log, TEXT("Currency Reward: Granted %d %s to %s via interface"),
			CurrencyAmount, *CurrencyType.ToString(), *Target->GetName());
	}

	if (bRewardGiven)
	{
		OnRewardGiven.Broadcast(Target, this, Quest);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Currency Reward: Target %s cannot receive currency. Add QuestRewardComponent or implement IRewardReceiver interface."),
			*Target->GetName());
	}

	bAlreadyGiven = true;
}

FText UCurrencyReward::GetRewardDescription_Implementation() const
{
	return FText::FromString(FString::Printf(TEXT("%d %s"),
		CurrencyAmount, *CurrencyType.ToString()));
}

// ITEM REWARD Implemtation

void UItemReward::GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest)
{
	if (bAlreadyGiven || !QuestComponent)
	{
		return;
	}

	AActor* Target = GetRewardTarget(QuestComponent, Quest);
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item Reward: Could not resolve reward target"));
		return;
	}

	bool bRewardGiven = false;

	// Priority 1: QuestRewardComponent
	if (UQuestRewardComponent* RewardComponent = Target->FindComponentByClass<UQuestRewardComponent>())
	{
		if (RewardComponent->AddItemToInventory(ItemClass, ItemID, Quantity))
		{
			bRewardGiven = true;
			FString ItemName = ItemClass ? ItemClass->GetName() : ItemID.ToString();
			UE_LOG(LogTemp, Log, TEXT("Item Reward: Granted %d x %s to %s via QuestRewardComponent"),
				Quantity, *ItemName, *Target->GetName());
		}
	}
	// Priority 2: Interface
	else if (Target->Implements<URewardReceiver>())
	{
		IRewardReceiver::Execute_ReceiveItem(Target, ItemClass, ItemID, Quantity);
		bRewardGiven = true;
		FString ItemName = ItemClass ? ItemClass->GetName() : ItemID.ToString();
		UE_LOG(LogTemp, Log, TEXT("Item Reward: Granted %d x %s to %s via interface"),
			Quantity, *ItemName, *Target->GetName());
	}

	if (bRewardGiven)
	{
		OnRewardGiven.Broadcast(Target, this, Quest);
	}
	else
	{
		FString ItemName = ItemClass ? ItemClass->GetName() : ItemID.ToString();
		UE_LOG(LogTemp, Warning, TEXT("Item Reward: Target %s cannot receive items. Add QuestRewardComponent or implement IRewardReceiver interface."),
			*Target->GetName());
	}

	bAlreadyGiven = true;
}

FText UItemReward::GetRewardDescription_Implementation() const
{
	FString ItemName = ItemClass ? ItemClass->GetName() : ItemID.ToString();

	if (Quantity > 1)
	{
		return FText::FromString(FString::Printf(TEXT("%d x %s"), Quantity, *ItemName));
	}
	return FText::FromString(ItemName);
}


// UNLOCK QUEST REWARD Implemtation

void UUnlockQuestReward::GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest)
{
	if (bAlreadyGiven || !QuestComponent || !QuestToUnlock)
	{
		return;
	}

	AActor* Target = GetRewardTarget(QuestComponent, Quest);
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unlock Quest Reward: Could not resolve reward target"));
		return;
	}

	bool bRewardGiven = false;

	// Priority 1: Interface
	if (Target->Implements<URewardReceiver>())
	{
		IRewardReceiver::Execute_ReceiveQuestUnlock(Target, QuestToUnlock, bAutoStartQuest);
		bRewardGiven = true;
		UE_LOG(LogTemp, Log, TEXT("Unlock Quest Reward: Unlocked quest '%s' for %s via interface"),
			*QuestToUnlock->QuestName.ToString(), *Target->GetName());
	}
	// Priority 2: Quest component on target
	else if (UQuestComponent* TargetQuestComponent = Target->FindComponentByClass<UQuestComponent>())
	{
		if (bAutoStartQuest)
		{
			TargetQuestComponent->BeginQuest(QuestToUnlock);
			UE_LOG(LogTemp, Log, TEXT("Unlock Quest Reward: Started quest '%s' for %s"),
				*QuestToUnlock->QuestName.ToString(), *Target->GetName());
		}
		else
		{
			// You might want to add an "UnlockQuest" method to your QuestComponent
			UE_LOG(LogTemp, Log, TEXT("Unlock Quest Reward: Unlocked quest '%s' for %s"),
				*QuestToUnlock->QuestName.ToString(), *Target->GetName());
		}
		bRewardGiven = true;
	}

	if (bRewardGiven)
	{
		OnRewardGiven.Broadcast(Target, this, Quest);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unlock Quest Reward: Target %s has no QuestComponent and doesn't implement IRewardReceiver"),
			*Target->GetName());
	}

	bAlreadyGiven = true;
}

FText UUnlockQuestReward::GetRewardDescription_Implementation() const
{
	if (QuestToUnlock)
	{
		return FText::Format(
			FText::FromString(TEXT("Unlock: {0}")),
			QuestToUnlock->QuestName
		);
	}
	return FText::FromString(TEXT("Unlock Quest"));
}

// BLUEPRINT REWARD Implemtation

void UBlueprintReward::GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest)
{
	if (bAlreadyGiven || !QuestComponent)
	{
		return;
	}

	AActor* Target = GetRewardTarget(QuestComponent, Quest);
	if (Target)
	{
		// Broadcast for Blueprint handling
		OnRewardGiven.Broadcast(Target, this, Quest);
	}

	// This is meant to be overridden in Blueprint for custom logic
	bAlreadyGiven = true;
}

FText UBlueprintReward::GetRewardDescription_Implementation() const
{
	return RewardDescription;
}