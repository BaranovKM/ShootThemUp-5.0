// Shooter copyright


#include "STUHealthPickup.h"
#include "Components/STUHealthComponent.h"
#include "STUUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogHealthPickup, All, All);

bool ASTUHealthPickup::GivePickUpTo(APawn *PlayerPawn)
{
    const auto HealthComponent = STUUtils::GetSTUPlayerComponent<USTUHealthComponent>(PlayerPawn);
    if (!HealthComponent)
        return false;

    return HealthComponent->TryToAddHealth(HealthAmount);
}