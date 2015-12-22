#pragma once

#include <boost/any.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& s, const std::string& delimiters);

class Pool;

struct InputTag {
    public:
        InputTag(const std::string& module, const std::string& parameter):
            module(module), parameter(parameter) {
            string_representation = module + "::" + parameter;
        }

        InputTag(const std::string& module, const std::string& parameter, size_t index):
            module(module), parameter(parameter), indexed(true), index(index) {
            string_representation = module + "::" + parameter + "/" + std::to_string(index);
        }

        InputTag() = default;

        /*!
         *  Check if a given string is an input tag. Expected format is
         *      Module::Parameter[/Index]
         *
         *  Delimiter is '::'.
         */
        static bool isInputTag(const std::string& tag) {
            if (tag.find("::") == std::string::npos)
                return false;

            if (tag.find("/") != std::string::npos) {
                auto tags = split(tag, "::");
                auto rtags = split(tags[1], "/");
                try {
                    size_t index = std::stoull(rtags[1]);
                    return true;
                } catch (std::invalid_argument e) {
                    return false;
                }
            } else {
                return true;
            }
        }

        /*!
         * Create a input tag from a string. No check is performed to ensure that
         * the string is an input tag. Use `isInputTag` first.
         */
        static InputTag fromString(const std::string& tag) {
            auto tags = split(tag, "::");
            auto rtags = split(tags[1], "/");

            if (rtags.size() == 1)
                return InputTag(tags[0], tags[1]);
            else
                return InputTag(tags[0], rtags[0], std::stoull(rtags[1]));
        }

        bool operator==(const InputTag& rhs) const {
            return ((module == rhs.module) && (parameter == rhs.parameter));
        }

        std::string toString() const {
            return string_representation;
        }

        bool isIndexed() const {
            return indexed;
        }

        void resolve(std::shared_ptr<Pool>) const;

        template<typename T> const T& get() const {
            if (! resolved) {
                throw tag_not_resolved_error("You must call 'resolve' once before calling 'get'"); 
            }

            if (isIndexed()) {
                auto ptr = boost::any_cast<std::shared_ptr<std::vector<T>>>(content);
                return (*ptr)[index];
            } else {
                auto ptr = boost::any_cast<std::shared_ptr<T>>(content);
                return (*ptr);
            }
        }

        std::string module;
        std::string parameter;

    private:
        class tag_not_resolved_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        bool indexed = false;
        size_t index;

        std::string string_representation;

        mutable bool resolved = false;
        mutable boost::any content;
};

namespace std {
    template<>
        struct hash<InputTag> {
            size_t operator()(const InputTag& tag) const {
                return string_hash(tag.module) + string_hash(tag.parameter);
            }

            std::hash<std::string> string_hash;
        };

};
