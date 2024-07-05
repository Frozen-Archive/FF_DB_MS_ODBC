#include "MS_ODBC_Connection.h"

// UE Includes.
#include "Kismet/KismetMathLibrary.h"

#define SQL_MAX_TEXT_LENGHT 65535

// CONNECTION

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

    SQLRETURN RetCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &SQL_Handle_Environment);
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

    RetCode = SQLAllocHandle(SQL_HANDLE_DBC, SQL_Handle_Environment, &this->SQL_Handle_Connection);
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
    RetCode = SQLAllocHandle(SQL_HANDLE_STMT, this->SQL_Handle_Connection, &Temp_Handle);

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

    FString RetCode_SetStatement;
    UMS_ODBC_Result* TempResultObject = NewObject<UMS_ODBC_Result>();

    if (!TempResultObject->SetStatementHandle(RetCode_SetStatement, Temp_Handle, bRecordResults))
    {
        Out_Code = "FF Microsoft ODBC : Query executed successfully but there was a problem to set query result ! : " + RetCode_SetStatement;
        return false;
    }

    Out_Result = TempResultObject;
    Out_Code = "FF Microsoft ODBC : Query executed and result object created successfully !";
    return true;
}

// RESULT - ADVANCE

bool UMS_ODBC_Result::SetStatementHandle(FString& Out_Code, const SQLHSTMT& In_Handle, bool bRecordResults)
{
    if (!In_Handle)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    SQLSMALLINT Temp_Count_Column = 0;
    SQLRETURN RetCode = SQLNumResultCols(In_Handle, &Temp_Count_Column);

    if (!SQL_SUCCEEDED(RetCode))
    {
        Out_Code = "FF Microsoft ODBC : There was a problem while counting columns !";
        return false;
    }

    this->Count_Column = Temp_Count_Column;
    this->SQL_Handle_Statement = In_Handle;

    if (bRecordResults)
    {
        FString RecordCode;

        if (!this->RecordResult(RecordCode))
        {
            Out_Code = "FF Microsoft ODBC : Result record problem : " + RecordCode;
            return false;
        }
    }

    Out_Code = "FF Microsoft ODBC : Statement set successfully !";
    return true;
}

bool UMS_ODBC_Result::RecordResult(FString& Out_Code)
{
    TMap<FVector2D, FMS_ODBC_DataValue> Temp_Data_Pool;
    int32 Temp_Count_Row = 0;

    try
    {
        while (SQLFetch(this->SQL_Handle_Statement) == SQL_SUCCESS)
        {
            for (int32 Index_Column_Raw = 0; Index_Column_Raw < this->Count_Column; Index_Column_Raw++)
            {
                const int32 Index_Column = Index_Column_Raw + 1;

                SQLCHAR Value[SQL_MAX_TEXT_LENGHT];
                SQLLEN indicator;
                FString ColumnValue;
                SQLRETURN RetCode = SQLGetData(this->SQL_Handle_Statement, Index_Column, SQL_CHAR, Value, sizeof(Value), &indicator);

                if (SQL_SUCCEEDED(RetCode))
                {
                    FString EachValueString = UTF8_TO_TCHAR((const char*)Value);
                    EachValueString.TrimEndInline();

                    FMS_ODBC_DataValue EachData;
                    EachData.ValueRepresentation = EachValueString;

                    Temp_Data_Pool.Add(FVector2D(Temp_Count_Row, Index_Column_Raw), EachData);
                }
            }

            Temp_Count_Row += 1;
        }
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return false;
    }

    this->Count_Row = Temp_Count_Row;
    this->Data_Pool = Temp_Data_Pool;

    return true;
}

bool UMS_ODBC_Result::ParseColumn(FString& Out_Code, TArray<FString>& Out_Values, int32 ColumnNumber)
{
    if (ColumnNumber < 1)
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
        int32 TempRowCount = 0;
        while (SQLFetch(this->SQL_Handle_Statement) == SQL_SUCCESS)
        {
            SQLCHAR Value[SQL_MAX_TEXT_LENGHT];
            SQLLEN indicator;
            FString ColumnValue;
            SQLGetData(this->SQL_Handle_Statement, ColumnNumber, SQL_CHAR, Value, sizeof(Value), &indicator);

            FString EachData = UTF8_TO_TCHAR((const char*)Value);
            EachData.TrimEndInline();
            Array_Temp.Add(EachData);

            TempRowCount += 1;
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

bool UMS_ODBC_Result::GetRow(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 RowIndex)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "Data pool is empty !";
        return false;
    }

    if (RowIndex < 0 && this->Count_Row >= RowIndex)
    {
        Out_Code = "Given row index is out of data pool's range !";
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