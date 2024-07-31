#include "MS_ODBC_Result.h"

bool UMS_ODBC_Result::SetQueryResult(const SQLHSTMT& In_Handle, const FString& In_Query)
{
    if (!In_Handle)
    {
        return false;
    }

    SQLLEN AffectedRows;
    SQLRowCount(In_Handle, &AffectedRows);

    SQLSMALLINT ColumnNumber;
    SQLNumResultCols(In_Handle, &ColumnNumber);

    this->Affected_Rows = AffectedRows;
    this->Count_Column = ColumnNumber;
    this->SQL_Handle_Statement = In_Handle;
    this->SentQuery = In_Query;

    return true;
}

bool UMS_ODBC_Result::GetEachMetaData(FMS_ODBC_MetaData& Out_MetaData, int32 ColumnIndex)
{
    if (!this->SQL_Handle_Statement)
    {
        return false;
    }

    if (this->Count_Column == 0)
    {
        return false;
    }

    SQLCHAR Column_Name[256];
    SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
    SQLULEN Column_Size;

    SQLRETURN RetCode = SQLDescribeColA(this->SQL_Handle_Statement, ColumnIndex, Column_Name, 256, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

    if (!SQL_SUCCEEDED(RetCode))
    {
        return false;
    }

    FString Column_Name_String = UTF8_TO_TCHAR((const char*)Column_Name);
    Column_Name_String.TrimEndInline();

    FMS_ODBC_MetaData EachMetaData;
    EachMetaData.Column_Name = Column_Name_String;
    EachMetaData.NameLenght = NameLen;
    EachMetaData.DataType = DataType;
    EachMetaData.DecimalDigits = DecimalDigits;
    EachMetaData.bIsNullable = Nullable == 1 ? true : false;
    EachMetaData.Column_Size = Column_Size;

    Out_MetaData = EachMetaData;
    return true;
}

// BLUEPRINT EXPOSED

bool UMS_ODBC_Result::Result_Record(FString& Out_Code)
{
    if (this->bIsResultRecorded)
    {
        Out_Code = "FF Microsoft ODBC : Result already recorded.";
        return false;
    }

    if (!this->SQL_Handle_Statement)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    if (this->Count_Column == 0)
    {
        Out_Code = "FF Microsoft ODBC : This query doesn't have result. It is for update only !";
        return false;
    }

    TMap<FVector2D, FMS_ODBC_DataValue> Temp_Data_Pool;
    int32 Index_Row = 0;

    try
    {
        bool bIsMetaDataCollected = false;
        TArray<FMS_ODBC_MetaData> Array_MetaData;

        while (SQLFetch(this->SQL_Handle_Statement) == SQL_SUCCESS)
        {
            for (int32 Index_Column_Raw = 0; Index_Column_Raw < this->Count_Column; Index_Column_Raw++)
            {
                const FVector2D Position = FVector2D(Index_Column_Raw, Index_Row);
                const int32 Index_Column = Index_Column_Raw + 1;

                if (!bIsMetaDataCollected)
                {
                    FMS_ODBC_MetaData EachMetaData;
                    if (this->GetEachMetaData(EachMetaData, Index_Column))
                    {
                        Array_MetaData.Add(EachMetaData);
                    }
                }

                SQLLEN PreviewLenght;
                SQLCHAR* PreviewData = (SQLCHAR*)malloc(SQL_MAX_TEXT_LENGHT);
                SQLRETURN RetCode = SQLGetData(this->SQL_Handle_Statement, Index_Column, SQL_CHAR, PreviewData, SQL_MAX_TEXT_LENGHT, &PreviewLenght);

                if (!SQL_SUCCEEDED(RetCode))
                {
                    free(PreviewData);
                    PreviewData = nullptr;

                    Out_Code = "FF Microsoft ODBC : There was a problem while getting SQL Data : " + Position.ToString();
                    return false;
                }

                FString PreviewString;
                PreviewString.AppendChars((const char*)PreviewData, PreviewLenght);
                PreviewString.TrimEndInline();

                FMS_ODBC_MetaData EachMetaData = Array_MetaData[Index_Column_Raw];

                FMS_ODBC_DataValue EachData;
                EachData.ColumnName = EachMetaData.Column_Name;
                EachData.DataType = EachMetaData.DataType;
                EachData.Preview = PreviewString;

                switch (EachMetaData.DataType)
                {
                    // NVARCHAR & DATE & TIME
                    case -9:
                    {
                        EachData.String = PreviewString;
                        break;
                    }

                    // INT64 & BIGINT
                    case -5:
                    {
                        EachData.Integer64 = FCString::Atoi64(*PreviewString);
                        break;
                    }

                    // TIMESTAMP
                    case -2:
                    {
                        std::string RawString = TCHAR_TO_UTF8(*PreviewString);
                        unsigned int TimeStampInt = std::stoul(RawString, nullptr, 16);

                        EachData.Integer64 = TimeStampInt;
                        EachData.Preview += " - " + FString::FromInt(TimeStampInt);
                        break;
                    }

                    // TEXT
                    case -1:
                    {
                        EachData.String = PreviewString;
                        break;
                    }

                    // INT32
                    case 4:
                    {
                        EachData.Integer64 = FCString::Atoi(*PreviewString);
                        break;
                    }

                    // FLOAT & DOUBLE
                    case 6:
                    {
                        EachData.Double = FCString::Atod(*PreviewString);
                        break;
                    }

                    // DATETIME
                    case 93:
                    {
                        TArray<FString> Array_Sections;
                        PreviewString.ParseIntoArray(Array_Sections, TEXT(" "));

                        FString Date = Array_Sections[0];
                        FString Time = Array_Sections[1];

                        TArray<FString> Array_Sections_Date;
                        Date.ParseIntoArray(Array_Sections_Date, TEXT("-"));
                        int32 Year = FCString::Atoi(*Array_Sections_Date[0]);
                        int32 Month = FCString::Atoi(*Array_Sections_Date[1]);
                        int32 Day = FCString::Atoi(*Array_Sections_Date[2]);

                        TArray<FString> Array_Sections_Time;
                        Time.ParseIntoArray(Array_Sections_Time, TEXT("."));
                        int32 Milliseconds = FCString::Atoi(*Array_Sections_Time[1]);
                        FString Clock = Array_Sections_Time[0];

                        TArray<FString> Array_Sections_Clock;
                        Clock.ParseIntoArray(Array_Sections_Clock, TEXT(":"));
                        int32 Hours = FCString::Atoi(*Array_Sections_Clock[0]);
                        int32 Minutes = FCString::Atoi(*Array_Sections_Clock[1]);
                        int32 Seconds = FCString::Atoi(*Array_Sections_Clock[2]);

                        EachData.DateTime = FDateTime(Year, Month, Day, Hours, Minutes, Seconds, Milliseconds);
                        break;
                    }

                    default:
                    {
                        EachData.Note = "Currently there is no parser for this data type. Please convert it to another known type in your query !";
                        break;
                    }
                }

                Temp_Data_Pool.Add(Position, EachData);

                free(PreviewData);
                PreviewData = nullptr;
            }

            bIsMetaDataCollected = true;
            Index_Row += 1;
        }
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return false;
    }

    this->Data_Pool = Temp_Data_Pool;
    this->Count_Row = Index_Row;
    this->bIsResultRecorded = true;

    Out_Code = "FF Microsoft ODBC : Result successfuly recorded !";
    return true;
}

void UMS_ODBC_Result::Result_Record_Async(FDelegate_MS_ODBC_Record DelegateRecord)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateRecord]()
        {
            FString Out_Code;

            if (this->Result_Record(Out_Code))
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateRecord, Out_Code]()
                    {
                        DelegateRecord.ExecuteIfBound(true, Out_Code);
                    }
                );

                return;
            }

            else
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateRecord, Out_Code]()
                    {
                        DelegateRecord.ExecuteIfBound(false, Out_Code);
                    }
                );

                return;
            }
        }
    );
}

bool UMS_ODBC_Result::Result_Fetch(FString& Out_Code, TArray<FString>& Out_Values, int32 ColumnIndex)
{
    if (ColumnIndex < 1)
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
    int32 Index_Row = 0;

    try
    {
        while (SQLFetch(this->SQL_Handle_Statement) == SQL_SUCCESS)
        {
            SQLLEN ReceivedLenght;
            SQLCHAR* TempData = (SQLCHAR*)malloc(SQL_MAX_TEXT_LENGHT);
            SQLGetData(this->SQL_Handle_Statement, ColumnIndex, SQL_CHAR, TempData, SQL_MAX_TEXT_LENGHT, &ReceivedLenght);

            FString EachData;
            EachData.AppendChars((const char*)TempData, ReceivedLenght);
            EachData.TrimEndInline();
            Array_Temp.Add(EachData);

            free(TempData);
            TempData = nullptr;

            Index_Row += 1;
        }
    }

    catch (const std::exception& Exception)
    {
        Out_Code = Exception.what();
        return false;
    }

    this->Count_Row = Index_Row;
    Out_Values = Array_Temp;
    Out_Code = "FF Microsoft ODBC : Result successfully fetched !";

    return true;
}

void UMS_ODBC_Result::Result_Fetch_Async(FDelegate_MS_ODBC_Fetch DelegateFetch, int32 ColumnIndex)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateFetch, ColumnIndex]()
        {
            FString Out_Code;
            TArray<FString> Out_Values;

            if (this->Result_Fetch(Out_Code, Out_Values, ColumnIndex))
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateFetch, Out_Code, Out_Values]()
                    {
                        DelegateFetch.ExecuteIfBound(true, Out_Code, Out_Values);
                    }
                );

                return;
            }

            else
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateFetch, Out_Code]()
                    {
                        DelegateFetch.ExecuteIfBound(false, Out_Code, TArray<FString>());
                    }
                );

                return;
            }
        }
    );
}

int32 UMS_ODBC_Result::GetColumnNumber()
{
    return this->Count_Column;
}

int32 UMS_ODBC_Result::GetRowNumber()
{
    return this->Count_Row;
}

int32 UMS_ODBC_Result::GetAffectedRows()
{
    return this->Affected_Rows;
}

bool UMS_ODBC_Result::GetRow(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 Index_Row)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Index_Row < 0 || Index_Row >= this->Count_Row)
    {
        Out_Code = "FF Microsoft ODBC : Given row index is out of data pool's range !";
        return false;
    }

    TArray<FMS_ODBC_DataValue> Temp_Array;

    for (int32 Index_Column = 0; Index_Column < this->Count_Column; Index_Column++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FMS_ODBC_DataValue* EachData = this->Data_Pool.Find(Position);

        if (!EachData)
        {
            Out_Code = "FF Microsoft ODBC : Found data is not valid : " + Position.ToString();
            return false;
        }

        Temp_Array.Add(*EachData);
    }

    Out_Code = "FF Microsoft ODBC : Row exported successfully !";
    Out_Values = Temp_Array;
    return true;
}

bool UMS_ODBC_Result::GetColumnFromIndex(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, int32 Index_Column)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Index_Column < 0 || Index_Column >= this->Count_Column)
    {
        Out_Code = "FF Microsoft ODBC : Given column index is out of data pool's range !";
        return false;
    }

    TArray<FMS_ODBC_DataValue> Temp_Array;

    for (int32 Index_Row = 0; Index_Row < this->Count_Row; Index_Row++)
    {
        const FVector2D Position = FVector2D(Index_Column, Index_Row);

        if (!this->Data_Pool.Contains(Position))
        {
            Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
            return false;
        }

        FMS_ODBC_DataValue* EachData = this->Data_Pool.Find(Position);

        if (!EachData)
        {
            Out_Code = "FF Microsoft ODBC : Found data is not valid : " + Position.ToString();
            return false;
        }

        Temp_Array.Add(*EachData);
    }

    Out_Code = "FF Microsoft ODBC : Column exported successfully !";
    Out_Values = Temp_Array;
    return true;
}

bool UMS_ODBC_Result::GetColumnFromName(FString& Out_Code, TArray<FMS_ODBC_DataValue>& Out_Values, FString ColumName)
{
    if (this->Count_Column == 0)
    {
        return false;
    }

    int32 TargetIndex = 0;
    for (int32 Index_Column_Raw = 0; Index_Column_Raw < this->Count_Column; Index_Column_Raw++)
    {
        const int32 Index_Column = Index_Column_Raw + 1;
        SQLCHAR Column_Name[256];
        SQLSMALLINT NameLen, DataType, DecimalDigits, Nullable;
        SQLULEN Column_Size;

        SQLRETURN RetCode = SQLDescribeColA(this->SQL_Handle_Statement, Index_Column, Column_Name, 256, &NameLen, &DataType, &Column_Size, &DecimalDigits, &Nullable);

        if (!SQL_SUCCEEDED(RetCode))
        {
            Out_Code = "FF Microsoft ODBC : There was a problem while getting index of target column !";
            return false;
        }

        FString EachName = UTF8_TO_TCHAR((const char*)Column_Name);
        EachName.TrimEndInline();

        if (EachName == ColumName)
        {
            TargetIndex = Index_Column_Raw;
            break;
        }
    }

    return this->GetColumnFromIndex(Out_Code, Out_Values, TargetIndex);
}

bool UMS_ODBC_Result::GetSingleData(FString& Out_Code, FMS_ODBC_DataValue& Out_Value, FVector2D Position)
{
    if (this->Data_Pool.IsEmpty())
    {
        Out_Code = "FF Microsoft ODBC : Data pool is empty !";
        return false;
    }

    if (Position.X < 0 || Position.Y < 0 || Position.X >= this->Count_Column || Position.Y >= this->Count_Row)
    {
        Out_Code = "FF Microsoft ODBC : Given position is out of data pool's range !";
        return false;
    }

    if (!this->Data_Pool.Contains(Position))
    {
        Out_Code = "FF Microsoft ODBC : Target position couldn't be found ! : " + Position.ToString();
        return false;
    }

    FMS_ODBC_DataValue* DataValue = this->Data_Pool.Find(Position);

    if (!DataValue)
    {
        Out_Code = "FF Microsoft ODBC : Found data is not valid !";
        return false;
    }

    Out_Value = *DataValue;
    return true;
}

bool UMS_ODBC_Result::GetMetaData(FString& Out_Code, TArray<FMS_ODBC_MetaData>& Out_MetaData)
{
    if (!this->SQL_Handle_Statement)
    {
        Out_Code = "FF Microsoft ODBC : Statement handle is not valid !";
        return false;
    }

    SQLSMALLINT Temp_Count_Column = 0;
    SQLRETURN RetCode = SQLNumResultCols(this->SQL_Handle_Statement, &Temp_Count_Column);

    if (Temp_Count_Column == 0)
    {
        Out_Code = "FF Microsoft ODBC : There is no column to get metadata !";
        return false;
    }

    TArray<FMS_ODBC_MetaData> Array_MetaData;

    for (int32 Index_Column_Raw = 0; Index_Column_Raw < Temp_Count_Column; Index_Column_Raw++)
    {
        FMS_ODBC_MetaData EachMetaData;
        if (this->GetEachMetaData(EachMetaData, Index_Column_Raw + 1))
        {
            Array_MetaData.Add(EachMetaData);
        }
    }

    Out_MetaData = Array_MetaData;
    Out_Code = "FF Microsoft ODBC : All metadata got successfully !";
    return true;
}