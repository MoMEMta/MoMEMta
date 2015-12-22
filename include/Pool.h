#pragma once

#include <memory>
#include <unordered_map>

#include <boost/any.hpp>

#include <InputTag.h>

// A simple memory pool
class Pool {
    public:
        template<typename T> std::shared_ptr<const T> get(const InputTag& tag) {
            if (tag.isIndexed()) {
                throw std::invalid_argument("Indexed input tag cannot be passed as argument of the pool. Use the `get` function of the input tag to retrieve its content.");
            }

            auto it = m_pool.find(tag);
            if (it == m_pool.end())
                throw tag_not_found_error("No such tag in pool: " + tag.toString());

            boost::any& v = it->second;
            std::shared_ptr<T>& ptr = boost::any_cast<std::shared_ptr<T>&>(v);

            return std::const_pointer_cast<const T>(ptr);
        }

        template<typename T> std::shared_ptr<T> put(const InputTag& tag) {
            auto it = m_pool.find(tag);
            if (it != m_pool.end())
                throw duplicated_tag_error("A module already produced the tag '" + tag.toString() + "'");

            std::shared_ptr<T> ptr(new T());
            m_pool.emplace(tag, boost::any(ptr));

            return ptr;
        }

        void alias(const InputTag& from, const InputTag& to);

    private:
        friend class MEMpp;

        class tag_not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class duplicated_tag_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        friend struct InputTag;

        boost::any raw_get(const InputTag&);

        Pool() = default;
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        std::unordered_map<InputTag, boost::any> m_pool;
};

using PoolPtr = std::shared_ptr<Pool>;
