// Copyright Unchained.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/UnchainedEvent_AddGoalToNPC.h"
#include "UnchainedNPCWalkRoute.generated.h"

/**
 * Level actor that defines a walk route for NPC World Partition catchup.
 *
 * Place this actor in the level and draw the route using the Spline Component.
 * The first spline point is the NPC's starting position; the last is the
 * destination (matching the goal). Intermediate points become checkpoints.
 *
 * Set StartTOD (when the NPC departs) and EndTOD (= ExpectedArrivalTOD on the goal).
 * GetCheckpoints() returns the intermediate points with auto-calculated TODs
 * based on the distance fraction along the spline.
 *
 * Assign this actor to the RouteActor property on UUnchainedEvent_AddGoalToNPC
 * to replace the manual WalkCheckpoints array.
 */
UCLASS()
class UNCHAINED_API AUnchainedNPCWalkRoute : public AActor
{
	GENERATED_BODY()

public:

	AUnchainedNPCWalkRoute();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Route")
	TObjectPtr<class USplineComponent> SplineComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Route")
	TObjectPtr<class UBillboardComponent> BillboardComponent;

	// Shown above the route in the editor viewport. Use this to identify which
	// NPC and goal this route belongs to, e.g. "Blacksmith - Huis naar Werkplek".
	// Not visible during play.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Route")
	TObjectPtr<class UTextRenderComponent> LabelComponent;

	// Human-readable label shown in the editor viewport above the route.
	// Example: "Blacksmith - Huis naar Werkplek"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FString Description;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Returns intermediate spline points as checkpoints with auto-calculated TODs.
	// First and last spline points are excluded (start and destination).
	// Returns an empty array if fewer than 3 spline points are present.
	UFUNCTION(BlueprintCallable, Category = "Route")
	TArray<FUnchainedNPCWalkCheckpoint> GetCheckpoints(float InStartTOD, float InEndTOD) const;

private:

	void UpdateLabel();

public:

#if WITH_GAMEPLAY_DEBUGGER
	// Set at runtime by UUnchainedEvent_AddGoalToNPC::OnActivate. Read by GDC_NPCLogger.
	TWeakObjectPtr<APawn> DebugAssignedNPC;
#endif
};
