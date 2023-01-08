// Shooter copyright


#include "STUHealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "STUGameModeBase.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

DEFINE_LOG_CATEGORY_STATIC(LogHealthComponent, All, All)

USTUHealthComponent::USTUHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void USTUHealthComponent::BeginPlay()
{
	Super::BeginPlay();
    check(MaxHealth > 0);

	SetHealth(MaxHealth);

	AActor* ComponentOwner = GetOwner();
    if (ComponentOwner)
    {
        ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &USTUHealthComponent::OnTakeAnyDamage);
        ComponentOwner->OnTakePointDamage.AddDynamic(this, &USTUHealthComponent::OnTakePointDamage);
    }

}

void USTUHealthComponent::OnTakePointDamage(AActor *DamagedActor, float Damage, class AController *InstigatedBy,
                                            FVector HitLocation,
                       class UPrimitiveComponent *FHitComponent, FName BoneName, FVector ShotFromDirection,
                       const class UDamageType *DamageType, AActor *DamageCauser)
{
    const auto FinalDamage = Damage * GetPointDamageModifier(DamagedActor, BoneName);
    UE_LOG(LogHealthComponent, Display, TEXT("On point damage: %f, final damage: %f, bone: %s"), Damage, FinalDamage, *BoneName.ToString());
    ApplyDamage(FinalDamage, InstigatedBy);
}

void USTUHealthComponent::ApplyDamage(float Damage, AController *InstigatedBy)
{
    if (Damage <= 0.0f || IsDead() || !GetWorld())
        return;

    SetHealth(Health - Damage);

    GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);

    if (IsDead())
    {
        Killed(InstigatedBy);
        OnDeath.Broadcast();
    }
    else if (AutoHeal)
    {
        GetWorld()->GetTimerManager().SetTimer(HealTimerHandle, this, &USTUHealthComponent::HealUpdate, HealUpdateTime,
                                               true, HealDelay);
    }
}
	

void USTUHealthComponent::USTUHealthComponent::OnTakeAnyDamage(AActor *DamagedActor, float Damage, const class UDamageType *DamageType,
                                              class AController *InstigatedBy, AActor *DamageCauser)
{
    UE_LOG(LogHealthComponent, Display, TEXT("On any damage: %f"), Damage);
}

void USTUHealthComponent::HealUpdate()
{
    SetHealth(Health + HealModifier);

    if (IsHealthFull() && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);
    }
}

void USTUHealthComponent::SetHealth(float NewHealth)
{
    const auto NextHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    const auto HealthDelta = NextHealth - Health;

    Health = NextHealth;
    OnHealthChanged.Broadcast(Health, HealthDelta);
}

bool USTUHealthComponent::TryToAddHealth(float HealthAmount)
{
    if (IsDead() || IsHealthFull())
        return false;

    SetHealth(Health + HealthAmount);
    return true;
}

bool USTUHealthComponent::IsHealthFull() const
{
    return FMath::IsNearlyEqual(Health, MaxHealth);
}

void USTUHealthComponent::Killed(AController *KillerController)
{
    if (!GetWorld())
        return;
    const auto GameMode = Cast<ASTUGameModeBase>(GetWorld()->GetAuthGameMode());
    if (!GameMode)
        return;

    const auto Player = Cast<APawn>(GetOwner());
    const auto VictimController = Player ? Player->Controller : nullptr;

    GameMode->Killed(KillerController, VictimController);
}

float USTUHealthComponent::GetPointDamageModifier(AActor *DamagedActor, const FName &BoneName)
{
    const auto Character = Cast<ACharacter>(DamagedActor);
    if (!Character || !Character->GetMesh() || !Character->GetMesh()->GetBodyInstance(BoneName))
        return 1.0f;

    const auto PhysMaterial = Character->GetMesh()->GetBodyInstance(BoneName)->GetSimplePhysicalMaterial();
    if (!PhysMaterial || !DamageModifiers.Contains(PhysMaterial))
        return 1.0f;

    return DamageModifiers[PhysMaterial];
}