#pragma once

#include <string>

// A small wrapper around dlopen / dlclose
struct SharedLibrary {
    public:
        SharedLibrary(const std::string& path);
        ~SharedLibrary();

    private:
        // FIXME: delete copy constructor
        void* m_handle = nullptr;
};
