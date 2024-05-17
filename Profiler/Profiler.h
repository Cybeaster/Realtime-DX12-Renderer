#pragma once
#include "easy/profiler.h"
#ifdef ENABLE_PROFILER
#define PROFILE_SCOPE() EASY_FUNCTION()
#define PROFILE_BLOCK_START(name) EASY_BLOCK(name)
#define PROFILE_BLOCK_END() EASY_END_BLOCK
#define ENABLE_PROFILER EASY_PROFILER_ENABLE
#else
#define PROFILE_SCOPE()
#define PROFILE_BLOCK_START(name)
#define PROFILE_BLOCK_END()
#define ENABLE_PROFILER
#endif
#define DUMP_PROFILE_TO_FILE(filename) profiler::dumpBlocksToFile(#filename);