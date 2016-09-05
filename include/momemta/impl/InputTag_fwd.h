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

/** \brief An identifier of a module's output
 *
 * An InputTag embed two values:
 *  - The name of a module
 *  - The name of the output
 *
 * Optionally, it can also embed an index if the output symbolized by the InputTag is a vector. The InputTag is then an indexed InputTag.
 */
struct InputTag {
    public:
        /** \brief Construct a new InputTag
         *
         * \param module The name of the module
         * \param parameter The name of the output
         */
        InputTag(const std::string& module, const std::string& parameter);
        /** \brief Construct a new indexed InputTag
         *
         * \param module The name of the module
         * \param parameter The name of the output
         * \param index The index inside the collection pointed by the InputTag
         */
        InputTag(const std::string& module, const std::string& parameter, size_t index);
        /// \brief Construct an empty InputTag
        InputTag() = default;

        /*! \brief Check if a given string represent an InputTag.
         *
         * InputTag are represented as string in the following format:
         * 
         * ```
         * Module::Parameter[/Index]
         * ```
         *
         * Delimiter is `::` between the module's name and output, and `/` between the output and the index.
         *
         * \warning **Index start at 1, and not at 0** like in C++. We follow here the Lua conventation for indexing.
         *
         * \param tag The string to test
         *
         * \return True if \p tag describes an InputTag, false otherwise
         */
        static bool isInputTag(const std::string& tag);

        /*! \brief Create a input tag from its string representation
         *
         * \param tag The string representation of the InputTag
         *
         * \warning No check is performed to ensure that the string is an input tag. Use isInputTag() first.
         *
         * \return A new InputTag, as described by \p tag.
         */
        static InputTag fromString(const std::string& tag);

        /** \brief Test equality between two InputTags
         *
         * Two InputTags are equals if their module's name, output and index are the same
         *
         * \return True if the two InputTag are equals, false otherwise
         */
        bool operator==(const InputTag& rhs) const;

        /**
         * \brief Convert the InputTag in its string representation
         *
         * \return A string representation of the InputTag
         */
        std::string toString() const;

        /// \return True if the InputTag is indexed, false otherwise
        bool isIndexed() const;

        /// \return True if the InputTag is empty, false otherwise
        bool empty() const;

        std::string module; ///< The module's name
        std::string parameter; ///< The module's output
        size_t index; ///< The index. Only meaningful if isIndexed() returns true

    private:
        class tag_not_resolved_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        bool indexed = false;

        std::string string_representation;
};

namespace std {
    /// \brief std::hash specialization for InputTag
    template<>
        struct hash<InputTag> {
            /// Compute hash of InputTag
            size_t operator()(const InputTag& tag) const {
                return string_hash(tag.module) + string_hash(tag.parameter);
            }

            private:
                std::hash<std::string> string_hash;
        };
}
