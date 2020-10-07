cmake_minimum_required(VERSION 3.12)

project(example-allegro5-physfs LANGUAGES C)

# This is the file created by the built-in data archive generation, and is
# referenced in add_executable to compile and include it in to the program.
set(BUILTIN_DATA_SRCFILE "${CMAKE_CURRENT_BINARY_DIR}/generated/builtin_data.c")

# ---

find_package(PkgConfig REQUIRED)

pkg_check_modules(ALLEGRO5        REQUIRED IMPORTED_TARGET allegro-5)
pkg_check_modules(ALLEGRO5_IMAGE  REQUIRED IMPORTED_TARGET allegro_image-5)
pkg_check_modules(ALLEGRO5_PHYSFS REQUIRED IMPORTED_TARGET allegro_physfs-5)
pkg_check_modules(PHYSFS          REQUIRED IMPORTED_TARGET physfs)

# ---

add_executable(example-allegro5-physfs
	main.c
	${BUILTIN_DATA_SRCFILE}
)

target_link_libraries(example-allegro5-physfs
	PkgConfig::ALLEGRO5
	PkgConfig::ALLEGRO5_PHYSFS
	PkgConfig::ALLEGRO5_IMAGE
	PkgConfig::PHYSFS
)

add_dependencies(example-allegro5-physfs loose_file_target)
add_dependencies(example-allegro5-physfs data_archive_target)

# ---
# Loose file copying

# We keep the data/ prefix on the files so that they are copied to the data
# sub-directory in the output directory.
set(LOOSE_DATA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/data/")
set(LOOSE_DATA_FILES
	happy.png
)

set(LOOSE_DATA_SRC_FILES ${LOOSE_DATA_FILES})
list(TRANSFORM LOOSE_DATA_SRC_FILES PREPEND "${LOOSE_DATA_ROOT}/")

set(LOOSE_DATA_DEST_FILES ${LOOSE_DATA_FILES})
list(TRANSFORM LOOSE_DATA_DEST_FILES PREPEND "${CMAKE_CURRENT_BINARY_DIR}/data/")

add_custom_command(OUTPUT ${LOOSE_DATA_DEST_FILES}
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${LOOSE_DATA_SRC_FILES} "${CMAKE_CURRENT_BINARY_DIR}/data/"
	DEPENDS ${LOOSE_DATA_SRC_FILES}
)

add_custom_target(loose_file_target DEPENDS ${LOOSE_DATA_DEST_FILES})

# ---
# Data archive generation (data.zip)

set(DATA_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/data.zip")
set(DATA_ARCHIVE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/data_archive/")
set(DATA_ARCHIVE_FILES
	happy.png
)

set(DATA_ARCHIVE_DEPS ${BUILTIN_DATA_ARCHIVE_FILES})
list(TRANSFORM DATA_ARCHIVE_DEPS PREPEND "${BUILTIN_DATA_ARCHIVE_ROOT}/")

# Use the zip command to generate data.zip
# The "-Z store" option stores files uncompressed
add_custom_command(
	OUTPUT "${DATA_ARCHIVE}"
	COMMAND zip -Z store "${DATA_ARCHIVE}" *
	WORKING_DIRECTORY "${DATA_ARCHIVE_ROOT}"
	DEPENDS ${DATA_ARCHIVE_DEPS}
)

add_custom_target(data_archive_target DEPENDS "${DATA_ARCHIVE}")

# ---
# Built-in data archive generation

# To embed a zip file in to the executable image, a program is used to convert
# the bytes of a file to a C array. This is a relatively portable method, but
# not the most efficient.

set(BUILTIN_DATA_SYMBOL builtin_data)
set(BUILTIN_DATA_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/generated/builtin.zip")
set(BUILTIN_DATA_ARCHIVE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/data_builtin/")
set(BUILTIN_DATA_ARCHIVE_FILES
	happy.png
)

# Generate the executable 'bin2c' to convert resources to C source code
# The program takes 3 arguments: symbol name, input filename, output filename
add_executable(bin2c
	bin2c.c
)

set(BUILTIN_DATA_ARCHIVE_DEPS ${BUILTIN_DATA_ARCHIVE_FILES})
list(TRANSFORM BUILTIN_DATA_ARCHIVE_DEPS PREPEND "${BUILTIN_DATA_ARCHIVE_ROOT}/")

# Similar to generating data.zip, but additionally uses the generated bin2c
# program to convert builtin_data.zip to a C source file
#
# BUILTIN_DATA_SYMBOL determines the generated symbol names
# e.g. "builtin_data" is accessed in code as:
#   extern unsigned char builtin_data[];
#   extern size_t builtin_data_size;
add_custom_command(
	OUTPUT "${BUILTIN_DATA_SRCFILE}"
	COMMAND zip -Z store "${BUILTIN_DATA_ARCHIVE}" *
	COMMAND bin2c "${BUILTIN_DATA_SYMBOL}" "${BUILTIN_DATA_ARCHIVE}" "${BUILTIN_DATA_SRCFILE}"
	WORKING_DIRECTORY "${BUILTIN_DATA_ARCHIVE_ROOT}"
	BYPRODUCTS "${BUILTIN_DATA_ARCHIVE}"
	DEPENDS ${BUILTIN_DATA_ARCHIVE_DEPS}
)

