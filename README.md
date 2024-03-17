### DirectX 12 Renderer

Rendering engine using DirectX 12 for backend.
[ImGUI is used for UI](https://github.com/Cybeaster/RenderEngine/blob/water_hot_fix/Examples/UI.jpg), boost.ptree for configurations. Models, shaders, materials and textures are loaded from config file.

Currently implemented features:
- Post processing effects: Gaussian blur, Billateral Blur Sobel edge detection.
- Dynamic reflections.
- Shader reflection.
- Render graph.
- Automatic PSO resolve.
- Instancing and frustrum culling.