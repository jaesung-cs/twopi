echo %~dp0
if not exist "../../bin/shaders" mkdir "../../bin/shaders"
C:/VulkanSDK/1.2.162.0/Bin32/glslc.exe %~dp0%1.vert -o %~dp0../../bin/shaders/%1.vert.spv
C:/VulkanSDK/1.2.162.0/Bin32/glslc.exe %~dp0%1.frag -o %~dp0../../bin/shaders/%1.frag.spv
