#include <ConfigurationSet.h>
#include <Module.h>

class EmptyModule: public Module {
    public:

        EmptyModule(PoolPtr pool, const ConfigurationSet& parameters): Module(pool, parameters.getModuleName()) {
            // Empty
        };

        virtual void work() override {

        }
};
REGISTER_MODULE(EmptyModule);
