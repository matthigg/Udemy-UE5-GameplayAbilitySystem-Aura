// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Kismet/GameplayStatics.h"


AAuraPlayerController::AAuraPlayerController()
{
	// Change being made on one machine (server) is broadcasted & heard by subscribing clients
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
	
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);
	
	// Subsystems are singletons
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	
	// The FInputModeGameAndUI allows us to accept input from a mouse & interact with UI widgets
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// We need to cast InputComponent to an EnhancedInputComponent because it stores
	// the address of and object of type EnhancedInputComponent. CastChecked will 
	// both cast the InputComponent and check it.
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();
	
	/**
	 * Line trace from cursor. There are several scenarios:
	 * 
	 *	A. LastActor is null and ThisActor is null
	 *		- Do nothing
	 *		
	 *	B. LastActor is null and ThisActor is valid
	 *		- Hightlight ThisActor
	 *		
	 *	C. LastActor is valid and ThisActor is null
	 *		- UnHighlight LastActor
	 *		
	 *	D. LastActor is valid and ThisActor is valid, but LastActor != ThisActor
	 *		- UnHighlight LastActor, and Highlight ThisActor
	 *		
	 *	E. LastActor is valid and ThisActor is valid, and LastActor == ThisActor
	 *		- Do nothing (since actor is already highlighted)
	 */
	
	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			// Case B
			ThisActor->HighlightActor();
		}
		else
		{
			// Case A - both are null, do nothing
		}
	}
	else // LastActor is valid
	{
		if (ThisActor == nullptr)
		{
			// Case C
			LastActor->UnHighlightActor();
		}
		else
		{
			if (LastActor != ThisActor)
			{
				// Case D
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
			else
			{
				// Case E - both are valid and it's the same actor, do nothing
			}
		}
	}

}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
    
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("InputVector: X=%f, Y=%f"), InputAxisVector.X, InputAxisVector.Y);
	// DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + (ForwardDirection * 100.f), FColor::Red, false, -1.f, 0, 5.f);
	// DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + (RightDirection * 100.f), FColor::Green, false, -1.f, 0, 5.f);
}


