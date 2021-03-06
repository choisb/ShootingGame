// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "Weapon.h"
#include "WeaponType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
UShooterAnimInstance::UShooterAnimInstance()
    : Speed(0.f)
    , bIsInAir(false)
    , bIsAccelerating(false)
    , MovementOffsetYaw(0.f)
    , LastMovementOffsetYaw(0.f)
    , bAiming(false)
    , TIPCharacterYaw(0.f)
    , TIPCharacterYawLastFrame(0.f)
    , RootYawOffset(0.f)
    , Pitch(0.f)
    , bReloading(false)
    , OffsetState(EOffsetState::EOS_Hip)
    , CharacterRotation(FRotator(0.f))
    , CharacterRotationLastFrame(FRotator(0.f))
    , bCrouching(false)
    , bEquipping(false)
    , RecoilWeight(1.0f)
    , bTurningInPlace(false)
    , EquippedWeaponType(EWeaponType::EWT_MAX)
    , bShouldUseFABRIK(false)
    , AimingWeight(0.75f)
    , bShouldFwdToBwd(false)
    , bShouldBwdToFwd(false)
{

}
void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if (ShooterCharacter == nullptr)
    {
        ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
    }

    if (ShooterCharacter)
    {
        bIsJump = ShooterCharacter->GetIsJump();
        bCrouching = ShooterCharacter->GetCrouching();
        bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
        bEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
        bShouldUseFABRIK = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied || ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress;

        // Get the lateral speed of the character from velocity
        FVector Velocity{ ShooterCharacter->GetVelocity() };
        Velocity.Z = 0;
        Speed = Velocity.Size();

        // Is the Character in  the air?
        bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

        // Is the character accelerating? 실제 물리학과 다르게 움직이고 있다면 true, 멈춰있다면 false 반환
        if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
        {
            bIsAccelerating = true;
        }
        else
        {
            bIsAccelerating = false;
        }

        FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

        MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

        if (ShooterCharacter->GetVelocity().Size() > 0.f)
        {
            LastMovementOffsetYaw = MovementOffsetYaw;
        }

        bAiming = ShooterCharacter->GetAiming();

        if (bReloading)
        {
            OffsetState = EOffsetState::EOS_Reloading;
        }
        else if (bIsInAir)
        {
            OffsetState = EOffsetState::EOS_InAir;
        }
        else if (ShooterCharacter->GetAiming())
        {
            OffsetState = EOffsetState::EOS_Aiming;
        }
        else
        {
            OffsetState = EOffsetState::EOS_Hip;
        }

        // Check if shootercharacter is valid Equipped Weapon
        if (ShooterCharacter->GetEquippedWeapon())
        {
            EquippedWeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
        }
    } 

    TurnInPlace();
    SetRecoilWeight();
    SetAimingWeight();
    Lean(DeltaTime);
    SetLocomotionAnimDirection();
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());

}

void UShooterAnimInstance::TurnInPlace()
{
    if (ShooterCharacter == nullptr) return;
    Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;



    if (Speed > 0 || bIsInAir)
    {
        // Don't want to turn in place; Character is moving
        RootYawOffset = 0.f;
        TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        RotationCurveLastFrame = 0.f;
        RotationCurve = 0.f;

    }
    else
    {
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };

        // Root Yaw Offset, updated and clamped to [-180, 180]
        RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);
        //RootYawOffset -= TIPYawDelta;
        // 1.0 if turning, 0.0 if not
        const float Turning{ GetCurveValue(TEXT("Turning")) };
        if (Turning > 0)
        {
            bTurningInPlace = true;
            RotationCurveLastFrame = RotationCurve;
            RotationCurve = GetCurveValue(TEXT("Rotation"));
            const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

            // RootYawOffset > 0, -> Turning Left, RootYawOffset < 0, -> Turning Right
            RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

            const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
            if (ABSRootYawOffset > 90.f)
            {
                const float YawExcess{ ABSRootYawOffset - 90.f };
                RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
            }
            //UE_LOG(LogTemp, Warning, TEXT("RotationCurve: %f | DeltaRotation: %f | RootYawOffset: %f"), RotationCurve, DeltaRotation, RootYawOffset);
            //if (GEngine)
            //    GEngine->AddOnScreenDebugMessage(3, -1, FColor::Green, FString::Printf(TEXT("RotationCurveLastFrame: %f | RotationCurve: %f"), RotationCurveLastFrame, RotationCurve));
        }
        else
        {
            bTurningInPlace = false;
        }  
    }
}
void UShooterAnimInstance::SetRecoilWeight()
{
    // set the Recoil Weight
    if (bTurningInPlace)
    {
        RecoilWeight = 0.f;
    }
    else // not turning in place
    {
        if (bCrouching)
        {
            if (bAiming)
            {
                RecoilWeight = 0.1f;
            }
            else 
            {
                RecoilWeight = 0.2f;
            }
        }
        else // standing
        {
            if (bAiming)
            {
                RecoilWeight = 0.3f;
            }
            else
            {
                RecoilWeight = 0.7f;
            }
        }
    }

}

void UShooterAnimInstance::SetAimingWeight()
{
    if (bCrouching)
    {
        AimingWeight = 0.5f;
    }
    else // standing
    {
        AimingWeight = 0.75f;
    }
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
    if (ShooterCharacter == nullptr) return;

    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = ShooterCharacter->GetActorRotation();

    const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };
    const float Target{ Delta.Yaw / DeltaTime };
    const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
    YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

}

void UShooterAnimInstance::SetLocomotionAnimDirection()
{
    LastLocomotionAnimDirection = LocomotionAnimDirection;
    bShouldFwdToBwd = false;
    bShouldBwdToFwd = false;
    if (abs(MovementOffsetYaw) < 60 && !bCrouching && bIsAccelerating)
    {
        LocomotionAnimDirection = ELocomotionDirection::ELM_Forward;
    }
    else if (abs(MovementOffsetYaw) > 120 && !bCrouching && bIsAccelerating)
    {
        LocomotionAnimDirection = ELocomotionDirection::ELM_Backward;
    }
    else
    {
        LocomotionAnimDirection = ELocomotionDirection::ELM_Regular;
    }
    if (LastLocomotionAnimDirection == ELocomotionDirection::ELM_Forward && LocomotionAnimDirection == ELocomotionDirection::ELM_Backward)
    {
        bShouldFwdToBwd = true;
        LocomotionAnimDirection = ELocomotionDirection::ELM_Regular;
    }
    else if (LastLocomotionAnimDirection == ELocomotionDirection::ELM_Backward && LocomotionAnimDirection == ELocomotionDirection::ELM_Forward)
    {
        bShouldBwdToFwd = true;
        LocomotionAnimDirection = ELocomotionDirection::ELM_Regular;
    }
}

