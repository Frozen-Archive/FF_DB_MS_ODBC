#include "MS_ODBC_Connection.h"

// UE Includes.
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

#define SQL_MAX_TEXT_LENGHT 65535

// CONNECTION

bool UMS_ODBC_Connection::SetConnectionId(FString In_Id)
{
    if (!this->ConnectionId.IsEmpty())
    {
        return false;
    }

    this->ConnectionId = In_Id;
    return true;
}

bool UMS_ODBC_Connection::ConnectDatabase(FString& Out_Code, FString& CreatedString, FString TargetServer, FString Username, FString Password)
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

    SQLRETURN RetCode = SQLAllocEnv(&SQL_Handle_Environment);
    if (!SQL_SUCCEEDED(RetCode))
    {
        Out_Code = "FF Microsoft ODBC : Failed to allocate SQL environment";
        return false;
    }

    RetCode = SQLSetEnvAttr(SQL_Handle_Environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);
    if (!SQL_SUCCEEDED(RetCode))
    {
        Out_Code = "FF Microsoft ODBC : Failed to set the ODBC version.";
        SQLFreeHandle(SQL_HANDLE_ENV, SQL_Handle_Environment);
        return false;
    }

    RetCode = SQLAllocConnect(SQL_Handle_Environment, &this->SQL_Handle_Connection);
    if (!SQL_SUCCEEDED(RetCode))
    {
        Out_Code = "FF Microsoft ODBC : Failed to allocate a connection handle.";
        SQLFreeHandle(SQL_HANDLE_ENV, SQL_Handle_Environment);
        return false;
    }

    CreatedString = "{SQL Server};SERVER=" + TargetServer + "\\SQLEXPRESS;DSN=" + TargetServer + ";UID=" + Username + ";PWD=" + Password;
    RetCode = SQLDriverConnectA(this->SQL_Handle_Connection, NULL, (SQLCHAR*)TCHAR_TO_UTF8(*CreatedString), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    if (!SQL_SUCCEEDED(RetCode))
    {
        SQLFreeHandle(SQL_HANDLE_DBC, this->SQL_Handle_Connection);
        SQLFreeHandle(SQL_HANDLE_ENV, this->SQL_Handle_Environment);

        Out_Code = "FF Microsoft ODBC : Connection couldn't made !";
        return false;
    }

    Out_Code = "FF Microsoft ODBC : Connection successfully established !";
    return true;
}

bool UMS_ODBC_Connection::SendQuery(FString& Out_Code, UMS_ODBC_Result*& Out_Result, const FString& SQL_Query, bool bRecordResults)
{
    if (!this->SQL_Handle_Connection)
    {
        Out_Code = "FF Microsoft ODBC : Connection handle is not valid !";
        return false;
    }

    SQLRETURN RetCode;

    SQLHSTMT Temp_Handle;
    RetCode = SQLAllocStmt(this->SQL_Handle_Connection, &Temp_Handle);

    if (!SQL_SUCCEEDED(RetCode))
    {
        Out_Code = "FF Microsoft ODBC : There was a problem while allocating statement handle : " + FString::FromInt(RetCode);
        return false;
    }

    const TCHAR* statementStringChar = *SQL_Query;
    SQLWCHAR* SQLWCHARStatementString = (SQLWCHAR*)statementStringChar;
    RetCode = SQLPrepare(Temp_Handle, SQLWCHARStatementString, SQL_NTS);

    if (!SQL_SUCCEEDED(RetCode))
    {
        Out_Code = "FF Microsoft ODBC : There was a problem while preparing statement : " + FString::FromInt(RetCode);
        return false;
    }

    RetCode = SQLExecute(Temp_Handle);

    if (!SQL_SUCCEEDED(RetCode))
    {
        Out_Code = "FF Microsoft ODBC : There was a problem while executing query : " + FString::FromInt(RetCode);
        return false;
    }

    SQLLEN AffectedRows;
    RetCode = SQLRowCount(Temp_Handle, &AffectedRows);

    SQLSMALLINT ColumnNumber;
    RetCode = SQLNumResultCols(Temp_Handle, &ColumnNumber);

    UMS_ODBC_Result* ResultObject = NewObject<UMS_ODBC_Result>();

    if (!ResultObject->SetStatementHandle(Temp_Handle, AffectedRows, ColumnNumber))
    {
        Out_Code = "FF Microsoft ODBC : Query executed successfully but return handle is invalid !";
        return false;
    }

    if (bRecordResults)
    {
        FString RecordResultCode;
        if (!ResultObject->RecordResult(RecordResultCode))
        {
            Out_Code = "FF Microsoft ODBC : Query executed successfully but there was a problem while recording result to the pool : " + UKismetStringLibrary::ParseIntoArray(RecordResultCode, " : ")[1];
            return false;
        }  
    }

    Out_Result = ResultObject;
    Out_Code = "FF Microsoft ODBC : Query executed and result object created successfully !";
    return true;
}

// RESULT - ADVANCE

bool UMS_ODBC_Result::SetStatementHandle(const SQLHSTMT& In_Handle, SQLLEN AffectedRows, SQLSMALLINT ColumnNumber)
{
    if (!In_Handle)
    {
        return false;
    }

    this->SQL_Handle_Statement = In_Handle;
    this->Affected_Rows = AffectedRows;
    this->Count_Column = ColumnNumber;

    return true;
}

bool UMS_ODBC_Result::GetEachMetaData(FMS_ODBC_MetaData& Out_MetaData, int32 ColumnIndex)
{
    SQLCHAR Column_Name[256];
    SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
    SQLULEN Column_Size;

    SQLRETURN RetCode = SQLDescribeColA(this->SQL_Handle_Statement, ColumnIndex, Column_Name, 256, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

    if (!SQL_SUCCEEDED(RetCode))
    {
        return false;
    }

    FString Column_Name_String = UTF8_TO_TCHAR((const char*)Column_Name);
    Column_Name_String.TrimEndInline();

    FMS_ODBC_MetaData EachMetaData;
    EachMetaData.Column_Name = Column_Name_String;
    EachMetaData.NameLenght = NameLen;
    EachMetaData.DataType = DataType;
    EachMetaData.DecimalDigits = DecimalDigits;
    EachMetaData.bIsNullable = Nullable == 1 ? true : false;
    EachMetaData.Column_Size = Column_Size;

    Out_MetaData = EachMetaData;
    return true;
}

bool UMS_ODBC_Result::RecordResult(FString& Out_Code)
{
    if (this->bIsResultRecorded)
    {
        Out_Code = "FF Microsoft ODBC : Result already recorded.";
        return false;
    }

    if (!this->SQL_Handle_Statement)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    TMap<FVector2D, FMS_ODBC_DataValue> Temp_Data_Pool;
    int32 Index_Row = 0;

    try
    {
        bool bIsMetaDataCollected = false;
        TArray<FMS_ODBC_MetaData> Array_MetaData;

        while (SQLFetch(this->SQL_Handle_Statement) == SQL_SUCCESS)
        {
            for (int32 Index_Column_Raw = 0; Index_Column_Raw < this->Count_Column; Index_Column_Raw++)
            {
                const int32 Index_Column = Index_Column_Raw + 1;

                if (!bIsMetaDataCollected)
                {
                    FMS_ODBC_MetaData EachMetaData;
                    if (this->GetEachMetaData(EachMetaData, Index_Column))
                    {
                        Array_MetaData.Add(EachMetaData);
                    }  
                }

                SQLLEN ReceivedLenght;
                SQLCHAR* TempData = (SQLCHAR*)malloc(SQL_MAX_TEXT_LENGHT);
                SQLRETURN RetCode = SQL_ERROR;
                
                RetCode = SQLGetData(this->SQL_Handle_Statement, Index_Column, SQL_CHAR, TempData, SQL_MAX_TEXT_LENGHT, &ReceivedLenght);
             
                if (SQL_SUCCEEDED(RetCode))
                {
                    FString EachValueString;
                    EachValueString.AppendChars((const char*)TempData, ReceivedLenght);
                    EachValueString.TrimEndInline();

                    FMS_ODBC_MetaData EachMetaData = Array_MetaData[Index_Column_Raw];

                    FMS_ODBC_DataValue EachData;
                    EachData.Preview = EachValueString;
                    EachData.ColumnName = EachMetaData.Column_Name;
                    EachData.DataType = EachMetaData.DataType;

                    Temp_Data_Pool.Add(FVector2D(Index_Row, Index_Column_Raw), EachData);
                    
                    free(TempData);
                    TempData = nullptr;
                }
            }

            bIsMetaDataCollected = true;
            Index_Row += 1;
        }
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return false;
    }

    this->Data_Pool = Temp_Data_Pool;
    this->Count_Row = Index_Row;
    this->bIsResultRecorded = true;
   
    Out_Code = "FF Microsoft ODBC : Result successfuly recorded !";
    return true;
}

bool UMS_ODBC_Result::ParseColumn(FString& Out_Code, TArray<FString>& Out_Values, int32 ColumnIndex)
{
    if (ColumnIndex < 1)
    {
        Out_Code = "FF Microsoft ODBC : Column index starts from 1 !";
        return false;
    }

    if (!this->SQL_Handle_Statement)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    TArray<FString> Array_Temp;

    try
    {
        while (SQLFetch(this->SQL_Handle_Statement) == SQL_SUCCESS)
        {
            SQLLEN ReceivedLenght;
            SQLCHAR* TempData = (SQLCHAR*)malloc(SQL_MAX_TEXT_LENGHT);
            SQLGetData(this->SQL_Handle_Statement, ColumnIndex, SQL_CHAR, TempData, SQL_MAX_TEXT_LENGHT, &ReceivedLenght);

            FString EachData;
            EachData.AppendChars((const char*)TempData, ReceivedLenght);
            EachData.TrimEndInline();
            Array_Temp.Add(EachData);

            free(TempData);
            TempData = nullptr;
        }
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return false;
    }

    Out_Values = Array_Temp;
    Out_Code = "FF Microsoft ODBC : Parsing completed successfully !";
    return true;
}

// RESULT - STANDARD

int32 UMS_ODBC_Result::GetColumnNumber()
{
    return this->Count_Column;
}

int32 UMS_ODBC_Result::GetRowNumber()
{
    return this->Count_Row;
}

int32 UMS_ODBC_Result::GetAffectedRows()
{
    return this->Affected_Rows;
}

bool UMS_ODBC_Result::GetRow(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 RowIndex)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (RowIndex < 0 && this->Count_Row >= RowIndex)
    {
        Out_Code = "FF Microsoft ODBC : Given row index is out of data pool's range !";
        return false;
    }

    TArray<FMS_ODBC_DataValue> Temp_Array;

    for (int32 Index_Column = 0; Index_Column < this->Count_Column; Index_Column++)
    {
        Temp_Array.Add(*this->Data_Pool.Find(FVector2D(RowIndex, Index_Column)));
    }

    Out_Values = Temp_Array;
    return true;
}

bool UMS_ODBC_Result::GetColumnFromIndex(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 ColumnIndex)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (ColumnIndex < 0 && this->Count_Column >= ColumnIndex)
    {
        Out_Code = "FF Microsoft ODBC : Given column index is out of data pool's range !";
        return false;
    }

    TArray<FMS_ODBC_DataValue> Temp_Array;

    for (int32 Index_Row = 0; Index_Row < this->Count_Row; Index_Row++)
    {
        Temp_Array.Add(*this->Data_Pool.Find(FVector2D(Index_Row, ColumnIndex)));
    }

    Out_Values = Temp_Array;
    return true;
}

bool UMS_ODBC_Result::GetColumnFromName(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, FString ColumName)
{
    if (this->Count_Column == 0)
    {
        return false;
    }

    int32 TargetIndex = 0;
    for (int32 Index_Column_Raw = 0; Index_Column_Raw < this->Count_Column; Index_Column_Raw++)
    {
        const int32 Index_Column = Index_Column_Raw + 1;
        SQLCHAR Column_Name[256];
        SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
        SQLULEN Column_Size;

        SQLRETURN RetCode = SQLDescribeColA(this->SQL_Handle_Statement, Index_Column, Column_Name, 256, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

        if (!SQL_SUCCEEDED(RetCode))
        {
            Out_Code = "FF Microsoft ODBC : There was a problem while getting index of target column !";
            return false;
        }

        FString EachName = UTF8_TO_TCHAR((const char*)Column_Name);
        EachName.TrimEndInline();
        
        if (EachName == ColumName)
        {
            TargetIndex = Index_Column_Raw;
            break;
        }
    }

    return this->GetColumnFromIndex(Out_Code, Out_Values, TargetIndex);
}

bool UMS_ODBC_Result::GetSingleData(FString& Out_Code, FMS_ODBC_DataValue& Out_Value, FVector2D TargetCell)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (TargetCell.X < 0 || TargetCell.Y < 0)
    {
        Out_Code = "FF Microsoft ODBC : Target position shouldn't be negative !";
        return false;
    }

    if (TargetCell.X >= this->Count_Row || TargetCell.Y >= this->Count_Column)
    {
        Out_Code = "FF Microsoft ODBC : Given position is out of data pool's range !";
        return false;
    }

    FMS_ODBC_DataValue* DataValue = this->Data_Pool.Find(TargetCell);

    if (!DataValue)
    {
        Out_Code = "FF Microsoft ODBC : Data couldn't be found !";
        return false;
    }

    Out_Value = *DataValue;
    return true;
}

bool UMS_ODBC_Result::GetMetaData(FString& Out_Code, TArray<FMS_ODBC_MetaData>& Out_MetaData)
{
    if (!this->SQL_Handle_Statement)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    SQLSMALLINT Temp_Count_Column = 0;
    SQLRETURN RetCode = SQLNumResultCols(this->SQL_Handle_Statement, &Temp_Count_Column);

    if (Temp_Count_Column == 0)
    {
        Out_Code = "FF Microsoft ODBC : There is no column to get metadata !";
        return false;
    }

    TArray<FMS_ODBC_MetaData> Array_MetaData;

    for (int32 Index_Column_Raw = 0; Index_Column_Raw < Temp_Count_Column; Index_Column_Raw++)
    {
        FMS_ODBC_MetaData EachMetaData;
        if (this->GetEachMetaData(EachMetaData, Index_Column_Raw + 1))
        {
            Array_MetaData.Add(EachMetaData);
        }
    }

    Out_MetaData = Array_MetaData;
    Out_Code = "FF Microsoft ODBC : All metadata got successfully !";
    return true;
}