/**
 * @brief C++ Wrapper interface for tentris_log to be used inside tentris-lib
 */

#ifndef TENTRIS_LOGGER_HPP
#define TENTRIS_LOGGER_HPP

#include <dice/logger/logger_interface.h>

#include <string_view>
#include <format>

namespace dice::logger {

	enum struct Level : std::underlying_type_t<tentris_log_level> {
		Error = TENTRIS_LL_ERROR,
		Warn  = TENTRIS_LL_WARN,
		Info  = TENTRIS_LL_INFO,
		Debug = TENTRIS_LL_DEBUG,
		Trace = TENTRIS_LL_TRACE,
	};

	template<typename ...Args>
	void log(Level lvl, char const *function, std::format_string<Args...> fmt, Args &&...args) {
		auto const msg = std::format(fmt, std::forward<Args>(args)...);
		tentris_log(static_cast<tentris_log_level>(lvl), function, msg.c_str());
	}

} // namespace dice::logger

#define TENTRIS_LOG(lvl, fmt, ...) ::dice::logger::log((lvl), __PRETTY_FUNCTION__, (fmt) __VA_OPT__(,) __VA_ARGS__)

#define TENTRIS_ERROR(fmt, ...) TENTRIS_LOG(::dice::logger::Level::Error, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define TENTRIS_WARN(fmt, ...) TENTRIS_LOG(::dice::logger::Level::Warn, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define TENTRIS_INFO(fmt, ...) TENTRIS_LOG(::dice::logger::Level::Info, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define TENTRIS_DEBUG(fmt, ...) TENTRIS_LOG(::dice::logger::Level::Debug, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define TENTRIS_TRACE(fmt, ...) TENTRIS_LOG(::dice::logger::Level::Trace, (fmt) __VA_OPT__(,) __VA_ARGS__)

#endif//TENTRIS_LOGGER_HPP
