arm-linux-gnueabihf-g++ -O2 main.cpp renderer.cpp assets_loader.cpp ../dependencies/include/imgui/imgui.cpp ../dependencies/include/imgui/imgui_draw.cpp ../dependencies/include/imgui/imgui_impl_opengl3.cpp ../dependencies/include/imgui/imgui_impl_sdl2.cpp ../dependencies/include/imgui/imgui_tables.cpp ../dependencies/include/imgui/imgui_widgets.cpp ../dependencies/include/imgui/imgui_demo.cpp -o snake_rpi -I../dependencies/include -I../dependencies/include/imgui -I%SYSROOT%/usr/local/include -I%SYSROOT%/usr/local/include/SDL2 -I%SYSROOT%/opt/vc/include -I%SYSROOT%/opt/vc/include/interface/vcos/pthreads -I%SYSROOT%/opt/vc/include/interface/vmcs_host/linux -D_REENTRANT -L%SYSROOT%/usr/local/lib -L%SYSROOT%/opt/vc/lib -Wl,-rpath,%SYSROOT%/usr/local/lib -Wl,--enable-new-dtags -lbcm_host -lbrcmEGL -lbrcmGLESv2 -lSDL2 -lSDL2_image -lSDL2_mixer -lm -D_RPI1 -DIMGUI_IMPL_OPENGL_ES2