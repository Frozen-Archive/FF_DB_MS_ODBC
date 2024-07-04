// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

// Custom Includes
#include "MS_ODBC_Connection.h"

#include "MS_ODBC_BPLibrary.generated.h"

UCLASS()
class UMS_ODBC_BPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, meta = (DisplayName = "NANODBC - INSERT INTO TEMPLATE", Keywords = "odbc, nanodbc, statement, template, query, insert, into"), Category = "Frozen Forest|Database|Microsoft")
	static FString MS_Insert_Into(FString Target, TMap<FString, FString> KeyValue);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "NANODBC - SELECT FROM TEMPLATE", Keywords = "odbc, nanodbc, statement, template, query, select, from"), Category = "Frozen Forest|Database|Microsoft")
	static FString MS_Select_From(FString Target, FString Where, TSet<FString> PlaceHolders);

};