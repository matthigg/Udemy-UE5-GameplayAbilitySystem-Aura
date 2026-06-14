// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraProjectile.h"
#include "NiagaraFunctionLibrary.h"
#include "Aura/Aura.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(FName("Sphere"));
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SetLifeSpan(LifeSpan);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

// It's possible that Destroyed is called before OnSphereOverlap on the client side
void AAuraProjectile::Destroyed()
{
	// Ensure the sound is played & the effect is spawned for the client
	if (!bHit && !HasAuthority())
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			ImpactEffect,
			GetActorLocation()
		);
	}
	
	if (LoopingSoundComponent && !LoopingSoundComponent->IsBeingDestroyed() && LoopingSoundComponent->IsActive())
	{
		// LoopingSoundComponent->Stop();
		LoopingSoundComponent->FadeOut(0.1f, 0.0f); 
	}
	
	Super::Destroyed();
}

void AAuraProjectile::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult
)
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		ImpactSound,
		GetActorLocation(),
		FRotator::ZeroRotator
	);
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		ImpactEffect,
		GetActorLocation()
	);
	
	if (LoopingSoundComponent && !LoopingSoundComponent->IsBeingDestroyed() && LoopingSoundComponent->IsActive())
	{
		// LoopingSoundComponent->Stop();
		LoopingSoundComponent->FadeOut(0.1f, 0.0f); 
	}
	
	// It's possible that the effect is destroyed before the sound is played & the effect is spawned on the client side
	if (HasAuthority())
	{
		Destroy();
	}
	else
	{
		bHit = true;
	}
}


