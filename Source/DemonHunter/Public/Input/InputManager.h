// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputManager.generated.h"
UCLASS()
class DEMONHUNTER_API AInputManager : public AActor
{
	GENERATED_BODY()

public:
	AInputManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
