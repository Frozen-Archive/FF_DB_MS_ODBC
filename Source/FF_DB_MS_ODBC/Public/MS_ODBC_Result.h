#pragma once

#include "CoreMinimal.h"

// Custom Includes.
#include "MS_ODBC_Includes.h"

#include "MS_ODBC_Result.generated.h"

USTRUCT(BlueprintType)
struct FF_DB_MS_ODBC_API FMS_ODBC_DataValue
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString String;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	int32 Integer32 = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	int64 Integer64 = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	float Double = (double)0.f;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	bool Boolean = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FDateTime DateTime;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	int32 DataType = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString DataTypeName;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString ColumnName;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString Preview;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	FString Note;

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

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegate_MS_ODBC_Record, bool, IsSuccessfull, FString, Out_Code);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDelegate_MS_ODBC_Fetch, bool, IsSuccessfull, FString, Out_Code, const TArray<FString>&, Out_Values);

UCLASS(BlueprintType)
class FF_DB_MS_ODBC_API UMS_ODBC_Result : public UObject
{
	GENERATED_BODY()

protected:

	FString SentQuery;
	SQLHSTMT SQL_Handle_Statement;

	TMap<FVector2D, FMS_ODBC_DataValue> Data_Pool;
	bool bIsResultRecorded = false;

	int32 Count_Row = 0;
	int32 Count_Column = 0;
	int32 Affected_Rows = 0;

public:

	virtual bool SetQueryResult(const SQLHSTMT& In_Handle, const FString& In_Query);
	virtual bool GetEachMetaData(FMS_ODBC_MetaData& Out_MetaData, int32 ColumnIndex);

	// BLUEPRINT EXPOSED

	UFUNCTION(BlueprintCallable)
	virtual bool Result_Record(FString& Out_Code);
	
	UFUNCTION(BlueprintCallable)
	virtual void Result_Record_Async(FDelegate_MS_ODBC_Record DelegateRecord);

	/*
	* If you use "Record Result" system in anywhere for this result object, you can't use this function whitout executing query again.
	* You can't use "Record Result" system after this function. Because it will exhaust query result handle. This is ODBC and SQL related limitation.
	* You can use this function only once per query.
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool Result_Fetch(FString& Out_Code, TArray<FString>& Out_Values, int32 ColumnIndex = 1);

	/*
	* If you use "Record Result" system in anywhere for this result object, you can't use this function whitout executing query again.
	* You can't use "Record Result" system after this function. Because it will exhaust query result handle. This is ODBC and SQL related limitation.
	* You can use this function only once per query.
	*/
	UFUNCTION(BlueprintCallable)
	virtual void Result_Fetch_Async(FDelegate_MS_ODBC_Fetch DelegateFetch, int32 ColumnIndex = 1);

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
	virtual bool GetRow(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 Index_Row);

	UFUNCTION(BlueprintCallable)
	virtual bool GetColumnFromIndex(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 Index_Column);

	UFUNCTION(BlueprintCallable)
	virtual bool GetColumnFromName(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, FString ColumName);

	UFUNCTION(BlueprintCallable)
	virtual bool GetSingleData(FString& Out_Code, FMS_ODBC_DataValue& Out_Value, FVector2D Position);

	UFUNCTION(BlueprintCallable)
	virtual bool GetMetaData(FString& Out_Code, TArray<FMS_ODBC_MetaData>& Out_MetaData);

};