

#include "QuestSettings.h"

#define LOCTEXT_NAMESPACE "QuestSettings"

UQuestSettings::UQuestSettings()
{
	MaxActiveQuests = 10;
	bAutoSaveQuestProgress = true;
	bShowQuestDebugInfo = false;
	DefaultQuestMarkerColor = FLinearColor::Yellow;
	OptionalObjectiveColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

FName UQuestSettings::GetCategoryName() const
{
	return FName(TEXT("Plugins"));
}

#if WITH_EDITOR
FText UQuestSettings::GetSectionText() const
{
	return LOCTEXT("QuestSettingsSection", "Quest System");
}
#endif

#undef LOCTEXT_NAMESPACE
