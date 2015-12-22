#include <Pool.h>

boost::any Pool::raw_get(const InputTag& tag) {
    auto it = m_pool.find(tag);
    if (it == m_pool.end())
        throw tag_not_found_error("No such tag in pool: " + tag.toString());
    return it->second;
}

void Pool::alias(const InputTag& from, const InputTag& to) {
    if (from.isIndexed() || to.isIndexed()) {
        throw std::invalid_argument("Indexed input tag cannot be passed as argument of the pool. Use the `get` function of the input tag to retrieve its content.");
    }

    auto from_it = m_pool.find(from);
    if (from_it == m_pool.end())
        throw tag_not_found_error("No such tag in pool: " + from.toString());

    auto to_it = m_pool.find(to);
    if (to_it != m_pool.end())
        throw duplicated_tag_error("A module already produced the tag '" + to.toString() + "'");

    m_pool[to] = m_pool[from];
}
