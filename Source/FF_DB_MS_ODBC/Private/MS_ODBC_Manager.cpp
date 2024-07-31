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

void AMS_ODBC_Manager::CreateConnection(FDelegate_MS_ODBC_Connection DelegateConnection, FString TargetServer, FString Username, FString Password, FString ServerInstance)
{
	if (TargetServer.IsEmpty())
	{
		DelegateConnection.ExecuteIfBound(false, "FF Microsoft ODBC : Target server shouldn't be empty !", "", nullptr);
		return;
	}

	if (Username.IsEmpty())
	{
		DelegateConnection.ExecuteIfBound(false, "FF Microsoft ODBC : Username shouldn't be empty !", "", nullptr);
		return;
	}

	UMS_ODBC_Connection* ConnectionObject = NewObject<UMS_ODBC_Connection>();

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection, ConnectionObject, TargetServer, Username, Password, ServerInstance]()
		{
			const FString ConnectionId = TargetServer + "&&" + Username;

			if (!ConnectionObject->SetConnectionId(ConnectionId))
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, ConnectionObject]()
					{
						DelegateConnection.ExecuteIfBound(false, "FF Microsoft ODBC : Connection ID shouldn't be emptyh !", "", ConnectionObject);
					}
				);

				return;
			}

			FString Out_Code;
			FString CreatedString;

			if (!ConnectionObject->ConnectDatabase(Out_Code, CreatedString, TargetServer, Username, Password, ServerInstance))
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, ConnectionObject, Out_Code, CreatedString]()
					{
						DelegateConnection.ExecuteIfBound(false, Out_Code, CreatedString, nullptr);
					}
				);

				return;
			}

			this->MAP_Connections.Add(ConnectionId, ConnectionObject);
			
			AsyncTask(ENamedThreads::GameThread, [DelegateConnection, ConnectionObject, Out_Code, CreatedString]()
				{
					DelegateConnection.ExecuteIfBound(true, Out_Code, CreatedString, ConnectionObject);
				}
			);
		}
	);
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