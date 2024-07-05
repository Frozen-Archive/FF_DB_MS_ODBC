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

USTRUCT(BlueprintType)
struct FF_DB_MS_ODBC_API FMS_ODBC_MetaData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FString Column_Name;

	UPROPERTY(BlueprintReadOnly)
	int32 NameLenght = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DataType = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DecimalDigits = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsNullable = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Column_Size = 0;
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
	bool bIsResultRecorded = false;

	int32 Count_Column = 0;
	int32 Count_Row = 0;
	int32 Affected_Rows = 0;

public:

	// ADVANCE

	virtual bool GetEachMetaData(FMS_ODBC_MetaData& Out_MetaData, int32 ColumnIndex);
	virtual bool SetStatementHandle(const SQLHSTMT& In_Handle, SQLLEN AffectedRows, SQLSMALLINT ColumnNumber);

	UFUNCTION(BlueprintCallable)
	virtual bool RecordResult(FString& Out_Code);

	/*
	* If you use "Record Result" system in anywhere for this result object, you can't use this function whitout executing query again.
	* You can't use "Record Result" system after this function. Because it will exhaust query result handle. This is ODBC and SQL related limitation.
	* You can use this function only once per query.
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool ParseColumn(FString& Out_Code, TArray<FString>& Out_Values, int32 ColumnIndex = 1);

	// STANDARD

	UFUNCTION(BlueprintPure)
	virtual int32 GetColumnNumber();

	/*
	* You can use this after "Record Result" function.
	*/
	UFUNCTION(BlueprintPure)
	virtual int32 GetRowNumber();

	UFUNCTION(BlueprintPure)
	virtual int32 GetAffectedRows();

	UFUNCTION(BlueprintCallable)
	virtual bool GetRow(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 RowIndex);

	UFUNCTION(BlueprintCallable)
	virtual bool GetColumnFromIndex(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 ColumnIndex);

	UFUNCTION(BlueprintCallable)
	virtual bool GetColumnFromName(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, FString ColumName);

	UFUNCTION(BlueprintCallable)
	virtual bool GetSingleData(FString& Out_Code, FMS_ODBC_DataValue& Out_Value, FVector2D TargetCell);

	UFUNCTION(BlueprintCallable)
	virtual bool GetMetaData(FString& Out_Code, TArray<FMS_ODBC_MetaData>& Out_MetaData);

};