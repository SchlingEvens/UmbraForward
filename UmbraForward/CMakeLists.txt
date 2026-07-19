# 1. 先寻找 DirectX-Headers 包（让 vcpkg 提前准备好环境和包含路径）
find_package(directx-headers CONFIG REQUIRED)

# 2. 收集所有源代码
set(SOURCES
    "UmbraForward.cpp" 
    "stdafx.cpp" 
    "stdafx.h" 
    "Win32Application.cpp" 
    "Win32Application.h"
    "RenderAppBase.cpp" 
    "RenderAppBase.h" 
    "DXSampleHelper.h"
)

add_executable(UmbraForward WIN32 ${SOURCES} "SampleTriangle.cpp" "SampleTriangle.h"  "DXSampleHelper.cpp")
if(MSVC)
    add_compile_options(/utf-8)
endif()

target_link_libraries(UmbraForward PRIVATE
    Microsoft::DirectX-Headers
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
)

set_property(TARGET UmbraForward PROPERTY CXX_STANDARD 20)
set_property(TARGET UmbraForward PROPERTY CXX_STANDARD_REQUIRED ON)

add_custom_command(TARGET UmbraForward POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders.hlsl"
    $<TARGET_FILE_DIR:UmbraForward>
)