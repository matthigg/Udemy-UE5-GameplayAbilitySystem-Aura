// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/SphereComponent.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}
void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// Here's how to manually get the ASC via the interface
	// IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(TargetActor);
	// if (ASCInterface)
	// {
	// 	ASCInterface->GetAbilitySystemComponent();
	// }
	
	// Here's how to easily get the ASC (it gets it via the AbilitySystemInterface under the hood)
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	FGameplayEffectContextHandle GEContextHandle = TargetASC->MakeEffectContext();
	
	if (TargetASC == nullptr) return;
	check(GameplayEffectClass);
	
	GEContextHandle.AddSourceObject(this);
	
	const FGameplayEffectSpecHandle GESpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass,1.f, GEContextHandle);
	TargetASC->ApplyGameplayEffectSpecToSelf(*GESpecHandle.Data.Get());
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
}


// ========== Apply Effect outside of Gameplay Effect ==========

// Fill out your copyright notice in the Description page of Project Settings.
//
// #include "Actor/AuraEffectActor.h"
//
// #include "AbilitySystemInterface.h"
// #include "AbilitySystem/AuraAttributeSet.h"
// #include "Components/SphereComponent.h"
//
// AAuraEffectActor::AAuraEffectActor()
// {
// 	PrimaryActorTick.bCanEverTick = false;
//
// 	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
// 	SetRootComponent(StaticMesh);
// 	
// 	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
// 	Sphere->SetupAttachment(GetRootComponent());
// 	
// }
//
// // We're using this to alter attributes outside of using Gameplay Effects just for demonstration purposes
// void AAuraEffectActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
// 	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
// {
// 	// TODO: Change this to apply a Gameplay Effect, for now we're using const_cast as a hack!
// 	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor))
// 	{
// 		const UAuraAttributeSet* AuraAttributeSet = Cast<UAuraAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAuraAttributeSet::StaticClass()));
// 		UAuraAttributeSet* MutableAuraAttributeSet = const_cast<UAuraAttributeSet*>(AuraAttributeSet);
// 		MutableAuraAttributeSet->SetHealth(AuraAttributeSet->GetHealth() + 25.f);
// 		MutableAuraAttributeSet->SetMana(AuraAttributeSet->GetMana() + 25.f);
// 		Destroy();
// 	};
// }
//
// // We're using this to alter attributes outside of using Gameplay Effects just for demonstration purposes
// void AAuraEffectActor::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
// 	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
// {
// 	
// }
//
// void AAuraEffectActor::BeginPlay()
// {
// 	Super::BeginPlay();
// 	
// 	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraEffectActor::OnOverlap);
// 	Sphere->OnComponentEndOverlap.AddDynamic(this, &AAuraEffectActor::OnEndOverlap);
// }
//
//
