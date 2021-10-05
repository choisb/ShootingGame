// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Potion.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API APotion : public AItem
{
	GENERATED_BODY()
public:

    APotion();

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

	UFUNCTION()
	void PotionSphrereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    // Mesh for the ammo pickup
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PotionMesh;

    // Overlap sphere for picking up the Potion
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Potion, meta = (AllowPrivateAccess = "true"))
    class USphereComponent* PotionCollisionSphere;

    // the effective amount of the health potion
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Potion, meta = (AllowPrivateAccess = "true"))
    float HealingAmount;
};
