#pragma once

#include "BeatMapNote.h"
#include "BeatMapSettings.h"
#include "BeatMap.generated.h"

USTRUCT(BlueprintType)
struct FBeatMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBeatMapSettings Settings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBeatMapNote> Notes;
};