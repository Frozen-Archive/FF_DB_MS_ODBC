// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// UE Includes.
#include "HAL/Runnable.h"

// Custom Includes.
#include "MS_ODBC_Connection.h"

// Fordward Declerations.
class FRunnableThread;
class AMS_ODBC_Manager;

class FF_DB_MS_ODBC_API FDB_Thread_MS_ODBC : public FRunnable
{

public:

	// Sets default values for this actor's properties
	FDB_Thread_MS_ODBC(AMS_ODBC_Manager* In_Parent_Actor);

	// Destructor.
	virtual ~FDB_Thread_MS_ODBC() override;

	virtual bool Init() override;

	virtual uint32 Run() override;

	virtual void Stop() override;

	virtual bool Toggle(bool bIsPause);

	AMS_ODBC_Manager* Parent_Actor = nullptr;

private:

	bool bStartThread = false;
	FRunnableThread* RunnableThread = nullptr;

};