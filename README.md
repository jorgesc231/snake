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

(Por hacer...)

### Raspberry Pi

(Por hacer...)


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