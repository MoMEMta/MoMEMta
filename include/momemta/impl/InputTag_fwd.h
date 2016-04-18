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

#include <boost/any.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <momemta/Utils.h>

class Pool;

struct InputTag {
    public:
        InputTag(const std::string& module, const std::string& parameter);
        InputTag(const std::string& module, const std::string& parameter, size_t index);
        InputTag() = default;

        /*!
         *  Check if a given string is an input tag. Expected format is
         *      Module::Parameter[/Index]
         *
         *  Delimiter is '::'.
         */
        static bool isInputTag(const std::string& tag);

        /*!
         * Create a input tag from a string. No check is performed to ensure that
         * the string is an input tag. Use `isInputTag` first.
         */
        static InputTag fromString(const std::string& tag);

        bool operator==(const InputTag& rhs) const;

        std::string toString() const;

        bool isIndexed() const;

        /**
         * \brief Link the InputTag to the memory pool
         *
         * \note You **must** call this function before calling InputTag::get()
         */
        void resolve(std::shared_ptr<Pool> pool) const;

        template<typename T> const T& get() const;

        std::string module;
        std::string parameter;
        size_t index;

    private:
        class tag_not_resolved_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        bool indexed = false;

        std::string string_representation;

        mutable bool resolved = false;
        mutable boost::any content;
        mutable std::shared_ptr<Pool> pool;
};

namespace std {
    template<>
        struct hash<InputTag> {
            size_t operator()(const InputTag& tag) const {
                return string_hash(tag.module) + string_hash(tag.parameter);
            }

            std::hash<std::string> string_hash;
        };
}
