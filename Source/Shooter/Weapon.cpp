// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Engine/Engine.h"

AWeapon::AWeapon()
    : AItem::AItem()
    , ThrowWeaponTime(1.75f)
    , bFalling(false)
    , Ammo(30)
    , MagazineCapacity(30)
    , WeaponType(EWeaponType::EWT_SubmachineGun)
    , AmmoType(EAmmoType::EAT_9mm)
    , ReloadMontageSection(FName(TEXT("Reload SMG")))
    , ClipBoneName(TEXT("smg_clip"))
    , SlideDisplacement(0.f)
    , SlideDisplacementTime(0.3f)
    , bMovingSlide(false)
    , MaxSlideDisplacement(6.f)
    , MaxRecoilRotation(10.f)
    , bAutomatic(true)
{
    PrimaryActorTick.bCanEverTick = true;

}
void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Keep the Weapon upright
    if (GetItemState() == EItemState::EIS_Falling && bFalling)
    {
        const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
        GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }
    // Update slide on pistol
    UpdateSlideDisplacement();
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    if (BoneToHide != FName(""))
    {
        GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);
    }
}
void AWeapon::ThrowWeapon()
{
    FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    GetItemMesh()->SetEnableGravity(true);

    const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
    const FVector MeshRight{ GetItemMesh()->GetRightVector() };
    // Direction in which we throw the Weapon
    FVector ImpulseDirection = MeshRight.RotateAngleAxis(-15.f, MeshForward);

    float RandomRotation{ FMath::FRandRange(20.f, 40.f) };
    ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
    ImpulseDirection *= 10'000.f;
    GetItemMesh()->AddImpulse(ImpulseDirection);

    bFalling = true;
    GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

    EnableGlowMaterial();
}
void AWeapon::StopFalling()
{
    bFalling = false;
    SetItemState(EItemState::EIS_Pickup);
    StartPulseTimer();
}
void AWeapon::OnConstruction(const FTransform & Transform)
{
    Super::OnConstruction(Transform);

    const FString WeaponTablePath{ TEXT("DataTable'/Game/_Game/DataTable/WeaponDataTable.WeaponDataTable'") };
    UDataTable* WeaponDataTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

    if (WeaponDataTableObject)
    {
        FWeaponDataTable* WeaponDataRow = nullptr;

        switch (WeaponType)
        {
        case EWeaponType::EWT_SubmachineGun:
            WeaponDataRow = WeaponDataTableObject->FindRow<FWeaponDataTable>(FName("SubMachineGun"), TEXT(""));
            break;

        case EWeaponType::EWT_AssaultRifle:
            WeaponDataRow = WeaponDataTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
            break;

        case EWeaponType::EWT_Pistol:
            WeaponDataRow = WeaponDataTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
            break;

        }

        if (WeaponDataRow)
        {
            AmmoType = WeaponDataRow->AmmoType;
            Ammo = WeaponDataRow->WeaponAmmo;
            MagazineCapacity = WeaponDataRow->MagazingCapacity;
            SetPickupSound(WeaponDataRow->PickupSound);
            SetEquipSound(WeaponDataRow->EquipSound);
            GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
            SetItemName(WeaponDataRow->ItemName);
            SetIconItem(WeaponDataRow->InventoryIcon);
            SetAmmoIcon(WeaponDataRow->AmmoIcon);
            SetMaterialInstance(WeaponDataRow->MaterialInstance);
            PreviousMaterialIndex = GetMaterialIndex();
            GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
            SetMaterialIndex(WeaponDataRow->MaterialIndex);
            SetClipBoneName(WeaponDataRow->ClipBoneName);
            SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
            GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);

            CrosshairsMiddle = WeaponDataRow->CrosshairsMiddle;
            CrosshairsLeft = WeaponDataRow->CrosshairsLeft;
            CrosshairsRight = WeaponDataRow->CrosshairsRight;
            CrosshairsTop = WeaponDataRow->CrosshairsTop;
            CrosshairsBottom = WeaponDataRow->CrosshairsBottom;
            AutoFireRate = WeaponDataRow->AutoFireRate;
            MuzzleFlash = WeaponDataRow->MuzzleFlash;
            FireSound = WeaponDataRow->FireSound;

            BoneToHide = WeaponDataRow->BoneToHide;
            bAutomatic = WeaponDataRow->bAutomatic;
            Damage = WeaponDataRow->Damage * GetAdditionalDamage();
            HeadShotDamage = WeaponDataRow->HeadShotDamage * GetAdditionalDamage();
        }
        if (GetMaterialInstance())
        {
            SetDynamicMaterialInstance( UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
            GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
            GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());

            EnableGlowMaterial();
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Damage: %d"), Damage);
    UE_LOG(LogTemp, Warning, TEXT("HeadShotDamage: %d"), HeadShotDamage);

}
void AWeapon::FinishMovingSlide()
{
    bMovingSlide = false;
}
void AWeapon::UpdateSlideDisplacement()
{
    if (SlideDisplacementCurve && bMovingSlide)
    {
        const float ElapsedTime{ GetWorldTimerManager().GetTimerElapsed(SlideTimer) };
        const float CurveValue{ SlideDisplacementCurve->GetFloatValue(ElapsedTime) };
        SlideDisplacement = CurveValue * MaxSlideDisplacement;
        RecoilRotation = CurveValue * MaxRecoilRotation;

    }
}
void AWeapon::DecrementAmmo()
{
    if (Ammo - 1 <= 0)
    {
        Ammo = 0;
    }
    else
    {
        --Ammo;
    }
}

void AWeapon::ReloadAmmo(int32 Amount)
{
    checkf(Ammo + Amount <= MagazineCapacity, TEXT("Attempted to reload with more than magazine capacity!"));
    Ammo += Amount;
}

bool AWeapon::ClipIsFull()
{
    return Ammo >= MagazineCapacity;
}

void AWeapon::StartSlideTimer()
{
    bMovingSlide = true;
    GetWorldTimerManager().SetTimer(SlideTimer, this, &AWeapon::FinishMovingSlide, SlideDisplacementTime);
}