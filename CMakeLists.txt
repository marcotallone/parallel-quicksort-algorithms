# ----------------------------- CMAKE SETTINGS ------------------------------ #

# Minimum Cmake version required
cmake_minimum_required(VERSION 3.28.1)

# Suppress messages
set(CMAKE_RULE_MESSAGES OFF CACHE INTERNAL "")
set(CMAKE_VERBOSE_MAKEFILE OFF CACHE INTERNAL "")

# Project description
project(ParallelQuickSort
		VERSION 1.0
		DESCRIPTION "Parallel QuickSort algorithms in C using OpenMP and the OpenMPI library"
		LANGUAGES C
)

# ------------------- C OPTIONS AND COMPILATION FLAGS ----------------------- #

# C Standard
set(CMAKE_C_STANDARD 11)

# Option serial or parallel compilation and for debugging
option(OMP "Use OpenMP for parallel compilation" OFF)
option(MPI "Use MPI for parallel compilation" OFF)
option(DEBUG "Enable debug mode" OFF)

# If MPI or OMP are set, search for the libraries
if(MPI)
	# Set the OMP variable true because always needed for MPI
	set(OMP ON)

	# Set a definition to include the mpi header
	add_definitions(-DUSE_MPI)

	find_package(MPI REQUIRED)
	if(MPI_FOUND)
		message(STATUS "✅ MPI Library Found")
		include_directories(${MPI_INCLUDE_PATH})
	else()
		message(FATAL_ERROR "⛔ MPI Library Not Found: Please install it or provide path in order to continue")
	endif()
endif()

if (OMP)
	find_package(OpenMP REQUIRED)
	if(OPENMP_FOUND)
		message(STATUS "✅ OpenMP Library Found")
	else()
		message(FATAL_ERROR "⛔ OpenMP Library Not Found:
		Please install it or provide path in order to continue")
	endif()
endif()

# Compilation flags for serial, parallel (either OMP only or MPI+OMP) and debug (all modes)

# Serial case, no debug
if (NOT OMP AND NOT MPI AND NOT DEBUG)
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror")
	# set(CMAKE_C_FLAGS "-Wall")
	set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native")
endif()

# Serial case, debug
if (NOT OMP AND NOT MPI AND DEBUG)
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -g -O0 -DDEBUG")
	# set(CMAKE_C_FLAGS "-Wall -g -O0 -DDEBUG")
endif()

# Parallel case (both OMP and MPI), no debug
if (OMP AND MPI AND NOT DEBUG)
	set(CMAKE_C_COMPILER mpicc)
	set(CMAKE_CXX_COMPILER mpicxx)
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -fopenmp")
	# set(CMAKE_C_FLAGS "-Wall -fopenmp")
	set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native")
endif()

# Parallel case (both OMP and MPI), debug
if (OMP AND MPI AND DEBUG)
	set(CMAKE_C_COMPILER mpicc)
	set(CMAKE_CXX_COMPILER mpicxx)
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -fopenmp -g -O0 -DDEBUG")
	# set(CMAKE_C_FLAGS "-Wall -fopenmp -g -O0 -DDEBUG")
endif()

# Parallel case (OMP only), no debug
if (OMP AND NOT MPI AND NOT DEBUG)
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -fopenmp")
	# set(CMAKE_C_FLAGS "-Wall -fopenmp")
	set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native")
endif()

# Parallel case (OMP only), debug
if (OMP AND NOT MPI AND DEBUG)
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -fopenmp -g -O0 -DDEBUG")
	# set(CMAKE_C_FLAGS "-Wall -fopenmp -g -O0 -DDEBUG")
endif()

# ------------------------------ FILES SETTINGS ----------------------------- #

# Option for a shared or static library (default is shared)
option(STATIC "Build a static library" OFF)

# If either SHARED or STATIC is set, choose the library name
set(LIBRARY_NAME "qsort_shared")
if (STATIC)
	set(LIBRARY_NAME "qsort_static")
endif()

# Setting the list of source files
set(SOURCE_FILES
	assessment.c
	serial_qsort.c
)

# Setting the list of main files
set(MAIN_FILES
	display.c
)

if (OMP)

	list(APPEND MAIN_FILES 
		omp_example.c
		omp_scaling.c
	)

	list(APPEND SOURCE_FILES
		omp_task_qsort.c
		omp_parallel_qsort.c
		omp_hyperquicksort.c
		omp_psrs.c
	)
endif()

if (MPI)
	list(APPEND MAIN_FILES 
		mpi_example.c
		mpi_scaling.c
	)

	list(APPEND SOURCE_FILES
		mpi_parallel_qsort.c
		mpi_hyperquicksort.c
		mpi_psrs.c
	)
endif()

# ----------------------------- DIRECTORIES --------------------------------- #

# Include directories
set(INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Adding subdirectories
add_subdirectory(src)  
add_subdirectory(apps) 
