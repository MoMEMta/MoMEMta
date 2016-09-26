#ifndef READ_SLHA_H
#define READ_SLHA_H

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace SLHA {

class Block {
  public:
    Block(const std::string& name = "");
    virtual ~Block() {}

    void set_entry(const std::vector<int>& indices, double value);
    double get_entry(const std::vector<int>& indices, double def_val = 0) const;
    void set_name(const std::string& name);
    std::string get_name() const;
    int get_indices() const;

  private:
    std::string _name;
    std::map<std::vector<int>, double> _entries;
    unsigned int _indices;
};

class Reader {
  public:
    Reader(const std::string& file_name = "");

    void read_slha_file(const std::string& file_name);
    double get_block_entry(const std::string& block_name, const std::vector<int>& indices,
                           double def_val = 0) const;
    double get_block_entry(const std::string& block_name, int index, double def_val = 0) const;
    void set_block_entry(const std::string& block_name, const std::vector<int>& indices, double value);
    void set_block_entry(const std::string& block_name, int index, double value);

  private:
    std::map<std::string, Block> _blocks;
};
}

#endif
