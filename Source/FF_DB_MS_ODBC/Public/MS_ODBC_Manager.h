// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// UE Includes.
#include "Containers/Queue.h"

// Custom Includes.
#include "Objects/MS_ODBC_Thread.h"
#include "Objects/MS_ODBC_Connection.h"

#include "MS_ODBC_Manager.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_FourParams(FDelegate_MS_ODBC_Connection, bool, IsSuccessfull, FString, Out_Code, FString, CreatedString, UMS_ODBC_Connection*, Out_Connection);

UCLASS()
class FF_DB_MS_ODBC_API AMS_ODBC_Manager : public AActor
{
	GENERATED_BODY()
	
protected:
	
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TMap<FString, UMS_ODBC_Connection*> MAP_Connections;

public:	
	
	// Sets default values for this actor's properties.
	AMS_ODBC_Manager();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void CreateConnection(FDelegate_MS_ODBC_Connection DelegateConnection, FString TargetServer, FString Username, FString Password, FString ServerInstance = "SQLEXPRESS");

	UFUNCTION(BlueprintCallable)
	virtual bool GetConnectionFromId(UMS_ODBC_Connection*& Out_Connection, FString In_Id);

};