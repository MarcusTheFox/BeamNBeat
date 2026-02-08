// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Judgement.generated.h"

UENUM(BlueprintType)
enum class EJudgement : uint8
{
	Perfect		UMETA(DisplayName = "Perfect"),
	Great		UMETA(DisplayName = "Great"),
	Good		UMETA(DisplayName = "Good"),
	Early		UMETA(DisplayName = "Early"),
	Late		UMETA(DisplayName = "Late"),
	Miss		UMETA(DisplayName = "Miss"),
	Lost		UMETA(DisplayName = "Lost"),
	Count		UMETA(DisplayName = "DO NOT USE", Hidden)
};

static_assert(static_cast<uint8>(EJudgement::Count) <= 16, "EJudgement enum has grown too large for 4-bit packing!");