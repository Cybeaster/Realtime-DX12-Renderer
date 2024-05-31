#pragma once
#include "Defines.h"
#include "easy/profiler.h"
#if ENABLE_PROFILER
#define PROFILE_SCOPE() EASY_FUNCTION()
#define PROFILE_BLOCK_START(name) EASY_BLOCK(name)
#define PROFILE_BLOCK_END() EASY_END_BLOCK
#else
#define PROFILE_SCOPE()
#define PROFILE_BLOCK_START(name)
#define PROFILE_BLOCK_END()
#endif
#define DUMP_PROFILE_TO_FILE(filename) profiler::dumpBlocksToFile(#filename);