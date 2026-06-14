// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"



AAuraPlayerController::AAuraPlayerController()
{
	// Change being made on one machine (server) is broadcasted & heard by subscribing clients
	bReplicates = true;
	
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	AutoRun();
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
	// the address of an object of type EnhancedInputComponent. CastChecked will 
	// both cast the InputComponent and check it.
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
	AuraInputComponent->BindAbilityActions(
		InputConfig,
		this, 
		&ThisClass::AbilityInputTagPressed, 
		&ThisClass::AbilityInputTagReleased, 
		&ThisClass::AbilityInputTagHeld
	);
}

void AAuraPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;
	
	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());
	
	// LastActor = ThisActor;
	// ThisActor = CursorHit.GetActor();
	
	if (LastActor != ThisActor)
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->HighlightActor();
	}
	
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
	
	// if (LastActor == nullptr)
	// {
	// 	if (ThisActor != nullptr)
	// 	{
	// 		// Case B
	// 		ThisActor->HighlightActor();
	// 		// IEnemyInterface::Execute_HighlightActor(ThisActor);
	// 		
	// 		UE_LOG(LogTemp, Warning, TEXT("Cursor Tracing: ThisActor != nullptr"));
	//
	// 	}
	// 	else
	// 	{
	// 		// Case A - both are null, do nothing
	// 	}
	// }
	// else // LastActor is valid
	// {
	// 	if (ThisActor == nullptr)
	// 	{
	// 		// Case C
	// 		LastActor->UnHighlightActor();
	// 	}
	// 	else
	// 	{
	// 		if (LastActor != ThisActor)
	// 		{
	// 			// Case D
	// 			LastActor->UnHighlightActor();
	// 			ThisActor->HighlightActor();
	// 		}
	// 		else
	// 		{
	// 			// Case E - both are valid and it's the same actor, do nothing
	// 		}
	// 	}
	// }

}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = ThisActor ? true : false;
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}
	
	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
	
	if (!bTargeting && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
				this, 
				ControlledPawn->GetActorLocation(), 
				CachedDestination
			))
			{
				Spline->ClearSplinePoints();
				
				// NavPath->PathPoints returns a TArray<FVector>, and these FVectors can be used to create a spline.
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
					
					DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);
				}
				CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				bAutoRunning = true;
			}
		}
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}
	
	if (bTargeting || bShiftKeyDown)
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		
		if (CursorHit.bBlockingHit) CachedDestination = CursorHit.ImpactPoint;
		
		if (APawn* ControlledPawn = GetPawn<APawn>())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>())
		);
	}
	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(
			ControlledPawn->GetActorLocation(),
			ESplineCoordinateSpace::World
		);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(
			LocationOnSpline,
			ESplineCoordinateSpace::World
		);
		ControlledPawn->AddMovementInput(Direction);
		
		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
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


