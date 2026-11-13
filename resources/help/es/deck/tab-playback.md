# Pestaña Reproducción
<!-- keywords: pestaña reproducción, cinta de opciones, película, vídeo, sonido, audio, reproducir, pausa, vista previa, recortar, subtítulos, insertar subtítulos, quitar subtítulos, webvtt, vtt, volumen, silenciar, iniciar, en la secuencia de clics, automáticamente, al hacer clic, reproducir a pantalla completa, ocultar si no se está reproduciendo, repetir hasta que se detenga, rebobinar tras la reproducción, herramientas de multimedia -->

La pestaña **Reproducción** define cómo se reproduce una película o un sonido, y agrega subtítulos a una película. Solo se muestra mientras hay seleccionada una película o un sonido, y desaparece al seleccionar otra cosa.

- **Vista previa** tiene **Reproducir**. Reproduce la película o el sonido de la diapositiva para que pueda verlo y oírlo. Mientras se reproduce, el botón dice **Pausa**.
- **Edición** tiene **Recortar**. Abre un cuadro con **Hora de inicio** y **Hora de finalización**, en segundos. La película o el sonido se reproducen solo desde la hora de inicio hasta la hora de finalización. El resto del archivo permanece dentro de la presentación. Para volver a reproducirlo hasta el final del archivo, establezca la hora de finalización en la duración completa.
  - **Insertar subtítulos** agrega subtítulos a una película desde un archivo WebVTT (un archivo de texto que termina en `.vtt`). Pide el archivo y después el idioma de los subtítulos, como un código corto, por ejemplo `en`, `es` o `pt-BR`. Los subtítulos se asocian a una película incrustada en la presentación; en el caso de una película solo vinculada desde fuera, el botón aparece atenuado. Una película conserva un conjunto de subtítulos por cada idioma: insertar un segundo archivo para el mismo idioma reemplaza al primero. Si el archivo no es WebVTT, no contiene subtítulos o es demasiado grande (el máximo es 4 MB, 20 000 subtítulos y 2000 caracteres en un solo subtítulo), se rechaza con un mensaje y no se cambia nada.
  - **Quitar subtítulos** elimina los subtítulos de la película. Si la película tiene subtítulos en más de un idioma, pregunta cuál quitar o si quiere quitarlos todos.
- **Opciones de multimedia** indica cómo se reproduce en la presentación con diapositivas:
  - **Volumen** puede ser **Bajo**, **Medio**, **Alto** o **Silenciar**.
  - **Inicio** indica qué lo inicia. **En la secuencia de clics** lo reproduce con un clic, junto con las animaciones de la diapositiva. **Automáticamente** lo reproduce por sí solo cuando le llega su turno. **Al hacer clic** lo reproduce cuando hace clic en la película o el sonido.
  - **Reproducir a pantalla completa** hace que una película ocupe toda la pantalla mientras se reproduce. Solo funciona con películas.
  - **Ocultar si no se está reproduciendo** oculta la imagen hasta que la película o el sonido se reproducen, y de nuevo cuando se detienen. La imagen del altavoz de un sonido permanece oculta todo el tiempo.
  - **Repetir hasta que se detenga** vuelve a reproducirlo cada vez que termina, hasta que se detenga o cambie la diapositiva.
  - **Rebobinar tras la reproducción** devuelve una película a su primera imagen al terminar. Sin esta opción, la película se queda en su última imagen.

Los subtítulos se muestran en la parte inferior de la película, con texto blanco sobre una franja oscura, mientras la película se reproduce en la presentación con diapositivas y en el editor. Están sincronizados con la película, y su tamaño sigue al de la película. Si una película tiene más de un idioma, se muestra el primero. Los subtítulos se guardan con la presentación, como parte del archivo `.pptx`, así que vuelven a aparecer al abrirla de nuevo. Un archivo de subtítulos en otro formato (que no sea WebVTT) se conserva tal cual y se vuelve a guardar sin cambios, pero no se muestra. Cuando una película tiene subtítulos, **Comprobar accesibilidad** deja de señalarla.

Cada cambio es un solo paso. **Ctrl+Z** lo deshace.

Un sonido o una película que se reproduce a lo largo de varias diapositivas, algo que algunos archivos establecen, sigue reproduciéndose durante tantas diapositivas como indique el archivo. JeffDeck lo reproduce así en la presentación con diapositivas y conserva esta configuración al guardar.

Consulte [Formas, imágenes, tablas y texto artístico](objects) y [Reproducir la presentación con diapositivas y usar la Vista Moderador](show).

## Relacionados

- [Formas, imágenes, tablas y texto artístico](objects)
- [Pestaña Insertar](tab-insert)
- [Reproducir la presentación con diapositivas y usar la Vista Moderador](show)
