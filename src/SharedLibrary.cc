#include <dlfcn.h>

#include <SharedLibrary.h>

SharedLibrary::SharedLibrary(const std::string& path) {
    m_handle = ::dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
}

SharedLibrary::~SharedLibrary() {
    if (m_handle != nullptr)
        ::dlclose(m_handle);
}
