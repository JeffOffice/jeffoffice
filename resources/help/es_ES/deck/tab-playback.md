# Pestaña Reproducción

<!-- keywords: pestaña reproducción, cinta de opciones, película, vídeo, sonido, audio, reproducir, pausa, vista previa, recortar, títulos, insertar títulos, quitar títulos, subtítulos, webvtt, vtt, volumen, silenciar, iniciar, en secuencia de clics, automáticamente, al hacer clic, reproducir a pantalla completa, ocultar si no se está reproduciendo, repetir hasta que se detenga, rebobinar al terminar, reproducir entre diapositivas, mantener reproduciendo, texto alternativo, alinear, alto, ancho, tamaño, herramientas de multimedia -->

La pestaña **Reproducción** define cómo se reproduce un vídeo o un sonido, y añade títulos a un vídeo. Solo se muestra mientras hay seleccionado un vídeo o un sonido, y desaparece cuando selecciona otra cosa.

- **Vista previa** tiene **Reproducir**. Reproduce el vídeo o el sonido de la diapositiva para que pueda verlo y oírlo. Mientras se reproduce, el botón dice **Pausa**.
- **Edición** tiene **Recortar**. Abre un cuadro con **Hora de inicio** y **Hora de finalización**, en segundos. El vídeo o el sonido solo se reproduce desde la hora de inicio hasta la hora de finalización. El resto del fichero permanece dentro de la presentación. Para volver a reproducirlo hasta el final, ajuste la hora de finalización a la duración completa.
  - **Insertar títulos** añade títulos a un vídeo desde un fichero WebVTT (un fichero de texto que termina en `.vtt`). Pide el fichero y, después, el idioma de los títulos, como un código corto como `en`, `es` o `pt-BR`. Los títulos acompañan a un vídeo que está incrustado en la presentación; en un vídeo que solo está vinculado desde fuera, el botón aparece atenuado. Un vídeo conserva un conjunto de títulos por cada idioma: insertar un segundo fichero para el mismo idioma sustituye al primero. Un fichero que no es WebVTT, que no contiene títulos o que es demasiado grande (el máximo es de 4 MB, 20 000 títulos y 2000 caracteres en un solo título) se rechaza con un mensaje, y no cambia nada.
  - **Quitar títulos** elimina los títulos del vídeo. Si el vídeo tiene títulos en más de un idioma, pregunta cuál quitar o si quitarlos todos.
- **Opciones de multimedia** indica cómo se reproduce en la presentación con diapositivas:
  - **Volumen** es **Bajo**, **Medio**, **Alto** o **Silenciar**.
  - **Iniciar** indica qué lo inicia. **En secuencia de clics** lo reproduce con un clic, junto con las animaciones de la diapositiva. **Automáticamente** lo reproduce por sí solo cuando le llega su turno. **Al hacer clic** lo reproduce al hacer clic en el vídeo o el sonido.
  - **Reproducir a pantalla completa** hace que un vídeo ocupe toda la pantalla mientras se reproduce. Solo funciona con vídeos.
  - **Ocultar si no se está reproduciendo** oculta la imagen hasta que el vídeo o el sonido se reproduce, y de nuevo después de detenerse. La imagen del altavoz de un sonido permanece oculta todo el tiempo.
  - **Repetir hasta que se detenga** vuelve a reproducirlo cada vez que termina, hasta que se detenga o cambie la diapositiva.
  - **Rebobinar al terminar** devuelve un vídeo a su primera imagen cuando termina. Si no está marcada, el vídeo se queda en su última imagen.
  - **Reproducir entre diapositivas** (para un sonido; un vídeo se dibuja solo en su propia diapositiva) mantiene el sonido en reproducción en la presentación con diapositivas cuando cambia la diapositiva, hasta que termina, se detiene o llega al final de la presentación. Un fichero de otro programa puede indicar en cuántas diapositivas se reproduce el sonido: ese número se mantiene mientras la casilla esté marcada, y el sonido se detiene después de esas diapositivas, contadas desde la diapositiva en la que empezó (ir y volver entre ellas no consume ninguna). Si la casilla no está marcada, el sonido se detiene con su diapositiva; si la marca de nuevo, continúa hasta el final de la presentación. Con **Repetir hasta que se detenga**, vuelve a empezar cada vez que termina. Ctrl+Z deshace el cambio, y un fichero .pptx lo conserva.

Los títulos se muestran sobre la parte inferior del vídeo, con texto blanco sobre una franja oscura, mientras el vídeo se reproduce en la presentación con diapositivas y en el editor. Están sincronizados con el vídeo, y su tamaño sigue al del vídeo. Si un vídeo tiene más de un idioma, se muestra el primero. Los títulos se guardan con la presentación, como parte del fichero `.pptx`, así que vuelven a aparecer al abrirla de nuevo. Un fichero de títulos en otro formato (que no sea WebVTT) se conserva tal como está y se vuelve a guardar sin cambios, pero no se muestra. Cuando un vídeo tiene títulos, **Comprobar accesibilidad** ya no lo incluye en la lista.

Los grupos **Accesibilidad**, **Organizar** y **Tamaño** son los mismos que tiene la pestaña Formato de imagen: **Texto alternativo**, **Alinear** y los cuadros de **Alto** y **Ancho**, para el vídeo o el sonido seleccionado.

Cada cambio es un paso. **Ctrl+Z** lo deshace.

Un sonido o un vídeo que se reproduce entre diapositivas, como establecen algunos ficheros, sigue sonando durante tantas diapositivas como indique el fichero. JeffDeck lo reproduce así en la presentación con diapositivas, y conserva el ajuste al guardar.

Consulte [Formas, imágenes, tablas y texto artístico](objects) y [Reproducir la presentación con diapositivas y usar la Vista Moderador](show).

## Relacionados

- [Formas, imágenes, tablas y texto artístico](objects)
- [Pestaña Insertar](tab-insert)
- [Reproducir la presentación con diapositivas y usar la Vista Moderador](show)
