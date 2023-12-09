cmake_minimum_required(VERSION 3.20)
project(DXRenderer)

set(CMAKE_CXX_STANDARD 20)


# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SHADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/OneSimpleCube/VertexShader.hlsl
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/OneSimpleCube/PixelShader.hlsl
)

source_group("Resources\\Shaders" FILES ${SHADER_FILES})

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/Shaders/OneSimpleCube/VertexShader.hlsl PROPERTIES
        VS_SHADER_TYPE Vertex
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/Shaders/OneSimpleCube/PixelShader.hlsl PROPERTIES
        VS_SHADER_TYPE Pixel
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main
)
set(SRC_FILES
        main.cpp
        Types/Types.h
        Externals/directx/d3dx12.h
        main.cpp
        Application/Application.h
        Application/Window/Window.h
        Application/Window/Window.cpp
        Application/CommandQueue/CommandQueue.h
        Application/CommandQueue/CommandQueue.cpp
        Types/Exception.h
        Types/ExitHelper.h
        Types/Logger.h
        Application/InputHandler/InputHandler.h
        Application/Application.cpp
        Application/Engine/Engine.h
        Application/Engine/Engine.cpp
        Application/Window/Window.h
        Application/Test/Test.h
        Application/Test/SimpleCubeTest/SimpleCubeTest.h
        Application/Test/SimpleCubeTest/SimpleCubeTest.cpp
        Types/Events.h
        Types/DXHelper.h
        Types/HighResolutionClock.h
        Types/HighResolutionClock.cpp
        Application/Test/Test.cpp
        main.cpp
        Types/Timer/Timer.h
)

# Add sources to the project
add_executable(DXRenderer
        ${SRC_FILES}
        ${SHADER_FILES}
)

# Specify the subsystem for Windows
if (MSVC)
    set_target_properties(DXRenderer PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif ()

target_include_directories(DXRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Externals/directx ${CMAKE_CURRENT_SOURCE_DIR}/Types ${CMAKE_CURRENT_SOURCE_DIR}/Application)

# Link necessary libraries
target_link_libraries(DXRenderer d3d12.lib dxgi.lib dxguid.lib d3dcompiler.lib Shlwapi.lib)
