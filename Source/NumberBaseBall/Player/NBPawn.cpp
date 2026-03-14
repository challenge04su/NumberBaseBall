#include "Player/NBPawn.h"
#include "NumberBaseBall.h"


ANBPawn::ANBPawn()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ANBPawn::BeginPlay()
{
	Super::BeginPlay();
	/*
	FString NetRoleString = NBFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::BeginPlay() %s [%s]"), *NBFunctionLibrary::GetNetModeString(this), *NetRoleString);
	NBFunctionLibrary::MyPrintString(this, CombinedString, 10.f);*/
}

void ANBPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	/*
	FString NetRoleString = NBFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::PossessedBy() %s [%s]"), *NBFunctionLibrary::GetNetModeString(this), *NetRoleString);
	NBFunctionLibrary::MyPrintString(this, CombinedString, 10.f);*/
}


