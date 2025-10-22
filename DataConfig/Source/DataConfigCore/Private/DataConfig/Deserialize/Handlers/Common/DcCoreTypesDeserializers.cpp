#include "DataConfig/Deserialize/Handlers/Common/DcCoreTypesDeserializers.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/SerDe/DcSerDeCoreTypesCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"

#include "Misc/EngineVersionComparison.h"

namespace DcCoreTypesHandlers {

struct FDcScopedSkipStructHandler
{
	FDcScopedSkipStructHandler(bool& bInSkipStructHandlers)
		: bSkipStructHandlers(bInSkipStructHandlers)
	{
		bSkipStructHandlers = true;
	}

	~FDcScopedSkipStructHandler()
	{
		bSkipStructHandlers = false;
	}

	bool& bSkipStructHandlers;
};

#define _DC_READ_MAP_FALLBACK \
	do {\
		EDcDataEntry Next;\
		DC_TRY(Ctx.Reader->PeekRead(&Next));\
		if (Next == EDcDataEntry::MapRoot)\
		{\
			FDcScopedSkipStructHandler ScopedSkipStructHandler(Ctx.bSkipStructHandlers);\
			return DcDeserializeUtils::RecursiveDeserialize(Ctx);\
		}\
	} while (0)

FDcResult HandlerGuidDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
    FGuid* GuidPtr = (FGuid*)Datum.DataPtr;

    DC_TRY(Ctx.Reader->ReadArrayRoot());
    DC_TRY(Ctx.Reader->ReadUInt32(&GuidPtr->A));
    DC_TRY(Ctx.Reader->ReadUInt32(&GuidPtr->B));
    DC_TRY(Ctx.Reader->ReadUInt32(&GuidPtr->C));
    DC_TRY(Ctx.Reader->ReadUInt32(&GuidPtr->D));
    DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

FDcResult _DeserializeVector2f(FDcVector2f* Vec2fPtr, FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Reader->ReadFloat(&Vec2fPtr->X));
	DC_TRY(Ctx.Reader->ReadFloat(&Vec2fPtr->Y));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}
FDcResult _DeserializeVector3f(FDcVector3f* Vec3fPtr, FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Reader->ReadFloat(&Vec3fPtr->X));
	DC_TRY(Ctx.Reader->ReadFloat(&Vec3fPtr->Y));
	DC_TRY(Ctx.Reader->ReadFloat(&Vec3fPtr->Z));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

FDcResult _DeserializeVector4f(FDcVector4f* Vec4fPtr, FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Reader->ReadFloat(&Vec4fPtr->X));
	DC_TRY(Ctx.Reader->ReadFloat(&Vec4fPtr->Y));
	DC_TRY(Ctx.Reader->ReadFloat(&Vec4fPtr->Z));
	DC_TRY(Ctx.Reader->ReadFloat(&Vec4fPtr->W));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

#if ENGINE_MAJOR_VERSION == 5
FDcResult _DeserializeVector2d(FDcVector2d* Vec2dPtr, FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Reader->ReadDouble(&Vec2dPtr->X));
	DC_TRY(Ctx.Reader->ReadDouble(&Vec2dPtr->Y));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

FDcResult _DeserializeVector3d(FDcVector3d* Vec3dPtr, FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Reader->ReadDouble(&Vec3dPtr->X));
	DC_TRY(Ctx.Reader->ReadDouble(&Vec3dPtr->Y));
	DC_TRY(Ctx.Reader->ReadDouble(&Vec3dPtr->Z));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

FDcResult _DeserializeVector4d(FDcVector4d* Vec4dPtr, FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Reader->ReadDouble(&Vec4dPtr->X));
	DC_TRY(Ctx.Reader->ReadDouble(&Vec4dPtr->Y));
	DC_TRY(Ctx.Reader->ReadDouble(&Vec4dPtr->Z));
	DC_TRY(Ctx.Reader->ReadDouble(&Vec4dPtr->W));
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

static_assert(sizeof(FDcVector4f) == sizeof(FPlane4f), "FPlane4 stale");
static_assert(sizeof(FDcVector4d) == sizeof(FPlane4d), "FPlane4 stale");
#endif // ENGINE_MAJOR_VERSION == 5

FDcResult HandlerVector2fDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	return _DeserializeVector2f((FDcVector2f*)Datum.DataPtr, Ctx);
}

FDcResult HandlerVector3fDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	return _DeserializeVector3f((FDcVector3f*)Datum.DataPtr, Ctx);
}

FDcResult HandlerVector4fDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	return _DeserializeVector4f((FDcVector4f*)Datum.DataPtr, Ctx);
}

FDcResult HandlerMatrix44fDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	FDcMatrix44f* Matrix44fPtr = (FDcMatrix44f*)Datum.DataPtr;

    DC_TRY(Ctx.Reader->ReadArrayRoot());
    DC_TRY(_DeserializeVector4f((FDcVector4f*)Matrix44fPtr->M[0], Ctx));
    DC_TRY(_DeserializeVector4f((FDcVector4f*)Matrix44fPtr->M[1], Ctx));
    DC_TRY(_DeserializeVector4f((FDcVector4f*)Matrix44fPtr->M[2], Ctx));
    DC_TRY(_DeserializeVector4f((FDcVector4f*)Matrix44fPtr->M[3], Ctx));
    DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

FDcResult HandlerColorDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FString ColorStr;
	DC_TRY(Ctx.Reader->ReadString(&ColorStr));
	FColor Color = FColor::FromHex(ColorStr);

	FColor* ColorPtr = (FColor*)Datum.DataPtr;
	*ColorPtr = Color;

	return DcOk();
}

FDcResult HandlerDateTimeDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FString DateTimeStr;
	DC_TRY(Ctx.Reader->ReadString(&DateTimeStr));
	FDateTime DateTime;
	if (FDateTime::Parse(DateTimeStr, DateTime))
	{
		FDateTime* DateTimePtr = (FDateTime*)Datum.DataPtr;
		*DateTimePtr = DateTime;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDSerDe, DateTimeParseFail) << DateTimeStr;
	}
}

FDcResult HandlerTimespanDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FString TimespanStr;
	DC_TRY(Ctx.Reader->ReadString(&TimespanStr));
	FTimespan Timespan;
	if (FTimespan::Parse(TimespanStr, Timespan))
	{
		FTimespan* TimespanPtr = (FTimespan*)Datum.DataPtr;
		*TimespanPtr = Timespan;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDSerDe, TimespanParseFail) << TimespanStr;
	}
}

FDcResult HandlerTextDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FText Value;
		DC_TRY(Ctx.Reader->ReadText(&Value));
		DC_TRY(Ctx.Writer->WriteText(Value));
		return DcOk();
	}
	else if (Next == EDcDataEntry::ArrayRoot)
	{
		DC_TRY(Ctx.Reader->ReadArrayRoot());

		FString Value1;
		FString Value2;
		DC_TRY(Ctx.Reader->ReadString(&Value1));
		DC_TRY(Ctx.Reader->ReadString(&Value2));

		DC_TRY(Ctx.Reader->PeekRead(&Next));
		if (Next == EDcDataEntry::ArrayEnd)
		{
			// [<string-table-id>, <text-id>]
			FText Result = FText::FromStringTable(FName(*Value1), Value2);
			DC_TRY(Ctx.Writer->WriteText(Result));
			DC_TRY(Ctx.Reader->ReadArrayEnd());
		}
		else
		{
			// [<namespace>, <text-id>, <source>]
			FString Value3;
			DC_TRY(Ctx.Reader->ReadString(&Value3));

#if UE_VERSION_OLDER_THAN(5, 5, 0)
			FText Result = Value3.IsEmpty()
				? FText::GetEmpty()
				: FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(*Value3, *Value1, *Value2);
#else
			FText Result = FText::AsLocalizable_Advanced(Value1, Value2, MoveTemp(Value3));
#endif // UE_VERSION_OLDER_THAN(5, 5, 0)

			DC_TRY(Ctx.Writer->WriteText(Result));
			DC_TRY(Ctx.Reader->ReadArrayEnd());
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDSerDe, DataEntryMismatch2)
			<< EDcDataEntry::String << EDcDataEntry::ArrayRoot << Next;
	}
}

FDcResult HandlerIntPointDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	FIntPoint* IntPointPtr = (FIntPoint*)Datum.DataPtr;

    DC_TRY(Ctx.Reader->ReadArrayRoot());
    DC_TRY(Ctx.Reader->ReadInt32(&IntPointPtr->X));
    DC_TRY(Ctx.Reader->ReadInt32(&IntPointPtr->Y));
    DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

FDcResult HandlerIntVectorDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	FIntVector* IntVectorPtr = (FIntVector*)Datum.DataPtr;

    DC_TRY(Ctx.Reader->ReadArrayRoot());
    DC_TRY(Ctx.Reader->ReadInt32(&IntVectorPtr->X));
    DC_TRY(Ctx.Reader->ReadInt32(&IntVectorPtr->Y));
    DC_TRY(Ctx.Reader->ReadInt32(&IntVectorPtr->Z));
    DC_TRY(Ctx.Reader->ReadArrayEnd());

    return DcOk();
}

#if ENGINE_MAJOR_VERSION == 5
FDcResult HandlerVector2dDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	return _DeserializeVector2d((FDcVector2d*)Datum.DataPtr, Ctx);
}

FDcResult HandlerVector3dDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	return _DeserializeVector3d((FDcVector3d*)Datum.DataPtr, Ctx);
}

FDcResult HandlerVector4dDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	return _DeserializeVector4d((FDcVector4d*)Datum.DataPtr, Ctx);
}

FDcResult HandlerMatrix44dDeserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	FDcMatrix44d* Matrix44dPtr = (FDcMatrix44d*)Datum.DataPtr;

    DC_TRY(Ctx.Reader->ReadArrayRoot());
    DC_TRY(_DeserializeVector4d((FDcVector4d*)Matrix44dPtr->M[0], Ctx));
    DC_TRY(_DeserializeVector4d((FDcVector4d*)Matrix44dPtr->M[1], Ctx));
    DC_TRY(_DeserializeVector4d((FDcVector4d*)Matrix44dPtr->M[2], Ctx));
    DC_TRY(_DeserializeVector4d((FDcVector4d*)Matrix44dPtr->M[3], Ctx));
    DC_TRY(Ctx.Reader->ReadArrayEnd());

	return DcOk();
}

FDcResult HandlerIntVector4Deserialize(FDcDeserializeContext& Ctx)
{
	_DC_READ_MAP_FALLBACK;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));
	FIntVector4* IntVector4Ptr = (FIntVector4*)Datum.DataPtr;

    DC_TRY(Ctx.Reader->ReadArrayRoot());
    DC_TRY(Ctx.Reader->ReadInt32(&IntVector4Ptr->X));
    DC_TRY(Ctx.Reader->ReadInt32(&IntVector4Ptr->Y));
    DC_TRY(Ctx.Reader->ReadInt32(&IntVector4Ptr->Z));
    DC_TRY(Ctx.Reader->ReadInt32(&IntVector4Ptr->W));
    DC_TRY(Ctx.Reader->ReadArrayEnd());

    return DcOk();
}
#endif // ENGINE_MAJOR_VERSION == 5

#undef _DC_READ_MAP_FALLBACK


} // namespace DcCommonHandlers


