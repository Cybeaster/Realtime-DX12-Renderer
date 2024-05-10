cmake_minimum_required(VERSION 3.27)
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
find_package(Python REQUIRED)

source_group("Resources\\Shaders" FILES ${SHADER_FILES})


add_definitions(-D_UNICODE -DUNICODE)
set(CMAKE_PREFIX_PATH Externals/EasyProfiler)
find_package(easy_profiler REQUIRED)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
set(SRC_FILES
        main.cpp
        Core/Types/Types.h
        main.cpp
        Core/Application/Application.h
        Core/Application/Window/Window.h
        Core/Application/Window/Window.cpp
        Core/Application/CommandQueue/CommandQueue.h
        Core/Application/CommandQueue/CommandQueue.cpp
        Core/Types/Exception.h
        Core/Types/ExitHelper.h
        Core/Types/Logger.h
        Core/Application/InputHandler/InputHandler.h
        Core/Application/Application.cpp
        Core/Application/Engine/Engine.h
        Core/Application/Engine/Engine.cpp
        Core/Application/Window/Window.h
        Core/Types/Events.h
        Core/Types/DirectX/DXHelper.h
        main.cpp
        Core/Types/Timer/Timer.h
        Core/Application/Camera/Camera.cpp
        Core/Application/Camera/Camera.h
        Core/Application/Engine/UploadBuffer/UploadBuffer.cpp
        Core/Application/Engine/UploadBuffer/UploadBuffer.h
        Core/Utils/DirectXUtils.h
        Core/Utils/MathUtils.h
        Core/Types/DirectX/FrameResource.h
        Core/Types/DirectX/ObjectConstants.h
        Core/Types/DirectX/RenderItem/RenderItem.h
        Core/Types/DirectX/RenderConstants.h
        Core/Types/DirectX/RenderConstants.h
        Core/Objects/GeomertryGenerator/GeometryGenerator.h
        Core/Objects/GeomertryGenerator/GeometryGenerator.cpp
        Core/Objects/GeomertryGenerator/GeometryGenerator.cpp
        Core/Objects/Geometry/Wave/Waves.cpp
        Core/Objects/Geometry/Wave/Waves.h
        Core/Materials/Material.h
        Core/Types/DirectX/Light/Light.h
        Core/Textures/DDSTextureLoader/DDSTextureLoader.cpp
        Core/Textures/DDSTextureLoader/DDSTextureLoader.h
        Core/Textures/Texture.h
        Core/Utils/PathUtils.h
        Core/Application/Engine/RenderTarget/Filters/Blur/BlurFilter.cpp
        Core/Application/Engine/RenderTarget/Filters/Blur/BlurFilter.h
        Core/Types/DirectX/ShaderTypes.h
        Core/Types/DirectX/ShaderTypes.h
        Core/Utils/DirectXUtils.cpp
        Core/Application/UI/UIManager/UiManager.cpp
        Core/Application/UI/UIManager/UiManager.h
        Core/Types/Settings.h
        Core/Types/Settings.cpp
        Core/Application/Engine/RenderTarget/Filters/SobelFilter/SobelFilter.cpp
        Core/Application/Engine/RenderTarget/Filters/SobelFilter/SobelFilter.h
        Core/Application/Engine/RenderTarget/Filters/FilterBase.h
        Core/Objects/Geometry/GPUWave/GpuWave.cpp
        Core/Objects/Geometry/GPUWave/GpuWave.h
        Core/Application/Engine/RenderTarget/RenderTarget.cpp
        Core/Application/Engine/RenderTarget/RenderTarget.h
        Core/Application/Engine/RenderTarget/Filters/BilateralBlur/BilateralBlurFilter.cpp
        Core/Application/Engine/RenderTarget/Filters/BilateralBlur/BilateralBlurFilter.h
        Core/Application/Engine/RenderTarget/RenderObject/RenderObject.h
        Core/Utils/Statics.h
        Core/Application/UI/Filters/BilateralFilterWidget.cpp
        Core/Application/UI/Filters/BilateralFilterWidget.h
        Core/Application/UI/Widget.h
        Core/Application/UI/Effects/FogWidget.h
        Core/Application/UI/Effects/FogWidget.cpp
        Core/Application/UI/Filters/GaussianBlurWidget.cpp
        Core/Application/UI/Filters/GaussianBlurWidget.h
        Core/Application/UI/Filters/GaussianBlurWidget.h
        Core/Application/UI/Effects/Light/LightWidget.cpp
        Core/Application/UI/Effects/Light/LightWidget.h
        Core/Application/UI/Filters/SobelFilterWidget.cpp
        Core/Application/UI/Filters/SobelFilterWidget.h
        Core/Application/UI/Geometry/GeometryEntity/GeometryEntityWidget.cpp
        Core/Application/UI/Geometry/GeometryEntity/GeometryEntityWidget.h
        Core/Application/UI/Geometry/GeometryManager.cpp
        Core/Application/UI/Geometry/GeometryManager.h
        Core/Application/UI/Geometry/GeometryEntity/GeometryTransform.h
        Core/Application/UI/Geometry/GeometryEntity/GeometryTransform.cpp
        Core/Application/UI/Filters/FilterManager.h
        Core/Application/UI/Engine/Camera.h
        Core/Application/UI/Engine/Camera.cpp
        Core/Types/DirectX/Vertex.h
        Core/Objects/MeshGenerator/MeshGenerator.cpp
        Core/Objects/MeshGenerator/MeshGenerator.h
        Core/Objects/MeshParser.cpp
        Core/Objects/MeshParser.h
        Core/Types/TextureConstants.h
        Core/Materials/MaterialManager/MaterialManager.cpp
        Core/Materials/MaterialManager/MaterialManager.h
        Core/Textures/TextureManager/TextureManager.cpp
        Core/Textures/TextureManager/TextureManager.h
        Core/Utils/EngineHelper.h
        Core/Application/UI/Material/MaterialPicker.cpp
        Core/Application/UI/Material/MaterialPicker.h
        Core/ConfigReader/ConfigReader.cpp
        Core/ConfigReader/ConfigReader.h
        Core/ConfigReader/MaterialsReader/MaterialsReader.h
        Core/Application/UI/Material/MaterialManager/MaterialManager.cpp
        Core/Application/UI/Material/MaterialManager/MaterialManager.h
        Core/ConfigReader/MaterialsReader/MaterialsReader.cpp
        Core/Types/Async.h
        Core/Application/UI/Geometry/GeometryEntity/PickedRenderItem.cpp
        Core/Application/UI/Geometry/GeometryEntity/PickedRenderItem.h
        Core/Application/Engine/RenderTarget/CubeMap/CubeRenderTarget.cpp
        Core/Application/Engine/RenderTarget/CubeMap/CubeRenderTarget.h
        Core/Application/Engine/RenderTarget/RenderObject/RenderObject.cpp
        Core/Application/Engine/RenderTarget/RenderObject/RenderObject.cpp
        Core/Application/Engine/RenderTarget/CubeMap/DynamicCubeMap/DynamicCubeMapTarget.cpp
        Core/Application/Engine/RenderTarget/CubeMap/DynamicCubeMap/DynamicCubeMapTarget.h
        Core/ConfigReader/TexturesReader/TexturesParser.cpp
        Core/ConfigReader/TexturesReader/TexturesParser.h
        Core/Application/UI/Material/TextureManager/TextureManager.cpp
        Core/Application/UI/Material/TextureManager/TextureManager.h
        Core/Application/UI/Material/TexturePicker.h
        Core/Application/UI/Base/PickerTable/PickerTableWidget.cpp
        Core/Application/UI/Base/PickerTable/PickerTableWidget.h
        Core/Textures/Texture.cpp
        Core/Application/RenderGraph/Graph/RenderGraph.cpp
        Core/Application/RenderGraph/Graph/RenderGraph.h
        Core/Application/RenderGraph/Nodes/RenderNode.cpp
        Core/Application/RenderGraph/Nodes/RenderNode.h
        Core/Application/Engine/Shader/Shader.cpp
        Core/Application/Engine/Shader/Shader.h
        Core/Application/Engine/InputLayour/InputLayout.cpp
        Core/Application/Engine/InputLayour/InputLayout.h
        Core/Application/ShaderCompiler/Compiler.cpp
        Core/Application/ShaderCompiler/Compiler.h
        Core/Application/GraphicsPipeline/GraphicsPipeline.cpp
        Core/Application/GraphicsPipeline/GraphicsPipeline.h
        Core/ConfigReader/ShaderReader/ShaderReader.cpp
        Core/ConfigReader/ShaderReader/ShaderReader.h
        Core/Application/GraphicsPipelineManager/GraphicsPipelineManager.cpp
        Core/Application/GraphicsPipelineManager/GraphicsPipelineManager.h
        Core/ConfigReader/PSOReader/PsoReader.cpp
        Core/ConfigReader/PSOReader/PsoReader.h
        Core/ConfigReader/RenderGraphReader/RenderGraphReader.cpp
        Core/ConfigReader/RenderGraphReader/RenderGraphReader.h
        Core/Types/RenderNodeInfo.h
        Core/Types/DirectX/RenderItem/RenderItem.cpp
        Core/Types/DirectX/ShaderTypes.cpp
        Core/Application/RenderGraph/Nodes/PresentNode/PresentNode.cpp
        Core/Application/RenderGraph/Nodes/PresentNode/PresentNode.h
        Core/Application/RenderGraph/Nodes/UINode/UiRenderNode.cpp
        Core/Application/RenderGraph/Nodes/UINode/UiRenderNode.h
        Core/Application/RenderGraph/Nodes/ReflectionNode/ReflectionNode.cpp
        Core/Application/RenderGraph/Nodes/ReflectionNode/ReflectionNode.h
        Core/Application/RenderGraph/Nodes/PostProcessNode/PostProcessNode.cpp
        Core/Application/RenderGraph/Nodes/PostProcessNode/PostProcessNode.h
        Core/Types/DirectX/Resource.h
        Core/Types/DirectX/Resource.cpp
        Core/Application/RenderGraph/Nodes/DefaultNode/DefaultRenderNode.cpp
        Core/Application/RenderGraph/Nodes/DefaultNode/DefaultRenderNode.h
        Core/Application/RenderGraph/Nodes/WaterNode/WaterNode.cpp
        Core/Application/RenderGraph/Nodes/WaterNode/WaterNode.h
        Core/Objects/Geometry/Water/Water.cpp
        Core/Objects/Geometry/Water/Water.h
        Core/Objects/Geometry/LightEmitting/SpotLightObject/SpotLightObject.cpp
        Core/Objects/Geometry/LightEmitting/SpotLightObject/SpotLightObject.h
        Core/Objects/Geometry/LightEmitting/OLightEmittingObject.cpp
        Core/Objects/Geometry/LightEmitting/OLightEmittingObject.h
        Core/Application/Components/LightComponent/LightComponent.cpp
        Core/Application/Components/LightComponent/LightComponent.h
        Core/Application/Components/RenderItemComponentBase.h
        Core/Application/Components/MeshComponent/MeshComponent.cpp
        Core/Application/Components/MeshComponent/MeshComponent.h
        Core/Types/DirectX/FrameResource.cpp
        Core/Application/Components/RenderItemComponentBase.cpp
        Core/Application/UI/Effects/Light/LightComponent/LightComponentWidget.cpp
        Core/Application/UI/Effects/Light/LightComponent/LightComponentWidget.h
        Core/Application/Engine/RenderTarget/ShadowMap/ShadowMap.cpp
        Core/Application/Engine/RenderTarget/ShadowMap/ShadowMap.h
        Core/Application/RenderGraph/Nodes/ShadowNode/ShadowMapNode.cpp
        Core/Application/RenderGraph/Nodes/ShadowNode/ShadowMapNode.h
        Core/Application/RenderGraph/Nodes/ShadowDebugNode/ShadowDebugNode.cpp
        Core/Application/RenderGraph/Nodes/ShadowDebugNode/ShadowDebugNode.h
        Core/Application/RenderGraph/Nodes/SSAO/SsaoNode.cpp
        Core/Application/RenderGraph/Nodes/SSAO/SsaoNode.h
        Core/Application/RenderGraph/Nodes/SSAO/SsaoNode.h
        Core/Application/Engine/RenderTarget/SSAORenderTarget/Ssao.cpp
        Core/Application/Engine/RenderTarget/SSAORenderTarget/Ssao.h
        Core/Application/UI/Graph/RenderGraphWidget.cpp
        Core/Application/UI/Graph/RenderGraphWidget.h
        Core/Application/UI/Graph/SSAORenderNodeWidget/SsaoRenderNodeWidget.cpp
        Core/Application/UI/Graph/SSAORenderNodeWidget/SsaoRenderNodeWidget.h
        Core/Application/UI/Graph/ORenderNodeWidgetBase.cpp
        Core/Application/UI/Graph/ORenderNodeWidgetBase.h
        Core/Objects/TinyObjLoader/TinyObjLoaderParser.h
        Core/Objects/TinyObjLoader/TinyObjLoaderParser.cpp
        Core/Objects/MeshGenerator/MeshPayload.h
        Core/Types/DirectX/MeshGeometry.h
        Core/Application/RenderGraph/Nodes/CopyNode/CopyRenderNode.cpp
        Core/Application/RenderGraph/Nodes/CopyNode/CopyRenderNode.h
        Profiler/Profiler.h
        Core/Application/UI/Engine/PerfomanceWidget.cpp
        Core/Application/UI/Engine/PerfomanceWidget.h
        Core/Application/Engine/RenderTarget/NormalTangetDebugTarget/NormalTangentDebugTarget.cpp
        Core/Application/Engine/RenderTarget/NormalTangetDebugTarget/NormalTangentDebugTarget.h
        Core/Application/RenderGraph/Nodes/TangentNormalDebugNode/TangentNormalDebugNode.cpp
        Core/Application/RenderGraph/Nodes/TangentNormalDebugNode/TangentNormalDebugNode.h
        Core/ConfigReader/SceneReader/SceneReader.cpp
        Core/ConfigReader/SceneReader/SceneReader.h
        Core/Application/Engine/Scene/SceneManager.cpp
        Core/Application/Engine/Scene/SceneManager.h
        Core/Application/Engine/Scene/Scene.cpp
        Core/Application/Engine/Scene/Scene.h
        Core/Application/UI/Engine/ShaderSettings.cpp
        Core/Application/UI/Engine/ShaderSettings.h
        Core/Types/DirectX/HLSL/HlslTypes.h
        Core/Application/Engine/RenderTarget/CSM/Csm.cpp
        Core/Application/Engine/RenderTarget/CSM/Csm.h
        Core/Application/RenderGraph/Nodes/FrustrumDebugNode/FrustumDebugNode.cpp
        Core/Application/RenderGraph/Nodes/FrustrumDebugNode/FrustumDebugNode.h
        Core/Application/Engine/Raytracer/Raytracer.cpp
        Core/Application/Engine/Raytracer/Raytracer.h
        Externals/DXR/BottomLevelASGenerator.cpp
        Externals/DXR/BottomLevelASGenerator.cpp
        Externals/DXR/TopLevelASGenerator.h
        Externals/DXR/TopLevelASGenerator.cpp
        Externals/DXR/ShaderBindingTableGenerator.h
        Externals/DXR/ShaderBindingTableGenerator.cpp
        Externals/DXR/RootSignatureGenerator.h
        Externals/DXR/RootSignatureGenerator.cpp
        Externals/DXR/RaytracingPipelineGenerator.h
        Externals/DXR/RaytracingPipelineGenerator.cpp
        Core/Application/Engine/Device/Device.h
        Core/Application/Engine/Device/Device.cpp
        Externals/DXR/DXRHelper.h
        Core/Application/RenderGraph/Nodes/AABBVisualizer/AabbVisNode.cpp
        Core/Application/RenderGraph/Nodes/AABBVisualizer/AabbVisNode.h
        Core/Types/DirectX/BoundingGeometry.h
        Core/Types/Color.h
)


set(DXCOMPILER_PATH_DLL ${CMAKE_SOURCE_DIR}/Externals/DXC/bin)
set(DXCOMPILER_PATH_LIB ${CMAKE_SOURCE_DIR}/Externals/DXC/bin/dxcompiler.lib)
set(EASY_PROFILER_DLL ${CMAKE_SOURCE_DIR}/Externals/EasyProfiler/bin/easy_profiler.dll)

# Add sources to the project
add_executable(DXRenderer
        ${SRC_FILES}
        ${SHADER_FILES})

file(GLOB IMGUI_SOURCES Externals/imgui/*.cpp Externals/imgui/*.h)
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

include_directories(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Externals/imgui)
include_directories(IMGUI_INCLUDE_DIR Externals/DXC/inc)

add_subdirectory(Externals/tinyobjloader)
add_subdirectory(Externals/DXMesh)

target_include_directories(DXRenderer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Externals
        ${CMAKE_CURRENT_SOURCE_DIR}/Profiler
        Core/Types
        Core/Application
        Core/Utils
        Core/Objects
        Core/Materials
        Core/Textures
        Core/ConfigReader
        Core/Application/Components


)

# Copy dxcompiler.dll to the output directory
add_custom_command(TARGET DXRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${DXCOMPILER_PATH_DLL}
        $<TARGET_FILE_DIR:DXRenderer>)

# Copy dxcompiler.dll to the output directory
add_custom_command(TARGET DXRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${EASY_PROFILER_DLL}
        $<TARGET_FILE_DIR:DXRenderer>)


target_link_libraries(DXRenderer
        d3d12.lib
        dxgi.lib
        dxguid.lib
        d3dcompiler.lib
        Shlwapi.lib
        imgui
        ${Boost_LIBRARIES}
        Bcrypt.lib
        ${DXCOMPILER_PATH_LIB}
        easy_profiler
        DirectXMesh)
