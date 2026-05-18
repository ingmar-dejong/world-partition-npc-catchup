// Copyright Unchained.

#include "AI/UnchainedNPCCatchupLibrary.h"

#include "AIController.h"
#include "AI/Activities/NPCGoalItem.h"
#include "GameFramework/Pawn.h"

bool UUnchainedNPCCatchupLibrary::TryApplyMidWalkCatchup(UNPCGoalItem* Goal, APawn* Pawn)
{
	if (!IsValid(Goal) || !IsValid(Pawn) || !Goal->bHasCatchupCheckpoint)
	{
		return false;
	}

	const FRotator CurrentRotation = Pawn->GetActorRotation();
	Pawn->SetActorLocationAndRotation(Goal->CatchupCheckpointLocation, CurrentRotation, false, nullptr, ETeleportType::None);

	Goal->bHasCatchupCheckpoint = false;
	Goal->CatchupCheckpointLocation = FVector::ZeroVector;

	if (AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
	{
		AIController->StopMovement();
	}

	return true;
}
