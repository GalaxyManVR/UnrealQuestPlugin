// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "QuestRewardComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

UQuestRewardComponent::UQuestRewardComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestRewardComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoDetectCommonStats)
	{
		InitializeAutoDetection();
	}
}

void UQuestRewardComponent::InitializeAutoDetection()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("QuestRewardComponent: Initializing auto-detection for %s"), *Owner->GetName());

	// Clear previous cache
	CachedPropertyOwners.Empty();
	CachedProperties.Empty();

	// Search for common stat names in owner actor
	for (const FName& StatName : CommonStatNames)
	{
		FProperty* Prop = FindPropertyInObject(Owner, StatName);
		if (Prop)
		{
			CachedPropertyOwners.Add(StatName, Owner);
			CachedProperties.Add(StatName, Prop);
			UE_LOG(LogTemp, Log, TEXT("  Found property '%s' in actor"), *StatName.ToString());
		}
	}

	// Search in all components
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		if (!Component)
		{
			continue;
		}

		for (const FName& StatName : CommonStatNames)
		{
			// Skip if we already found this stat
			if (CachedProperties.Contains(StatName))
			{
				continue;
			}

			FProperty* Prop = FindPropertyInObject(Component, StatName);
			if (Prop)
			{
				CachedPropertyOwners.Add(StatName, Component);
				CachedProperties.Add(StatName, Prop);
				UE_LOG(LogTemp, Log, TEXT("  Found property '%s' in component %s"),
					*StatName.ToString(), *Component->GetName());
			}
		}
	}

	// Also check stat configurations
	for (const FStatRewardConfig& Config : StatConfigurations)
	{
		if (CachedProperties.Contains(Config.PropertyName))
		{
			continue; // Already found
		}

		UObject* SearchTarget = Owner;

		// If a specific component class is specified, find it
		if (Config.ComponentClass)
		{
			SearchTarget = Owner->GetComponentByClass(Config.ComponentClass);
			if (!SearchTarget)
			{
				UE_LOG(LogTemp, Warning, TEXT("  Could not find component of class %s"),
					*Config.ComponentClass->GetName());
				continue;
			}
		}
		else if (!Config.bSearchInOwnerActor)
		{
			continue; // Need to specify where to search
		}

		FProperty* Prop = FindPropertyInObject(SearchTarget, Config.PropertyName);
		if (Prop)
		{
			CachedPropertyOwners.Add(Config.PropertyName, SearchTarget);
			CachedProperties.Add(Config.PropertyName, Prop);
			UE_LOG(LogTemp, Log, TEXT("  Found configured property '%s'"),
				*Config.PropertyName.ToString());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("QuestRewardComponent: Detection complete. Found %d properties."),
		CachedProperties.Num());
}

bool UQuestRewardComponent::ModifyStatByName(FName StatName, int32 Amount)
{
	// Check if we have this stat cached
	if (CachedProperties.Contains(StatName) && CachedPropertyOwners.Contains(StatName))
	{
		UObject* Owner = CachedPropertyOwners[StatName];
		FProperty* Prop = CachedProperties[StatName];

		// Apply multiplier if configured
		int32 FinalAmount = Amount;
		for (const FStatRewardConfig& Config : StatConfigurations)
		{
			if (Config.PropertyName == StatName)
			{
				FinalAmount = FMath::RoundToInt(Amount * Config.Multiplier);
				break;
			}
		}

		return ModifyNumericProperty(Owner, Prop, FinalAmount);
	}

	// If not cached, try to find it
	return FindAndModifyNumericProperty(StatName, Amount);
}

bool UQuestRewardComponent::FindAndModifyNumericProperty(FName PropertyName, int32 Amount)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Search in owner actor first
	FProperty* Prop = FindPropertyInObject(Owner, PropertyName);
	if (Prop)
	{
		bool bSuccess = ModifyNumericProperty(Owner, Prop, Amount);
		if (bSuccess)
		{
			// Cache it for next time
			CachedPropertyOwners.Add(PropertyName, Owner);
			CachedProperties.Add(PropertyName, Prop);
		}
		return bSuccess;
	}

	// Search in all components
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		if (!Component)
		{
			continue;
		}

		Prop = FindPropertyInObject(Component, PropertyName);
		if (Prop)
		{
			bool bSuccess = ModifyNumericProperty(Component, Prop, Amount);
			if (bSuccess)
			{
				// Cache it for next time
				CachedPropertyOwners.Add(PropertyName, Component);
				CachedProperties.Add(PropertyName, Prop);
			}
			return bSuccess;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("QuestRewardComponent: Could not find property '%s' in %s or its components"),
		*PropertyName.ToString(), *Owner->GetName());
	return false;
}

FProperty* UQuestRewardComponent::FindPropertyInObject(UObject* Object, FName PropertyName)
{
	if (!Object)
	{
		return nullptr;
	}

	for (TFieldIterator<FProperty> PropIt(Object->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (Property && Property->GetFName() == PropertyName)
		{
			return Property;
		}
	}

	return nullptr;
}

bool UQuestRewardComponent::ModifyNumericProperty(UObject* Object, FProperty* Property, int32 Amount)
{
	if (!Object || !Property)
	{
		return false;
	}

	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);

	// Handle different numeric types
	if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
	{
		int32 CurrentValue = IntProp->GetPropertyValue(ValuePtr);
		int32 NewValue = CurrentValue + Amount;
		IntProp->SetPropertyValue(ValuePtr, NewValue);

		UE_LOG(LogTemp, Log, TEXT("QuestRewardComponent: Modified %s.%s: %d -> %d (+%d)"),
			*Object->GetName(), *Property->GetName(), CurrentValue, NewValue, Amount);
		return true;
	}
	else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
	{
		float CurrentValue = FloatProp->GetPropertyValue(ValuePtr);
		float NewValue = CurrentValue + static_cast<float>(Amount);
		FloatProp->SetPropertyValue(ValuePtr, NewValue);

		UE_LOG(LogTemp, Log, TEXT("QuestRewardComponent: Modified %s.%s: %.2f -> %.2f (+%d)"),
			*Object->GetName(), *Property->GetName(), CurrentValue, NewValue, Amount);
		return true;
	}
	else if (FInt64Property* Int64Prop = CastField<FInt64Property>(Property))
	{
		int64 CurrentValue = Int64Prop->GetPropertyValue(ValuePtr);
		int64 NewValue = CurrentValue + static_cast<int64>(Amount);
		Int64Prop->SetPropertyValue(ValuePtr, NewValue);

		UE_LOG(LogTemp, Log, TEXT("QuestRewardComponent: Modified %s.%s: %lld -> %lld (+%d)"),
			*Object->GetName(), *Property->GetName(), CurrentValue, NewValue, Amount);
		return true;
	}
	else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Property))
	{
		double CurrentValue = DoubleProp->GetPropertyValue(ValuePtr);
		double NewValue = CurrentValue + static_cast<double>(Amount);
		DoubleProp->SetPropertyValue(ValuePtr, NewValue);

		UE_LOG(LogTemp, Log, TEXT("QuestRewardComponent: Modified %s.%s: %.2f -> %.2f (+%d)"),
			*Object->GetName(), *Property->GetName(), CurrentValue, NewValue, Amount);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("QuestRewardComponent: Property %s is not a supported numeric type"),
		*Property->GetName());
	return false;
}

bool UQuestRewardComponent::AddItemToInventory(TSubclassOf<UObject> ItemClass, FName ItemID, int32 Quantity)
{
	UActorComponent* InventoryComp = GetInventoryComponent();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestRewardComponent: No inventory component found"));
		return false;
	}

	// Try to find and call the add item function
	FName FunctionName = InventoryConfig.AddItemFunctionName;
	UFunction* AddItemFunc = InventoryComp->FindFunction(FunctionName);

	if (!AddItemFunc)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestRewardComponent: Inventory component has no function named '%s'"),
			*FunctionName.ToString());
		return false;
	}

	// Prepare function parameters
	struct FAddItemParams
	{
		TSubclassOf<UObject> ItemClass;
		FName ItemID;
		int32 Quantity;
	};

	FAddItemParams Params;
	Params.ItemClass = ItemClass;
	Params.ItemID = ItemID;
	Params.Quantity = Quantity;

	// Call the function
	InventoryComp->ProcessEvent(AddItemFunc, &Params);

	FString ItemName = ItemClass ? ItemClass->GetName() : ItemID.ToString();
	UE_LOG(LogTemp, Log, TEXT("QuestRewardComponent: Added %d x %s to inventory"),
		Quantity, *ItemName);

	return true;
}

UActorComponent* UQuestRewardComponent::GetInventoryComponent()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	if (InventoryConfig.InventoryComponentClass)
	{
		return Owner->GetComponentByClass(InventoryConfig.InventoryComponentClass);
	}

	// Fallback: try to find any component with "Inventory" in the name
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		if (Component && Component->GetName().Contains(TEXT("Inventory")))
		{
			return Component;
		}
	}

	return nullptr;
}