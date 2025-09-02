# Run this from a developer command prompt so that cmake can find cl.exe

mkdir libs
mkdir libs_install

$install_dir = (Join-Path $pwd "/libs_install")

cd libs 

# Install GLFW
git clone --depth 1 --branch 3.4 https://github.com/glfw/glfw.git glfw
cd glfw
mkdir build
cd build
cmake -S .. -B . -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DCMAKE_INSTALL_PREFIX:PATH="$install_dir"
cmake --build . --config Release --target INSTALL -j
cd ../../

cd ..



