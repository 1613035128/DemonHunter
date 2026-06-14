// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/InputCommandComponent.h"
#include "GameFramework/Actor.h"
#include "HunterActor.generated.h"

class UMovementConfigData;

USTRUCT()
struct FActorMovement
{
	GENERATED_BODY()
	
	FVector2D MoveDirection {0.f, 0.f};
	
	float CurrentMoveSpeed = 0.f;
	
	bool bIsMoving = false;
};

UCLASS()
class DEMONHUNTER_API AHunterActor : public APawn
{
	GENERATED_BODY()

public:
	AHunterActor();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
	void SetMovement(const FVector2D& Value);
protected:
	void TickMovement(float DeltaTime);
public:
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMovementConfigData> MovementConfigData;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY()
	TObjectPtr<UInputCommandComponent> InputCommand {nullptr};
	
	UPROPERTY()
	FActorMovement Movement;
};
