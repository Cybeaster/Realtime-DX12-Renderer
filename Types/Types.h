#pragma once

#include "boost/signals2.hpp"

#include <algorithm> // For std::min and std::max.
#include <cassert>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using std::array;
using std::make_shared;
using std::make_unique;
using std::map;
using std::pair;
using std::queue;
using std::shared_ptr;
using std::string;
using std::to_wstring;
using std::unique_ptr;
using std::vector;
using std::weak_ptr;
using std::wstring;
