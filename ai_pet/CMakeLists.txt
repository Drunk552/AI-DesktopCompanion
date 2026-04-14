cmake_minimum_required(VERSION 3.16)
project(ai_pet LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===== 依赖 =====
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(FREETYPE REQUIRED freetype2)

# ===== LVGL =====
set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/third_party/lv_conf.h CACHE STRING "" FORCE)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(LV_CONF_BUILD_DISABLE_DEMOS ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/lvgl)

# 移除 ARM NEON 汇编文件（x86_64 不支持）
get_target_property(LVGL_SOURCES lvgl SOURCES)
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*\\.S$")
set_target_properties(lvgl PROPERTIES SOURCES "${LVGL_SOURCES}")

# 让 lvgl target 能找到 FreeType 头文件
target_include_directories(lvgl PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(lvgl PRIVATE ${FREETYPE_LIBRARIES})

# ===== 头文件路径 =====
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/third_party/lvgl
    ${OpenCV_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

# ===== 源文件 =====
set(SOURCES
    main.cpp
    config/config.cpp
    logger/logger.cpp
    vision/camera.cpp
    vision/face.cpp
    vision/emotion.cpp
    ai/gemma.cpp
    ai/prompt.cpp
    ai/personality.cpp
    ai/strategy.cpp
    ai/persona_loader.cpp
    memory/sqlite.cpp
    controller/brain.cpp
    ui/ui.cpp
)

# 自动收集 UI 资源源码（如 LVGL 图片转换生成的 .c 文件）
file(GLOB_RECURSE UI_ASSET_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/ui/assets/*.c
)

list(APPEND SOURCES ${UI_ASSET_SOURCES})

# ===== 可执行文件 =====
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_ASSET_DIR="${CMAKE_SOURCE_DIR}/ui/assets"
)

# ===== 链接库 =====
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${CURL_LIBRARIES}
    SQLite::SQLite3
    ${AVUTIL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    lvgl
    pthread
    stdc++fs
)

# ===== 资源目录：构建时自动创建符号链接到 build/ =====
# models/ 目录（预训练模型）
file(CREATE_LINK
    ${CMAKE_SOURCE_DIR}/models
    ${CMAKE_BINARY_DIR}/models
    SYMBOLIC
)

# personas/ 目录（角色文件）
if(EXISTS ${CMAKE_SOURCE_DIR}/personas)
    file(CREATE_LINK
        ${CMAKE_SOURCE_DIR}/personas
        ${CMAKE_BINARY_DIR}/personas
        SYMBOLIC
    )
endif()

# data/ 目录（数据库等运行时数据）
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
