#pragma once

#include "CoreMinimal.h"

// UE Includes.
#include "JsonObjectWrapper.h"
#include "JsonUtilities.h"

// Custom Includes.
#include "MS_ODBC_Includes.h"

#include "MS_ODBC_Connection.generated.h"

USTRUCT(BlueprintType)
struct FF_DB_MS_ODBC_API FMS_ODBC_DataValue
{
	GENERATED_BODY()

public:

	FString ValString;
	int32 ValInt32 = 0;
	float ValDouble = (double)0.f;
	bool ValBool = false;
	FDateTime ValDateTime;

	UPROPERTY(BlueprintReadOnly)
	int32 DataType = 0;

	UPROPERTY(BlueprintReadOnly)
	FString DataTypeName;

	UPROPERTY(BlueprintReadOnly)
	FString ColumnName;

	UPROPERTY(BlueprintReadOnly)
	FString ValueRepresentation;

};

UCLASS(BlueprintType)
class FF_DB_MS_ODBC_API UMS_ODBC_Connection : public UObject
{
	GENERATED_BODY()

protected:

	SQLHENV SQL_Handle_Environment;
	SQLHDBC SQL_Handle_Connection;

public:

	virtual bool ConnectDatabase(FString& Out_Code, FString& CreatedString, FString TargetServer, FString Username, FString Password);

	UFUNCTION(BlueprintCallable)
	virtual bool SendQuery(FString& Out_Code, UMS_ODBC_Result*& Out_Result, const FString& SQL_Query);
};

UCLASS(BlueprintType)
class FF_DB_MS_ODBC_API UMS_ODBC_Result : public UObject
{
	GENERATED_BODY()

protected:

	TMap<FVector2D, FString> Data_Pool;
	SQLHSTMT* SQL_Handle_Statement;
	int32 RowCount = 0;

public:

	virtual bool SetStatementHandle(FString& Out_Code, SQLHSTMT* In_Handle);
	virtual bool RecordDataToPool(FString& Out_Code);

	UFUNCTION(BlueprintPure)
	virtual int32 GetColumnNumber();

	UFUNCTION(BlueprintCallable)
	virtual bool GetRow(FString& Out_Code, TArray<FString>& Out_Values, int32 RowIndex);

};