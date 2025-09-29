#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <ctime>
#include <iomanip>
#include <mutex>



class Logger {

    public:
        enum LogType{
            Simple,
            Input,
            Result,
            Info,
            Warning,
            Error,
            Count
        };

    private:
        std::mutex write_mutex;
        std::array<std::ostream*, LogType::Count> outputs {
            &std::cout, // Simple
            &std::cout, // Input
            &std::cout, // Result
            &std::cout, // Info
            &std::cout, // Warning
            &std::cout  // Error
        };

        inline std::string color_wrapper(const std::string &text, const std::string &color_code, std::ostream &stream) {
            if (&stream == &std::cout || &stream == &std::cerr) {
                return color_code + text + "\033[0m";
            }
            return text;
        }

        Logger() = default;
        ~Logger() = default;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

        void write(const std::string &text, const LogType log_type = LogType::Info) {
            std::lock_guard<std::mutex> lock(write_mutex);
            std::ostream& out = *outputs.at(static_cast<std::size_t>(log_type));
            std::time_t now = std::time(nullptr);
            std::ostringstream current_time;
            current_time << std::put_time(std::localtime(&now), "%F %T: ");
            std::string log_level, color;
            switch (log_type) {
                case Simple:    out << text << std::endl; return;
                case Input:     log_level = " [INPUT]:   "; color = "\033[34m"; break;
                case Result:    log_level = " [RESULT]:  "; color = "\033[36m"; break;
                case Info:      log_level = " [INFO]:    "; color = "\033[32m"; break;
                case Warning:   log_level = " [WARNING]: "; color = "\033[33m"; break;
                case Error:     log_level = " [ERROR]:   "; color = "\033[31m"; break;
                default:        log_level = " [UNKNOWN]: "; color = "";         break;
            }
            std::string preffix = color_wrapper(current_time.str() + log_level, color, out);
            out << preffix << text << std::flush << std::endl;
        }

    public:
        static Logger& instance() {
            static Logger instance;
            return instance;
        }

        void  init_stdio() {
            std::ios::sync_with_stdio(false);
            std::cin.tie(nullptr);
        }

        void set_output(LogType log_type, std::ostream &stream) {
            outputs.at(static_cast<std::size_t>(log_type)) = &stream;
        }

        template<typename... Args>
        void log(LogType log_type, Args &&... args) {
            std::ostringstream oss;
            (oss << ... << args);
            write(oss.str(), log_type);
        }
};


#define LOG_SIMPLE(...)   Logger::instance().log(Logger::Simple,  ##__VA_ARGS__)
#define LOG_INPUT(...)    Logger::instance().log(Logger::Input,   ##__VA_ARGS__)
#define LOG_RESULT(...)   Logger::instance().log(Logger::Result,  ##__VA_ARGS__)
#define LOG_INFO(...)     Logger::instance().log(Logger::Info,    ##__VA_ARGS__)
#define LOG_WARNING(...)  Logger::instance().log(Logger::Warning, "\033[33m\033[4m", __FILE__, ":", std::to_string(__LINE__), "\033[0m::", __func__, ": ", ##__VA_ARGS__)
#define LOG_ERROR(...)    Logger::instance().log(Logger::Error,   "\033[31m\033[4m", __FILE__, ":", std::to_string(__LINE__), "\033[0m::", __func__, ": ", ##__VA_ARGS__)