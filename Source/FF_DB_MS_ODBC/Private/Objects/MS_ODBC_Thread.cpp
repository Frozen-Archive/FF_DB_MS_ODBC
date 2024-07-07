// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/MS_ODBC_Thread.h"
#include "MS_ODBC_Manager.h"

// UE Includes.
#include "Async/Async.h"

FDB_Thread_MS_ODBC::FDB_Thread_MS_ODBC(AMS_ODBC_Manager* In_Parent_Actor)
{
	this->Parent_Actor = In_Parent_Actor;

	this->RunnableThread = FRunnableThread::Create(this, TEXT("Test"));
}

FDB_Thread_MS_ODBC::~FDB_Thread_MS_ODBC()
{
	if (this->RunnableThread)
	{
		this->RunnableThread->Kill(true);
		delete this->RunnableThread;
	}
}

bool FDB_Thread_MS_ODBC::Init()
{
	this->bStartThread = true;

	return true;
}

uint32 FDB_Thread_MS_ODBC::Run()
{
	while (this->bStartThread)
	{

	}

	return 0;
}

void FDB_Thread_MS_ODBC::Stop()
{
	this->bStartThread = false;
}

bool FDB_Thread_MS_ODBC::Toggle(bool bIsPause)
{
	if (!this->RunnableThread)
	{
		return false;
	}

	this->RunnableThread->Suspend(bIsPause);

	return true;
}