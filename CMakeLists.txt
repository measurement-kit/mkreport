cmake_minimum_required(VERSION 3.1.0)
project(mkreport LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(MkUtils)

# Download dependencies
# ---------------------

MkDownloadMMDBDatabases()
MkDownloadCatchorgCatch2()
MkDownloadMeasurementKitMkBouncer()
MkDownloadMeasurementKitMkCollector()
MkDownloadMeasurementKitMkIPLookup()
MkDownloadMeasurementKitMkMock()
MkDownloadMeasurementKitMkMMDB()
MkDownloadAdishavitArgh()
MkDownloadHowardHinnantDate()
MkDownloadNlohmannJSON()

if("${MSVC}")
  MkDownloadMeasurementKitPrebuiltWindowsLibmaxminddb()
  list(APPEND CMAKE_REQUIRED_INCLUDES "${MK_WINDOWS_LIBMAXMINDDB_INCLUDE_PATH}")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_WINDOWS_LIBMAXMINDDB_LIBRARY_PATH}")
endif()

# Checks
# ------

include(CheckIncludeFiles)

check_include_files(maxminddb.h MKREPORT_HAVE_MAXMINDDB_H)
find_library(LIBMAXMINDDB_LIBRARY maxminddb)
if (("${MKREPORT_HAVE_MAXMINDDB_H}" STREQUAL "") OR
    ("${LIBMAXMINDDB_LIBRARY}" STREQUAL "LIBMAXMINDDB_LIBRARY-NOTFOUND"))
  message(FATAL_ERROR "Cannot find libmaxminddb")
endif()
list(APPEND MKREPORT_LIBS "${LIBMAXMINDDB_LIBRARY}")

# Compiler flags
# --------------

MkSetCompilerFlags()

# Library and binary
# ------------------

set(MKREPORT_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_REQUIRED_INCLUDES})

include_directories(${MKREPORT_INCLUDES})

if("${WIN32}" OR "${MINGW}")
  list(APPEND MKREPORT_LIBS "ws2_32")
  if ("${MINGW}")
      list(APPEND MKREPORT_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()
list(APPEND MKREPORT_LIBS Threads::Threads)
link_libraries("${MKREPORT_LIBS}")

add_library(mkreport mkreport.cpp)
add_executable(mkreport-client mkreport-client.cpp)
target_link_libraries(mkreport-client mkreport)
add_executable(tests tests.cpp)
target_link_libraries(tests mkreport)

# Testing
# -------

set(BUILD_TESTING "ON" CACHE BOOL "Whether to build tests")
if(${BUILD_TESTING})
  enable_testing()
  add_test(NAME mocked_tests COMMAND tests)
endif()
