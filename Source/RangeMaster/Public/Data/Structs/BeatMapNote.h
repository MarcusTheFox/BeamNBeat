#pragma once

#include "CoreMinimal.h"
#include "NoteProperties.h"
#include "BeatMapNote.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapNote
{
	GENERATED_BODY()

	UPROPERTY()
	float Beat = 0.0f;

	UPROPERTY()
	int32 ID = 0;

	UPROPERTY()
	FNoteProperties Properties;
};