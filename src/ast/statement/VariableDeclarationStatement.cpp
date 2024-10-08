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

#include <ast/ASTNodeException.hpp>
#include <ast/statement/VariableDeclarationStatement.hpp>
#include <core/Runtime.hpp>
#include <core/SymbolTable.hpp>

#if defined(__unix__) || defined(__linux__)
#   include <dlfcn.h>
#elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#   include <Windows.h>
#else
#   error "Unsupported architecture for shared objects or dynamic libraries."
#endif

DynamicObject VariableDeclarationStatement::visit(SymbolTable& symbols) {
    if(!this->nativePath.empty()) {
        for(const auto& [key, value] : this->declarations) {
            std::string name = key.getImage();

            symbols.setSymbol(name, DynamicObject(
                VariableDeclarationStatement::loadNativeFunction(
                    this->nativePath,
                    name,
                    std::move(this->address)
                )
            ));
        }

        return {};
    }

    for(const auto& [key, value] : this->declarations)
        symbols.setSymbol(
            key.getImage(),
            value->visit(symbols)
        );

    return {};
}

NativeFunction VariableDeclarationStatement::loadNativeFunction(
    std::string& libName,
    std::string& funcName,
    std::unique_ptr<Token> address
) {
    void* handle;
    if(Runtime::hasLoadedLibrary(libName))
        #if defined(__unix__) || defined(__linux__)
        handle = Runtime::getLoadedLibrary(libName);
        #elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
        handle = static_cast<HMODULE>(
            Runtime::getLoadedLibrary(libName)
        );
        #endif
    else {
        #if defined(__unix__) || defined(__linux__)
        handle = dlopen(libName.c_str(), RTLD_LAZY);
        #elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
        handle = LoadLibrary(libName.c_str());
        #endif

        Runtime::addLoadedLibrary(libName, handle);
    }

    if(!handle) {
        #if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
        DWORD errorMessageId = GetLastError();
        LPSTR messageBuffer = nullptr;

        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorMessageId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR) &messageBuffer,
            0,
            NULL
        );

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        #endif

        throw ASTNodeException(
            std::move(address),
            "Failed to load library: " + libName +
            "\r\n                 " +
            #if defined(__unix__) || defined(__linux__)
            dlerror()
            #elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
            std::string(message)
            #endif
        );
    }

    std::string name = funcName;
    std::replace(name.begin(), name.end(), '.', '_');

    #if defined(__unix__) || defined(__linux__)

    auto func = reinterpret_cast<NativeFunction>(dlsym(handle, name.c_str()));

    #elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    #   pragma GCC diagnostic push
    #   pragma GCC diagnostic ignored "-Wcast-function-type"

    auto func = reinterpret_cast<NativeFunction>(GetProcAddress(
        (HMODULE) handle,
        name.c_str()
    ));

    #   pragma GCC diagnostic pop
    #endif

    if(!func) {
        #if defined(__unix__) || defined(__linux__)
        dlclose(handle);
        #elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
        FreeLibrary((HMODULE) handle);
        #endif

        throw std::runtime_error("Failed to find function: " + funcName);
    }

    return func;
}
