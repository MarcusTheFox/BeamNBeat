#pragma once

#include "BeatMapNote.h"
#include "BeatMapSettings.h"
#include "BeatMap.generated.h"

USTRUCT(BlueprintType)
struct FBeatMap
{
	GENERATED_BODY()

	UPROPERTY()
	FBeatMapSettings Settings;

	UPROPERTY()
	TArray<FBeatMapNote> Notes;
};