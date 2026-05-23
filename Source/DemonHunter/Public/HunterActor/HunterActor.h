// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/InputCommandComponent.h"
#include "GameFramework/Actor.h"
#include "HunterActor.generated.h"

UCLASS()
class DEMONHUNTER_API AHunterActor : public AActor
{
	GENERATED_BODY()

public:
	AHunterActor();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	
protected:
	UPROPERTY()
	TObjectPtr<UInputCommandComponent> InputCommand {nullptr};
};
