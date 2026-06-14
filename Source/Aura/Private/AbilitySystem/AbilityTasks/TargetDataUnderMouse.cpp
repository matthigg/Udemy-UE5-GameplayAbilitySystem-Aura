#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogTargetDataDebug, Display, All);

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
    UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
    return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
    const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();

    if (bIsLocallyControlled)
    {
        UE_LOG(LogTargetDataDebug, Warning,
            TEXT("[CLIENT] Activate: SpecHandle=%s ActivationKey=%d"),
            *GetAbilitySpecHandle().ToString(),
            GetActivationPredictionKey().Current);

        SendMouseCursorData();
    }
    else
    {
        const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
        const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

        UE_LOG(LogTargetDataDebug, Warning,
            TEXT("[SERVER] Listening: SpecHandle=%s ActivationKey=%d Valid=%s"),
            *SpecHandle.ToString(),
            ActivationPredictionKey.Current,
            ActivationPredictionKey.IsValidKey() ? TEXT("TRUE") : TEXT("FALSE"));

        AbilitySystemComponent->AbilityTargetDataSetDelegate(
            SpecHandle,
            ActivationPredictionKey
        ).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);

        const bool bCalledDelegate =
            AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(
                SpecHandle,
                ActivationPredictionKey
            );

        UE_LOG(LogTargetDataDebug, Warning,
            TEXT("[SERVER] CallReplicatedTargetDataDelegatesIfSet returned %s"),
            bCalledDelegate ? TEXT("TRUE") : TEXT("FALSE"));

        if (!bCalledDelegate)
        {
            SetWaitingOnRemotePlayerData();
        }
    }
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
    FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

    APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
    if (!PC)
    {
        UE_LOG(LogTargetDataDebug, Error, TEXT("[CLIENT] No PlayerController"));
        return;
    }

    FHitResult CursorHit;
    PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

    const FPredictionKey ActivationKey = GetActivationPredictionKey();
    const FPredictionKey ScopedKey = AbilitySystemComponent->ScopedPredictionKey;

    UE_LOG(LogTargetDataDebug, Warning,
        TEXT("[CLIENT] Sending TargetData")
        TEXT(" SpecHandle=%s")
        TEXT(" ActivationKey=%d")
        TEXT(" ScopedKey=%d")
        TEXT(" HitLocation=%s"),
        *GetAbilitySpecHandle().ToString(),
        ActivationKey.Current,
        ScopedKey.Current,
        *CursorHit.Location.ToString());

    FGameplayAbilityTargetDataHandle DataHandle;
    FGameplayAbilityTargetData_SingleTargetHit* Data =
        new FGameplayAbilityTargetData_SingleTargetHit();

    Data->HitResult = CursorHit;
    DataHandle.Add(Data);

    AbilitySystemComponent->ServerSetReplicatedTargetData(
        GetAbilitySpecHandle(),
        ActivationKey,
        DataHandle,
        FGameplayTag(),
        ScopedKey
    );

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(DataHandle);
    }
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(
    const FGameplayAbilityTargetDataHandle& DataHandle,
    FGameplayTag ActivationTag)
{
    UE_LOG(LogTargetDataDebug, Warning,
        TEXT("[SERVER] OnTargetDataReplicatedCallback fired!")
        TEXT(" SpecHandle=%s")
        TEXT(" ActivationKey=%d")
        TEXT(" NumData=%d"),
        *GetAbilitySpecHandle().ToString(),
        GetActivationPredictionKey().Current,
        DataHandle.Num());

    if (DataHandle.Num() > 0)
    {
        const auto* HitData =
            static_cast<const FGameplayAbilityTargetData_SingleTargetHit*>(DataHandle.Get(0));

        if (HitData)
        {
            UE_LOG(LogTargetDataDebug, Warning,
                TEXT("[SERVER] Received HitLocation=%s"),
                *HitData->HitResult.Location.ToString());
        }
    }

    AbilitySystemComponent->ConsumeClientReplicatedTargetData(
        GetAbilitySpecHandle(),
        GetActivationPredictionKey());

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(DataHandle);
    }
}

// ORIGINAL CODE ====================================================================================================

// // Fill out your copyright notice in the Description page of Project Settings.
//
//
// #include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
// #include "AbilitySystemComponent.h"
//
// UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
// {
// 	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
// 	return MyObj;
// }
//
// void UTargetDataUnderMouse::Activate()
// {
//
// 	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
// 	if (bIsLocallyControlled)
// 	{
// 		SendMouseCursorData();
// 	}
// 	else
// 	{
// 		// TODO: We are on the server, so listen for target data
// 		
// 		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
// 		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
// 		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(
// 			SpecHandle, 
// 			ActivationPredictionKey
// 		).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
// 		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(
// 			SpecHandle, 
// 			ActivationPredictionKey
// 		);
// 		
// 		if (!bCalledDelegate)
// 		{
// 			SetWaitingOnRemotePlayerData();
// 		}
// 	}
// }
//
// void UTargetDataUnderMouse::SendMouseCursorData()
// {
// 	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
// 	
// 	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
// 	FHitResult CursorHit;
// 	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
// 	
// 	FGameplayAbilityTargetDataHandle DataHandle;
// 	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
// 	Data->HitResult = CursorHit;
// 	DataHandle.Add(Data);
// 	
// 	AbilitySystemComponent->ServerSetReplicatedTargetData(
// 		GetAbilitySpecHandle(), 
// 		GetActivationPredictionKey(), 
// 		DataHandle,
// 		FGameplayTag(),
// 		AbilitySystemComponent->ScopedPredictionKey
// 	);
// 	
// 	if (ShouldBroadcastAbilityTaskDelegates())
// 	{
// 		ValidData.Broadcast(DataHandle);
// 	}
// }
//
// void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(
// 	const FGameplayAbilityTargetDataHandle& DataHandle,
// 	FGameplayTag ActivationTag
// )
// {
// 	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
// 	if (ShouldBroadcastAbilityTaskDelegates())
// 	{
// 		ValidData.Broadcast(DataHandle);
// 	}
// }
