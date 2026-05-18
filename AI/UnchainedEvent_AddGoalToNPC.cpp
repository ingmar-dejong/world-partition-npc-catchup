// Copyright Unchained.

#include "AI/UnchainedEvent_AddGoalToNPC.h"
#include "AI/UnchainedNPCWalkRoute.h"
#include "AI/Activities/NPCGoalItem.h"
#include "UnrealFramework/OwnSystemNPCCharacter.h"
#include "UnrealFramework/OwnSystemGameState.h"

void UUnchainedEvent_AddGoalToNPC::OnActivate_Implementation(APawn* Target, APlayerController* Controller, UTalesComponent* OwnSystemComponent)
{
	const AOwnSystemNPCCharacter* PreActivateNPC = Cast<AOwnSystemNPCCharacter>(Target);
	const bool bIsInitialLoadCatchup = IsValid(PreActivateNPC) && PreActivateNPC->IsCharacterPendingLoad();

	// Reset checkpoint catchup flags before re-adding.
	// GoalToAdd is a persistent editor object that retains flags from a previous catchup
	// cycle; without this reset a later schedule repetition could incorrectly reuse them.
	if (GoalToAdd)
	{
		GoalToAdd->bHasCatchupCheckpoint = false;
		GoalToAdd->CatchupCheckpointLocation = FVector::ZeroVector;
	}

	// Base class handles: IsValid check, GetActivityComponent, AddGoal -> sets ActiveGoal.
	Super::OnActivate_Implementation(Target, Controller, OwnSystemComponent);

	// Apply WP stream-in catchup logic for goals added during the initial BeginPlay
	// schedule catch-up (NPC should already be at, or partway to, the destination).
	AOwnSystemNPCCharacter* NPC = Cast<AOwnSystemNPCCharacter>(Target);
	if (!NPC || !ActiveGoal)
	{
		return;
	}

	if (bIsInitialLoadCatchup && ExpectedArrivalTOD > 0.f)
	{
		AOwnSystemGameState* GS = Cast<AOwnSystemGameState>(GetWorld()->GetGameState());
		if (!GS)
		{
			return;
		}

		// Resolve checkpoints: route actor takes precedence over the manual array.
		TArray<FUnchainedNPCWalkCheckpoint> ResolvedCheckpoints;
		if (AUnchainedNPCWalkRoute* Route = RouteActor.Get())
		{
			ResolvedCheckpoints = Route->GetCheckpoints(StartTOD, ExpectedArrivalTOD);
		}
		else
		{
			ResolvedCheckpoints = WalkCheckpoints;
		}

		if (ResolvedCheckpoints.Num() <= 0)
		{
			return;
		}

		const float CurrentTOD = GS->GetTimeOfDay();
		FVector BestLocation = FVector::ZeroVector;
		float BestTOD = -1.f;
		bool bFoundCheckpoint = false;

		for (const FUnchainedNPCWalkCheckpoint& Checkpoint : ResolvedCheckpoints)
		{
			if (CurrentTOD >= Checkpoint.TODWhenReached && Checkpoint.TODWhenReached > BestTOD)
			{
				BestTOD = Checkpoint.TODWhenReached;
				BestLocation = Checkpoint.CheckpointLocation;
				bFoundCheckpoint = true;
			}
		}

		if (bFoundCheckpoint)
		{
			// NPC was mid-walk -- remember the furthest reached checkpoint so the
			// interact activity can do a one-time physical correction before normal flow resumes.
			ActiveGoal->bHasCatchupCheckpoint = true;
			ActiveGoal->CatchupCheckpointLocation = BestLocation;
		}
		else
		{
			// No checkpoint reached yet: use the first checkpoint as a bootstrap point
			// so the NPC does not restart from its original route start after WP reload.
			FVector FirstLocation = FVector::ZeroVector;
			float LowestTOD = FLT_MAX;
			for (const FUnchainedNPCWalkCheckpoint& CP : ResolvedCheckpoints)
			{
				if (CP.TODWhenReached < LowestTOD)
				{
					LowestTOD = CP.TODWhenReached;
					FirstLocation = CP.CheckpointLocation;
				}
			}

			ActiveGoal->bHasCatchupCheckpoint = true;
			ActiveGoal->CatchupCheckpointLocation = FirstLocation;
		}
	}

#if WITH_GAMEPLAY_DEBUGGER
	if (AUnchainedNPCWalkRoute* Route = RouteActor.Get())
	{
		Route->DebugAssignedNPC = NPC;
	}
#endif
}
