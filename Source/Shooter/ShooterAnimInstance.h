// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponType.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
    EOS_Aiming UMETA(DisplayName = "Aiming"),
    EOS_Hip UMETA(DisplayName = "Hip"),
    EOS_Reloading UMETA(DisplayName = "Reloading"),
    EOS_InAir UMETA(DisplayName = "InAir"),

    EOS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class ELocomotionDirection : uint8
{
    ELM_Forward UMETA(DisplayName = "Forward"),
    ELM_Backward UMETA(DisplayName = "Backward"),
    ELM_Regular UMETA(DisplayName = "Regular"),


    ELM_MAX UMETA(DisplayName = "MAX")
};
/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
    UShooterAnimInstance();

    UFUNCTION(BlueprintCallable)
    void UpdateAnimationProperties(float DeltaTime);

    virtual void NativeInitializeAnimation() override;

protected:

    // Handle turning in place variables
    void TurnInPlace();

    // Handle calculations for leaning while running
    void Lean(float DeltaTime);

    // Setting recoil weight
    void SetRecoilWeight();

    // Setting aiming weight
    void SetAimingWeight();

    // Setting anim direction based on yawoffset
    void SetLocomotionAnimDirection();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    class AShooterCharacter* ShooterCharacter;

    // The Speed of the character
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float Speed;

    // Whether or not the charcter is in the air
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    // Whether or not the character is moving
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bIsAccelerating;

    // Offset yaw used for strafing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float MovementOffsetYaw;

    // Offset yaw the frame before we stopped moving
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float LastMovementOffsetYaw;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bAiming;

    // Yaw of the Character this frame; Only updated when standing still and not in air
    float TIPCharacterYaw;

    // Yaw of the Character the previous frame; Only updated when standing still and not in air
    float TIPCharacterYawLastFrame;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    float RootYawOffset;

    // Rotation curve value this frame
    float RotationCurve;

    // Rotation curve value last frame
    float RotationCurveLastFrame;

    // The Picht of the aim rotation, used for Aim offset
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    float Pitch;

    // True when reloading, used to prevent Aim Offset while reloading
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    bool bReloading;

    // Offset state; used to determine which Aim Offset to use
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    EOffsetState OffsetState;


    // Character Yaw this frame
    FRotator CharacterRotation;

    // Character Yaw Last frame
    FRotator CharacterRotationLastFrame;
    
    // Yaw delta used for leaning in the running blendspace
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
    float YawDelta;

    // True when crouching
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
    bool bCrouching;

    // True when Equipping
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
    bool bEquipping;

    // Change the recoil weight based on turning in place and aiming
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float RecoilWeight;

    // True when turning in place
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bTurningInPlace;

    // Weapon Type for the currently equipped weapon
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    EWeaponType EquippedWeaponType;

    // True when not reloading or equipping
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bShouldUseFABRIK;

    // True when jump not just in air
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bIsJump;

    // Change the aiming weight based on standing and crouching
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float AimingWeight;

    // Locomotion anim direction based on yawoffset
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    ELocomotionDirection LocomotionAnimDirection;

    // Last Locomotion anim direction for play FwdToBwd / BwdToFwd animation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    ELocomotionDirection LastLocomotionAnimDirection;

    // True when forward to backward
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bShouldFwdToBwd;

    // True when backward to backward
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bShouldBwdToFwd;
};
