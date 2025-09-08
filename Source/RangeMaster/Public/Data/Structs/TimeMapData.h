#pragma once

#include "CoreMinimal.h"
#include "TimeMapData.generated.h"

USTRUCT(BlueprintType)
struct FTimeMapData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Time = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 SpawnerID = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 ShotPower = 0;
};
