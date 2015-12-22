#include <InputTag.h>
#include <Pool.h>

std::vector<std::string> split(const std::string& s, const std::string& delimiters) {

    std::vector<std::string> result;

    size_t current;
    size_t next = -1;
    do
    {
        next = s.find_first_not_of(delimiters, next + 1);
        if (next == std::string::npos)
            break;
        next -= 1;

        current = next + 1;
        next = s.find_first_of(delimiters, current);
        result.push_back(s.substr(current, next - current));
    }
    while (next != std::string::npos);

    return result;
}

void InputTag::resolve(PoolPtr pool) const {
    if (resolved)
        return;

    content = pool->raw_get(*this);
    resolved = true;
}
