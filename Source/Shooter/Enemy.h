// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BulletHitInterface.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"

#include "Enemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    EET_BabyGrux UMETA(DisplayName = "BabyGrux"),
    EET_GruxWarChief UMETA(DisplayName = "GruxWarChief"),
    EET_GruxHalloween UMETA(DisplayName = "GruxHalloween"),
    EET_GruxChestplate UMETA(DisplayName = "GruxChestplate"),
    EET_GruxBeetleRed UMETA(DisplayName = "GruxBeetleRed"),
    EET_GruxMolten UMETA(DisplayName = "GruxMolten"),
    EET_BabyKhaimera UMETA(DisplayName = "BabyKhaimera"),
    EET_KhaimeraBengal UMETA(DisplayName = "KhaimeraBengal"),
    EET_KhaimeraGruxPelt UMETA(DisplayName = "KhaimeraGruxPelt"),
    EET_KhaimeraWhiteTiger UMETA(DisplayName = "KhaimeraWhiteTiger"),
    EET_KhaimeraHalloween UMETA(DisplayName = "KhaimeraHalloween"),

    EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};

USTRUCT(BlueprintType)
struct FEnemyDataTable : public FTableRowBase
{
    GENERATED_BODY()

    //      scale, AgroSphere, CombatRangeSphere
    //    Walk Speed

    //    stun chance
    //    MaxHealth / Health / Healthbar Size

    //    Attack Waith Time / Base Damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMesh* EnemyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EnemyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UAnimInstance> AnimBP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AgroSphereRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CombatRangeSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RunAnimationPlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StunChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D HealthbarSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackWaithTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimMontage* HitMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimMontage* DeathMontage;
};
UCLASS()
class SHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AEnemy();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION(BlueprintNativeEvent)
    void ShowHealthBar();
    void ShowHealthBar_Implementation();

    UFUNCTION(BlueprintImplementableEvent)
    void HideHealthBar();

    void Die();
    void PlayHitMontage(FName Section, float PlayRate = 1.0f);

    void ResetHitReactTimer();

    UFUNCTION(BlueprintCallable)
    void StoreHitNumber(UUserWidget* HitNumber, FVector Location);
 
    UFUNCTION()
    void DestroyHitNumber(UUserWidget* HitNumber);

    void UpdateHitNumbers();

    // Called when someting overlaps with the agro sphere
    UFUNCTION()
    void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable)
    void SetStunned(bool Stunned);
  
    UFUNCTION()
    void CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable)
    void PlayAttackMontage(FName Section, float PlayRate = 1.f);

    UFUNCTION(BlueprintPure)
    FName GetAttackSectionName();

    UFUNCTION()
    void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Activate / deactivate collision for weapon boxes
    UFUNCTION(BlueprintCallable)
    void ActivateLeftWeapon();
    UFUNCTION(BlueprintCallable)
    void DeactivateLeftWeapon();

    UFUNCTION(BlueprintCallable)
    void ActivateRightWeapon();
    UFUNCTION(BlueprintCallable)
    void DeactivateRightWeapon();

    void DoDamage(class AShooterCharacter* Victim);
    void SpawnBlood(AShooterCharacter* Victim, FName SocketName);
    //Attempt to stun character
    void StunCharacter(AShooterCharacter* Victim);

    void ResetCanAttack();

    UFUNCTION(BlueprintCallable)
    void FinishDeaht();

    void DestroyEnemy();
private:
    // Particle to spawn when hit by bullets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* ImpactParticles;

    // Sound to play when hit by bullets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    class USoundCue* ImpactSound;

    // Current health of the enemy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float Health;

    // Maximum health of the enemy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float MaxHealth;

    // Name of the head bone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FString HeadBone;

    // Time to display health bar once shot 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HealthBarDisplayTime;

    FTimerHandle HealthBarTimer;

    // Montage contaning hit and death animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* HitMontage;

    FTimerHandle HitReactTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HitReactTimeMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HitReactTimeMax;

    bool bCanHitReact;

    // Map to store hit number widget and their hit location
    UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TMap<UUserWidget*, FVector> HitNumbers;

    // Time before a HitNumber is removed from screen
    UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HitNumberDestroyTime;

    // Behavior tree for the AI character  
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
    class UBehaviorTree* BehaviorTree;

    // Point for the enemy to move to
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
    FVector PatrolPoint;

    // Secon Point for the enemy to move to
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
    FVector PatrolPoint2;

    class AEnemyController* EnemyController;

    // Overlap sphere for when the enemy becomes hostile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class USphereComponent* AgroSphere;

    // True when playing the Hit animation
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bStunned;

    // Chance of being stunned. 0: no stun chance, 1: 100% stun chance 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float StunChance;

    // True when in attack range; Time to attack
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bInAttackRange;

    // Sphere for attack range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    USphereComponent* CombatRangeSphere;

    // Montage contaning diffrent attacks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* AttackMontage;

    // The four attack montage section name
    FName AttackLFast;
    FName AttackRFast;
    FName AttackL;
    FName AttackR;

    // Collision Volume for the left weapon
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UBoxComponent* LeftWeaponCollision;

    // Collision Volume for the right weapon
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UBoxComponent* RightWeaponCollision;

    // BaseDamage for enemy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FName LeftWeaponSocket;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FName RightWeaponSocket;

    // True when enemy can attack
    UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bCanAttack;

    FTimerHandle AttackWaitTimer;

    // Minmum waith time between attacks 
    UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float AttackWaitTime;

    // Death anim montage for the enemy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* DeathMontage;

    UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bDying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FTimerHandle DeathTimer;

    // Time after death until destroy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float DeathTime;

    // Target the enemy is attacking
    UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    AShooterCharacter* Target;

    // Type of Enemy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    EEnemyType EnemyType;

    // Animation play rate for enemy run animation
    UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float RunAnimationPlayRate;

    // Health bar widget size
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FVector2D HealthbarSize;


public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    FORCEINLINE FString GetHeadBone() const { return HeadBone; }

    UFUNCTION(BlueprintImplementableEvent)
    void ShowHitNumber(int32 Damage, FVector HitLocation, bool bHeadShot);
    UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
    FORCEINLINE bool GetDying() const { return bDying; }
    FORCEINLINE float GetRunAnimationPlayRate() const { return RunAnimationPlayRate; }
};