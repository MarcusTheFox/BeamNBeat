// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BeamNBeatPlayerState.h"

ABeamNBeatPlayerState::ABeamNBeatPlayerState()
{
	ScoreSystem = CreateDefaultSubobject<UScoreSystemComponent>(TEXT("ScoreSystem"));
	JudgementSystem = CreateDefaultSubobject<UJudgementSystemComponent>(TEXT("JudgementSystem"));
}
