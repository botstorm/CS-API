cmake_minimum_required(VERSION 3.1)

project(csconnector)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_definitions(-DNOMINMAX)
endif()


#include_directories(csdb_v2/include)
#include_directories(cpp-gen)
#add_subdirectory(csdb_v2)



add_definitions(-DUSE_STD_THREAD)

# Вот этот большой список нужен, чтобы thrift не собирал ничего лишнего.
if (MSVC)
  option(BUILD_COMPILER "" OFF)
endif(MSVC)
option(BUILD_TESTING "" OFF)
option(BUILD_TUTORIALS "" OFF)
option(BUILD_EXAMPLES "" OFF)
option(BUILD_PYTHON "" OFF)
option(WITH_SHARED_LIB "" OFF)
option(WITH_STATIC_LIB "" ON)
option(WITH_C_GLIB "" OFF)
option(WITH_JAVA "" OFF)
option(WITH_PYTHON "" OFF)
option(WITH_QT4 "" OFF)
option(WITH_QT5 "" OFF)
option(WITH_ZLIB "" OFF)
option(WITH_STDTHREADS "" ON)
option(WITH_LIBEVENT "" OFF)
option(WITH_OPENSSL "" OFF)
add_subdirectory(thrift)

if (NOT TARGET thrift-compiler)
  message(FATAL_ERROR "Thrift compiler not found!")
endif(NOT TARGET thrift-compiler)

# Boost лучше искать так:
find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "BOOST headers are required to use thrift.")
endif()

# Конечно, лучше помещать сгенерённые файлы не в ${CMAKE_CURRENT_SOURCE_DIR}, а в
# ${CMAKE_CURRENT_BINARY_DIR}. Но если уже в ${CMAKE_CURRENT_SOURCE_DIR}, надо каталог
# добавить в .gitignore
set(THRIFT_GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cpp-gen)
file(MAKE_DIRECTORY ${THRIFT_GEN_DIR})

# Список файлов лучше задать заранее. На этапе генерайии thirft compiler ещё
# банально может не быть собран. При изменении thrift-файла список надо будет
# обновить вручную.
set(THRIFT_GENERATED_FILES
  ${THRIFT_GEN_DIR}/API.cpp
  ${THRIFT_GEN_DIR}/api_constants.cpp
  ${THRIFT_GEN_DIR}/api_types.cpp
  ${THRIFT_GEN_DIR}/API.h
  ${THRIFT_GEN_DIR}/api_constants.h
  ${THRIFT_GEN_DIR}/api_types.h
  )

# Вызов thrift compiler лучше добавлять в фазу сборки - иначе
# в случае изменения thrift-файла он не будет перекомпилирован.
add_custom_command(
  OUTPUT ${THRIFT_GENERATED_FILES}
  COMMAND $<TARGET_FILE:thrift-compiler> -r -gen cpp:no_skeleton,pure_enums,moveable_types
    -out ${THRIFT_GEN_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/api.thrift
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/api.thrift
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/api.thrift
  )


set(SHA1 ${CMAKE_CURRENT_SOURCE_DIR}/extlibs/sha1)
include_directories(${SHA1})

# Не рекомендуется использовать file(GLOB, поскольку он вызывается только на стадии
# генератора Cmake. При добавлении файлов в папку он вызван не будет - и список файлов
# не обновится.
add_library(${PROJECT_NAME}
  include/csconnector/csconnector.h
  src/csconnector.cpp
  src/APIHandlerBase.cpp
  src/APIHandlerBase.h
  src/APIHandler.cpp
  src/APIHandler.h
  src/DBHandlers.cpp
  src/DBHandlers.h
  src/DebugLog.h
  src/CallStats.cpp
  src/CallStats.h
        ${THRIFT_GENERATED_FILES}
        ${SHA1}/sha1.cpp
  )

target_link_libraries (${PROJECT_NAME} thrift_static csdb)

# INCLUDE DIRECTORIES лучше задавать не глобально, а для конкретного проекта.
# INCLUDE DIRECTORIES из подключаемых библиотек (в данном случае thrift и csdb)
# задавать не надо. Они включены в INTERFACE библиотек и подключатся автоматически
# в target_link_libraries
target_include_directories(${PROJECT_NAME} PUBLIC
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${THRIFT_GEN_DIR}
  )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
