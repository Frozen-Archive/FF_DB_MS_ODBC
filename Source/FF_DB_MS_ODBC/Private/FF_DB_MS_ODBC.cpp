// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF_DB_MS_ODBC.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FFF_DB_MS_ODBCModule"

void FFF_DB_MS_ODBCModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FFF_DB_MS_ODBCModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFF_DB_MS_ODBCModule, FFF_DB_MS_ODBC)