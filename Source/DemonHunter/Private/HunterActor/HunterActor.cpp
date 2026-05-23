// Fill out your copyright notice in the Description page of Project Settings.


#include "HunterActor/HunterActor.h"


// Sets default values
AHunterActor::AHunterActor()
{
	PrimaryActorTick.bCanEverTick = true;
	InputCommand = CreateDefaultSubobject<UInputCommandComponent>("InputCommand");
}

// Called when the game starts or when spawned
void AHunterActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHunterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

