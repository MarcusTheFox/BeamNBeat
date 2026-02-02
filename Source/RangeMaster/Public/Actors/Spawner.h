#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/Target.h"
#include "Components/StaticMeshComponent.h"
#include "Spawner.generated.h"

UCLASS()
class ASpawner : public AActor
{
    GENERATED_BODY()
public:
    ASpawner();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    ATarget* SpawnTarget(TSubclassOf<ATarget> TargetClass, int32 Power, float Lifetime);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
    FVector SpawnOffset = FVector(0, 0, 200);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
    UStaticMeshComponent* OverlapComponent;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                       bool bFromSweep, const FHitResult& SweepResult);
}; 