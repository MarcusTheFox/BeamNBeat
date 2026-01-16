#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HittableInterface.h"
#include "Target.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetDestroyed, ATarget*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetHit, ATarget*, Target);

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

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Target")
    FOnTargetDestroyed OnTargetDestroyed;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Target")
    FOnTargetHit OnTargetHit;
}; 