cmake_minimum_required(VERSION 3.20)
project(DXRenderer)

set(CMAKE_CXX_STANDARD 20)

set(IMGUI_BACKENDS
        Externals/imgui/backends/imgui_impl_dx12.cpp
        Externals/imgui/backends/imgui_impl_win32.h
        Externals/imgui/backends/imgui_impl_dx12.h
        Externals/imgui/backends/imgui_impl_win32.cpp
)

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

source_group("Resources\\Shaders" FILES ${SHADER_FILES})

add_definitions(-D_UNICODE -DUNICODE)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
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
        Types/Events.h
        Types/DirectX/DXHelper.h
        Application/Test/Test.cpp
        main.cpp
        Types/Timer/Timer.h
        Application/Camera/Camera.cpp
        Application/Camera/Camera.h
        Application/Engine/UploadBuffer/UploadBuffer.cpp
        Application/Engine/UploadBuffer/UploadBuffer.h
        Utils/DirectXUtils.h
        Utils/Math.h
        Types/DirectX/FrameResource.h
        Types/DirectX/ObjectConstants.h
        Types/DirectX/RenderItem.h
        Types/DirectX/RenderConstants.h
        Types/DirectX/RenderConstants.h
        Objects/GeomertryGenerator/GeometryGenerator.h
        Objects/GeomertryGenerator/GeometryGenerator.cpp
        Objects/GeomertryGenerator/GeometryGenerator.cpp
        Objects/Geometry/Wave/Waves.cpp
        Objects/Geometry/Wave/Waves.h
        Materials/Material.h
        Types/DirectX/MaterialData.h
        Types/DirectX/MaterialData.h
        Types/DirectX/Light/Light.h
        Textures/DDSTextureLoader/DDSTextureLoader.cpp
        Textures/DDSTextureLoader/DDSTextureLoader.h
        Textures/Texture.h
        Application/Test/TextureTest/TextureWaves.cpp
        Application/Test/TextureTest/TextureWaves.h
        Utils/Path.h
        Application/Filters/Blur/BlurFilter.cpp
        Application/Filters/Blur/BlurFilter.h
        Types/DirectX/ShaderTypes.h
        Types/DirectX/ShaderTypes.h
        Utils/DirectXUtils.cpp
        Application/UI/UIManager/UiManager.cpp
        Application/UI/UIManager/UiManager.h
        Types/Settings.h
        Types/Settings.cpp
        Application/Filters/SobelFilter/SobelFilter.cpp
        Application/Filters/SobelFilter/SobelFilter.h
        Application/Filters/FilterBase.h
        Objects/Geometry/GPUWave/GpuWave.cpp
        Objects/Geometry/GPUWave/GpuWave.h
        Application/Engine/RenderTarget/RenderTarget.cpp
        Application/Engine/RenderTarget/RenderTarget.h
        Application/Filters/BilateralBlur/BilateralBlurFilter.cpp
        Application/Filters/BilateralBlur/BilateralBlurFilter.h
        Application/Engine/RenderObject/RenderObject.h
        Utils/Statics.h
        Application/UI/Filters/BilateralFilterWidget.cpp
        Application/UI/Filters/BilateralFilterWidget.h
        Application/UI/Widget.h
        Application/UI/Effects/FogWidget.h
        Application/UI/Effects/FogWidget.cpp
        Application/UI/Filters/GaussianBlurWidget.cpp
        Application/UI/Filters/GaussianBlurWidget.h
        Application/UI/Filters/GaussianBlurWidget.h
        Application/UI/Effects/Light/LightWidget.cpp
        Application/UI/Effects/Light/LightWidget.h
        Application/UI/Filters/SobelFilterWidget.cpp
        Application/UI/Filters/SobelFilterWidget.h
        Application/UI/Geometry/GeometryEntity/GeometryEntityWidget.cpp
        Application/UI/Geometry/GeometryEntity/GeometryEntityWidget.h
        Application/UI/Geometry/GeometryManager.cpp
        Application/UI/Geometry/GeometryManager.h
        Application/UI/Geometry/GeometryEntity/GeometryTransform.h
        Application/UI/Geometry/GeometryEntity/GeometryTransform.cpp
        Application/UI/Filters/FilterManager.h
        Application/UI/Engine/Camera.h
        Application/UI/Engine/Camera.cpp
        Types/DirectX/InstanceData.h
        Types/DirectX/Vertex.h
        Objects/MeshGenerator/MeshGenerator.cpp
        Objects/MeshGenerator/MeshGenerator.h
        Objects/MeshParser.cpp
        Objects/MeshParser.h
        Types/TextureConstants.h
        Materials/MaterialManager/MaterialManager.cpp
        Materials/MaterialManager/MaterialManager.h
        Textures/TextureManager/TextureManager.cpp
        Textures/TextureManager/TextureManager.h
        Utils/EngineHelper.h
        Application/UI/Material/MaterialPicker.cpp
        Application/UI/Material/MaterialPicker.h
        Config/ConfigReader.cpp
        Config/ConfigReader.h
        Config/MaterialsReader/MaterialsReader.h
        Application/UI/Material/MaterialManager/MaterialManager.cpp
        Application/UI/Material/MaterialManager/MaterialManager.h
        Config/MaterialsReader/MaterialsReader.cpp
        Types/Async.h
        Application/UI/Geometry/GeometryEntity/PickedRenderItem.cpp
        Application/UI/Geometry/GeometryEntity/PickedRenderItem.h
        Application/Engine/RenderTarget/CubeMap/CubeRenderTarget.cpp
        Application/Engine/RenderTarget/CubeMap/CubeRenderTarget.h
        Application/Engine/RenderObject/RenderObject.cpp
        Application/Engine/RenderObject/RenderObject.cpp

)

file(GLOB IMGUI_SOURCES Externals/imgui/*.cpp Externals/imgui/*.h)


# Add sources to the project
add_executable(DXRenderer
        ${SRC_FILES}
        ${SHADER_FILES}
)

add_library(imgui ${IMGUI_SOURCES})

# Include them in your library or executable
target_sources(imgui PRIVATE ${IMGUI_BACKENDS})


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

include_directories(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Externals/imgui)

target_include_directories(DXRenderer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Externals/directx
        ${CMAKE_CURRENT_SOURCE_DIR}/Types
        ${CMAKE_CURRENT_SOURCE_DIR}/Application
        ${CMAKE_CURRENT_SOURCE_DIR}/Types/DirectX
)

# Link necessary libraries
target_link_libraries(DXRenderer d3d12.lib dxgi.lib dxguid.lib d3dcompiler.lib Shlwapi.lib imgui ${Boost_LIBRARIES} Bcrypt.lib)
