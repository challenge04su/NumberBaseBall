#include "NBGameModeBase.h"
#include "NBGameStateBase.h"
#include "Player/NBPlayerController.h"
#include "EngineUtils.h"
#include "Player/NBPlayerState.h"

void ANBGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ANBPlayerController* NBPlayerController = Cast<ANBPlayerController>(NewPlayer);
	if (IsValid(NBPlayerController) == true)
	{
		NBPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(NBPlayerController);

		ANBPlayerState* NBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
		if (IsValid(NBPS) == true)
		{
			NBPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ANBGameStateBase* NBGameStateBase = GetGameState<ANBGameStateBase>();
		if (IsValid(NBGameStateBase) == true)
		{
			NBGameStateBase->MulticastRPCBroadcastLoginMessage(NBPS->PlayerNameString);
		}
	}

	ANBPlayerState* NBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
	if (IsValid(NBPS))
	{
		NBPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		NBPS->MaxGuessCount = 3;
		NBPS->CurrentGuessCount = 1;
	}
}

FString ANBGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ANBGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	/*bool bCanPlay = false;

	do {

		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;

	} while (false);

	return bCanPlay;*/
	if (InNumberString.Len() != 3)
	{
		return false;
	}

	TSet<TCHAR> UniqueDigits;

	for (TCHAR C : InNumberString)
	{
		if (!FChar::IsDigit(C) || C == '0')
		{
			return false;
		}

		UniqueDigits.Add(C);
	}

	return UniqueDigits.Num() == 3;
}

FString ANBGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ANBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *SecretNumberString);
}

void ANBGameModeBase::PrintChatMessageString(ANBPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	ANBPlayerState* NBPS = InChattingPlayerController->GetPlayerState<ANBPlayerState>();
	if (!IsValid(NBPS)) return;

	if (NBPS->CurrentGuessCount > NBPS->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("시스템: 모든 기회를 사용하셨습니다. 다른 플레이어를 기다리세요."));
		return; 
	}

	if (IsGuessNumberString(InChatMessageString))
	{
		
		FString JudgeResultString = JudgeResult(SecretNumberString, InChatMessageString);

		FString FullMessage = NBPS->GetPlayerInfoString() + TEXT(" : ") + InChatMessageString + TEXT(" -> ") + JudgeResultString;

		for (TActorIterator<ANBPlayerController> It(GetWorld()); It; ++It)
		{
			if (IsValid(*It))
			{
				(*It)->ClientRPCPrintChatMessageString(FullMessage);
			}
		}

		IncreaseGuessCount(InChattingPlayerController);

		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
		JudgeGame(InChattingPlayerController, StrikeCount);
	}
	else
	{
		FString WarningMessage = NBPS->PlayerNameString + TEXT(" : ") + InChatMessageString + TEXT(" (숫자가 아닙니다. 다시 입력하세요!)");

		for (TActorIterator<ANBPlayerController> It(GetWorld()); It; ++It)
		{
			if (IsValid(*It))
			{
				(*It)->ClientRPCPrintChatMessageString(WarningMessage);
			}
		}
	}
}

void ANBGameModeBase::IncreaseGuessCount(ANBPlayerController* InChattingPlayerController)
{
	ANBPlayerState* NBPS = InChattingPlayerController->GetPlayerState<ANBPlayerState>();
	if (IsValid(NBPS) == true)
	{
		NBPS->CurrentGuessCount++;
	}
}

void ANBGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Warning, TEXT("New Secret Number: %s"), *SecretNumberString);
	
	for (const auto& CXPlayerController : AllPlayerControllers)
	{
		ANBPlayerState* NBPS = CXPlayerController->GetPlayerState<ANBPlayerState>();
		if (IsValid(NBPS))
		{
			NBPS->CurrentGuessCount = 1;
		}

		if (IsValid(CXPlayerController))
		{
			CXPlayerController->NotificationText = FText::GetEmpty();
		}
	}
}

void ANBGameModeBase::JudgeGame(ANBPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ANBPlayerState* NBPS = InChattingPlayerController->GetPlayerState<ANBPlayerState>();
		if (IsValid(NBPS))
		{
			FString CombinedMessageString = NBPS->PlayerNameString + TEXT(" has won the game.");

			for (const auto& NBPlayerController : AllPlayerControllers)
			{
				NBPlayerController->NotificationText = FText::FromString(CombinedMessageString);
			}

			GetWorldTimerManager().SetTimer(GameResetTimerHandle, this, &ANBGameModeBase::ResetGame, 3.0f, false);
		}
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& NBPlayerController : AllPlayerControllers)
		{
			ANBPlayerState* NBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
			if (IsValid(NBPS) && NBPS->CurrentGuessCount <= NBPS->MaxGuessCount)
			{
				bIsDraw = false;
				break;
			}
		}

		if (bIsDraw)
		{
			for (const auto& NBPlayerController : AllPlayerControllers)
			{
				NBPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));
			}

			GetWorldTimerManager().SetTimer(GameResetTimerHandle, this, &ANBGameModeBase::ResetGame, 3.0f, false);
		}
	}
}
