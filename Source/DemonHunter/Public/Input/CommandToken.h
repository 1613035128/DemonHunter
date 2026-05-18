// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "UObject/Object.h"
#include "CommandToken.generated.h"

USTRUCT()
struct FCommandToken
{
	GENERATED_BODY()
	
	FName CommandName {NAME_None};
	
	float TimeStamp {0.f};
	
	int32_t Priority {-1};
	
	float LifeTime {0.f};
	
	UPROPERTY()
	UInputAction* SourceInput {nullptr};
	
	TPair<bool, int> ComboIndex {false, 0};
};
