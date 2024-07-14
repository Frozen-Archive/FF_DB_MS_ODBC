#include "Objects/MS_ODBC_Connection.h"

bool UMS_ODBC_Connection::SetConnectionId(FString In_Id)
{
    if (!this->ConnectionId.IsEmpty())
    {
        return false;
    }

    this->ConnectionId = In_Id;
    return true;
}

bool UMS_ODBC_Connection::ConnectDatabase(FString& Out_Code, FString& CreatedString, FString ODBC_Name, FString Username, FString Password, FString ServerInstance)
{
    if (ODBC_Name.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Target server shouldn't be empty !";
        return false;
    }

    if (Username.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Username shouldn't be empty !";
        return false;
    }

    if (ServerInstance.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Server instance shouldn't be empty !";
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

    CreatedString = "{SQL Server};SERVER=" + ODBC_Name + "\\" + ServerInstance + ";DSN=" + ODBC_Name + ";UID=" + Username + ";PWD=" + Password;
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

    UMS_ODBC_Result* ResultObject = NewObject<UMS_ODBC_Result>();

    if (!ResultObject->SetQueryResult(Temp_Handle))
    {
        Out_Code = "FF Microsoft ODBC : Query executed successfully but return handle is invalid !";
        return false;
    }

    if (bRecordResults)
    {
        FString RecordResultCode;
        if (!ResultObject->Result_Record(RecordResultCode))
        {
            Out_Code = "FF Microsoft ODBC : Query executed successfully but there was a problem while recording result to the pool : " + UKismetStringLibrary::ParseIntoArray(RecordResultCode, " : ")[1];
            return false;
        }  
    }  

    Out_Result = ResultObject;
    Out_Code = "FF Microsoft ODBC : Query executed and result object created successfully !";
    return true;
}