#pragma once

//	TODO doesn't work yet
#ifndef DC_NODISCARD
	#if __cplusplus >= 201703L
	#	define DC_NODISCARD		([[nodiscard]])
	#else
	#	define DC_NODISCARD
	#endif
#endif




