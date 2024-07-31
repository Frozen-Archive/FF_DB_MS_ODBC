// Fill out your copyright notice in the Description page of Project Settings.

#include "MS_ODBC_Manager.h"

// Sets default values
AMS_ODBC_Manager::AMS_ODBC_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMS_ODBC_Manager::BeginPlay()
{
	Super::BeginPlay();
}

void AMS_ODBC_Manager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AMS_ODBC_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AMS_ODBC_Manager::SetConnectionId(FString In_Id)
{
	if (!this->ConnectionId.IsEmpty())
	{
		return false;
	}

	this->ConnectionId = In_Id;
	return true;
}

bool AMS_ODBC_Manager::ConnectDatabase(FString& Out_Code, FString& CreatedString, FString ODBC_Name, FString Username, FString Password, FString ServerInstance)
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

	SQLRETURN RetCode = SQLAllocEnv(&this->SQL_Handle_Environment);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : Failed to allocate SQL environment";
		return false;
	}

	RetCode = SQLSetEnvAttr(this->SQL_Handle_Environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : Failed to set the ODBC version.";
		SQLFreeHandle(SQL_HANDLE_ENV, this->SQL_Handle_Environment);
		return false;
	}

	RetCode = SQLAllocConnect(this->SQL_Handle_Environment, &this->SQL_Handle_Connection);
	if (!SQL_SUCCEEDED(RetCode))
	{
		Out_Code = "FF Microsoft ODBC : Failed to allocate a connection handle.";
		SQLFreeHandle(SQL_HANDLE_ENV, this->SQL_Handle_Environment);
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

void AMS_ODBC_Manager::CreateConnection(FDelegate_MS_ODBC_Connection DelegateConnection, FString TargetServer, FString Username, FString Password, FString ServerInstance)
{
	if (TargetServer.IsEmpty())
	{
		DelegateConnection.ExecuteIfBound(false, "FF Microsoft ODBC : Target server shouldn't be empty !", "");
		return;
	}

	if (Username.IsEmpty())
	{
		DelegateConnection.ExecuteIfBound(false, "FF Microsoft ODBC : Username shouldn't be empty !", "");
		return;
	}

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection, TargetServer, Username, Password, ServerInstance]()
		{
			const FString ConnectionId = TargetServer + "&&" + Username;

			if (!this->SetConnectionId(ConnectionId))
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateConnection]()
					{
						DelegateConnection.ExecuteIfBound(false, "FF Microsoft ODBC : Connection ID shouldn't be emptyh !", "");
					}
				);

				return;
			}

			FString Out_Code;
			FString CreatedString;

			if (!this->ConnectDatabase(Out_Code, CreatedString, TargetServer, Username, Password, ServerInstance))
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, Out_Code, CreatedString]()
					{
						DelegateConnection.ExecuteIfBound(false, Out_Code, CreatedString);
					}
				);

				return;
			}
			
			AsyncTask(ENamedThreads::GameThread, [DelegateConnection, Out_Code, CreatedString]()
				{
					DelegateConnection.ExecuteIfBound(true, Out_Code, CreatedString);
				}
			);
		}
	);
}

FString AMS_ODBC_Manager::GetConnectionId()
{
	return this->ConnectionId;
}

bool AMS_ODBC_Manager::SendQuery(FString& Out_Code, UMS_ODBC_Result*& Out_Result, const FString& SQL_Query, bool bRecordResults)
{
	if (SQL_Query.IsEmpty())
	{
		return false;
	}

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

	if (!ResultObject->SetQueryResult(Temp_Handle, SQL_Query))
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

void AMS_ODBC_Manager::SendQueryAsync(FDelegate_MS_ODBC_Execute DelegateExecute, const FString& SQL_Query, bool bRecordResults)
{
	UMS_ODBC_Result* Out_Result = NewObject<UMS_ODBC_Result>();
	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateExecute, &Out_Result, SQL_Query, bRecordResults]()
		{
			FString Out_Code;

			if (this->SendQuery(Out_Code, Out_Result, SQL_Query, bRecordResults))
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateExecute, Out_Code, Out_Result]()
					{
						DelegateExecute.ExecuteIfBound(true, Out_Code, Out_Result);
					}
				);

				return;
			}

			else
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateExecute, Out_Code]()
					{
						DelegateExecute.ExecuteIfBound(false, Out_Code, nullptr);
					}
				);

				return;
			}
		}
	);
}