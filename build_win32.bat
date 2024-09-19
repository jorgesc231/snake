@echo off

set SRC_PATH=src
set DEP_PATH=dependencies/include
set BUILD_DIR=build_win32

IF NOT EXIST %BUILD_DIR%\assets MD %BUILD_DIR%\assets
IF NOT EXIST %BUILD_DIR%\assets\graphics MD %BUILD_DIR%\assets\graphics
IF NOT EXIST %BUILD_DIR%\assets\shaders MD %BUILD_DIR%\assets\shaders
IF NOT EXIST %BUILD_DIR%\assets\sound MD %BUILD_DIR%\assets\sound
COPY assets\graphics %BUILD_DIR%\assets\graphics
COPY assets\shaders %BUILD_DIR%\assets\shaders
COPY assets\sound %BUILD_DIR%\assets\sound

REM Generate the icon resorce
windres assets\snake_res.rc -o assets\snake_res.o

REM Release
g++ -O3 --static %SRC_PATH%/main.cpp %SRC_PATH%/renderer.cpp %SRC_PATH%/assets_loader.cpp %SRC_PATH%/gui.cpp assets/snake_res.o %DEP_PATH%/imgui/imgui.cpp %DEP_PATH%/imgui/imgui_draw.cpp %DEP_PATH%/imgui/imgui_impl_opengl3.cpp %DEP_PATH%/imgui/imgui_impl_sdl2.cpp %DEP_PATH%/imgui/imgui_tables.cpp %DEP_PATH%/imgui/imgui_widgets.cpp %DEP_PATH%/imgui/imgui_demo.cpp -Wl,-subsystem,windows -o %BUILD_DIR%\snake_win32 -I%DEP_PATH% -I%DEP_PATH%/SDL2 -I%DEP_PATH%/angle -I%DEP_PATH%/imgui -D_REENTRANT -Ldependencies/lib -lmingw32 -l:libEGL.dll.lib -l:libGLESv2.dll.lib -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_mixer.dll -lm -D_WIN32 -DIMGUI_IMPL_OPENGL_ES2

REM Debug
REM g++ -g --static %SRC_PATH%/main.cpp %SRC_PATH%/renderer.cpp %SRC_PATH%/assets_loader.cpp %SRC_PATH%/gui.cpp %DEP_PATH%/imgui/imgui.cpp %DEP_PATH%/imgui/imgui_draw.cpp %DEP_PATH%/imgui/imgui_impl_opengl3.cpp %DEP_PATH%/imgui/imgui_impl_sdl2.cpp %DEP_PATH%/imgui/imgui_tables.cpp %DEP_PATH%/imgui/imgui_widgets.cpp %DEP_PATH%/imgui/imgui_demo.cpp -o %BUILD_DIR%\snake_win32 -I%DEP_PATH% -I%DEP_PATH%/SDL2 -I%DEP_PATH%/angle -I%DEP_PATH%/imgui -D_REENTRANT -Ldependencies/lib -lmingw32 -l:libEGL.dll.lib -l:libGLESv2.dll.lib -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_mixer.dll -lm -D_WIN32 -DIMGUI_IMPL_OPENGL_ES2 -DNDEBUG


IF %ERRORLEVEL% EQU 0 (
	DEL assets\snake_res.o
	ECHO BUILD SUCCESSFUL
)