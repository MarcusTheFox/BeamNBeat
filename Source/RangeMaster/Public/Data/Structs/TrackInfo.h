#pragma once

#include "CoreMinimal.h"
#include "TrackInfo.generated.h"

USTRUCT(BlueprintType)
struct FTrackAudioInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FText Artist;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString FileName;
};

USTRUCT(BlueprintType)
struct FTrackLevelInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FString> Authors;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString Version;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString Difficulty;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString BeatmapFileName;
};


USTRUCT(BlueprintType)
struct FTrackInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName ID;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTrackAudioInfo AudioInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTrackLevelInfo LevelInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Bpm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 TotalTargets = 0;
};