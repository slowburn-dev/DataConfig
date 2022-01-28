#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "Misc/StringBuilder.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"

static_assert(DcTypeUtils::TIsUClass<UClass>::Value, "test TIsUClass");
static_assert(!DcTypeUtils::TIsUClass<FDcReader>::Value, "test TIsUClass");

static_assert(!DcTypeUtils::TIsUStruct<FVector2D>::Value, "TIsUStruct has limitation that can't detect core types, see `TBaseStructure`.");
static_assert(!DcTypeUtils::TIsUStruct<UClass>::Value, "test TIsUStruct");
static_assert(!DcTypeUtils::TIsUStruct<FDcReader>::Value, "test TIsUStruct");

static_assert(TIsSame<DcPropertyUtils::TPropertyTypeMap<int32>::Type, FIntProperty>::Value, "test DcPropertyUtils");

static_assert(DcPropertyUtils::TIsInPropertyMap<int32>::Value, "test TIsInPropertyMap");
static_assert(!DcPropertyUtils::TIsInPropertyMap<FDcReader>::Value, "test TIsInPropertyMap");

FString DcGetFixturePath(const FString& Str)
{
#if IS_PROGRAM
	FString Root = FPaths::LaunchDir();
#else
	FString Root = IPluginManager::Get().FindPlugin("DataConfig")->GetBaseDir();
#endif
	FPaths::NormalizeDirectoryName(Root);
	return FString::Printf(TEXT("%s/Tests/%s"), *Root, *Str);
}


//	Trim and reindent a string literal to the first non empty indent level
FString DcReindentStringLiteral(FString Str, FString* Prefix)
{
	TArray<FString> Lines;
	Str.ParseIntoArrayLines(Lines);

	int MinIndent = TNumericLimits<int>::Max();

	auto _IsWhitespaceLine = [](const FString& Str)
	{
		for (TCHAR Ch : Str)
		{
			if (!FChar::IsWhitespace(Ch))
				return false;
		}

		return true;
	};

	int LineCount = Lines.Num();
	int FirstNonEmptyIx = 0;
	int LastNonEmptyIx = LineCount;
	{
		for (int Ix = 0; Ix < LineCount; Ix++)
		{
			if (!_IsWhitespaceLine(Lines[Ix]))
			{
				FirstNonEmptyIx = Ix;
				break;
			}
		}

		for (int Ix = 0; Ix < LineCount; Ix++)
		{
			if (!_IsWhitespaceLine(Lines[LineCount- Ix - 1]))
			{
				LastNonEmptyIx = LineCount-Ix;
				break;
			}
		}
	}

	TStringBuilder<1024> Sb;
	{
		for (int Ix = FirstNonEmptyIx; Ix < LastNonEmptyIx; Ix++)
		{
			FString& Line = Lines[Ix];

			Line.ConvertTabsToSpacesInline(4);
			Line.TrimEndInline();
			int Spaces = 0;
			for (TCHAR Char : Line)
			{
				if (Char == ' ')
					Spaces++;
				else
					break;
			}

			if (Spaces < MinIndent)
				MinIndent = Spaces;
		}

		for (int Ix = FirstNonEmptyIx; Ix < LastNonEmptyIx; Ix++)
		{
			FString& Line = Lines[Ix];

			Line.RightChopInline(MinIndent);
			if (Prefix) Sb.Append(*Prefix);
			Sb.Append(Line);
			Sb.Append('\n');
		}
	}

	return Sb.ToString();
}
