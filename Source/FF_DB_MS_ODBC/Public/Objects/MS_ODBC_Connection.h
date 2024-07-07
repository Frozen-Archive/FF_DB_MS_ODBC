#pragma once

#include "CoreMinimal.h"

// Custom Includes.
#include "MS_ODBC_Includes.h"
#include "MS_ODBC_Result.h"

#include "MS_ODBC_Connection.generated.h"

UCLASS(BlueprintType)
class FF_DB_MS_ODBC_API UMS_ODBC_Connection : public UObject
{
	GENERATED_BODY()

protected:

	FString ConnectionId;
	SQLHENV SQL_Handle_Environment;
	SQLHDBC SQL_Handle_Connection;

public:

	virtual bool SetConnectionId(FString In_Id);
	virtual bool ConnectDatabase(FString& Out_Code, FString& CreatedString, FString TargetServer, FString Username, FString Password);

	UFUNCTION(BlueprintCallable)
	virtual bool SendQuery(FString& Out_Code, UMS_ODBC_Result*& Out_Result, const FString& SQL_Query, bool bRecordResults);

};