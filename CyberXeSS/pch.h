#pragma once
#include "framework.h"
#include "Config.h"

//inline std::mutex CreateFeatureMutex;

#define SAFE_RELEASE(p)		\
do {						\
	if(p && p != nullptr)	\
	{						\
		(p)->Release();		\
		(p) = nullptr;		\
	}						\
} while((void)0, 0);		

void PrepareLogger();
void CloseLogger();