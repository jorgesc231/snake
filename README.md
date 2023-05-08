Snake usando SDL2 con OpenGL ES 2
=======================================================

*Implementacion del juego Snake para todas las plataformas que soportan OpenGL ES 2.*

Ejemplo de como desarrollar un juego usando **OpenGL ES 2** para multiples plataformas, incluyendo Raspberry Pi 1 B+, desde Windows usando el **IDE Eclipse**.


**Se puede probar aqui:** <https://jorgesc231.github.io/snake/>

### Controles

- **Espacio** -> Iniciar
- **Escape** -> Pausa
- **Flechas de direccion** -> Controlar la serpiente
- **F1** -> Abrir/Cerrar menu de depuración
- **F5** -> Cierra el juego


## Compilar

### Windows

Instalar:

[https://www.msys2.org/]()


**Asegurarse de instalar *gcc* siguiendo el tutorial de la pagina**.

*Las dependencias estan en el repositorio del proyecto.*

Desde la terminal de *msys2* ir a la carpeta win32_build del proyecto y ejecutar el script:

```
build_win32.bat
```

Esto generará el ejecutable del juego llamado: **snake_win32.exe**.

*Depende de los dlls que estan en la carpeta para ejecutarse.*



### Linux

(Por hacer...)


### Android

(Por hacer...)

### WebAssembly

##### en Windows

Instalar el SDK de Emscripten:

[https://emscripten.org/]()

y seguir el tutorial de instalación.

Abrir la terminal de Windows en la carpeta donde se descargo **emsdk** y ejecutar el siguiente archivo:

```
emsdk_env.bat
```

Para colocar el compilador el path.

Ir a la carpeta **web_build** que esta en el repositorio y ejecutar:


```
build_web.bat
```

Se puede ejecutar de forma local con el comando:


```
emrun index.html
```


##### en Linux


Instalar el SDK de Emscripten:

[https://emscripten.org/]()

Abrir la terminal en el directorio del **emsdk** y ejecutar el siguiente archivo:

```
emsdk_env.sh
```

Para colocar el compilador el path.

Ir a la carpeta **web_build** que esta en el repositorio y ejecutar:


```
build_web.sh
```

Se puede ejecutar de forma local con el comando:


```
emrun index.html
```

### Raspberry Pi

**Para la Raspberry Pi 1 B+, en las versiones mas nuevas deberia funciona con el SDL2 que viene en los repositorios.**

Seguir el tutorial para compilar SDL2 para Raspberry Pi: 

[https://github.com/jorgesc231/tutoriales_raspberry_pi/tree/master/sdl2_gles2_rpi1]()

*Comprobar que el ejemplo con OpenGL ES funcione correctamente.*

Una vez completado, en la misma terminal ir a la carpeta **src/** del repositorio del juego y ejecutar:

```
build_rpi1.bat
```

Deberia ejecutar el binario para la Raspberry Pi, traspasarlo usando **scp** y ejecutarlo.

*NOTA: Por ahora dear imgui no funciona en Raspberry pi.*


## TODO

- Ordenar el codigo
- Subir proyecto de Eclipse
- Port android
- Bug de chocar consigo mismo si se aprietan dos direcciones muy rapido
- Corregir el bloqueo de FPS
- Renderizar texto
- Aspect ratio
- Mejora en el procesamiento de entrada
- Optimizacion