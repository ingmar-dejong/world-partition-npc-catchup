// Copyright Unchained.

#pragma once

#include "CoreMinimal.h"
#include "AI/Activities/OwnSystemEvent_AddGoalToNPC.h"
#include "UnchainedEvent_AddGoalToNPC.generated.h"

class AUnchainedNPCWalkRoute;

// A single waypoint along a movement route, used for mid-walk WP catchup.
// Generated automatically by AUnchainedNPCWalkRoute::GetCheckpoints(), or set manually.
USTRUCT(BlueprintType)
struct FUnchainedNPCWalkCheckpoint
{
	GENERATED_BODY()

	// TOD (0-2400) at which the NPC is expected to pass this checkpoint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float TODWhenReached = 0.f;

	// World location of this checkpoint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FVector CheckpointLocation = FVector::ZeroVector;
};

/**
 * Extended version of OwnSystemEvent_AddGoalToNPC with minimal World Partition mid-walk catchup support.
 *
 * This class no longer tries to handle broad start/skip-entry catchup. Upstream OwnSystem Pro
 * now handles schedule/start-state recovery well enough on its own. The only remaining custom
 * responsibility here is resolving a believable mid-walk checkpoint when an NPC streams back in.
 *
 * Use this class for schedule triggers where an NPC may need to resume from a route checkpoint
 * instead of restarting from its original spawn/location.
 */
UCLASS(DisplayName="AI: Add Goal To NPC (Unchained)")
class UNCHAINED_API UUnchainedEvent_AddGoalToNPC : public UOwnSystemEvent_AddGoalToNPC
{
	GENERATED_BODY()

public:

	// TOD (0-2400) when the NPC starts walking toward the goal destination.
	// Used together with ExpectedArrivalTOD to calculate checkpoint TODs along the route.
	// Only relevant when RouteActor is set.
	UPROPERTY(EditAnywhere, Category = "NPC Activity|WP Catchup")
	float StartTOD = 0.f;

	// TOD (0-2400) when the NPC is expected to reach the final interaction destination.
	// Used together with StartTOD to determine which checkpoint should already have been
	// reached when the NPC streams back in.
	// 0 = do not attempt mid-walk checkpoint resolution for this goal.
	UPROPERTY(EditAnywhere, Category = "NPC Activity|WP Catchup")
	float ExpectedArrivalTOD = 0.f;

	// Route actor in the level that defines waypoints via a Spline Component.
	// When set, checkpoints are generated automatically — WalkCheckpoints is ignored.
	// StartTOD and EndTOD on the route actor must match this goal's schedule window.
	UPROPERTY(EditAnywhere, Category = "NPC Activity|WP Catchup")
	TSoftObjectPtr<AUnchainedNPCWalkRoute> RouteActor;

	// Manual fallback checkpoints. Only used when RouteActor is not set.
	// Prefer using RouteActor for new goals.
	UPROPERTY(EditAnywhere, Category = "NPC Activity|WP Catchup")
	TArray<FUnchainedNPCWalkCheckpoint> WalkCheckpoints;

protected:

	virtual void OnActivate_Implementation(APawn* Target, APlayerController* Controller, class UTalesComponent* OwnSystemComponent) override;
};
