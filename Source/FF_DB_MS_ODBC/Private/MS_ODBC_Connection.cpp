#include "MS_ODBC_Connection.h"

// UE Includes.
#include "Kismet/KismetMathLibrary.h"

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

bool UMS_ODBC_Connection::SendQuery(FString& Out_Code, UMS_ODBC_Result*& Out_Result, const FString& SQL_Query)
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

    SQLSMALLINT ColumnCount = 0;
    SQLNumResultCols(Temp_Handle, &ColumnCount);
    Out_Code = "FF Microsoft ODBC : Query executed sucessfully. Found column number : " + FString::FromInt(ColumnCount);

    return true;
}

bool UMS_ODBC_Result::SetStatementHandle(FString& Out_Code, SQLHSTMT* In_Handle)
{
    if (!In_Handle)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    this->SQL_Handle_Statement = In_Handle;

    FString RecordError;
    bool bRecordResult = this->RecordDataToPool(RecordError);

    Out_Code = bRecordResult ? "FF Microsoft ODBC : Statement is set !" : RecordError;
    return bRecordResult;
}

int32 UMS_ODBC_Result::GetColumnNumber()
{
    if (!this->SQL_Handle_Statement)
    {
        return 0;
    }

    SQLSMALLINT ColumnCount = 0;
    if (SQL_SUCCESS == SQLNumResultCols(this->SQL_Handle_Statement, &ColumnCount))
    {
        return (int32)ColumnCount;
    }

    else
    {
        return 0;
    }
}

bool UMS_ODBC_Result::RecordDataToPool(FString& Out_Code)
{
    if (!this->SQL_Handle_Statement)
    {
        return false;
    }

    const int32 ColumnCount = this->GetColumnNumber();

    if (ColumnCount == 0)
    {
        return false;
    }

    try
    {
        TMap<FVector2D, FString> All_Data;

        int32 TempRowCount = 0;
        while (SQLFetch(this->SQL_Handle_Statement))
        {
            for (int32 Index_Column = 0; Index_Column < ColumnCount; Index_Column++)
            {
                SQLCHAR* ValuePointer = (SQLCHAR*)malloc(64);
                SQLGetData(SQL_Handle_Statement, Index_Column, SQL_CHAR, ValuePointer, 64, NULL);

                FString EachString = UTF8_TO_TCHAR((const char*)ValuePointer);
                All_Data.Add(FVector2D(TempRowCount, Index_Column), EachString);
            }

            TempRowCount += 1;
            GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "New Row");
        }

        this->Data_Pool = All_Data;
        this->RowCount = TempRowCount;
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return false;
    }

    return true;
}

bool UMS_ODBC_Result::GetRow(FString& Out_Code, TArray<FString>& Out_Values, int32 RowIndex)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "Data pool is empty !";
        return false;
    }

    if (RowIndex < 0 && this->RowCount >= RowIndex)
    {
        Out_Code = "Given row index is out of data pool's range !";
        return false;
    }

    const int32 ColumnsCount = this->GetColumnNumber();
    TArray<FString> Temp_Array;

    for (int32 Index_Column = 0; Index_Column < ColumnsCount; Index_Column++)
    {
        Temp_Array.Add(*this->Data_Pool.Find(FVector2D(RowIndex, Index_Column)));
    }

    Out_Values = Temp_Array;
    return true;
}