#pragma once

#include "CoreMinimal.h"
#include "Data/Structs/TargetEventData.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HittableInterface.h"
#include "Target.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetEvent, ATarget*, Target, const FTargetEventData&, EventData);

UCLASS()
class ATarget : public AActor, public IHittableInterface
{
    GENERATED_BODY()
public:
    ATarget();

    virtual void OnHit_Implementation() override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void DestroyTarget();

    UFUNCTION(BlueprintCallable)
    void SetImpulse(const FVector& Impulse);

    UFUNCTION(BlueprintCallable)
    void Lost();

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Target")
    FOnTargetEvent OnTargetEvent;
}; 