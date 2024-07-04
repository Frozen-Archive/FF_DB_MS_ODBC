// Fill out your copyright notice in the Description page of Project Settings.

#include "MS_ODBC_Manager.h"

// Sets default values
AMS_ODBC_Manager::AMS_ODBC_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMS_ODBC_Manager::BeginPlay()
{
	Super::BeginPlay();
}

void AMS_ODBC_Manager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AMS_ODBC_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AMS_ODBC_Manager::CreateConnection(UMS_ODBC_Connection*& Out_Connection, FString& Out_Code, FString& CreatedString, FString TargetServer, FString Username, FString Password)
{
	UMS_ODBC_Connection* ConnectionObject = NewObject<UMS_ODBC_Connection>();

	if (ConnectionObject->ConnectDatabase(Out_Code, CreatedString, TargetServer, Username, Password))
	{
		Out_Connection = ConnectionObject;
		return true;
	}

	else
	{
		return false;
	}
}