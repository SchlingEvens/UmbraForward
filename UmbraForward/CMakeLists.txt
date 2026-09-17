set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ==========================================
# 项目公共配置
# ==========================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ProjectOptions INTERFACE)

if(MSVC)
    target_compile_options(ProjectOptions INTERFACE /utf-8)
endif()

# ==========================================
# 依赖包
# ==========================================

find_package(directx-headers CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_library(DirectX12Common INTERFACE)

target_link_libraries(DirectX12Common INTERFACE
    Microsoft::DirectX-Headers
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
)

# ==========================================
# Graphics
# ==========================================

set(GRAPHICS_SOURCES
    Graphics/VertexData.h
    Graphics/VertexData.cpp
    Graphics/MathHelper.h
    Graphics/MathHelper.cpp
    Graphics/d3dUtil.h
    Graphics/d3dUtil.cpp
    Graphics/GeometryGenerator.h
    Graphics/GeometryGenerator.cpp
)

add_library(Graphics STATIC ${GRAPHICS_SOURCES})

target_include_directories(Graphics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
)

target_link_libraries(Graphics
    PUBLIC
        ProjectOptions
        DirectX12Common
)

# ==========================================
# UmbraForward
# ==========================================

set(APP_SOURCES
    UmbraForward.cpp
    stdafx.cpp
    stdafx.h
    Win32Application.cpp
    Win32Application.h
    FrameResource.h
    FrameResource.cpp
    RenderAppBase.cpp
    RenderAppBase.h
    DXSampleHelper.h
    DXSampleHelper.cpp
    SampleTriangle.cpp
    SampleTriangle.h
    shaders.hlsl
)

set_source_files_properties(
    shaders.hlsl
    PROPERTIES HEADER_FILE_ONLY TRUE
)

add_executable(UmbraForward WIN32 ${APP_SOURCES})

target_link_libraries(UmbraForward
    PRIVATE
        Graphics
        imgui::imgui
)

# ==========================================
# 构建后复制着色器
# ==========================================

add_custom_command(TARGET UmbraForward POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders.hlsl"
        "$<TARGET_FILE_DIR:UmbraForward>"
)