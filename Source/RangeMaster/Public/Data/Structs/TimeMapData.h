#pragma once

#include "CoreMinimal.h"
#include "TimeMapData.generated.h"

USTRUCT(BlueprintType)
struct FTimeMapData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Time;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 SpawnerID;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 ShotPower;
};
