@echo off

set BUILD_DIR=build_web

IF NOT EXIST %BUILD_DIR%\assets MD %BUILD_DIR%\assets
IF NOT EXIST %BUILD_DIR%\assets\graphics MD %BUILD_DIR%\assets\graphics
IF NOT EXIST %BUILD_DIR%\assets\shaders MD %BUILD_DIR%\assets\shaders
IF NOT EXIST %BUILD_DIR%\assets\sound MD %BUILD_DIR%\assets\sound
COPY assets\graphics %BUILD_DIR%\assets\graphics
COPY assets\shaders %BUILD_DIR%\assets\shaders
COPY assets\sound %BUILD_DIR%\assets\sound
COPY assets\favicon.ico %BUILD_DIR%

PUSHD %BUILD_DIR%

REM Release
REM call emcc -Os --use-preload-plugins ../src/main.cpp ../src/renderer.cpp ../src/assets_loader.cpp ../src/gui.cpp ../dependencies/include/imgui/imgui.cpp ../dependencies/include/imgui/imgui_draw.cpp ../dependencies/include/imgui/imgui_impl_opengl3.cpp ../dependencies/include/imgui/imgui_impl_sdl2.cpp ../dependencies/include/imgui/imgui_tables.cpp ../dependencies/include/imgui/imgui_widgets.cpp ../dependencies/include/imgui/imgui_demo.cpp -I../dependencies/include -I../dependencies/include/imgui --emrun --shell-file new_shell.html --preload-file assets -s USE_SDL=2 -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS=["mp3"] -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=["png"] -s STACK_SIZE=2mb -s ALLOW_MEMORY_GROWTH -s MAX_WEBGL_VERSION=2 -o index.html

REM Debug
call emcc -g -O0 --profiling --use-preload-plugins ../src/main.cpp ../src/renderer.cpp ../src/assets_loader.cpp ../src/gui.cpp ../dependencies/include/imgui/imgui.cpp ../dependencies/include/imgui/imgui_draw.cpp ../dependencies/include/imgui/imgui_impl_opengl3.cpp ../dependencies/include/imgui/imgui_impl_sdl2.cpp ../dependencies/include/imgui/imgui_tables.cpp ../dependencies/include/imgui/imgui_widgets.cpp ../dependencies/include/imgui/imgui_demo.cpp -I../dependencies/include -I../dependencies/include/imgui --emrun --shell-file new_shell.html --preload-file assets -s USE_SDL=2 -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS=["mp3"] -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=["png"] -s MAX_WEBGL_VERSION=2 -s STACK_SIZE=2mb -s ALLOW_MEMORY_GROWTH -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=2 -s ASSERTIONS=2 -DNDEBUG -o index.html

REM echo %ERRORLEVEL%

RD /S /Q assets

POPD