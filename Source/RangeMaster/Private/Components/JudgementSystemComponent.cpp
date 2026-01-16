#include "Components/JudgementSystemComponent.h"

UJudgementSystemComponent::UJudgementSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UJudgementSystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UJudgementSystemComponent::RegisterJudgement(const EJudgement Judgement)
{
	JudgementEvents.Add(Judgement);
	OnJudgementRegistered.Broadcast(Judgement);
}

const TArray<EJudgement>& UJudgementSystemComponent::GetJudgementEvents() const
{
	return JudgementEvents;
}

int32 UJudgementSystemComponent::GetJudgementCount(const EJudgement Judgement) const
{
	int32 JudgementCount = 0;
	for (const EJudgement& Event : JudgementEvents)
	{
		if (Event == Judgement) JudgementCount++;
	}
	return JudgementCount;
}

void UJudgementSystemComponent::ClearJudgements()
{
	JudgementEvents.Empty();
}
