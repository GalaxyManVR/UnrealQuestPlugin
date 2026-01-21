// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "QuestFactory.generated.h"

/**
 * Factory for creating Quest assets
 */
UCLASS()
class QUESTSYSTEMEDITOR_API UQuestFactory : public UFactory
{
	GENERATED_BODY()

public:
	UQuestFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
	virtual uint32 GetMenuCategories() const override;
	// End of UFactory interface
};
