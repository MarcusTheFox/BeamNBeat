// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BeamNBeatPlayerState.h"
#include "Settings/BeamNBeatScoreSettings.h"

ABeamNBeatPlayerState::ABeamNBeatPlayerState()
{
	ScoreSystem = CreateDefaultSubobject<UScoreSystemComponent>(TEXT("ScoreSystem"));
	JudgementSystem = CreateDefaultSubobject<UJudgementSystemComponent>(TEXT("JudgementSystem"));
}

void ABeamNBeatPlayerState::RegisterHit() const
{
	JudgementSystem->RegisterJudgement(EJudgement::Perfect);
	ScoreSystem->IncreaseCombo();

	const UBeamNBeatScoreSettings* Settings = UBeamNBeatScoreSettings::Get();
	const int32 Points = Settings->BasePoints;
	const int32 ComboMultiplier = ScoreSystem->GetComboMultiplier();

	ScoreSystem->AddScore(Points * ComboMultiplier);
}

void ABeamNBeatPlayerState::RegisterMiss() const
{
	JudgementSystem->RegisterJudgement(EJudgement::Miss);
	ScoreSystem->ResetCombo();
}

void ABeamNBeatPlayerState::RegisterLost() const
{
	JudgementSystem->RegisterJudgement(EJudgement::Lost);
	ScoreSystem->ResetCombo();
	ScoreSystem->SubtractScore(50);
}

void ABeamNBeatPlayerState::Reset()
{
	Super::Reset();

	JudgementSystem->ClearJudgements();
	ScoreSystem->ResetAllStats();
}
