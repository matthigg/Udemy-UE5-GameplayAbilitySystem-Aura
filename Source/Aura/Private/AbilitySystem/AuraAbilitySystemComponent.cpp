// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
}

void UAuraAbilitySystemComponent::EffectApplied(
	UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec, 
	FActiveGameplayEffectHandle ActiveEffectHandle
)
{
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, FString(TEXT("Effect Applied!")));

	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	
	const FString CountMsg = FString::Printf(TEXT("Asset Tags: %d"), TagContainer.Num());
	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, CountMsg);
	
	//
	
	FGameplayTagContainer GrantedTags;
	EffectSpec.GetAllGrantedTags(GrantedTags);

	GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan,FString::Printf(TEXT("Granted Tags: %d"), GrantedTags.Num()));
	
	for (const FGameplayTag& Tag : TagContainer)
	{
		// TODO: Broadcast the tag to the Widget Controller
		const FString Msg = FString::Printf(TEXT("GE Tag: %s"), *Tag.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, Msg);
	}
}
