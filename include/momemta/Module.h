/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2016  Universite catholique de Louvain (UCL), Belgium
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#include <momemta/impl/Pool.h>
#include <momemta/InputTag.h>
#include <momemta/ModuleFactory.h>

/*! \defgroup modules Modules
 * \brief MoMEMta's built-in modules
 *
 * Anyone of these module can be declared and used in the configuration file. More details about each module can be found in the class description.
 */


/** \ingroup modules
 *
 * \brief Parent class for all the modules.
 */
class Module {
    public:
        // FIXME: Naming
        enum class Status: std::int8_t {
            OK,
            NEXT,
            ABORT
        };

        static std::string statusToString(const Status& status);

        /**
         * \brief Constructor
         *
         * This constructor is called when the module is instanciate. It's the only place where
         * you can access the memory pool, so you **must** declare here all your module's inputs and outputs
         *
         * \param pool The memory pool. Use this to grab the input you needs from other modules, and to declare your outputs
         * \param name The name of this module.
         */
        Module(PoolPtr pool, const std::string& name):
            m_name(name), m_pool(pool) {
            // Empty
        }

        /**
         * \brief Called once at the beginning of the job
         */
        virtual void configure() { };
        
        /**
         * \brief Called once at the beginning of the integration
         */
        virtual void beginIntegration() {};

        /**
         * \brief Called once at the beginning of a loop
         *
         * Only relevant if the module is inside a loop
         */
        virtual void beginLoop() {};
        
        /**
         * \brief Main function
         *
         * This method is called for each integration step. The module's logic and work happen here.
         *
         * You'll usually want to override this function if you want your module to perform some task.
         */
        virtual Status work() { return Status::OK; };

        /**
         * \brief Called once at the end of a loop
         *
         * Only relevant if the module is inside a loop
         */
        virtual void endLoop() {};
        
        /**
         * \brief Called once at the end of the integration
         */
        virtual void endIntegration() {};
        
        /** 
         * \brief Called once at the end of the job
         */
        virtual void finish() { };

        /**
         * \brief Check if module produces an output or not
         *
         * If a module produces no output, or if its output is not used as input by any other module, 
         * it is by default removed from the list of modules that are called on each phase-space point. 
         * 
         * This function allows to flag the module so that it doesn't get removed. A typical use case would 
         * be a module that only produces an object that is retrieved directly from the Pool by the user, 
         * after the integration.
         *
         * \return True if the module produces no output, False otherwise. 
         * 
         * \note Default value is False;
         */
        virtual bool leafModule() const {
            return false;
        }

        virtual std::string name() const final {
            return m_name;
        }

        /**
         * \brief Test if a given name correspond to a virtual module
         *
         * Some names are reserved for internal usage and are mapped to virtual modules.
         *
         * \param name The name to test
         * \return True if `name` is the name of a virtual module, false otherwise.
         */
        inline static bool is_virtual_module(const std::string& name) {
            return (name == "momemta") || (name == "input") || (name == "cuba");
        }

    protected:
        /**
         * \brief Add a new output to the module
         *
         * Call this function to add a new output to the module. This output will then be available to all the modules.
         *
         * Example:
         * ```
         * // In constructor
         * std::shared_ptr<double> my_output = produce<double>("output");
         * std::shared_ptr<TH1D> my_hist = produce<TH1D>("output_hist", "name", "title", 5, 0, 5); // Use constructor call
         *
         * // In work()
         * *my_output = 10;
         * ```
         *
         * \tparam T The type of the output.
         * \param name The name of the output. Must be unique to this module.
         * \param args Variable list of optional arguments. Will be passed to the constructor of T.
         *
         * \return A std::shared_ptr<T> pointing to newly allocated memory in the memory pool. Change the content of this pointer to change the output.
         */
        template<typename T, typename... Args> std::shared_ptr<T> produce(const std::string& name, Args... args) {
            return m_pool->put<T>({m_name, name}, std::forward<Args>(args)...);
        }

        template<typename T> Value<T> get(const std::string& module, const std::string& name) {
            return m_pool->get<T>({module, name});
        }

        template<typename T> Value<T> get(const InputTag& tag) {
            return m_pool->get<T>(tag);
        }

    private:
        
        const std::string m_name;

    protected:

        PoolPtr m_pool;
};

using ModulePtr = std::shared_ptr<Module>;
