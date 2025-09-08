#pragma once

#include "CoreMinimal.h"
#include "BeatMapDefaults.h"
#include "BeatMapSettings.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapSettings : public FBeatMapDefaults
{
	GENERATED_BODY()
    
	UPROPERTY()
	float Offset = 0.0f;
};