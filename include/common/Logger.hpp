#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace em {

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void set_level(LogLevel lvl) { min_level_ = lvl; }

    void log(LogLevel lvl, const std::string& component, const std::string& msg) {
        if (lvl < min_level_) return;
        std::lock_guard<std::mutex> lk(mu_);
        std::cout << timestamp() << " [" << level_str(lvl) << "] "
                  << std::left << std::setw(20) << component << " | "
                  << msg << '\n';
    }

private:
    Logger() = default;
    std::mutex mu_;
    LogLevel   min_level_{LogLevel::DEBUG};

    static std::string timestamp() {
        using namespace std::chrono;
        auto now  = system_clock::now();
        auto ms   = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        auto time = system_clock::to_time_t(now);
        std::tm tm{};
        localtime_r(&time, &tm);
        std::ostringstream ss;
        ss << std::put_time(&tm, "%H:%M:%S") << '.'
           << std::setw(3) << std::setfill('0') << ms.count();
        return ss.str();
    }

    static const char* level_str(LogLevel l) {
        switch (l) {
            case LogLevel::DEBUG: return "DBG";
            case LogLevel::INFO:  return "INF";
            case LogLevel::WARN:  return "WRN";
            case LogLevel::ERROR: return "ERR";
        }
        return "???";
    }
};

#define LOG_DEBUG(comp, msg) em::Logger::instance().log(em::LogLevel::DEBUG, comp, msg)
#define LOG_INFO(comp, msg)  em::Logger::instance().log(em::LogLevel::INFO,  comp, msg)
#define LOG_WARN(comp, msg)  em::Logger::instance().log(em::LogLevel::WARN,  comp, msg)
#define LOG_ERROR(comp, msg) em::Logger::instance().log(em::LogLevel::ERROR, comp, msg)

} // namespace em
