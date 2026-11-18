# Empaquetar y llevar: enviar un libro con lo que necesita

<!-- keywords: empaquetar y llevar, empaquetar, enviar, compartir, otro equipo, zip, carpeta, fuentes, archivos vinculados, imágenes vinculadas, imágenes que faltan, copiar, llevar contigo, unidad USB, licencia, vínculo externo, libro vinculado, celdas, gráficos, paquete, comprimir, tipografías, ficheros vinculados -->

Cuando envía un libro a otra persona, las imágenes y los otros libros a los que enlaza pueden perderse por el camino. **Empaquetar y llevar** guarda el libro junto con esos ficheros y con sus fuentes, en un fichero ZIP o en una carpeta, para que se vea igual en otro equipo.

## Cómo usarlo

1. Haga clic en **Archivo** y, después, en **Exportar**.
2. Haga clic en **Empaquetar y llevar: guardar para otro equipo** para crear un único fichero ZIP, o en **Empaquetar y llevar: guardar en una carpeta** para crear una carpeta nueva.
3. Elija dónde guardarlo. Para un fichero ZIP, escriba un nombre. Para una carpeta, elija la carpeta en la que se creará la nueva.
4. Lea el mensaje que aparece. Indica dónde está el paquete, cuántos ficheros vinculados y cuántos ficheros de fuentes se han incluido, y qué no se ha podido empaquetar.

Un **fichero ZIP** es un único fichero que contiene otros ficheros, por lo que es fácil de enviar. La otra persona lo abre para extraer los ficheros. Una carpeta resulta más práctica si quiere copiarlo todo a una unidad.

## Qué incluye el paquete

- **El propio libro**, con el mismo nombre que tiene ahora. Es una copia. Su fichero original no cambia.
- **Imágenes vinculadas.** Una imagen del libro puede estar vinculada a su fichero. El paquete contiene una copia de cada una. Las imágenes insertadas directamente en el libro ya están dentro de él.
- **Libros vinculados.** Una fórmula puede obtener números de otro libro. El paquete contiene también una copia de ese libro, y el vínculo ahora apunta a la copia. Las fuentes son las que se usan en las celdas, los gráficos y las formas.
- **Fuentes.** Una fuente es el aspecto de las letras. El paquete contiene los ficheros de fuente que vienen con el programa y que usa el libro, en una carpeta llamada **fonts**. Empaquetar y llevar nunca incluye una fuente cuya licencia indique que no puede distribuirse. El mensaje nombra cada fuente que se ha dejado fuera. Las fuentes que usted instaló en su equipo no se incluyen, así que la otra persona puede necesitarlas.

## Cómo cambian los vínculos

En la copia, cada vínculo que se empaquetó apunta al fichero dentro del paquete, mediante un nombre corto, como **linked/photo.png**. Los vínculos ya no usan la ruta de su equipo, así que funcionan dondequiera que se abra el paquete. Si dos ficheros tienen el mismo nombre, al segundo se le añade un número.

Un vínculo que Empaquetar y llevar no puede seguir se mantiene como estaba, y el mensaje lo indica. Empaquetar y llevar no incluye:

- Un fichero que no se encuentra.
- Una dirección web. Estas se mantienen tal cual.
- Un fichero de otro equipo de su red.
- Un vínculo que sube y sale de la carpeta del libro, como **../../notas.png**.
- Un documento, libro u otro objeto que no esté en la carpeta del libro (ni en una carpeta dentro de ella), aunque el vínculo indique la ruta completa. Solo se empaquetan imágenes, vídeos y sonidos desde cualquier ubicación.
- Un vínculo que sale de la carpeta del libro a través de un acceso directo a otra carpeta (un vínculo simbólico).
- Un vínculo al propio libro.
- Un fichero que no es una imagen, un vídeo, un sonido ni un documento. Nunca se empaqueta un programa.

Empaquetar y llevar no busca más vínculos dentro de los ficheros vinculados, de modo que un fichero que enlace de vuelta con este no puede provocar un bucle.

## Límites

Para evitar que un paquete crezca demasiado, Empaquetar y llevar le pregunta antes si hay más de 200 ficheros vinculados o más de 256 MB en total. Si elige **No**, no se guarda nada. Empaquetar y llevar nunca incluye más de 2000 ficheros ni más de 1 GB, sea cual sea su respuesta. Los ficheros que superen ese límite se dejan fuera y se enumeran en el mensaje.

Si el libro tiene contraseña, la copia que hay en el paquete conserva la misma contraseña.
