// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MovementConfigData.generated.h"

/**
 * 
 */
UCLASS()
class DEMONHUNTER_API UMovementConfigData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveAcceleration {0.2f};
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxMoveSpeed {5.f};
};
