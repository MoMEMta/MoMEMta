#include <ModuleFactory.h>

template<> PluginFactory<ModuleFactory::type>& PluginFactory<ModuleFactory::type>::get() {
    static PluginFactory<ModuleFactory::type> s_instance;
    return s_instance;
}
