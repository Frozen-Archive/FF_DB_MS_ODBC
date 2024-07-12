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

bool AMS_ODBC_Manager::CreateConnection(UMS_ODBC_Connection*& Out_Connection, FString& Out_Code, FString& CreatedString, FString TargetServer, FString Username, FString Password, FString ServerInstance)
{
	if (TargetServer.IsEmpty())
	{
		Out_Code = "FF Microsoft ODBC : Target server shouldn't be empty !";
		return false;
	}

	if (Username.IsEmpty())
	{
		Out_Code = "FF Microsoft ODBC : Username shouldn't be empty !";
		return false;
	}

	const FString ConnectionId = TargetServer + "&&" + Username;

	UMS_ODBC_Connection* ConnectionObject = NewObject<UMS_ODBC_Connection>();

	if (!ConnectionObject->SetConnectionId(ConnectionId))
	{
		return false;
	}

	if (!ConnectionObject->ConnectDatabase(Out_Code, CreatedString, TargetServer, Username, Password, ServerInstance))
	{
		return false;
	}

	this->MAP_Connections.Add(ConnectionId, ConnectionObject);
	Out_Connection = ConnectionObject;
	return true;
}

bool AMS_ODBC_Manager::GetConnectionFromId(UMS_ODBC_Connection*& Out_Connection, FString In_Id)
{
	if (In_Id.IsEmpty())
	{
		return false;
	}

	if (this->MAP_Connections.IsEmpty())
	{
		return false;
	}

	if (!this->MAP_Connections.Contains(In_Id))
	{
		return false;
	}

	UMS_ODBC_Connection* ConnectionObject = *this->MAP_Connections.Find(In_Id);

	if (!ConnectionObject)
	{
		return false;
	}

	Out_Connection = ConnectionObject;
	return false;
}