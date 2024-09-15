Snake usando SDL2 con OpenGL ES 2
=======================================================

*Implementacion del juego Snake para todas las plataformas que soportan OpenGL ES 2.*

Ejemplo de como desarrollar un juego usando **OpenGL ES 2** para multiples plataformas, incluyendo Raspberry Pi 1 B+, desde Windows usando el **IDE Eclipse**.


**Se puede probar aqui:** <https://jorgesc231.github.io/snake/>


## Compilar

### Windows

Instalar:

[https://www.msys2.org/]()


**Asegurarse de instalar *gcc* siguiendo el tutorial de la pagina**.

*Las dependencias estan en el repositorio del proyecto.*

Desde la terminal de *msys2* ir a la carpeta del proyecto y ejecutar el script:

```
build_win32.bat
```

Esto generará el ejecutable del juego en la carpeta *build_win32* y copiara los assets.

*Depende de los dlls que estan en la carpeta para ejecutarse.*



### Linux

(Por hacer...)


### Android

(Por hacer...)


### WebAssembly

##### en Linux


Instalar el SDK de Emscripten:

[https://emscripten.org/]()

Abrir la terminal en el directorio del **emsdk** y ejecutar el siguiente archivo:

```
emsdk_env.sh
```

Para colocar el compilador el path.

Ir al directorio del repositorio y ejecutar:


```
build_web.sh
```

El resultado estara en *build_web*.

También se puede descomentar la linea de compilación de debug para poder ejecutar de forma local con el comando:

```
emrun index.html
```


### Raspberry Pi

**En la Raspberry Pi 1 B+ el SDL2 que esta en los repositorios no funciona hay que compilar SDL2 desde el codigo fuente usando el siguiente tutorial:**

[https://github.com/jorgesc231/tutoriales_raspberry_pi/tree/master/sdl2_gles2_rpi1]()

*Comprobar que el ejemplo con OpenGL ES funcione correctamente.*

**En las versiones más nuevas deberia funcionar con el SDL2 de los repositorios.**

Una vez instaladas las bibliotecas SDL2, en la terminal ir al directorio del repositorio y ejecutar:

```
build_rpi1.bat
```

Deberia generar el binario para la Raspberry Pi y copiar los assets en el directorio *build_rpi*, traspasar todo el contenido del directorio usando **scp** y ejecutarlo.