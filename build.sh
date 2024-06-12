#!/bin/bash

# Make verbose:
# set -x

# Check if the build directory exists before removing it
if [ -d "build/" ]; then
    rm -r build/ || { echo "⛔ Failed to remove build directory."; exit 1; }
fi

# Check if the arguments "omp", "mpi" or "debug" has been passed from the command line
# to set all the possible combinations of compilation options

# Serial (debug)
if [ "$1" == "debug" ] && [ "$2" != "omp" ] && [ "$2" != "mpi" ]; then
    echo "ℹ️ Compiling in serial mode with 🚧 debug."
    cmake -S . -B build/ -Wno-dev -DOMP=OFF -DDEBUG=ON || { echo "⛔ CMake serial debug failed."; exit 1; }

# omp only (no debug)
elif [ "$1" == "omp" ]; then
    echo "🅾 Ⓜ 🅿 Compiling in parallel mode (OpenMP only)."
    cmake -S . -B build/ -Wno-dev -DOMP=ON || { echo "⛔ CMake parallel (OpenMP only) failed."; exit 1; }

# omp only (debug)
elif [ "$1" == "debug" ] && [ "$2" == "omp" ]; then
    echo "🅾 Ⓜ 🅿 Compiling in parallel mode (OpenMP only) with 🚧 debug."
    cmake -S . -B build/ -Wno-dev -DOMP=ON -DDEBUG=ON || { echo "⛔ CMake parallel (OpenMP only) debug failed."; exit 1; }

# mpi (no debug)
elif [ "$1" == "mpi" ]; then
    echo "Ⓜ 🅿 ℹ Compiling with MPI support."
    cmake -S . -B build/ -DMPI=ON || { echo "⛔ CMake MPI failed."; exit 1; }

# mpi (debug)
elif [ "$1" == "debug" ] && [ "$2" == "mpi" ]; then
    echo "Ⓜ 🅿 ℹ Compiling with MPI support in 🚧 debug."
    cmake -S . -B build/ -DMPI=ON -DDEBUG=ON || { echo "⛔ CMake MPI debug failed."; exit 1; }

# Serial (no debug) [DEFAULT]
else
    echo "ℹ️ Compiling in serial mode."
    cmake -S . -B build/ || { echo "⛔ CMake serial failed."; exit 1; }
fi

make -s -C build/ -j4 || { echo "⛔ Make failed."; exit 1; }

# set +x

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful."
else
    echo "⛔ Compilation failed."
    exit 1
fi
