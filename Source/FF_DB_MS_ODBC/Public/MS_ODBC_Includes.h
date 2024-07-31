#pragma once

#define SQL_MAX_TEXT_LENGHT 65535

// UE Includes.
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

#include "JsonObjectWrapper.h"
#include "JsonUtilities.h"

#include "Containers/Queue.h"

THIRD_PARTY_INCLUDES_START

#define WIN32_LEAN_AND_MEAN
#include "Windows/AllowWindowsPlatformTypes.h"
#include <sqlext.h>
#include "Windows/HideWindowsPlatformTypes.h"

#include <string>

THIRD_PARTY_INCLUDES_END