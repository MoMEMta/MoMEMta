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

#include <momemta/ModuleFactory.h>
#include <momemta/Pool.h>

class Module {
    public:
        Module(PoolPtr pool, const std::string& name):
            m_name(name), m_pool(pool) {
            // Empty
        }

        virtual void configure() { };
        virtual void work() { };
        virtual void finish() { };

        virtual size_t dimensions() const {
            return 0;
        }

        virtual std::string name() const final {
            return m_name;
        }

    protected:
        template<typename T> std::shared_ptr<T> produce(const std::string& name) {
            return m_pool->put<T>({m_name, name});
        }

        template<typename T> std::shared_ptr<const T> get(const std::string& module, const std::string& name) {
            return m_pool->get<T>({module, name});
        }

        template<typename T> std::shared_ptr<const T> get(const InputTag& tag) {
            return m_pool->get<T>(tag);
        }

    private:
        std::string m_name;

    protected:
        PoolPtr m_pool;
};

using ModulePtr = std::shared_ptr<Module>;
