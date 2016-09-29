#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>

#include <momemta/SLHAReader.h>

namespace SLHA {

std::vector<std::string> parse_line_elements(const std::string& line, char delimiter=' ') {
    std::vector<std::string> elements;
    std::string temp;
    std::stringstream linestr(line);
    while (getline(linestr, temp, delimiter)) {
        if (temp == std::string(&delimiter, 1))
            continue;
        elements.push_back(temp);
    }
    return elements;
}

class invalid_card_error: public std::runtime_error {
    using std::runtime_error::runtime_error;
};

Block::Block(const std::string& name /* = ""*/) { _name = name; }

void Block::set_name(const std::string& name) { _name = name; }

std::string Block::get_name() const { return _name; }

int Block::get_indices() const { return _indices; }

void Block::set_entry(const std::vector<int>& indices, double value) {
    if (_entries.empty())
        _indices = indices.size();
    else if (indices.size() != _indices)
        throw invalid_card_error("Wrong number of indices in set_entry");

    _entries[indices] = value;
}

double Block::get_entry(const std::vector<int>& indices, double def_val) const {
    auto it = _entries.find(indices);
    if (it == _entries.end()) {
        return def_val;
    }

    return it->second;
}

Reader::Reader(const std::string& file_name /* = ""*/) {
    if (!file_name.empty())
        read_slha_file(file_name);
}

void Reader::read_slha_file(const std::string& file_name) {
    std::ifstream param_card(file_name.c_str(), std::ifstream::in);
    if (!param_card.is_open())
        throw invalid_card_error("Error while opening param card");
    std::string line;
    std::string block("");

    try {

        while (std::getline(param_card, line)) {
            // Change to lowercase
            std::transform(line.begin(), line.end(), line.begin(), ::tolower);
            // Remove comments
            line = std::regex_replace(line, std::regex("#.*"), "");
            // Remove leading, trailing and duplicate whitespaces
            line = std::regex_replace(line, std::regex("^ *| *$"), "");
            line = std::regex_replace(line, std::regex(" +"), " ");
            
            if (line == "")
                continue;
            
            // Look for block
            if (line.find("block ") != line.npos) {
                line = line.substr(6);
                // Find end of block name
                size_t space_pos = line.find(' ');
                if (space_pos != line.npos)
                    line = line.substr(0, space_pos);
                block = line;
                continue;
            }
            // Look for decay (not part of a block)
            if (line.find("decay ") == 0) {
                line = line.substr(6);
                block = "";
                auto elements = parse_line_elements(line);
                if (elements.size() == 2) {
                    int pdg_id = std::stoi(elements[0]);
                    double value = std::stod(elements[1]);
                    set_block_entry("decay", pdg_id, value);
                } else {
                    throw invalid_card_error("Wrong format for decay block " + line);
                }
                continue;
            }
            // We have a block, now read this block's entries line by line
            if (block != "") {
                auto elements = parse_line_elements(line);
                // Look for double index blocks
                if (elements.size() == 3) {
                    int id1 = std::stoi(elements[0]);
                    int id2 = std::stoi(elements[1]);
                    double value = std::stod(elements[2]);
                    set_block_entry(block, { id1, id2 }, value);
                    // Done with this line, read next
                    continue;
                } else if (elements.size() == 2) {
                // Look for single index blocks
                    int id = std::stoi(elements[0]);
                    double value = std::stod(elements[1]);
                    set_block_entry(block, { id }, value);
                    // Done with this line, read next
                    continue;
                } else {
                    throw invalid_card_error("Wrong format for entry in block " + block + "; line: " + line);
                }
            }
        }

        if (_blocks.size() == 0)
            throw invalid_card_error("No information read from SLHA card");

    // End try
    } catch(std::exception &e) {
        param_card.close();
        throw e;
    }

    param_card.close();
}

double Reader::get_block_entry(const std::string& block_name, const std::vector<int>& indices, double def_val) const {
    auto it = _blocks.find(block_name);
    if (it == _blocks.end()) {
        return def_val;
    }

    return it->second.get_entry(indices, def_val);
}

double Reader::get_block_entry(const std::string& block_name, int index, double def_val) const {
    std::vector<int> indices { index };
    return get_block_entry(block_name, indices, def_val);
}

void Reader::set_block_entry(const std::string& block_name, const std::vector<int>& indices, double value) {
    auto it = _blocks.find(block_name);
    if (it == _blocks.end()) {
        Block block(block_name);
        _blocks[block_name] = block;
        it = _blocks.find(block_name);
    }
    it->second.set_entry(indices, value);
}

void Reader::set_block_entry(const std::string& block_name, int index, double value) {
    std::vector<int> indices { index };
    set_block_entry(block_name, indices, value);
}

}
