#pragma once

#include "CoreMinimal.h"

enum class EDataConfigVariantType
{
	Unknown,	// in a bad state
	List,
	Mapping,
	Float,
	Integer,
	String,
	Boolean,
	Nil,
	Blob,		//	need this for some very specific things
};

DATACONFIGCORE_API void Hello();


