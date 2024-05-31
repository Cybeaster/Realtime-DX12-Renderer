# DirectX 12 Renderer
### Render engine with DirectX 12 for backend.

####  Full list of features:
##### Post Processing
- Gaussian blur
- Bilateral blur
- Sobel filter
- Screen space ambient occlusion
##### Lighting
- Shadow mapping
- Normal mapping
- Cascade shadows (With Orthographic frustum culling)
- Directional, Spot, Point lights
- BlinnPhong BRDF
- Disney BRDF
##### Color
- Gamma correction
- ACES Tone mapping
##### Other
- Planar Reflections
- Cube mapping
- Shader reflection
- Shader Hot Reload
- Material editor
- Dynamic Render graph
- Automatic PSO resolve
- Configuration files
- Camera animations
- Debug shaders
- UI settings

#### Build
1. Clone repository.
2. Download [vcpkg](https://github.com/microsoft/vcpkg).
3. Add environment VCPKG_ROOT variable with path to vcpkg directory.
4. Install cmake and Visual Studio 2019/2022.
5. Run setup.cmd
