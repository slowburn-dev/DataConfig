#pragma once

#ifndef DC_BUILD_DEBUG
	#define DC_BUILD_DEBUG 0
#endif

// note that DC_BUILD_FAST does nothing atm, we'll do a performance pass in a future release
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

#ifndef DC_NODISCARD
	#if __cplusplus >= 201703L
	#define DC_NODISCARD [[nodiscard]]
	#else
	#define DC_NODISCARD
	#endif
#endif

