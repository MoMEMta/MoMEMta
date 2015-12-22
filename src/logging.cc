#include <spdlog/spdlog.h>

namespace logging {
    std::shared_ptr<spdlog::logger>& get() {
        static std::shared_ptr<spdlog::logger> s_logger = spdlog::stdout_logger_st("MEMpp");

        return s_logger;
    }
}
