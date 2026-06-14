// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCotroller/HunterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCotroller/HunterCameraManager.h"

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
	
	if (!HunterCameraManager.IsValid())
	{
		HunterCameraManager = MakeWeakObjectPtr<>(AHunterCameraManager::GetInstance());
		HunterCameraManager->SetMainActor(MakeWeakObjectPtr(HunterActor));
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
		InputComp->BindAction(InputConfig->MoveAction, ETriggerEvent::Started, this, &AHunterPlayerController::OnMoveStart);
		InputComp->BindAction(InputConfig->MoveAction, ETriggerEvent::Completed, this, &AHunterPlayerController::OnMoveEnd);
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

	if (InputConfig->ViewRotateAction)
	{
		InputComp->BindAction(InputConfig->ViewRotateAction, ETriggerEvent::Triggered, this, &AHunterPlayerController::OnViewRotate);
	}
}

void AHunterPlayerController::OnMoveStart(const FInputActionValue& Value)
{
	if (!HunterActor)
		return;
	
	HunterActor->SetMovement(Value.Get<FVector2D>());
}

void AHunterPlayerController::OnMoveEnd(const FInputActionValue& Value)
{
	HunterActor->SetMovement(FVector2D::Zero());
}

void AHunterPlayerController::OnViewRotate(const FInputActionValue& Value)
{
	if (!HunterCameraManager.IsValid())
		return;

	HunterCameraManager->SetRotate(Value.Get<FVector2D>());
}

void AHunterPlayerController::OnLook(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("[PC] >>> OnLook FIRED: %f"), Value.Get<float>());
}

void AHunterPlayerController::OnLightAttack(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("[PC] >>> OnLightAttack FIRED: %f"), Value.Get<float>());
}

void AHunterPlayerController::OnHeavyAttack(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("[PC] >>> OnHeavyAttack FIRED: %f"), Value.Get<float>());
}

void AHunterPlayerController::OnDodge(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("[PC] >>> OnDodge FIRED: %f"), Value.Get<float>());
}

void AHunterPlayerController::OnBlock(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("[PC] >>> OnBlock FIRED: %f"), Value.Get<float>());
}
