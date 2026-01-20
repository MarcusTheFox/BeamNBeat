// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetEventType.generated.h"

UENUM(BlueprintType)
enum class ETargetEventType : uint8
{
	None			UMETA(DisplayName = "None"),
	Hit				UMETA(DisplayName = "Hit"),
	Lost			UMETA(DisplayName = "Lost"),
	Destroyed		UMETA(DisplayName = "Destroyed"),
}; 