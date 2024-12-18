/*
 * Copyright (c) 2024 - Nathanne Isip
 * This file is part of N8.
 * 
 * N8 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * N8 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with N8. If not, see <https://www.gnu.org/licenses/>.
 */

#include <n8/ast/ASTNodeException.hpp>
#include <n8/core/Runtime.hpp>
#include <n8/core/SymbolTable.hpp>

SymbolTable& SymbolTable::operator=(const SymbolTable& other) {
    if(this != &other) {
        this->parent = std::move(other.parent);
        this->table = std::move(other.table);
    }

    return *this;
}

SymbolTable::~SymbolTable() {
    for(std::thread& t : this->threads)
        if(t.joinable())
            t.join();

    this->threads.clear();
}

DynamicObject SymbolTable::getSymbol(
    std::shared_ptr<Token> reference,
    const std::string& name
) {
    if(this->hasSymbol(name))
        return std::move(this->table[name]);
    else if(this->parent)
        return this->parent->getSymbol(
            std::move(reference),
            name
        );

    #ifdef _MSC_VER
    #   pragma warning(disable : 5272)
    #endif
    throw ASTNodeException(
        std::move(reference),
        "Cannot resolve symbol: " + name
    );
}

void SymbolTable::setSymbol(const std::string& name, DynamicObject value) {
    std::lock_guard<std::mutex> lock(this->mtx);

    if(this->hasSymbol(name))
        this->table[name] = std::move(value);
    else if(this->parent && this->parent->hasSymbol(name))
        this->parent->setSymbol(name, std::move(value));

    this->table[name] = std::move(value);
}

void SymbolTable::removeSymbol(const std::string& name) {
    if(this->hasSymbol(name))
        this->table.erase(name);
}

bool SymbolTable::hasSymbol(const std::string& name) {
    return this->table.find(name) != this->table.end() ||
        (this->parent && this->parent->hasSymbol(name));
}

void SymbolTable::addParallelism(std::thread par) {
    this->threads.push_back(std::move(par));
}

void SymbolTable::waitForThreads() {
    if(!this->threads.empty()) {
        #pragma omp parallel for
        for(auto& thread : this->threads)
            if(thread.joinable())
                thread.join();
        this->threads.clear();
    }

    this->mtx.unlock();
}

void SymbolTable::detachParallelNodes() {
    if(this->threads.empty())
        return;

    #pragma omp parallel for
    for(auto& thread : this->threads)
        thread.detach();

    this->threads.clear();
    this->mtx.unlock();

    Runtime::cleanUp();
}