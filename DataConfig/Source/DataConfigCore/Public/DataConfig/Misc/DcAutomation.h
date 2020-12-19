#pragma once

#include "CoreTypes.h"
#include "DataConfig/DcMacros.h"
#include "Misc/AutomationTest.h"
#include "Misc/AssertionMacros.h"
#include "Containers/UnrealString.h"

///	DC_TEST("Dc.Foo.Bar")
///	{
///		// test body
///		return true;
/// }

#define DC_TEST(PrettyName) \
	namespace __DC_DETAIL { \
	class DC_UNIQUE(FDcAutomationTest) : public FAutomationTestBase \
	{ \
	public: \
		DC_UNIQUE(FDcAutomationTest)(const FString& InName) \
			: FAutomationTestBase(InName, false) {} \
		virtual uint32 GetTestFlags() const override { return EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter; } \
		virtual bool IsStressTest() const { return false; } \
		virtual uint32 GetRequiredDeviceNum() const override { return 1; } \
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
