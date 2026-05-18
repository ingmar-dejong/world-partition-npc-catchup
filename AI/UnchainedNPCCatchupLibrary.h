// Copyright Unchained.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnchainedNPCCatchupLibrary.generated.h"

class APawn;
class UNPCGoalItem;

UCLASS()
class UNCHAINED_API UUnchainedNPCCatchupLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Applies the minimal mid-walk catchup correction for an interact goal.
	// Returns true when the pawn was moved to a checkpoint and the goal flag was consumed.
	UFUNCTION(BlueprintCallable, Category = "Unchained|NPC AI|Catchup")
	static bool TryApplyMidWalkCatchup(UNPCGoalItem* Goal, APawn* Pawn);
};
