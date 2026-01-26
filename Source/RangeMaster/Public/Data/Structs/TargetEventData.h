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

	FTargetEventData() = default;
	explicit FTargetEventData(const ETargetEventType Type) : TargetEventType(Type) {}

	static FTargetEventData CreateHit() { return FTargetEventData(ETargetEventType::Hit); }
	static FTargetEventData CreateLost() { return FTargetEventData(ETargetEventType::Lost); }
};