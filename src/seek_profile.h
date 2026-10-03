// End-to-end instrumentation of the video seek path, active only when the
// AEGISUB_PROFILE_SEEK environment variable is set (see the "wxQt Edition"
// section of README.md for usage and the rationale for keeping it in the
// tree). Every line is prefixed with "[seek]" and stamped with steady-clock
// microseconds so that lines written from the UI thread and the video worker
// thread can be correlated even if they interleave. The wxWidgets side has a
// matching probe in its Qt GL canvas that reacts to the same variable.
#pragma once

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>

namespace seek_profile {
inline bool enabled() {
	static bool value = std::getenv("AEGISUB_PROFILE_SEEK") != nullptr;
	return value;
}

inline long long stamp() {
	return std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Time of the last active line change, written from the UI thread.
inline std::atomic<long long>& active_line_stamp() {
	static std::atomic<long long> value{0};
	return value;
}

// Duration of the last frame render on the worker thread.
inline std::atomic<long long>& worker_render_us() {
	static std::atomic<long long> value{0};
	return value;
}
}
