# Pack and Go: enviar un documento con lo que necesita
<!-- keywords: pack and go, empaquetar, paquete, enviar, compartir, otro equipo, zip, carpeta, fuentes, tipografías, archivos vinculados, imágenes vinculadas, imágenes que faltan, copiar, llevar, unidad USB, licencia, INCLUDEPICTURE, INCLUDETEXT, campo -->

Cuando envía un documento a otra persona, las imágenes y las fuentes que usa pueden perderse por el camino. **Pack and Go** guarda el documento junto con lo que necesita, en un archivo ZIP o en una carpeta, para que se vea igual en otro equipo.

## Cómo usarlo

1. Haga clic en **Archivo** y, después, en **Exportar**.
2. Haga clic en **Pack and Go: guardar para otro equipo** para crear un archivo ZIP, o en **Pack and Go: guardar en una carpeta** para crear una carpeta nueva.
3. Elija dónde guardarlo. Para un archivo ZIP, escriba un nombre. Para una carpeta, elija la carpeta donde se creará la nueva.
4. Lea el mensaje que aparece. Indica dónde está el paquete, cuántos archivos vinculados y archivos de fuentes se han incluido, y qué no se pudo empaquetar.

Un **archivo ZIP** es un único archivo que contiene otros archivos, así que es fácil de enviar. La otra persona lo abre para extraer los archivos. Una carpeta es mejor cuando quiere copiar todo a una unidad.

## Qué incluye el paquete

- **El propio documento**, con el mismo nombre que tiene ahora. Es una copia. Su archivo original no cambia.
- **Imágenes vinculadas.** Una imagen vinculada muestra una imagen de un archivo en lugar de contener la imagen en sí. El paquete incluye una copia de cada una.
- **Archivos indicados mediante campos.** Un campo es un código que muestra su propio resultado. Un campo **INCLUDEPICTURE** muestra una imagen de un archivo, y un campo **INCLUDETEXT** muestra texto de otro documento. El paquete incluye una copia de cada archivo, y el campo pasa a apuntar a esa copia.
- **Fuentes.** Una fuente es el aspecto de las letras. El paquete incluye los archivos de fuentes que vienen con el programa y que usa el documento, en una carpeta llamada **fonts**. Pack and Go nunca incluye una fuente cuya licencia indique que no puede distribuirse. El mensaje indica cada fuente que se ha dejado fuera. Las fuentes que usted instaló en su equipo no se incluyen, así que la otra persona podría necesitarlas.

## Cómo cambian los vínculos

En la copia, cada vínculo que se empaquetó apunta al archivo dentro del paquete, mediante un nombre corto, como **linked/photo.png**. Los vínculos ya no usan la ruta de su equipo, así que funcionan dondequiera que se abra el paquete. Si dos archivos tienen el mismo nombre, al segundo se le añade un número.

Un vínculo que Pack and Go no puede seguir se mantiene como estaba, y el mensaje lo indica. Pack and Go no empaqueta:

- Un archivo que no se encuentra.
- Una dirección web. Estas se mantienen tal cual.
- Un archivo de otro equipo de su red.
- Un vínculo que sube y sale de la carpeta del documento, como **../../notas.png**.
- Un documento, libro u otro objeto que no esté en la carpeta del documento (ni en una carpeta contenida en ella), aunque el vínculo indique la ruta completa. Solo se empaquetan imágenes, vídeos y sonidos desde cualquier ubicación.
- Un vínculo que sale de la carpeta del documento a través de un acceso directo a otra carpeta (un vínculo simbólico).
- Un vínculo al propio documento.
- Un archivo que no sea una imagen, un vídeo, un sonido o un documento. Nunca se empaqueta un programa.

Pack and Go no busca más vínculos dentro de los archivos vinculados, así que un archivo que enlace de vuelta con este no puede provocar un bucle.

## Límites

Para que un paquete no crezca demasiado, Pack and Go le pregunta antes cuando hay más de 200 archivos vinculados o más de 256 MB de ellos. Si elige **No**, no se guarda nada. Pack and Go nunca empaqueta más de 2000 archivos ni más de 1 GB, independientemente de su respuesta. Los archivos que superen ese límite no se incluyen y se enumeran en el mensaje.

Si el documento tiene contraseña, la copia que hay en el paquete tiene la misma contraseña.
