// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorMotionData.generated.h"

USTRUCT()
struct FHunterActorMotionData
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector ActorFacingDirection {FVector::ForwardVector};
};