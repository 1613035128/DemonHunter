// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCotroller/HunterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AHunterPlayerController::BeginPlay()
{
	if (auto InputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputLocalPlayerSubsystem->AddMappingContext(InputMapping.Get(), 10);
	}
	
	if (HunterActorClass)
	{
		HunterActor = GetWorld()->SpawnActor<AHunterActor>(HunterActorClass);
	}
	
}

void AHunterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput || !InputConfig) return;
	
	InputComp = EnhancedInput;
	InitInput();
}

void AHunterPlayerController::InitInput()
{
	if (InputConfig->MoveAction)
	{
		InputComp->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &AHunterPlayerController::OnMove);
	}
	
	if (InputConfig->LookAction)
	{
		InputComp->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &AHunterPlayerController::OnLook);
	}
	
	if (InputConfig->LightAttackAction)
	{
		InputComp->BindAction(InputConfig->LightAttackAction, ETriggerEvent::Triggered, this, &AHunterPlayerController::OnLightAttack);
	}
	
	if (InputConfig->HeavyAttackAction)
	{
		InputComp->BindAction(InputConfig->HeavyAttackAction, ETriggerEvent::Triggered, this, &AHunterPlayerController::OnHeavyAttack);
	}
	
	if (InputConfig->DodgeAction)
	{
		InputComp->BindAction(InputConfig->DodgeAction, ETriggerEvent::Triggered, this, &AHunterPlayerController::OnDodge);
	}
	
	if (InputConfig->BlockAction)
	{
		InputComp->BindAction(InputConfig->BlockAction, ETriggerEvent::Triggered, this, &AHunterPlayerController::OnBlock);
	}
}

void AHunterPlayerController::OnMove(const FInputActionValue& Value)
{
	UE_LOG(LogInput, Display, TEXT("Move : %f, %f"), Value.Get<FVector2D>().X, Value.Get<FVector2D>().Y);
}

void AHunterPlayerController::OnLook(const FInputActionValue& Value)
{
	UE_LOG(LogInput, Display, TEXT("Look : %f"), Value.Get<float>());
}

void AHunterPlayerController::OnLightAttack(const FInputActionValue& Value)
{
	UE_LOG(LogInput, Display, TEXT("LightAttack : %f"), Value.Get<float>());
}

void AHunterPlayerController::OnHeavyAttack(const FInputActionValue& Value)
{
	UE_LOG(LogInput, Display, TEXT("HeavyAttack : %f"), Value.Get<float>());
}

void AHunterPlayerController::OnDodge(const FInputActionValue& Value)
{
	UE_LOG(LogInput, Display, TEXT("Dodge : %f"), Value.Get<float>());
}

void AHunterPlayerController::OnBlock(const FInputActionValue& Value)
{
	UE_LOG(LogInput, Display, TEXT("Block : %f"), Value.Get<float>());
}
