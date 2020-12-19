#pragma once

#include "CoreTypes.h"
#include "DataConfig/DcMacros.h"
#include "Misc/AutomationTest.h"
#include "Misc/AssertionMacros.h"
#include "Containers/UnrealString.h"

class DATACONFIGCORE_API FDcAutomationBase : public FAutomationTestBase
{
public:
	using FAutomationTestBase::FAutomationTestBase;

	uint32 GetTestFlags() const override;
	uint32 GetRequiredDeviceNum() const override; 

	bool TestOk(const TCHAR* Description, const FDcResult& Result);
	bool TestOk(const FString& Description, const FDcResult& Result);
};

#define UTEST_OK(What, Result)\
	do {									\
		if (!TestOk(What, Result)) {		\
			return false;					\
		}									\
	} while (0)

///	Example:
///
///		DC_TEST("Dc.Foo.Bar")
///		{
///			// test body
///			return true;
///		}

#define DC_TEST(PrettyName) \
	namespace __DC_DETAIL { \
	class DC_UNIQUE(FDcAutomationTest) : public FDcAutomationBase \
	{ \
	public: \
		DC_UNIQUE(FDcAutomationTest)(const FString& InName) \
			: FDcAutomationBase(InName, false) {} \
		virtual FString GetTestSourceFileName() const override { return __FILE__; } \
		virtual int32 GetTestSourceFileLine() const override { return __LINE__; } \
	protected: \
		virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const override \
		{ \
			OutBeautifiedNames.Add(PrettyName); \
			OutTestCommands.Add(FString()); \
		} \
		virtual bool RunTest(const FString& Parameters) override; \
		virtual FString GetBeautifiedTestName() const override { return PrettyName; } \
	}; \
	namespace { \
		DC_UNIQUE(FDcAutomationTest) DC_UNIQUE(_DcAutomationInstance)(TEXT(DC_STRINGIFY(PrettyName))); \
	} \
	} \
	bool __DC_DETAIL::DC_UNIQUE(FDcAutomationTest)::RunTest(const FString& Parameters)


struct DATACONFIGCORE_API FDcAutomationConsoleRunner
{
	void Prepare();
	void RunTests();
};


