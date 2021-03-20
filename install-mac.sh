# git clone https://github.com/microsoft/vcpkg
# ./vcpkg/bootstrap-vcpkg.sh

./vcpkg/vcpkg install glad:x64-osx
./vcpkg/vcpkg install glfw3:x64-osx
./vcpkg/vcpkg install glm:x64-osx
./vcpkg/vcpkg install stb:x64-osx
# Assimp fails to install on MacOS: Target 'osx' not supported by irrlicht!
# ./vcpkg/vcpkg install assimp:x64-osx

./vcpkg/vcpkg integrate install
