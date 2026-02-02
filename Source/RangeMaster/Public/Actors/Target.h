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

private:
    float Lifetime = 0;
    
public:
    ATarget();

    virtual void OnHit_Implementation() override;

    void Initialize(const float LifetimeValue);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void SetTargetState();
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void SetMissedState();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void DestroyTarget();

    UFUNCTION(BlueprintCallable)
    void SetImpulse(const FVector& Impulse);

    UFUNCTION(BlueprintCallable)
    void Lost();

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Target")
    FOnTargetEvent OnTargetEvent;

    UPROPERTY()
    FTimerHandle TargetStateTimerHandle;
    
    UPROPERTY()
    FTimerHandle MissedStateTimerHandle;
}; 