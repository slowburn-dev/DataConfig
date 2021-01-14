#pragma once

//	TODO move to Build.cs
#define DC_BUILD_DEBUG 1

#ifndef DC_BUILD_DEBUG
	#define DC_BUILD_DEBUG 0
#endif

#ifndef DC_BUILD_FAST
	#define DC_BUILD_FAST 0
#endif

#if DC_BUILD_DEBUG + DC_BUILD_FAST != 1
	#error Exactly one of [DC_BUILD_DEBUG DC_BUILD_FAST] should be define to 1
#endif

#define DC_STRINGIFY(x) DC_STRINGIFY2(x)
#define DC_STRINGIFY2(x) #x

#define DC_JOIN(a, b) DC_JOIN2(a, b)
#define DC_JOIN2(a, b) a##b
#define DC_UNIQUE(a) DC_JOIN(a, __LINE__)

//	TODO dcCheck() that doesn't depend on check()


//	TODO doesn't work yet
#ifndef DC_NODISCARD
	#if __cplusplus >= 201703L
	#	define DC_NODISCARD		[[nodiscard]]
	#else
	#	define DC_NODISCARD
	#endif
#endif

