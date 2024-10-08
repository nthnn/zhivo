# Copyright (c) 2024 - Nathanne Isip
# This file is part of Zhivo.
# 
# Zhivo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
# 
# Zhivo is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Zhivo. If not, see <https://www.gnu.org/licenses/>.

import os
import platform
import subprocess
import sys

PLATFORM = platform.system()
COMPILER = 'g++'
OUT_DIR = 'dist'
OUTPUT_EXECUTABLE = os.path.join(
    'dist',
    'zhivo-' +
        sys.platform + '-' +
        platform.machine().lower()
)

cpp_files = []
for root, dirs, files in os.walk('src'):
    for file in files:
        if file.endswith('.cpp'):
            cpp_files.append(os.path.join(root, file))

if not cpp_files:
    print("No .cpp files found in the src directory.")
    exit(1)

if PLATFORM == 'Darwin':
    COMPILER = '/opt/homebrew/opt/llvm/bin/clang++'

gpp_command = [
    COMPILER, '-Iinclude',
    '-Wall', '-pedantic', '-Wdisabled-optimization',
    '-pedantic-errors', '-Wextra', '-Wcast-align',
    '-Wcast-qual', '-Wchar-subscripts', '-Wcomment',
    '-Wconversion', '-Werror', '-Wfloat-equal', '-Wformat',
    '-Wformat=2', '-Wformat-nonliteral', '-Wformat-security',
    '-Wformat-y2k', '-Wimport', '-Winit-self', '-Winvalid-pch',
    '-Wunsafe-loop-optimizations', '-Wlong-long',
    '-Wmissing-braces', '-Wmissing-field-initializers',
    '-Wmissing-format-attribute', '-Wmissing-include-dirs',
    '-Weffc++', '-Wpacked', '-Wparentheses', '-Wpointer-arith',
    '-Wredundant-decls', '-Wreturn-type', '-Wsequence-point',
    '-Wshadow', '-Wsign-compare', '-Wstack-protector',
    '-Wstrict-aliasing', '-Wstrict-aliasing=2', '-Wswitch',
    '-Wswitch-default', '-Wswitch-enum', '-Wtrigraphs',
    '-Wuninitialized', '-Wunknown-pragmas', '-Wunreachable-code',
    '-Wunused', '-Wunused-function', '-Wunused-label',
    '-Wunused-parameter', '-Wunused-value', '-Wunused-variable',
    '-Wvariadic-macros', '-Wvolatile-register-var', '-Wwrite-strings',
    '-pipe', '-Ofast', '-s', '-std=c++17', '-fopenmp',
    '-msse', '-msse2', '-msse3', '-mfpmath=sse',
    '-march=native', '-funroll-loops', '-ffast-math'
]

if PLATFORM != 'Windows':
    gpp_command.append('-flto=auto')

if PLATFORM == 'Darwin':
    gpp_command.append('-Xpreprocessor')
    gpp_command.append('-O3')
    gpp_command.append('-Wno-header-guard')
    gpp_command.append('-Wno-pessimizing-move')
    gpp_command.remove('-Wunsafe-loop-optimizations')
    gpp_command.remove('-Wvolatile-register-var')
    gpp_command.remove('-Weffc++')
    gpp_command.remove('-Ofast')
    gpp_command.remove('-msse')
    gpp_command.remove('-msse2')
    gpp_command.remove('-msse3')
    gpp_command.remove('-mfpmath=sse')
    gpp_command.remove('-s')

gpp_command += cpp_files + ['-o', OUTPUT_EXECUTABLE]
nvcc_command = ['nvcc']

if PLATFORM == 'Windows':
    nvcc_command.append('-Xcompiler')
    nvcc_command.append('/std:c++17')

nvcc_command.append('-Iinclude')
nvcc_command += cpp_files + ['-o', OUTPUT_EXECUTABLE + '-cuda']

try:
    os.makedirs(OUT_DIR, exist_ok=True)

    print("Compiling with command:")
    print(' '.join(gpp_command))

    subprocess.run(gpp_command, check=True)
    print(f"Compilation successful! Executable created at: {OUTPUT_EXECUTABLE}")

    if PLATFORM != 'Darwin':
        print("Compiling for CUDA with command:")
        print(' '.join(nvcc_command))

        subprocess.run(nvcc_command, check=True)
        print(f"Compilation successful! Executable created at: {OUTPUT_EXECUTABLE + '-cuda'}")

except subprocess.CalledProcessError as e:
    print(f"Compilation failed with error: {e}")
