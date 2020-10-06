cmake_minimum_required(VERSION 3.18)
project(test)

INCLUDE(ExternalProject)

SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_FLAGS "-g -O2 -flto -rtlib=compiler-rt -fno-plt")

Execute_Process(COMMAND sh -c "php-config --include-dir | tr -d '\n'" OUTPUT_VARIABLE PHP_INCLUDE_DIR)
MESSAGE( STATUS "PHP_INCLUDE_DIR:" ${PHP_INCLUDE_DIR})

ExternalProject_Add(
  clickhouse-cpp
  PREFIX          external/clickhouse-cpp
  GIT_REPOSITORY  https://github.com/oraoto/clickhouse-cpp.git
  GIT_TAG         eb5971f84daa7728ae8abc72eca77a6705792970
  # Hack: C++ 20, pic, lto, O2
  PATCH_COMMAND   sed "s/USE_CXX17()/add_compile_options(-Wno-deprecated-enum-enum-conversion -fpic -O2 -flto -g)/g" CMakeLists.txt -i
  STEP_TARGETS    build
  CMAKE_ARGS      -DCMAKE_CXX_COMPILER=clang++
                  -DCMAKE_C_COMPILER=clang
                  -DCMAKE_CXX_STANDARD=20
                  -DCMAKE_CXX_STANDARD_REQUIRED=ON
		  -DCMAKE_INSTALL_PREFIX=.
		  -DCMAKE_CONFIGURATION_TYPES=Release
		  -DCMAKE_BUILD_TYPE=Release
		  )

ExternalProject_Add(
  PHP-CPP
  PREFIX          external/PHP-CPP
  GIT_REPOSITORY  https://github.com/NobletSolutions/PHP-CPP.git
  GIT_TAG         86d3624857a4d6a5ca8f760bb233ed083079e462
  INSTALL_COMMAND sh -c "cp libphpcpp.a.2.2.0 libphpcpp.a && ln -sf include phpcpp"
  CONFIGURE_COMMAND ""
  # Hack: Expose zval, pic, lto, O2
  PATCH_COMMAND   sh -c "cp -p ../../../../../value.h include/value.h && sed 's/^STATIC_COMPILER_FLAGS.*=.*$/STATIC_COMPILER_FLAGS=-fpic -O2 -flto -fno-plt -g/' -i Makefile"
  BUILD_ALWAYS    true
  BUILD_IN_SOURCE true
  # Not using cmake
  BUILD_COMMAND   CC=clang CXX=clang++ make -j4
  )

SET (ch-src
  src/lib.cpp
  src/Columns.cpp
  src/FetchColumn.cpp
  src/ClientOptions.cpp
  src/Client.cpp
)

# Library and dependencies
Add_Library(orzclick SHARED ${ch-src})
Set_Target_Properties(orzclick PROPERTIES LINKER_LANGUAGE CXX)
Add_Dependencies(orzclick PHP-CPP)
Add_Dependencies(orzclick clickhouse-cpp)

# Include headers
Target_Include_Directories(orzclick PRIVATE "${CMAKE_BINARY_DIR}/external/clickhouse-cpp/src/clickhouse-cpp")
Target_Include_Directories(orzclick PRIVATE "${CMAKE_BINARY_DIR}/external/PHP-CPP/src/PHP-CPP/include/")
Target_Include_Directories(orzclick PRIVATE "${CMAKE_BINARY_DIR}/external/PHP-CPP/src/PHP-CPP/")
Target_Include_Directories(orzclick PRIVATE "${PHP_INCLUDE_DIR}" "${PHP_INCLUDE_DIR}/main" "${PHP_INCLUDE_DIR}/TSRM" "${PHP_INCLUDE_DIR}/Zend" "${PHP_INCLUDE_DIR}/ext" "${PHP_INCLUDE_DIR}/ext/date/lib")

# Linker
Target_Link_Directories(orzclick PRIVATE "${CMAKE_BINARY_DIR}/external/PHP-CPP/src/PHP-CPP/")
Target_Link_Directories(orzclick PRIVATE "${CMAKE_BINARY_DIR}/external/clickhouse-cpp/src/clickhouse-cpp-build/lib/")
Target_Link_Directories(orzclick PRIVATE "${CMAKE_BINARY_DIR}/external/clickhouse-cpp/src/clickhouse-cpp-build/contrib/cityhash/")
Target_Link_Directories(orzclick PRIVATE "${CMAKE_BINARY_DIR}/external/clickhouse-cpp/src/clickhouse-cpp-build/contrib/lz4/")

Target_Link_Libraries(orzclick :libphpcpp.a)
Target_Link_Libraries(orzclick :libclickhouse-cpp-lib-static.a)
Target_Link_Libraries(orzclick :liblz4-lib.a)
Target_Link_Libraries(orzclick :libcityhash-lib.a)
