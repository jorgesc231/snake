BUILD_DIR=build_web

cp -r ./assets ./$BUILD_DIR

pushd $BUILD_DIR

rm -r assets/graphics/vector
rm assets/snake_res.rc
mv assets/favicon.ico .

emcc -Os --use-preload-plugins ../src/main.cpp ../src/renderer.cpp ../src/assets_loader.cpp ../src/gui.cpp ../dependencies/include/imgui/imgui.cpp ../dependencies/include/imgui/imgui_draw.cpp ../dependencies/include/imgui/imgui_impl_opengl3.cpp ../dependencies/include/imgui/imgui_impl_sdl2.cpp ../dependencies/include/imgui/imgui_tables.cpp ../dependencies/include/imgui/imgui_widgets.cpp ../dependencies/include/imgui/imgui_demo.cpp -I../dependencies/include -I../dependencies/include/imgui --shell-file new_shell.html --preload-file assets -s USE_SDL=2 -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS=["mp3"] -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=["png"] -s STACK_SIZE=2mb -s ALLOW_MEMORY_GROWTH -s MAX_WEBGL_VERSION=2 -o index.html

rm -r assets

popd