#pragma once

#include "CoreMinimal.h"
#include "BeatMapTypes.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="BeatMap Settings")
	float StartBPM = 120.0f;
    
	UPROPERTY(BlueprintReadWrite, Category="BeatMap Settings")
	int32 DefaultPower = 1500;
    
	UPROPERTY(BlueprintReadWrite, Category="BeatMap Settings")
	float TimeOffset = 0.0f;
};