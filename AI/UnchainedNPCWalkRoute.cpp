// Copyright Unchained.

#include "AI/UnchainedNPCWalkRoute.h"
#include "Components/SplineComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"

AUnchainedNPCWalkRoute::AUnchainedNPCWalkRoute()
{
	PrimaryActorTick.bCanEverTick = false;

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	SetRootComponent(BillboardComponent);

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComponent->SetupAttachment(BillboardComponent);

	LabelComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	LabelComponent->SetupAttachment(BillboardComponent);
	LabelComponent->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	LabelComponent->SetHorizontalAlignment(EHTA_Center);
	LabelComponent->SetWorldSize(40.f);
	LabelComponent->bHiddenInGame = true;
}

#if WITH_EDITOR
void AUnchainedNPCWalkRoute::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AUnchainedNPCWalkRoute, Description))
	{
		UpdateLabel();
	}
}
#endif

void AUnchainedNPCWalkRoute::UpdateLabel()
{
	if (LabelComponent)
	{
		LabelComponent->SetText(FText::FromString(Description));
	}
}

TArray<FUnchainedNPCWalkCheckpoint> AUnchainedNPCWalkRoute::GetCheckpoints(float InStartTOD, float InEndTOD) const
{
	TArray<FUnchainedNPCWalkCheckpoint> Result;

	if (!SplineComponent || InEndTOD <= InStartTOD)
	{
		return Result;
	}

	const int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
	const float TotalLength = SplineComponent->GetSplineLength();

	// Need at least 3 points: start, at least one intermediate, end.
	if (NumPoints < 3 || TotalLength <= 0.f)
	{
		return Result;
	}

	// Skip point 0 (start) and the last point (destination).
	for (int32 i = 1; i < NumPoints - 1; ++i)
	{
		FUnchainedNPCWalkCheckpoint Checkpoint;
		Checkpoint.CheckpointLocation = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

		const float DistAtPoint = SplineComponent->GetDistanceAlongSplineAtSplinePoint(i);
		const float Fraction = DistAtPoint / TotalLength;
		Checkpoint.TODWhenReached = InStartTOD + Fraction * (InEndTOD - InStartTOD);

		Result.Add(Checkpoint);
	}

	return Result;
}
