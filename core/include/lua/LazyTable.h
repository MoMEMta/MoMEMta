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

#include <memory>
#include <string>

#include <momemta/ParameterSet.h>

#include <lua/utils.h>

namespace lua {

/**
 * \brief Lazy table field in lua (delayed table access)
 *
 * It's a wrapper around table's field access. Evaluation of this lazy value means accessing the field of the table.
 */
struct LazyTableField : public Lazy {
    std::string table_name; ///< The name of the global table
    std::string key; ///< The name of the field inside the table to retrieve when evaluated

    virtual boost::any operator()() const override;

    /**
     * \brief Ensure the global table referenced by this `struct` exist. If not, create it.
     */
    void ensure_created();

    /**
     * \brief Replace the value of the table field by a new one
     */
    void set(const boost::any& value);

    LazyTableField(lua_State *L, const std::string& table_name, const std::string& key);
};

/** \brief A specialization of ParameterSet for lazy loading of lua tables
 *
 * This class is used to represent global tables that can be modified **after** the parsing of the configuration file (like the global `parameters` table). This means that each field of the table *must* have a delayed evaluation (see lua::LazyTableField).
 *
 * Actual evaluation of the fields of the table happens during the freezing of this ParameterSet.
 *
 */

class LazyTable: public ParameterSet {
    friend class ConfigurationReader;

public:
    LazyTable(std::shared_ptr<lua_State> L, const std::string& name);
    virtual LazyTable* clone() const override;

protected:
    virtual void create(const std::string& name, const boost::any& value) override;
    virtual void setInternal(const std::string& name, Element& element, const boost::any& value) override;

    virtual bool lazy() const override;

    virtual void freeze() override;

private:
    std::shared_ptr<lua_State> m_lua_state;
};

}