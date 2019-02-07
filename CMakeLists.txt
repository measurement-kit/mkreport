cmake_minimum_required(VERSION 3.1.0)
project(mkreport LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(MkUtils)

# Download dependencies
# ---------------------

MkDownloadCaBundle()
MkDownloadMMDBDatabases()
MkDownloadCatchorgCatch2()
MkDownloadMeasurementKitMkBouncer()
MkDownloadMeasurementKitMkCollector()
MkDownloadMeasurementKitMkCurl()
MkDownloadMeasurementKitMkData()
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
  MkDownloadMeasurementKitPrebuiltWindowsCurl()
  list(APPEND CMAKE_INCLUDE_PATH "${MK_WINDOWS_CURL_INCLUDE_PATH}")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_WINDOWS_CURL_LIBRARY_PATH}")
  add_definitions(${MK_WINDOWS_CURL_DEFINITIONS})
  list(APPEND MKREPORT_LIBS ${MK_WINDOWS_CURL_EXTRA_LINK_LIBS})
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

find_package(CURL REQUIRED)

# Compiler flags
# --------------

MkSetCompilerFlags()

# Library and binary
# ------------------

set(MKREPORT_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CURL_REQUIRED_INCLUDES} ${CMAKE_REQUIRED_INCLUDES})

include_directories(${MKREPORT_INCLUDES})

list(APPEND MKREPORT_LIBS ${CURL_LIBRARIES})
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
add_executable(integration-tests integration-tests.cpp)
target_link_libraries(integration-tests mkreport)

# Testing
# -------

set(BUILD_TESTING "ON" CACHE BOOL "Whether to build tests")
if(${BUILD_TESTING})
  enable_testing()
  add_test(NAME mocked_tests COMMAND tests)
  add_test(NAME integration_tests COMMAND integration-tests)
endif()
