#include <LibraryManager.h>
#include <logging.h>

LibraryManager& LibraryManager::get() {
    static LibraryManager s_instance;

    return s_instance;
}

void LibraryManager::registerLibrary(const std::string& path) {
    LOG(debug) << "Loading library: " << path;
    m_libraries.push_back(std::make_shared<SharedLibrary>(path));
}
