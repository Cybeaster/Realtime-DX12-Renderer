cmake_minimum_required(VERSION 3.20)
project(DXRenderer)

set(CMAKE_CXX_STANDARD 20)


# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

source_group("Resources\\Shaders" FILES ${SHADER_FILES})

add_definitions(-D_UNICODE -DUNICODE)

set(SRC_FILES
        main.cpp
        Types/Types.h
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
        Types/DirectX/DXHelper.h
        Application/Test/Test.cpp
        main.cpp
        Types/Timer/Timer.h
        Application/Test/WindowTest/WindowTest.cpp
        Application/Test/WindowTest/WindowTest.h
        Application/Camera/Camera.cpp
        Application/Camera/Camera.h
        Application/Engine/UploadBuffer/UploadBuffer.cpp
        Application/Engine/UploadBuffer/UploadBuffer.h
        Utils/DirectX.h
        Utils/Math.h
        Types/DirectX/FrameResource.h
        Types/DirectX/ObjectConstants.h
        Types/DirectX/RenderItem.h
        Types/DirectX/RenderConstants.h
        Types/DirectX/RenderConstants.h
        Application/Test/ShapesTest/ShapesTest.cpp
        Application/Test/ShapesTest/ShapesTest.h
        Objects/GeomertryGenerator/GeometryGenerator.h
        Objects/GeomertryGenerator/GeometryGenerator.cpp
        Objects/GeomertryGenerator/GeometryGenerator.cpp
        Application/Test/LandTest/LandTest.h
        Application/Test/LandTest/LandTest.cpp
        Objects/Geometry/Wave/Waves.cpp
        Objects/Geometry/Wave/Waves.h
        Materials/Material.h
        Application/Test/LitWaves/LitWaves.h
        Application/Test/LitWaves/LitWaves.cpp
        Types/DirectX/MaterialConstants.h
        Types/DirectX/MaterialConstants.h
        Types/DirectX/Light/Light.h
        Application/Textures/DDSTextureLoader/DDSTextureLoader.cpp
        Application/Textures/DDSTextureLoader/DDSTextureLoader.h
        Application/Textures/Texture.h
        Application/Test/TextureTest/TextureWaves.cpp
        Application/Test/TextureTest/TextureWaves.h
        Utils/Path.h
        Application/Test/StencilingTest/StencilingTest.h
        Application/Test/StencilingTest/StencilingTest.h
        Application/Test/StencilingTest/StencilingTest.cpp
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
    add_compile_options(/wd4996)
endif ()

add_custom_command(TARGET DXRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Shaders $<TARGET_FILE_DIR:DXRenderer>/Shaders)

add_custom_command(TARGET DXRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Resources $<TARGET_FILE_DIR:DXRenderer>/Resources/)

target_include_directories(DXRenderer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Externals/directx
        ${CMAKE_CURRENT_SOURCE_DIR}/Types
        ${CMAKE_CURRENT_SOURCE_DIR}/Application
        ${CMAKE_CURRENT_SOURCE_DIR}/Types/DirectX)

# Link necessary libraries
target_link_libraries(DXRenderer d3d12.lib dxgi.lib dxguid.lib d3dcompiler.lib Shlwapi.lib)
