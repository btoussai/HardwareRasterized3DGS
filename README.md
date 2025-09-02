
Step 1:
install libs:

Open a developper command prompt ("Developper PowerShell for VS 2022")

> ./install_script_win.ps1

> $install_dir = (Join-Path $pwd "/libs_install")
> mkdir build
> cd build
> cmake .. -DCMAKE_PREFIX_PATH="$install_dir" -DCMAKE_GENERATOR_TOOLSET="cuda=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6"
> cmake --build . --target INSTALL --config Release -j
> cd ../

> cp ./libs_install/bin/glfw3.dll ./build/Release/

Run from root directory with:
> ./build/Release/HardwareRasterized3DGS.exe

Pre-trained models can be downloaded from:
"https://github.com/graphdeco-inria/gaussian-splatting/tree/main"

