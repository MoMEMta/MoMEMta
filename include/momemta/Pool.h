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

#include <assert.h>
#include <memory>
#include <unordered_map>

#include <boost/any.hpp>

#include <momemta/impl/InputTag_fwd.h>
#include <momemta/Configuration.h>
#include <momemta/Value.h>

// A simple memory pool

/**
 * A simple container for the memory block inside the global memory pool
 */
struct PoolContent {
    boost::any ptr; /// Pointer to the memory allocated for this block
    bool valid; /// The state of the memory block. If false, it means that a module requested this block in read-mode, but no module actually provides the block.
};

// FIXME: Use a more descriptive name, like "ModuleDependencies"
struct Description {
    Configuration::Module module;
    std::vector<InputTag> inputs;
    std::vector<std::string> outputs;
};

class Pool {
    public:
        using DescriptionMap = std::unordered_map<std::string, Description>;

        Pool() = default;

        /**
         * \brief Allocate a new block in the memory pool.
         *
         * \param tag The input tag describing the memory block
         * \param args Optional constructor arguments for creating a new instance of \p T
         *
         * \return A pointer to the newly allocated memory block.
         */
        template<typename T, typename... Args> std::shared_ptr<T> put(const InputTag& tag, Args... args);

        template<typename T> Value<T> get(const InputTag& tag) const;

        void alias(const InputTag& from, const InputTag& to);

        /** \brief Check if input tag exists in the pool.
         *
         * \warning No check is performed to ensure that the string is an input tag. Use InputTag::isInputTag() first.
         *
         * \param tag Input tag that will be searched in the pool.
         * \return True if the input tag exists in the pool, False otherwise.
         */
        bool exists(const InputTag& tag) const;

        /**
         * \brief Return the description of the state of the memory pool
         *
         * \note Result is only valid after a call to Pool::freeze()
         */
        const DescriptionMap& description() const {
            return m_description;
        }

    private:
        friend class MoMEMta;
        friend class Module;

        template <typename U>
        friend class ValueProxy;

        using PoolStorage = std::unordered_map<InputTag, PoolContent>;

        friend struct InputTag;

        void remove(const InputTag&, bool force = true);
        void remove_if_invalid(const InputTag&);

        boost::any reserve(const InputTag&);

        template<typename T> std::shared_ptr<const T> raw_get(const InputTag& tag) const;

        template<typename T, typename... Args> PoolStorage::iterator create(const InputTag& tag,
                bool valid = true, Args... args) const;

    public:
        /**
         * \brief Inform the pool of which module is currently created.
         *
         * It helps tracking down module dependencies.
         *
         * \param module The module beeing created
         */
        virtual void current_module(const Configuration::Module& module) final;

        /**
         * \brief Inform the pool of which module is currently created.
         *
         * It helps tracking down module dependencies. This signature indicates
         * that the current module is virtual.
         *
         * \param name Name of the current virtual module
         */
        virtual void current_module(const std::string& name) final;

        class tag_not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class duplicated_tag_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class constructor_tag_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };


private:

        /**
         * \brief Freeze the memory pool.
         *
         * Once frozen, no modification is allowed inside the pool.
         *
         * \note This is not yet enforced
         */
        virtual void freeze() final;

        /**
         * \brief Retrieve the description for the module being created.
         *
         * @return A reference to the module's description. If none exists, a new one is created.
         */
        virtual Description& get_description() const final;

        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        Configuration::Module m_current_module; /// Module currently created.
        bool m_frozen = false; /// If true, no modification of the pool is allowed

        mutable PoolStorage m_storage;
        mutable DescriptionMap m_description; /// Mutable so that get() can be marked const
};

using PoolPtr = std::shared_ptr<Pool>;
