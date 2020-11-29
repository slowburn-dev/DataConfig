#pragma once

#define DC_TRY(expr)						\
	do {									\
		::FDcResult Ret = (expr);			\
		if (!Ret.Ok()) {					\
			return Ret;						\
		}									\
	} while (0)


#ifndef DC_NODISCARD
	#if __cplusplus >= 201703L
	#	define DC_NODISCARD		([[nodiscard]])
	#else
	#	define DC_NODISCARD
	#endif
#endif




