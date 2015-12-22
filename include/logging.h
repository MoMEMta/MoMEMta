#pragma once

#include <spdlog/spdlog.h>

namespace logging {
    std::shared_ptr<spdlog::logger>& get();
}

#define LOG(severity) \
    ::logging::get()->severity()
