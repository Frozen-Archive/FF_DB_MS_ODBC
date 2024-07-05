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
	virtual bool SendQuery(FString& Out_Code, UMS_ODBC_Result*& Out_Result, const FString& SQL_Query, bool bRecordResults);
};

UCLASS(BlueprintType)
class FF_DB_MS_ODBC_API UMS_ODBC_Result : public UObject
{
	GENERATED_BODY()

protected:

	SQLHSTMT SQL_Handle_Statement;

	TMap<FVector2D, FMS_ODBC_DataValue> Data_Pool;
	int32 Count_Column = 0;
	int32 Count_Row = 0;

public:

	// ADVANCE

	virtual bool SetStatementHandle(FString& Out_Code, const SQLHSTMT& In_Handle, bool bRecordResults);

	UFUNCTION(BlueprintCallable, meta = (ToolTip = ""))
	virtual bool RecordResult(FString& Out_Code);

	UFUNCTION(BlueprintCallable, meta = (ToolTip = ""))
	virtual bool ParseColumn(FString& Out_Code, TArray<FString>& Out_Values, int32 ColumnNumber = 1);

	// STANDARD

	UFUNCTION(BlueprintPure)
	virtual int32 GetColumnNumber();

	UFUNCTION(BlueprintPure)
	virtual int32 GetRowNumber();

	UFUNCTION(BlueprintCallable)
	virtual bool GetRow(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 RowIndex);

};