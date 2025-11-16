#pragma once

#include "CoreMinimal.h"
#include "NoteProperties.h"
#include "BeatMapSettings.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapSettings
{
	GENERATED_BODY()
    
	UPROPERTY()
	float Bpm = 0.0f;

	UPROPERTY()
	float Offset = 0.0f;

	UPROPERTY()
	FNoteProperties Properties; 
};