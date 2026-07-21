# 1. 寻找依赖包
find_package(directx-headers CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# ==========================================
# 全局设置：相同的源文件/头文件/着色器过滤 (Filters)
# ==========================================
source_group("Source Files" REGULAR_EXPRESSION "\\.cpp$")
source_group("Header Files" REGULAR_EXPRESSION "\\.h$")
source_group("Shaders" REGULAR_EXPRESSION "\\.hlsl$")

# ==========================================
# 共享项目: Graphics
# ==========================================
set(GRAPHICS_SOURCES
    "Graphics/VertexData.h"
    "Graphics/VertexData.cpp"
    "Graphics/MathHelper.h"
    "Graphics/MathHelper.cpp" 
    "Graphics/d3dUtil.h"
    "Graphics/d3dUtil.cpp")

add_library(Graphics STATIC ${GRAPHICS_SOURCES})

target_include_directories(Graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Graphics)


if(MSVC)
    target_compile_options(Graphics PRIVATE /utf-8)
endif()
set_property(TARGET Graphics PROPERTY CXX_STANDARD 20)
set_property(TARGET Graphics PROPERTY CXX_STANDARD_REQUIRED ON)

# Graphics 依赖的库
target_link_libraries(Graphics PRIVATE
    Microsoft::DirectX-Headers
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
)

# ==========================================
# 主程序项目: UmbraForward
# ==========================================
set(APP_SOURCES
    "UmbraForward.cpp" 
    "stdafx.cpp" 
    "stdafx.h" 
    "Win32Application.cpp" 
    "Win32Application.h"
    "FrameResource.h"
    "FrameResource.cpp"
    "RenderAppBase.cpp" 
    "RenderAppBase.h" 
    "DXSampleHelper.h"
    "DXSampleHelper.cpp"
    "SampleTriangle.cpp"
    "SampleTriangle.h"
    "shaders.hlsl"
)

set_source_files_properties("shaders.hlsl" PROPERTIES HEADER_FILE_ONLY TRUE)

# 构建主程序可执行文件
add_executable(UmbraForward WIN32 ${APP_SOURCES})

# 编译与标准设置
if(MSVC)
    target_compile_options(UmbraForward PRIVATE /utf-8)
endif()
set_property(TARGET UmbraForward PROPERTY CXX_STANDARD 20)
set_property(TARGET UmbraForward PROPERTY CXX_STANDARD_REQUIRED ON)

# 主程序链接 Graphics 库以及其他依赖库
target_link_libraries(UmbraForward PRIVATE
    Graphics                    # 链接你预留的共享模块
    Microsoft::DirectX-Headers
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
    imgui::imgui
)

# 编译后事件：复制着色器
add_custom_command(TARGET UmbraForward POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders.hlsl"
    $<TARGET_FILE_DIR:UmbraForward>
)