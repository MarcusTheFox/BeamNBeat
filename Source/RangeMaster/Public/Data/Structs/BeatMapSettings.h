#pragma once

#include "CoreMinimal.h"
#include "BeatMapSettings.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="BeatMap Settings")
	float Bpm = 0.0f;
    
	UPROPERTY(BlueprintReadWrite, Category="BeatMap Settings")
	int32 Power = 0;
    
	UPROPERTY(BlueprintReadWrite, Category="BeatMap Settings")
	float Offset = 0.0f;
};