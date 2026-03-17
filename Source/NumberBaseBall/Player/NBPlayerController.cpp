#include "NBPlayerController.h"
#include "UI/NBChatInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NumberBaseBall/NumberBaseBall.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Game/NBGameModeBase.h"
#include "NBPlayerState.h"
#include "Net/UnrealNetwork.h"

ANBPlayerController::ANBPlayerController()
{
	bReplicates = true;
}

void ANBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UNBChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}

void ANBPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	if (IsLocalController() == true)
	{
		ANBPlayerState* NBPS = GetPlayerState<ANBPlayerState>();
		if (IsValid(NBPS) == true)
		{
			FString CombinedMessageString = NBPS->GetPlayerInfoString() + TEXT(" : ") + InChatMessageString;

			ServerRPCPrintChatMessageString(InChatMessageString);
		}
	}
}

void ANBPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	NBFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void ANBPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ANBPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ANBGameModeBase* NBGM = Cast<ANBGameModeBase>(GM);
		if (IsValid(NBGM) == true)
		{
			NBGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}

void ANBPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}
