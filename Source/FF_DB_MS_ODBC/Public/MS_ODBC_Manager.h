// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "MS_ODBC_Result.h"
#include "MS_ODBC_Thread.h"

#include "MS_ODBC_Manager.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDelegate_MS_ODBC_Connection, bool, IsSuccessfull, FString, Out_Code, FString, CreatedString);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDelegate_MS_ODBC_Execute, bool, IsSuccessfull, FString, Out_Code, UMS_ODBC_Result*, Out_Result);

UCLASS()
class FF_DB_MS_ODBC_API AMS_ODBC_Manager : public AActor
{
	GENERATED_BODY()
	
protected:
	
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game ends or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	FString ConnectionId;
	SQLHENV SQL_Handle_Environment = NULL;
	SQLHDBC SQL_Handle_Connection = NULL;

	virtual bool SetConnectionId(FString In_Id);
	virtual bool ConnectDatabase(FString& Out_Code, FString& CreatedString, FString ODBC_Name, FString Username, FString Password, FString ServerInstance = "SQLEXPRESS");

public:	
	
	// Sets default values for this actor's properties.
	AMS_ODBC_Manager();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void CreateConnection(FDelegate_MS_ODBC_Connection DelegateConnection, FString TargetServer, FString Username, FString Password, FString ServerInstance = "SQLEXPRESS");

	UFUNCTION(BlueprintCallable)
	virtual bool SendQuery(FString& Out_Code, UMS_ODBC_Result*& Out_Result, const FString& SQL_Query, bool bRecordResults);

	UFUNCTION(BlueprintCallable)
	virtual void SendQueryAsync(FDelegate_MS_ODBC_Execute DelegateExecute, const FString& SQL_Query, bool bRecordResults);

	UFUNCTION(BlueprintPure)
	virtual FString GetConnectionId();

};