
![image](Window.png)

# Main features

This project contains a C++ OpenGL viewer for 3D gaussian splatting scenes.

Pre-trained models can be downloaded from:
"https://github.com/graphdeco-inria/gaussian-splatting/tree/main"

The main particularity is that the gaussians are rendered with standard alpha blending instead of the custom tile-based renderer used in the 3DGS code.

Note: An nvidia GPU supporting OpenGL 4.6 is required, with the cuda toolkit installed.

The rendering loop is as follows:

- Cull the gaussians outside of the view frustum (see [resources/shaders/testVisibility.cp](resources/shaders/testVisibility.cp))
- Sort the gaussians either front to back or back to front (with cub's radix sort and opengl/cuda interop), see [src/Sort.cu](src/Sort.cu)
- Compute oriented bounding boxes for the remaining gaussians (see [resources/shaders/computeBoundingBoxes.cp](resources/shaders/computeBoundingBoxes.cp))
- Evaluate the spherical harmonic basis of each gaussian ([resources/shaders/predict_colors.cp](resources/shaders/predict_colors.cp))
- Render the gaussians with the hardware rasterizer and alpha blending (see [resources/shaders/quad.vs](resources/shaders/quad.vs) and [resources/shaders/quad.fs](resources/shaders/quad.fs).

The main rendering loop is in [src/GaussianCloud.cpp](src/GaussianCloud.cpp).

The opengl shaders are located in the [resources/shaders](resources/shaders) directory. The file extensions are .cp for compute shaders, .vs for vertex shaders and .fs for fragment shaders. GLSL header files are located in [resources/shaders/common](resources/shaders/common). 
The shaders use special comments (/\*-- and --\*/ ) so that they can be edited with a standard C++ IDE. These comments are remove at runtime with a regex to produce valid GLSL code.

Note: The viewer will search for a file named "bicycle.ply" in the current directory when clicking "Load ply" (see [src/Window.cu:239](src/Window.cu:239) to change the file name).

# Installation


- Step 1:

Install glfw. Open a developper command prompt ("Developper PowerShell for VS 2022").

> ./install_script_win.ps1

- Step 2: 
Build the program. Make sure to adapt the command line to your specific cuda version, paths, etc...

> $install_dir = (Join-Path $pwd "/libs_install")
> mkdir build
> cd build
> cmake .. -DCMAKE_PREFIX_PATH="$install_dir" -DCMAKE_GENERATOR_TOOLSET="cuda=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6"
> cmake --build . --target INSTALL --config Release -j
> cd ../

> cp ./libs_install/bin/glfw3.dll ./build/Release/

Run from root directory with:
> ./build/Release/HardwareRasterized3DGS.exe
