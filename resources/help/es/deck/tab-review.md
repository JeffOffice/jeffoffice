# Pestaña Revisar
<!-- keywords: pestaña revisar, comparar, panel de revisión, aceptar, rechazar, cinta de opciones, revisión, ortografía, sinónimos, recuento de palabras, comentarios, nuevo comentario, eliminar comentario, anterior, siguiente, mostrar comentarios, idioma, idioma de revisión, establecer idioma, no comprobar ortografía, traducir, traducción, libretranslate, gramática, comprobar gramática, estilo, línea azul, línea punteada, comprobar gramática mientras escribe, accesibilidad, texto alternativo -->

La pestaña **Revisar** comprueba sus palabras, las traduce y gestiona los comentarios.

- **Revisión** incluye **Ortografía** (F7), **Sinónimos** (Mayús+F7), **Recuento de palabras** y **Comprobar accesibilidad**. Consulte [Dar formato al texto](text).
- **Idioma** incluye **Traducir**, **Establecer idioma de revisión** y **No comprobar ortografía**. Consulte más abajo.
- **Comparar** incluye **Comparar**, **Aceptar**, **Rechazar**, **Anterior**, **Siguiente** y **Panel de revisión**. Consulte [Comparar presentaciones](compare).
- **Comentarios** incluye **Nuevo comentario** (Ctrl+Alt+M), **Eliminar** con su menú, **Anterior**, **Siguiente** y **Mostrar comentarios**. Consulte [Comentarios](comments).

## Comprobar la gramática mientras escribe

JeffDeck también busca errores claros de gramática y de estilo en el texto de las diapositivas y en las notas del orador. Un problema aparece subrayado con una línea azul de puntos, y una palabra mal escrita conserva su línea roja ondulada. Haga clic con el botón derecho en el problema para ver qué falla y elegir una sugerencia, o elija **Ignorar este problema** o **Ignorar todo**. **Ortografía** (F7) revisa después los problemas de gramática de las diapositivas, tras la ortografía. El texto en un idioma que no se revisa, y el texto marcado como «no comprobar», se omite. Para desactivarla, haga clic en **Archivo > Opciones > Revisión** y desmarque **Comprobar gramática mientras escribe**. Es una única opción para todas las aplicaciones de JeffOffice.

**Qué idiomas se revisan.** La gramática se revisa solo en inglés, en todas sus variantes regionales (estadounidense, británico, canadiense, australiano y el resto). El texto en cualquier otro idioma se revisa solo en ortografía y no recibe marcas de gramática. Un párrafo se revisa en el idioma con el que está marcado, y el texto sin idioma se revisa como inglés. En un párrafo con más de un idioma, solo se revisa el inglés y el resto se omite. El texto que un archivo marca como «no comprobar» también se omite.

**Elegir qué busca.** Haga clic en **Archivo > Opciones > Revisión > Configuración de gramática** para elegir los tipos de problemas: **Gramática** (verbos que no concuerdan, forma incorrecta de un verbo, «a» y «an»), **Puntuación** (espacios, comas y guiones sobrantes o que faltan), **Mayúsculas**, **Palabras confundidas** (palabras que suenan igual), **Repetición** y **Estilo**. **Estilo** está desactivado hasta que lo active. Sugiere mejoras y no se refiere a errores: oraciones de más de 40 palabras, la voz pasiva, tres oraciones seguidas que empiezan por la misma palabra y frases largas con rodeos («in order to», «due to the fact that»). La elección se conserva al hacer clic en **Aceptar** en Opciones. Se aplica de inmediato a las líneas azules y al panel Ortografía (F7) en las ventanas abiertas de esta aplicación, y a las demás aplicaciones de JeffOffice la próxima vez que se inicien.

## Establecer el idioma de un texto

La ortografía revisa cada fragmento de texto en su propio idioma. Para cambiar el idioma de algunas palabras, selecciónelas, o seleccione un cuadro de texto o un marcador de posición completo, y haga clic en **Establecer idioma de revisión** en el grupo **Idioma**. Elija el idioma en la lista y haga clic en **Aceptar**. Las palabras en ese idioma se revisan con su diccionario, y el archivo conserva la elección al guardarlo como presentación o como presentación OpenDocument. El texto que no haya marcado se revisa en inglés estadounidense.

## No comprobar algunos textos

Hay textos que no conviene revisar: un código, un nombre o una línea en un idioma inventado. Selecciónelo y haga clic en **No comprobar ortografía** en el grupo **Idioma**. Las líneas rojas onduladas desaparecen. Haga clic de nuevo para revisar el texto otra vez. El cuadro de notas solo contiene texto sin formato, por lo que no tiene idioma propio.

## Traducir texto

1. Seleccione las palabras de un cuadro de texto o de las notas. Para traducir todo el contenido de un cuadro de texto, seleccione el cuadro.
2. Haga clic en **Traducir** en el grupo **Idioma**.
3. Elija el idioma al que traducir.
4. Haga clic en **Aceptar**.

A continuación, su navegador web abre un servicio de traducción llamado LibreTranslate, con su texto. No se envía nada hasta que haga clic en **Aceptar**. Puede cambiar la dirección del servicio en el cuadro, por ejemplo por una que usted mismo gestione. Solo se usan direcciones web seguras (https), y direcciones http para este equipo. Se envían como máximo 5000 caracteres.

## Comprobar la accesibilidad

**Comprobar accesibilidad** abre una pequeña ventana que enumera lo que puede resultar difícil de usar para algunas personas: una imagen o una forma sin texto alternativo, una diapositiva sin título, una tabla sin fila de encabezado, una tabla con celdas combinadas, un vídeo sin subtítulos, una diapositiva cuyo título tiene también otra diapositiva (un consejo), un vínculo cuyo texto es su propia dirección o no indica adónde lleva (como «aquí» o «leer más»), texto difícil de leer sobre su fondo, y objetos que un lector de pantalla lee en un orden distinto al que sugiere su posición. El texto necesita un contraste de 4,5 a 1 con su fondo (3 a 1 para texto de 18 puntos o más, o de 14 puntos o más en negrita); cuando el texto está sobre una imagen, un consejo le pide comprobar el contraste a simple vista. Un lector de pantalla lee los objetos de una diapositiva desde el fondo de la pila hacia delante. Haga clic en **Orden de lectura** para verlos en ese orden, y use **Leer antes** y **Leer después** (o arrastre) para cambiarlo; cada movimiento es un paso de Deshacer. Un lector de pantalla usa la primera fila de una tabla para nombrar sus columnas: seleccione un problema de tabla y haga clic en **Usar la primera fila como encabezado** para activar la fila de encabezado (un paso de Deshacer). Un vídeo sin subtítulos aparece como consejo: los vídeos necesitan subtítulos para las personas que no pueden oírlos. No hay una corrección con un solo clic; seleccione el vídeo y use **Insertar títulos** en la pestaña **Reproducción**, y el consejo desaparece. Haga clic en un problema para ir a su diapositiva y seleccionar el objeto. Dé a una imagen texto alternativo en el panel **Formato de imagen**, o márquela como decorativa, y después haga clic en **Comprobar de nuevo**. También puede iniciar la comprobación desde **Archivo > Información**.

## Temas relacionados

- [Comparar presentaciones](compare)
- [Comentarios](comments)
- [Dar formato al texto](text)
