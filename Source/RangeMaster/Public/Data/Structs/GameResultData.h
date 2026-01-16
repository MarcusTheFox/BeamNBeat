#pragma once

#include "CoreMinimal.h"
#include "Data/Enums/Judgement.h"
#include "Data/Enums/TrackRank.h"
#include "GameResultData.generated.h"

USTRUCT(BlueprintType)
struct FGameResultData
{
    GENERATED_BODY();

    UPROPERTY(BlueprintReadOnly)
    int32 Score = 0;

    UPROPERTY(BlueprintReadOnly)
    ETrackRank Rank = ETrackRank::None;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxCombo = 0;
}; 