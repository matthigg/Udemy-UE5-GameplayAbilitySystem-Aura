// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
	

}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		// GiveAbility(AbilitySpec);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::EffectApplied(
	UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec, 
	FActiveGameplayEffectHandle ActiveEffectHandle
)
{
	// Asset Tags
	FGameplayTagContainer AssetTagContainer;
	EffectSpec.GetAllAssetTags(AssetTagContainer);
	EffectAssetTags.Broadcast(AssetTagContainer);
	// const FString AssetTagsCount = FString::Printf(TEXT("Asset Tags: %d"), AssetTagContainer.Num());
	// GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, AssetTagsCount);
	
	// Granted Tags
	// FGameplayTagContainer GrantedTagContainer;
	// EffectSpec.GetAllGrantedTags(GrantedTagContainer);
	// const FString GrantedTagsCount = FString::Printf(TEXT("Granted Tags: %d"), GrantedTagContainer.Num());
	// GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, GrantedTagsCount);
	
}
