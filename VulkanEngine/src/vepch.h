#pragma once

#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <random>

#include <fstream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "Core/Base.h"
#include "Events/Event.h"

#ifdef VE_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
