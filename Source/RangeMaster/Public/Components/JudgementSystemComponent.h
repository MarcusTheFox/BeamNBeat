#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Enums/Judgement.h"
#include "JudgementSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJudgementRegistered, const EJudgement&, Judgement);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UJudgementSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UJudgementSystemComponent();

protected:
	virtual void BeginPlay() override;

private:
	TArray<EJudgement> JudgementEvents;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnJudgementRegistered OnJudgementRegistered;
	
	UFUNCTION(BlueprintCallable)
	int32 GetJudgementCount(const EJudgement Judgement) const;

	UFUNCTION(BlueprintCallable)
	void RegisterJudgement(const EJudgement Judgement);
	
	const TArray<EJudgement>& GetJudgementEvents() const;
	void ClearJudgements();
};
