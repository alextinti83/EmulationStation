@echo off
mkdir build
cd build

set ES_LIB_DIR=d:\src\lib
cmake -g "Visual Studio 14 2015 x86" .. -DLUA_LIBRARIES=%ES_LIB_DIR%\lua5.1\lib\lua51.lib -DLUA_INCLUDE_DIR=%ES_LIB_DIR%\lua5.1\include -DEIGEN3_INCLUDE_DIR=%ES_LIB_DIR%\eigen -DFREETYPE_INCLUDE_DIRS=%ES_LIB_DIR%\freetype-2.7\include -DFREETYPE_LIBRARY=%ES_LIB_DIR%\freetype-2.7\objs\vc2010\Win32\freetype27.lib -DFreeImage_INCLUDE_DIR=%ES_LIB_DIR%\FreeImage\Source -DFreeImage_LIBRARY=%ES_LIB_DIR%\FreeImage\Dist\x32\FreeImage.lib -DSDL2_INCLUDE_DIR=%ES_LIB_DIR%\SDL2-2.0.5\include -DSDL2_LIBRARY=%ES_LIB_DIR%\SDL2-2.0.5\build\Release\SDL2.lib;%ES_LIB_DIR%\SDL2-2.0.5\build\Release\SDL2main.lib;Imm32.lib;version.lib -DBOOST_ROOT=%ES_LIB_DIR%\boost_1_61_0 -DBoost_LIBRARY_DIR=%ES_LIB_DIR%\boost_1_61_0\lib32-msvc-14.0 -DCURL_INCLUDE_DIR=%ES_LIB_DIR%\curl-7.50.3\include -DCURL_LIBRARY=%ES_LIB_DIR%\curl-7.50.3\builds\libcurl-vc14-x86-release-dll-ipv6-sspi-winssl\lib\libcurl.lib -DVLC_INCLUDE_DIR=%ES_LIB_DIR%\libvlc-2.2.2\include -DVLC_LIBRARIES=%ES_LIB_DIR%\libvlc-2.2.2\lib\msvc\libvlc.lib;%ES_LIB_DIR%\libvlc-2.2.2\lib\msvc\libvlccore.lib -DVLC_VERSION=1.0.0     
pause

rem pi@retropie:~/RetroStation/build $ sudo apt-get install liblua5.1-0-dev
