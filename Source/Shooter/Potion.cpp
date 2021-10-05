// Fill out your copyright notice in the Description page of Project Settings.


#include "Potion.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "ShooterCharacter.h"

APotion::APotion()
    : HealingAmount(20.f)
{
    PotionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PotionMesh"));
    SetRootComponent(PotionMesh);

    GetCollisionBox()->SetupAttachment(GetRootComponent());
    GetAreaSphere()->SetupAttachment(GetRootComponent());

    PotionCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PotionCollisionSphere"));
    PotionCollisionSphere->SetupAttachment(GetRootComponent());
    PotionCollisionSphere->SetSphereRadius(50.f);

}

void APotion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APotion::BeginPlay()
{
    Super::BeginPlay();

    PotionCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APotion::PotionSphrereOverlap);
}

void APotion::PotionSphrereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
    if (Cast<AShooterCharacter>(OtherActor))
    {
        PlayPickupSound(true);
        ShooterCharacter->HealedByPotion(HealingAmount);
        Destroy();
    }
}
