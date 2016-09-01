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


#include <momemta/InputTag.h>

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

InputTag::InputTag(const std::string& module, const std::string& parameter):
    module(module), parameter(parameter) {
    string_representation = module + "::" + parameter;
}

InputTag::InputTag(const std::string& module, const std::string& parameter, size_t index):
    module(module), parameter(parameter), index(index), indexed(true) {
    string_representation = module + "::" + parameter + "/" + std::to_string(index + 1);
}

bool InputTag::isInputTag(const std::string& tag) {
    if (tag.find("::") == std::string::npos)
        return false;

    if (tag.find("/") != std::string::npos) {
        auto tags = split(tag, "::");
        auto rtags = split(tags[1], "/");
        try {
            int64_t index = std::stoll(rtags[1]) - 1;
            if (index < 0)
                return false;

            return true;
        } catch (std::invalid_argument e) {
            return false;
        }
    } else {
        return true;
    }
}

InputTag InputTag::fromString(const std::string& tag) {
    auto tags = split(tag, "::");
    auto rtags = split(tags[1], "/");

    if (rtags.size() == 1)
        return InputTag(tags[0], tags[1]);
    else
        return InputTag(tags[0], rtags[0], std::stoull(rtags[1]) - 1);
}

bool InputTag::operator==(const InputTag& rhs) const {
    return ((module == rhs.module) && (parameter == rhs.parameter));
}

std::string InputTag::toString() const {
    return string_representation;
}

bool InputTag::isIndexed() const {
    return indexed;
}

bool InputTag::empty() const {
    return module.empty() || parameter.empty();
}