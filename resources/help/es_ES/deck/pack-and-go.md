# Empaquetar y llevar: llevar una presentación consigo
<!-- keywords: empaquetar y llevar, empaquetar, enviar, compartir, otro equipo, zip, carpeta, fuentes, archivos vinculados, imágenes vinculadas, imágenes que faltan, copiar, llevar consigo, unidad USB, licencia, película, sonido, vídeo, audio, presentación, ficheros vinculados -->

Una presentación puede reproducir una película o un sonido desde un fichero que no está dentro de ella. En otro equipo, ese fichero no está, y la presentación tiene un hueco. **Empaquetar y llevar** guarda la presentación junto con sus ficheros vinculados y sus fuentes, en un único fichero ZIP o en una sola carpeta.

## Cómo usarlo

1. Haga clic en **Archivo** y después en **Exportar**.
2. Haga clic en **Empaquetar y llevar: guardar para otro equipo** para crear un único fichero ZIP, o en **Empaquetar y llevar: guardar en una carpeta** para crear una carpeta nueva.
3. Elija dónde guardarlo. Para un fichero ZIP, escriba un nombre. Para una carpeta, elija la carpeta en la que se creará la nueva.
4. Lea el mensaje que aparece. Indica dónde está el paquete, cuántos ficheros vinculados y cuántos ficheros de fuentes se han incluido, y qué no se pudo empaquetar.

Un **fichero ZIP** es un único fichero que contiene otros ficheros, por lo que es fácil de enviar. La otra persona lo abre para extraer los ficheros. Una carpeta es más práctica si quiere copiarlo todo a una unidad.

## Qué incluye el paquete

- **La propia presentación**, con el mismo nombre que tiene ahora. Es una copia. Su fichero original no cambia.
- **Películas y sonidos vinculados.** Una película o un sonido puede estar vinculado, de modo que la presentación lo reproduce desde su fichero en lugar de contenerlo. El paquete incluye una copia de cada uno, para que la presentación se reproduzca igual en el otro equipo. Si no incluyó el fichero en la presentación, así es como se lleva consigo.
- **Imágenes vinculadas.** El paquete incluye una copia de cada imagen vinculada a su fichero.
- **Fuentes.** Una fuente es el aspecto de las letras. El paquete incluye los ficheros de fuente que vienen con el programa y que usa la presentación, en una carpeta llamada **fonts**. Empaquetar y llevar nunca incluye una fuente cuya licencia indique que no puede distribuirse. El mensaje nombra cada fuente que dejó fuera. Las fuentes que usted instaló en su equipo no se incluyen, así que la otra persona podría necesitarlas.

## Cómo cambian los vínculos

En la copia, cada vínculo que se empaquetó apunta al fichero dentro del paquete, mediante un nombre corto, como **linked/photo.png**. Los vínculos ya no usan la ruta de su equipo, así que funcionan dondequiera que se abra el paquete. Si dos ficheros tienen el mismo nombre, al segundo se le añade un número.

Un vínculo que Empaquetar y llevar no puede seguir se mantiene como estaba, y el mensaje lo indica. Empaquetar y llevar no incluye:

- Un fichero que no se puede encontrar.
- Una dirección web. Estas se mantienen tal como están.
- Un fichero de otro equipo de su red.
- Un vínculo que sube y sale de la carpeta de la presentación, como **../../notas.png**.
- Un documento, un libro u otro objeto que no esté en la carpeta de la presentación (ni en una carpeta dentro de ella), aunque el vínculo indique su ruta completa. Solo se empaquetan imágenes, películas y sonidos desde cualquier lugar.
- Un vínculo que sale de la carpeta de la presentación a través de un acceso directo a otra carpeta (un vínculo simbólico).
- Un vínculo a la propia presentación.
- Un fichero que no sea una imagen, una película, un sonido o un documento. Nunca se empaqueta un programa.

Empaquetar y llevar no busca más vínculos dentro de los ficheros vinculados, así que un fichero que enlace de vuelta a esta presentación no puede provocar un bucle.

## Límites

Para que el paquete no crezca demasiado, Empaquetar y llevar le pregunta primero cuando hay más de 200 ficheros vinculados o más de 256 MB de ellos. Si elige **No**, no se guarda nada. Empaquetar y llevar nunca incluye más de 2000 ficheros ni más de 1 GB, sea cual sea su respuesta. Los ficheros que superen ese límite se dejan fuera y aparecen en el mensaje.

Si usa una contraseña en la presentación, la copia del paquete tiene la misma contraseña.
