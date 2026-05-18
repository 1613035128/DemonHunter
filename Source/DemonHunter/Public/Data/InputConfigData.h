// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Engine/DataAsset.h"
#include "InputConfigData.generated.h"

/**
 * 
 */
UCLASS()
class DEMONHUNTER_API UInputConfigData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Movement")
	TObjectPtr<UInputAction> LookAction;

	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UInputAction> LightAttackAction;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UInputAction> BlockAction;
};
