// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemy::AEnemy()
    : Health(100.f)
    , MaxHealth(100.f)
    , HeadBone(TEXT("head"))
    , HealthBarDisplayTime(4.f)
    , HitReactTimeMin(0.4f)
    , HitReactTimeMax(0.8f)
    , bCanHitReact(true)
    , HitNumberDestroyTime(1.5f)
    , bStunned(false)
    , StunChance(0.5f)
    , AttackLFast(TEXT("AttackLFast"))
    , AttackRFast(TEXT("AttackRFast"))
    , AttackL(TEXT("AttackL"))
    , AttackR(TEXT("AttackR"))
    , BaseDamage(20.f)
    , LeftWeaponSocket(TEXT("FX_Trail_L_02"))
    , RightWeaponSocket(TEXT("FX_Trail_R_02"))
    , bCanAttack(true)
    , AttackWaitTime(1.f)
    , bDying(false)
    , DeathTime(4.f)
    , EnemyType(EEnemyType::EET_BabyGrux)
    , RunAnimationPlayRate(1.0f)
    , HealthbarSize(FVector2D(125.f, 25.f))
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Create AgroSphere
    AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
    AgroSphere->SetupAttachment(GetRootComponent());

    // Create Combat Range Sphere
    CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
    CombatRangeSphere->SetupAttachment(GetRootComponent());

    // Construct left and right weapon collision box
    LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Weapon Box"));
    LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));

    RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Box"));
    RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));


}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
    Super::BeginPlay();

    // Get the AI controller
    EnemyController = Cast<AEnemyController>(GetController());
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("CanAttack")), true);
    }

    AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
    CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
    CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeEndOverlap);

    // Bind function overlap event for weapon boxes
    LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
    RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Set Collsion presets for weapon boxes
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);


    const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
    const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
    //DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);
    //DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Red, true);

    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
        EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
        EnemyController->RunBehaviorTree(BehaviorTree);
    }

}
void AEnemy::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    const FString EnemyTablePath{ TEXT("DataTable'/Game/_Game/DataTable/EnemyDataTable.EnemyDataTable'") };
    UDataTable* EnemyDataTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *EnemyTablePath));

    if (EnemyDataTableObject)
    {
        FEnemyDataTable* EnemyDataRow = nullptr;

        switch (EnemyType)
        {
        case EEnemyType::EET_BabyGrux:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("BabyGrux"), TEXT(""));
            break;
        case EEnemyType::EET_GruxWarChief:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("GruxWarChief"), TEXT(""));
            break;
        case EEnemyType::EET_GruxHalloween:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("GruxHalloween"), TEXT(""));
            break;
        case EEnemyType::EET_GruxChestplate:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("GruxChestplate"), TEXT(""));
            break;
        case EEnemyType::EET_GruxBeetleRed:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("GruxBeetleRed"), TEXT(""));
            break;
        case EEnemyType::EET_GruxMolten:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("GruxMolten"), TEXT(""));
            break;

        case EEnemyType::EET_BabyKhaimera:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("BabyKhaimera"), TEXT(""));
            break;

        case EEnemyType::EET_KhaimeraBengal:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("KhaimeraBengal"), TEXT(""));
            break;

        case EEnemyType::EET_KhaimeraGruxPelt:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("KhaimeraGruxPelt"), TEXT(""));
            break;

        case EEnemyType::EET_KhaimeraWhiteTiger:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("KhaimeraWhiteTiger"), TEXT(""));
            break;

        case EEnemyType::EET_KhaimeraHalloween:
            EnemyDataRow = EnemyDataTableObject->FindRow<FEnemyDataTable>(FName("KhaimeraHalloween"), TEXT(""));
            break;

        }

        if (EnemyDataRow)
        {
            GetMesh()->SetSkeletalMesh(EnemyDataRow->EnemyMesh);
            GetMesh()->SetAnimInstanceClass(EnemyDataRow->AnimBP);
            SetActorScale3D(FVector(EnemyDataRow->Scale));
            AgroSphere->SetSphereRadius(EnemyDataRow->AgroSphereRadius);
            CombatRangeSphere->SetSphereRadius(EnemyDataRow->CombatRangeSphere);
            GetCharacterMovement()->MaxWalkSpeed = EnemyDataRow->WalkSpeed;
            RunAnimationPlayRate = EnemyDataRow->RunAnimationPlayRate;
            StunChance = EnemyDataRow->StunChance;
            MaxHealth = EnemyDataRow->MaxHealth;
            Health = EnemyDataRow->Health;
            HealthbarSize = EnemyDataRow->HealthbarSize;
            AttackWaitTime = EnemyDataRow->AttackWaithTime;
            BaseDamage = EnemyDataRow->BaseDamage;
            
            HitMontage = EnemyDataRow->HitMontage;         
            AttackMontage = EnemyDataRow->AttackMontage;           
            DeathMontage = EnemyDataRow->DeathMontage;
        }
    }
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

}
void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }
    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
    }

    return;
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
    if (bDying) return 0.f;

    // Set the target blackboard key to agro
    if (EnemyController)
    {
        AShooterCharacter* Character = Cast<AShooterCharacter>(DamageCauser);
        if (Character)
        {
            EnemyController->GetBlackboardComponent()->SetValueAsObject(FName(TEXT("Target")), Character);
            Target = Character;
        }
    }
    if (Health - DamageAmount <= 0.f)
    {
        float retVal = Health;
        Health = 0.f;
        Die();
        return retVal;        
    }
    else
    {
        Health -= DamageAmount;
        ShowHealthBar();
        // Determine whether bullet hit stuns
        const float Stunned = FMath::FRandRange(0.f, 1.f);
        if (Stunned <= StunChance)
        {
            // Stun the enemy
            PlayHitMontage(FName("HitReactFront"));
            SetStunned(true);
        }
    }


    return DamageAmount;
}

void AEnemy::ShowHealthBar_Implementation()
{
    GetWorldTimerManager().ClearTimer(HealthBarTimer);
    GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
    bDying = true;
    Target->RemoveAttackingEnemy(this);
    HideHealthBar();
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && DeathMontage)
    {
        AnimInstance->Montage_Play(DeathMontage);
    }

    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("Dead")), true);
        EnemyController->StopMovement();
    }
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AEnemy::PlayHitMontage(FName Section, float PlayRate /*1.0f*/)
{
    if (bCanHitReact)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(HitMontage, PlayRate);
            AnimInstance->Montage_JumpToSection(Section, HitMontage);
        }
        bCanHitReact = false;
        const float HitReactTime{ FMath::FRandRange(HitReactTimeMin, HitReactTimeMax) };
        GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);
    }

}

void AEnemy::ResetHitReactTimer()
{
    bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
    HitNumbers.Add(HitNumber, Location);

    FTimerHandle HitNumberTimer;
    FTimerDelegate HitNumberDelegate;
    HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
    GetWorld()->GetTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
    HitNumbers.Remove(HitNumber);
    HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
    for (auto& HitPair : HitNumbers)
    {
        UUserWidget* HitNumber{ HitPair.Key };
        const FVector Location{ HitPair.Value };

        FVector2D ScreenPosition;
        UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Location, ScreenPosition);
        HitNumber->SetPositionInViewport(ScreenPosition);
    }
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor == nullptr) return;
    auto Character = Cast<AShooterCharacter>(OtherActor);
    if (Character)
    {
        if (EnemyController)
        {
            if (EnemyController->GetBlackboardComponent())
            {
                //Set the value of the Target Blackboard key
                EnemyController->GetBlackboardComponent()->SetValueAsObject(FName(TEXT("Target")), Character);
                Target = Character;
            }
        }

        Character->AddAttackingEnemy(this);
    }
}

void AEnemy::SetStunned(bool Stunned)
{
    bStunned = Stunned;

    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
    }
}

void AEnemy::CombatRangeOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor == nullptr) return;
    auto Character = Cast<AShooterCharacter>(OtherActor);
    if (Character)
    {
        bInAttackRange = true;
        if (EnemyController)
        {
            EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
        }
    }
}

void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{

    if (OtherActor == nullptr) return;
    auto Character = Cast<AShooterCharacter>(OtherActor);
    if (Character)
    {
        bInAttackRange = false;
        if (EnemyController)
        {
            EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
        }
    }
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate /*1.f*/)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AttackMontage)
    {
        AnimInstance->Montage_Play(AttackMontage, PlayRate);
        AnimInstance->Montage_JumpToSection(Section, AttackMontage);
    }
    bCanAttack = false;
    GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("CanAttack")), bCanAttack);
    }
}

FName AEnemy::GetAttackSectionName()
{
    FName SectionName;
    const int32 Section{ FMath::RandRange(1,4) };
    switch (Section)
    {
    case 1:
        SectionName = AttackLFast;
        break;
    case 2:
        SectionName = AttackRFast;
        break;

    case 3:
        SectionName = AttackL;
        break;

    case 4:
        SectionName = AttackR;
        break;
    }

    return SectionName; 
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    auto Character = Cast<AShooterCharacter>(OtherActor);
    if (Character)
    {
        DoDamage(Character);
        SpawnBlood(Character, LeftWeaponSocket);
        StunCharacter(Character);
        DeactivateLeftWeapon();
    }
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    auto Character = Cast<AShooterCharacter>(OtherActor);
    if (Character)
    {
        DoDamage(Character);
        SpawnBlood(Character, RightWeaponSocket);
        StunCharacter(Character);
        DeactivateRightWeapon();
    }
}

void AEnemy::ActivateLeftWeapon()
{
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DoDamage(AShooterCharacter* Victim)
{
    if (Victim == nullptr) return;

    UGameplayStatics::ApplyDamage(Victim, BaseDamage, EnemyController, this, UDamageType::StaticClass());
    if (Victim->GetMeleeImpactSound())
    {
        UGameplayStatics::PlaySoundAtLocation(this, Victim->GetMeleeImpactSound(), GetActorLocation());
    }
}
void AEnemy::SpawnBlood(AShooterCharacter* Victim, FName SocketName)
{
    const USkeletalMeshSocket* TipSocket{ GetMesh()->GetSocketByName(SocketName) };
    if (TipSocket)
    {
        const FTransform SocketTransform{ TipSocket->GetSocketTransform(GetMesh()) };
        if (Victim->GetBloodParticles())
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Victim->GetBloodParticles(), SocketTransform);
        }
    }
}

void AEnemy::StunCharacter(AShooterCharacter * Victim)
{
    if (Victim)
    {
        const float Stun{ FMath::FRandRange(0.f, 1.f) };
        if (Stun <= Victim->GetStunChance())
        {
            Victim->Stun();
        }
    }
}

void AEnemy::ResetCanAttack()
{
    bCanAttack = true;
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("CanAttack")), bCanAttack);
    }
}

void AEnemy::FinishDeaht()
{
    //UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    //if (AnimInstance && DeathMontage)
    //{
    //    AnimInstance->Montage_Pause(DeathMontage);
    //}
    GetMesh()->bPauseAnims = true;
    GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy,DeathTime);
}

void AEnemy::DestroyEnemy()
{
    Destroy();
}

