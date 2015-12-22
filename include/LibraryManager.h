#pragma once

#include <memory>
#include <vector>

#include <SharedLibrary.h>

class LibraryManager {
    public:
        static LibraryManager& get();

        void registerLibrary(const std::string& path);
    
    private:
        LibraryManager() = default;
        LibraryManager(const LibraryManager&) = delete;
        const LibraryManager& operator=(const LibraryManager&) = delete;

        std::vector<std::shared_ptr<SharedLibrary>> m_libraries;
};
