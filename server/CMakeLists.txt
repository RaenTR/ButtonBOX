cmake_minimum_required(VERSION 3.20)
project(ETS2ButtonBoxServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(include src)

# Add Source Files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# App Manifest for Administrator privileges
set(APP_MANIFEST "app.manifest")

add_executable(${PROJECT_NAME} ${SOURCES} ${APP_MANIFEST})

# Ensure the linker uses the manifest for UAC
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
endif()

# Copy config file to binary directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/buttons.json" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# Windows Specific: Win32 Socket (Winsock2) and Input API
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 user32 gdi32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601) # Windows 7 or higher
endif()

# Future dependencies (placeholder)
# find_package(nlohmann_json REQUIRED)
# find_package(spdlog REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog)
