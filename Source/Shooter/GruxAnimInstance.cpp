// Fill out your copyright notice in the Description page of Project Settings.


#include "GruxAnimInstance.h"
#include "Enemy.h"

UGruxAnimInstance::UGruxAnimInstance()
    : RunAnimationPlayRate(1.f)
{

}
void UGruxAnimInstance::NativeBeginPlay()
{
    if (Enemy == nullptr)
    {
        Enemy = Cast<AEnemy>(TryGetPawnOwner());
    }
    if (Enemy)
    {
        RunAnimationPlayRate = Enemy->GetRunAnimationPlayRate();
    }
}

void UGruxAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if (Enemy == nullptr)
    {
        Enemy = Cast<AEnemy>(TryGetPawnOwner());
    }
    if (Enemy)
    {
        FVector Velocity{ Enemy->GetVelocity() };
        Velocity.Z = 0.f;
        Speed = Velocity.Size();
    }
}


