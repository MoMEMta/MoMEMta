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

#include <memory>
#include <unordered_map>

#include <boost/any.hpp>

#include <momemta/impl/InputTag_fwd.h>

// A simple memory pool

/**
 * A simple container for the memory block inside the global memory pool
 */
struct PoolContent {
    boost::any ptr; /// Pointer to the memory allocated for this block
    bool valid; /// The state of the memory block. If false, it means that a module requested this block in read-mode, but no module actually provides the block.
};

struct Description {
    std::string module_name;
    std::vector<InputTag> inputs;
    std::vector<std::string> outputs;
};

class Pool {
    public:
        using DescriptionMap = std::unordered_map<std::string, Description>;

        template<typename T> std::shared_ptr<const T> get(const InputTag& tag) const {
            if (tag.isIndexed()) {
                throw std::invalid_argument("Indexed input tag cannot be passed as argument of the pool. Use the `get` function of the input tag to retrieve its content.");
            }

            auto it = m_storage.find(tag);
            if (it == m_storage.end())
                it = create<T>(tag, false);

            PoolContent& v = it->second;
            std::shared_ptr<T>& ptr = boost::any_cast<std::shared_ptr<T>&>(v.ptr);

            if (! m_frozen) {
                // Update current module description
                assert(! m_current_module.empty());
                Description& description = m_description[m_current_module];
                description.inputs.push_back(tag);
            }

            return std::const_pointer_cast<const T>(ptr);
        }

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

        using PoolStorage = std::unordered_map<InputTag, PoolContent>;

        class tag_not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class duplicated_tag_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class constructor_tag_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        friend struct InputTag;

        void remove(const InputTag&, bool force = true);
        void remove_if_invalid(const InputTag&);

        boost::any reserve(const InputTag&);
        boost::any raw_get(const InputTag&);

        template<typename T, typename... Args> std::shared_ptr<T> put(const InputTag& tag, Args... args) {
            auto it = m_storage.find(tag);
            if (it != m_storage.end()) {
                if (it->second.valid)
                    throw duplicated_tag_error("A module already produced the tag '" + tag.toString() + "'");
                // A module already requested this block in read-mode. This will only work if the block does not require a non-trivial constructor:
                if (sizeof...(Args))
                    throw constructor_tag_error("A module already requested the tag '" + tag.toString() + "' which seems to require a constructor call. This is currently not supported.");
                // Since the memory is allocated, simply consider the block as valid.
                it->second.valid = true;

                // If the block is empty, it's a delayed instanciation. Simply flag the block as valid, and allocate memory for it
                if (it->second.ptr.empty()) {
                    std::shared_ptr<T> ptr(new T(std::forward<Args>(args)...));
                    it->second.ptr = boost::any(ptr);
                }

            } else {
                it = create<T, Args ...>(tag, true, std::forward<Args>(args)...);
            }

            // Update current module description
            assert(! m_current_module.empty());
            Description& description = m_description[m_current_module];
            description.outputs.push_back(tag.parameter);

            return boost::any_cast<std::shared_ptr<T>>(it->second.ptr);
        }

        template<typename T, typename... Args> PoolStorage::iterator create(const InputTag& tag,
                bool valid = true, Args... args) const {

            std::shared_ptr<T> ptr(new T(std::forward<Args>(args)...));
            PoolContent content = { boost::any(ptr), valid };

            return m_storage.emplace(tag, content).first;
        }

        /**
         * \brief Inform the pool of which module is currently created.
         *
         * It helps tracking down module dependencies.
         *
         * \param module The name of the module beeing created
         */
        virtual void current_module(const std::string& module) final;

        /**
         * \brief Freeze the memory pool.
         *
         * Once frozen, no modification is allowed inside the pool.
         *
         * \note This is not yet enforced
         */
        virtual void freeze() final;

        Pool() = default;
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        std::string m_current_module; /// Name of the module currently created.
        bool m_frozen = false; /// If true, no modification of the pool is allowed

        mutable PoolStorage m_storage;
        mutable DescriptionMap m_description; /// Mutable so that get() can be marked const
};

using PoolPtr = std::shared_ptr<Pool>;
