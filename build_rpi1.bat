@echo off

REM Build the Raspberry Pi version from Windows using the cross-compiler

IF NOT EXIST build_rpi MD build_rpi

arm-linux-gnueabihf-g++ -O3 src/main.cpp src/renderer.cpp src/assets_loader.cpp src/gui.cpp dependencies/include/imgui/imgui.cpp dependencies/include/imgui/imgui_draw.cpp dependencies/include/imgui/imgui_impl_opengl3.cpp dependencies/include/imgui/imgui_impl_sdl2.cpp dependencies/include/imgui/imgui_tables.cpp dependencies/include/imgui/imgui_widgets.cpp dependencies/include/imgui/imgui_demo.cpp -o %BUILD_DIR%/snake -Idependencies/include -Idependencies/include/imgui -I%SYSROOT%/usr/local/include -I%SYSROOT%/usr/local/include/SDL2 -I%SYSROOT%/opt/vc/include -I%SYSROOT%/opt/vc/include/interface/vcos/pthreads -I%SYSROOT%/opt/vc/include/interface/vmcs_host/linux -D_REENTRANT -L%SYSROOT%/usr/local/lib -L%SYSROOT%/opt/vc/lib -Wl,-rpath,%SYSROOT%/usr/local/lib -Wl,--enable-new-dtags -lbcm_host -lbrcmEGL -lbrcmGLESv2 -lSDL2 -lSDL2_image -lSDL2_mixer -lm -D_RPI1 -DIMGUI_IMPL_OPENGL_ES2

IF %ERRORLEVEL% EQU 0 (
	MD %BUILD_DIR%\assets
	MD %BUILD_DIR%\assets\graphics
	MD %BUILD_DIR%\assets\shaders
	MD %BUILD_DIR%\assets\sound
    COPY assets\graphics %BUILD_DIR%\assets\graphics
    COPY assets\shaders %BUILD_DIR%\assets\shaders
    COPY assets\sound %BUILD_DIR%\assets\sound
)