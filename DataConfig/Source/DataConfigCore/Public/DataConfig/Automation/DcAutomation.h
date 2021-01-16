#pragma once

#include "CoreTypes.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/AssertionMacros.h"
#include "Containers/UnrealString.h"

class DATACONFIGCORE_API FDcAutomationBase : public FAutomationTestBase
{
public:
	constexpr static uint32 _FLAGS = EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter;

	using FAutomationTestBase::FAutomationTestBase;

	uint32 GetTestFlags() const override;
	uint32 GetRequiredDeviceNum() const override; 

	bool TestOk(const TCHAR* Description, const FDcResult& Result);
	bool TestOk(const FString& Description, const FDcResult& Result);

	bool TestDiagnostic(const TCHAR* Description, const FDcResult& Result, uint16 Category, uint16 Code);
	bool TestDiagnostic(const FString& Description, const FDcResult& Result, uint16 Category, uint16 Code);

	bool RunTest(const FString& Parameters) override;

	virtual bool DcRunTestBody(const FString& Parameters) = 0;
};

#define UTEST_OK(What, Result)\
	do {									\
		if (!TestOk(What, Result)) {		\
			return false;					\
		}									\
	} while (0)

#define UTEST_DIAG(What, Result, DiagNamespace, DiagID)\
	do {\
		TDcStoreThenReset<bool> __ScopedExpectFail(DcEnv().bExpectFail, true);\
		if (!TestDiagnostic(What, Result, DiagNamespace::Category, DiagNamespace::DiagID)) {\
			return false;\
		}\
	} while (0)\

///	Example:
///
///		DC_TEST("Dc.Foo.Bar")
///		{
///			// test body
///			return true;
///		}

#define DC_TEST_IMPL(ID, PrettyName) \
	namespace { \
	class DC_JOIN(FDcAutomationTest, ID) : public FDcAutomationBase \
	{ \
	public: \
		DC_JOIN(FDcAutomationTest, ID)(const FString& InName) \
			: FDcAutomationBase(InName, false) {} \
		virtual FString GetTestSourceFileName() const override { return __FILE__; } \
		virtual int32 GetTestSourceFileLine() const override { return __LINE__; } \
	protected: \
		virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const override \
		{ \
			OutBeautifiedNames.Add(PrettyName); \
			OutTestCommands.Add(FString()); \
		} \
		virtual bool DcRunTestBody(const FString& Parameters) override; \
		virtual FString GetBeautifiedTestName() const override { return PrettyName; } \
	}; \
	DC_JOIN(FDcAutomationTest, ID) DC_JOIN(_DcAutomationInstance, ID)(TEXT(PrettyName)); \
	} \
	bool DC_JOIN(FDcAutomationTest, ID)::DcRunTestBody(const FString& Parameters)

#define DC_TEST(PrettyName) DC_TEST_IMPL(__COUNTER__, PrettyName)

struct DATACONFIGCORE_API FDcAutomationConsoleRunner
{
	struct FArgs
	{
		TArray<FString> Filters;
		uint32 RequestedTestFilter;
	};

	void Prepare(const FArgs& Args);
	int32 RunTests();

	TArray<FAutomationTestInfo> SelectedTests;
};


