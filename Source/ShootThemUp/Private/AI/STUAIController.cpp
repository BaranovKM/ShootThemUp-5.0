// Shooter copyright


#include "STUAIController.h"
#include "STUAICharacter.h"
#include "Components/STUAIPerceptionComponent.h"
#include "Components/STURespawnComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

ASTUAIController::ASTUAIController()
{
    STUAIPerceptionComponent = CreateDefaultSubobject<USTUAIPerceptionComponent>("STUPerceptionComponent");
    SetPerceptionComponent(*STUAIPerceptionComponent);
    RespawnComponent = CreateDefaultSubobject<USTURespawnComponent>("RespawnComponent");
    bWantsPlayerState = true;
}

void ASTUAIController::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);

    const auto STUCharacter = Cast<ASTUAICharacter>(InPawn);
    if (STUCharacter)
    {
        RunBehaviorTree(STUCharacter->BehaviorTreeAsset);
    }
}

void ASTUAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    const auto AimActor = STUAIPerceptionComponent->GetClosestEnemy();
    SetFocus(AimActor);
}

AActor *ASTUAIController::GetFocusOnActor() const
{
    if (!GetBlackboardComponent()) return nullptr;
    return Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(FocusOnKeyName));

}