// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

// Custom Includes
#include "Objects/MS_ODBC_Connection.h"

#include "MS_ODBC_BPLibrary.generated.h"

UCLASS()
class UMS_ODBC_BPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, meta = (DisplayName = "MS - INSERT INTO TEMPLATE", Keywords = "odbc, ms, microsoft, statement, template, query, insert, into"), Category = "Frozen Forest|Database|Microsoft")
	static FString MS_Insert_Into(FString TargetDb, TMap<FString, FString> KeyValue);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "MS - SELECT FROM TEMPLATE", Keywords = "odbc, ms, microsoft, statement, template, query, select, from"), Category = "Frozen Forest|Database|Microsoft")
	static FString MS_Select_From(FString TargetDb, FString Condition, TSet<FString> PlaceHolders);

};