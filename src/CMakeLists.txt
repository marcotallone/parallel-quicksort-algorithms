# Library Compilation message
message(STATUS "🛠 Compiling library:     ${LIBRARY_NAME}")

# Setting lib directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Including directories
include_directories(${INCLUDE_DIRS})

# Compiling C++ libraries
# If the SHARED option is enabled (default)
add_library(${LIBRARY_NAME} SHARED ${SOURCE_FILES})
if (STATIC)
    # If the STATIC option is enabled
    add_library(${LIBRARY_NAME} STATIC ${SOURCE_FILES})
endif()

# Setting library version
set_target_properties(${LIBRARY_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)

# Linking libraries (if needed)
target_link_libraries(${LIBRARY_NAME} PRIVATE m)
# target_link_libraries(${LIBRARY_NAME} PRIVATE ${THIRD_PARTY_LIBRARIES})

# Linking to OpenMPI if MPI option is enabled (no debug)
if (MPI AND NOT DEBUG)
    target_link_libraries(${LIBRARY_NAME} PRIVATE MPI::MPI_C)

    if(MPI_COMPILE_FLAGS)
      set_target_properties(${LIBRARY_NAME} PROPERTIES
        COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
    endif()

    if(MPI_LINK_FLAGS)
      set_target_properties(${LIBRARY_NAME} PROPERTIES
        LINK_FLAGS "${MPI_LINK_FLAGS}")
    endif()
endif()

# Linking to OpenMPI (debug)
if (MPI AND DEBUG)
    target_link_libraries(${LIBRARY_NAME} PRIVATE MPI::MPI_C)

    if(MPI_COMPILE_FLAGS_DEBUG)
      set_target_properties(${LIBRARY_NAME} PROPERTIES
        COMPILE_FLAGS "${MPI_COMPILE_FLAGS} -g -O0 -DDEBUG")
    endif()

    if(MPI_LINK_FLAGS_DEBUG)
      set_target_properties(${LIBRARY_NAME} PROPERTIES
        LINK_FLAGS "${MPI_LINK_FLAGS}")
    endif()
endif()