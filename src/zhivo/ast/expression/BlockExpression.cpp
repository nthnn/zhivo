/*
 * Copyright (c) 2024 - Nathanne Isip
 * This file is part of Zhivo.
 * 
 * Zhivo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * Zhivo is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Zhivo. If not, see <https://www.gnu.org/licenses/>.
 */

#include <zhivo/ast/expression/BlockExpression.hpp>
#include <zhivo/core/SymbolTable.hpp>
#include <memory>

DynamicObject BlockExpression::visit(SymbolTable& symbols) {
    SymbolTable& table(symbols);
    DynamicObject value;

    try {
        for(auto& stmt : this->statements)
            value = stmt->visit(table);
    }
    catch(const TerminativeReturnSignal& rs) {
        return rs.getObject();
    }

    return value;
}
