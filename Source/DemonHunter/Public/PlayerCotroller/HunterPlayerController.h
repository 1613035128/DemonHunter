// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "Data/InputConfigData.h"
#include "GameFramework/PlayerController.h"
#include "HunterActor/HunterActor.h"
#include "HunterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DEMONHUNTER_API AHunterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
protected:
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;
	
	void InitInput();
	
	void OnMove(const FInputActionValue& Value);
	
	void OnLook(const FInputActionValue& Value);
	
	void OnLightAttack(const FInputActionValue& Value);
	
	void OnHeavyAttack(const FInputActionValue& Value);
	
	void OnDodge(const FInputActionValue& Value);
	
	void OnBlock(const FInputActionValue& Value);
public:
	
protected:
	UPROPERTY(EditAnywhere, meta = (Category="Input"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;
	
	UPROPERTY(EditAnywhere, meta = (Category="Input"))
	TObjectPtr<UInputConfigData> InputConfig;
	
	UPROPERTY(EditAnywhere, meta = (Category="PlayerActor"))
	TSubclassOf<AHunterActor> HunterActorClass {nullptr};
	
	UPROPERTY()
	UEnhancedInputComponent* InputComp {nullptr}; 
	
	UPROPERTY()
	TObjectPtr<AHunterActor> HunterActor {nullptr};
};
