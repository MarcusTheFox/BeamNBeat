#pragma once

#include "CoreMinimal.h"
#include "Data/Enums/TargetEventType.h"
#include "TargetEventData.generated.h"

USTRUCT(BlueprintType)
struct FTargetEventData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	ETargetEventType TargetEventType = ETargetEventType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float NormalizedTime = 0;

	FTargetEventData() = default;
	explicit FTargetEventData(const ETargetEventType Type) : TargetEventType(Type) {}

	static FTargetEventData CreateHit(const float HitTime)
	{
		FTargetEventData Data;
		Data.TargetEventType = ETargetEventType::Hit;
		Data.NormalizedTime = HitTime;
		return Data;
	}
	static FTargetEventData CreateLost() { return FTargetEventData(ETargetEventType::Lost); }
};