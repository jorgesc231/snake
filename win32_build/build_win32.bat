g++ -O2 ../src/main.cpp ../src/renderer.cpp ../src/assets_loader.cpp ../dependencies/include/imgui/imgui.cpp ../dependencies/include/imgui/imgui_draw.cpp ../dependencies/include/imgui/imgui_impl_opengl3.cpp ../dependencies/include/imgui/imgui_impl_sdl2.cpp ../dependencies/include/imgui/imgui_tables.cpp ../dependencies/include/imgui/imgui_widgets.cpp ../dependencies/include/imgui/imgui_demo.cpp -o snake_win32 -I../dependencies/include -I../dependencies/include/SDL2 -I../dependencies/include/angle -I../dependencies/include/imgui -D_REENTRANT -L../dependencies/lib -lmingw32 -l:libEGL.dll.lib -l:libGLESv2.dll.lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lm -D_WIN32 -DIMGUI_IMPL_OPENGL_ES2