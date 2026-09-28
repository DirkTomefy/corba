cmake_minimum_required(VERSION 3.15)
project(mycppcorba VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(FindPkgConfig)

pkg_check_modules(OMNIORB REQUIRED IMPORTED_TARGET
    omniORB4
    omniDynamic4
    omniCOS4
)

add_executable(mycppcorba
    src/main.cpp
    src/dbutil.cpp
    src/etudiant_servant.cpp
    src/generated/etudiant.cpp
    src/historiquecpp_servant.cpp
)

target_include_directories(mycppcorba PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/generated
)

target_link_libraries(mycppcorba
    mariadb
    PkgConfig::OMNIORB
)