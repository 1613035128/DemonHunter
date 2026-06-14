// Fill out your copyright notice in the Description page of Project Settings.


#include "HunterActor/HunterActor.h"

#include "Data/MovementConfigData.h"


// Sets default values
AHunterActor::AHunterActor()
{
	PrimaryActorTick.bCanEverTick = true;
	InputCommand = CreateDefaultSubobject<UInputCommandComponent>("InputCommand");
	InputCommand->SetupAttachment(RootComponent);
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComponent");
	Mesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AHunterActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHunterActor::SetMovement(const FVector2D& Value)
{
	if (Value.IsNearlyZero())
	{
		Movement.bIsMoving = false;
		return;
	}
	
	Movement.bIsMoving = true;
	FVector2D InputAngle = Value.GetSafeNormal();
	Movement.MoveDirection = InputAngle;
}

void AHunterActor::TickMovement(float DeltaTime)
{
	if (!MovementConfigData)
		return;
	
	float MoveSpeedAcceleration =  MovementConfigData->MoveAcceleration * Movement.bIsMoving ? 1 : -1;
	Movement.CurrentMoveSpeed = FMath::Clamp(Movement.CurrentMoveSpeed + MoveSpeedAcceleration * DeltaTime,
								0.f, MovementConfigData->MaxMoveSpeed);
	
	FVector2D MoveDelta = Movement.MoveDirection * Movement.CurrentMoveSpeed;
	SetActorLocation(GetActorLocation() + FVector(MoveDelta, 0.f));
	
	if (Mesh)
	{
		if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
		{
			
		}
	}
}

// Called every frame
void AHunterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TickMovement(DeltaTime);
}

