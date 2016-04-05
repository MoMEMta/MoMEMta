#include <algorithm>
#include <fstream>
#include <iostream>

#include <momemta/SLHAReader.h>

namespace SLHA {

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
    std::ifstream param_card;
    param_card.open(file_name.c_str(), std::ifstream::in);
    if (!param_card.good())
        throw invalid_card_error("Error while opening param card");
    char buf[200];
    std::string line;
    std::string block("");

    while (param_card.good()) {
        param_card.getline(buf, 200);
        line = buf;
        // Change to lowercase
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        if (line != "" && line[0] != '#') {
            if (block != "") {
                // Look for double index blocks
                int dindex1 = 0, dindex2 = 0;
                double value = 0;
                std::stringstream linestr2(line);
                if (linestr2 >> dindex1 >> dindex2 >> value) {
                    set_block_entry(block, {dindex1, dindex2}, value);
                    // Done with this line, read next
                    continue;
                }
                std::stringstream linestr1(line);
                // Look for single index blocks
                if (linestr1 >> dindex1 >> value) {
                    std::vector<int> indices {dindex1};
                    set_block_entry(block, indices, value);
                    // Done with this line, read next
                    continue;
                }
            }
            // Look for block
            if (line.find("block ") != line.npos) {
                line = line.substr(6);
                // Get rid of spaces between block and block name
                while (line[0] == ' ')
                    line = line.substr(1);
                // Now find end of block name
                size_t space_pos = line.find(' ');
                if (space_pos != line.npos)
                    line = line.substr(0, space_pos);
                block = line;
                continue;
            }
            // Look for decay
            if (line.find("decay ") == 0) {
                line = line.substr(6);
                block = "";
                std::stringstream linestr(line);
                int pdg_code;
                double value;
                if (linestr >> pdg_code >> value)
                    set_block_entry("decay", pdg_code, value);
                else
                    throw invalid_card_error("Wrong format for decay block " + line);
                continue;
            }
        }
    }

    if (_blocks.size() == 0)
        throw invalid_card_error("No information read from SLHA card");

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
