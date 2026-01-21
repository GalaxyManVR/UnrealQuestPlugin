// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FQuestSystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
