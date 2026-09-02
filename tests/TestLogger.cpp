#include <dice/logger/logger_interface.h>

#include <chrono>
#include <iostream>
#include <thread>

static char const *log_level_to_string(tentris_log_level lvl) noexcept {
	switch (lvl) {
		case TENTRIS_LL_ERROR: {
			return "ERROR";
		}
		case TENTRIS_LL_WARN: {
			return "WARNING";
		}
		case TENTRIS_LL_INFO: {
			return "INFO";
		}
		case TENTRIS_LL_DEBUG: {
			return "DEBUG";
		}
		case TENTRIS_LL_TRACE: {
			return "TRACE";
		}
	}
}

extern "C" void tentris_log(tentris_log_level lvl, char const *function, char const *msg) {
	auto now = std::chrono::system_clock::now();

	std::ostringstream tid_oss;
	tid_oss << std::this_thread::get_id(); // thread::id does not support std::format

	std::cerr << std::format("{} {} {} (thread: {}): {}", now, log_level_to_string(lvl), function, tid_oss.view(), msg) << std::endl;
}
