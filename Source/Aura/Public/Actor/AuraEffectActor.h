// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "ActiveGameplayEffectHandle.h"
#include "GameplayEffectTypes.h" // This includes FActiveGameplayEffectHandle
#include "GameFramework/Actor.h"
#include "AuraEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EEffectApplicationPolicy
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply
};

UENUM(BlueprintType)
enum class EEffectRemovalPolicy
{
	RemoveOnEndOverlap,
	DoNotApply
};

UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraEffectActor();
	


protected:
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);
	
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bDestroyOnEffectRemoval = false;

	// Instant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	
	// Duration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	
	// Infinite
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectRemovalPolicy InfiniteEffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;
	
	// Map ActiveGEHandle to TargetASC
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveGEHandleTargetASCMap;

private:
	

	
};




// ========== Apply Effect outside of Gameplay Effect ==========

// Fill out your copyright notice in the Description page of Project Settings.
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "GameFramework/Actor.h"
// #include "AuraEffectActor.generated.h"
//
// class USphereComponent;
//
// UCLASS()
// class AURA_API AAuraEffectActor : public AActor
// {
// 	GENERATED_BODY()
// 	
// public:	
// 	AAuraEffectActor();
// 	
// 	// We're using this to alter attributes outside of using Gameplay Effects just for demonstration purposes
// 	UFUNCTION()
// 	virtual void OnOverlap(
// 		UPrimitiveComponent* OverlappedComponent, 
// 		AActor* OtherActor, 
// 		UPrimitiveComponent* OtherComponent,
// 		int32 OtherBodyIndex,
// 		bool bFromSweep,
// 		const FHitResult& SweepResult
// 	);
// 	
// 	// We're using this to alter attributes outside of using Gameplay Effects just for demonstration purposes
// 	UFUNCTION()
// 	virtual void OnEndOverlap(
// 		UPrimitiveComponent* OverlappedComponent, 
// 		AActor* OtherActor, 
// 		UPrimitiveComponent* OtherComp, 
// 		int32 OtherBodyIndex
// 	);
//
// protected:
// 	virtual void BeginPlay() override;
//
// private:
// 	
// 	UPROPERTY(VisibleAnywhere)
// 	TObjectPtr<USphereComponent> Sphere;
//
// 	UPROPERTY(VisibleAnywhere)
// 	TObjectPtr<UStaticMeshComponent> StaticMesh;
// };
