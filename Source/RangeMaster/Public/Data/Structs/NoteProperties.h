#pragma once

#include "CoreMinimal.h"
#include "NoteProperties.generated.h"

USTRUCT(BlueprintType)
struct FNoteProperties
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Power = 0;

	UPROPERTY()
	int32 Bpm = 0;
};