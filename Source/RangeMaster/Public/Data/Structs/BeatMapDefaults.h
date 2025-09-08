#pragma once

#include "CoreMinimal.h"
#include "BeatMapDefaults.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapDefaults
{
	GENERATED_BODY()

	UPROPERTY()
	float Bpm = 0.0f;
    
	UPROPERTY()
	int32 Power = 0;
};