# Pestaña Revisar
<!-- keywords: pestaña revisar, comparar, panel de revisión, aceptar, rechazar, cinta de opciones, revisión, ortografía, sinónimos, recuento de palabras, comentarios, nuevo comentario, eliminar comentario, anterior, siguiente, mostrar comentarios, idioma, idioma de revisión, establecer idioma, no comprobar ortografía, traducir, traducción, libretranslate, gramática, comprobar gramática, estilo, línea azul, línea punteada, comprobar gramática mientras escribe, accesibilidad -->

La pestaña **Revisar** comprueba sus palabras, las traduce y gestiona los comentarios.

- **Revisión** incluye **Ortografía** (F7), **Sinónimos** (Mayús+F7), **Recuento de palabras** y **Comprobar accesibilidad**. Consulte [Dar formato al texto](text).
- **Idioma** incluye **Traducir**, **Establecer idioma de revisión** y **No comprobar ortografía**. Consulte más abajo.
- **Comparar** incluye **Comparar**, **Aceptar**, **Rechazar**, **Anterior**, **Siguiente** y **Panel de revisión**. Consulte [Comparar presentaciones](compare).
- **Comentarios** incluye **Nuevo comentario** (Ctrl+Alt+M), **Eliminar** con su menú, **Anterior**, **Siguiente** y **Mostrar comentarios**. Consulte [Comentarios](comments).

## Comprobar la gramática mientras escribe

JeffDeck también busca errores claros de gramática y de estilo en el texto, en las diapositivas y en las notas del orador. Un problema aparece con una línea azul punteada debajo, y una palabra mal escrita conserva su línea roja ondulada. Haga clic con el botón derecho en el problema para ver qué está mal y elegir una sugerencia, o elija **Omitir este problema** u **Omitir todo**. **Ortografía** (F7) continúa con los problemas de gramática de las diapositivas después de la ortografía. El texto en un idioma que no se comprueba, y el texto marcado como «no comprobar», se omite. Para desactivarla, haga clic en **Archivo > Opciones > Revisión** y desmarque **Comprobar gramática mientras escribe**. Es una única opción para todas las aplicaciones de JeffOffice.

**Idiomas que se comprueban.** Inglés, francés, alemán y español, cada uno con sus propias reglas. El francés revisa el espacio sin separación antes de ; : ! ? y dentro de « », el apóstrofo en palabras como «l'ami» y «qu'il», y confusiones como «a» y «à». El alemán revisa «das» y «dass», «seid» y «seit», «wider» y «wieder», la coma antes de «dass» y «weil», y las mayúsculas en sustantivos comunes. El español busca el ¿ o el ¡ que falte, las tildes en las palabras interrogativas, «a ver» y «haber», y «del» y «al». Un párrafo se comprueba en el idioma en que está marcado, y el texto sin idioma se comprueba como inglés. En un párrafo con más de un idioma se usa el primer idioma comprobable y se omite el resto.

**Elija qué busca.** Haga clic en **Archivo > Opciones > Revisión > Configuración de gramática** para elegir los tipos de problemas: **Gramática** (verbos que no concuerdan, forma incorrecta de un verbo, «a» y «an»), **Puntuación** (espacios, comas y guiones sobrantes o que faltan), **Mayúsculas**, **Palabras confundidas** (palabras que suenan igual), **Repetición** y **Estilo**. **Estilo** está desactivado hasta que lo active. Sugiere mejoras y no corrige errores: oraciones de más de 40 palabras, la voz pasiva, tres oraciones seguidas que empiezan por la misma palabra, y frases redundantes («in order to», «due to the fact that»). La elección se mantiene al hacer clic en **Aceptar** en Opciones. Se aplica de inmediato a las líneas azules y al panel de Ortografía (F7) en las ventanas abiertas de esta aplicación, y a las demás aplicaciones de JeffOffice la próxima vez que se inicien.

## Establecer el idioma de algún texto

La ortografía comprueba cada fragmento de texto en su propio idioma. Para cambiar el idioma de algunas palabras, selecciónelas, o seleccione un cuadro de texto o un marcador de posición completo, y haga clic en **Establecer idioma de revisión** en el grupo **Idioma**. Elija el idioma en la lista y haga clic en **Aceptar**. Las palabras en ese idioma se comprueban con su diccionario, y el archivo conserva la elección al guardarlo como presentación o como presentación de OpenDocument. El texto que no esté marcado se comprueba en inglés de Estados Unidos.

## No comprobar algún texto

Hay texto que no debe comprobarse: un código, un nombre o una línea en un idioma inventado. Selecciónelo y haga clic en **No comprobar ortografía** en el grupo **Idioma**. Las líneas rojas onduladas desaparecen. Haga clic de nuevo para comprobar el texto otra vez. El cuadro de notas solo contiene texto sin formato, por lo que no tiene idioma propio.

## Traducir texto

1. Seleccione las palabras de un cuadro de texto o de las notas. Para traducir todo el contenido de un cuadro de texto, seleccione el cuadro.
2. Haga clic en **Traducir** en el grupo **Idioma**.
3. Elija el idioma al que traducir.
4. Haga clic en **Aceptar**.

Entonces su navegador web abre un servicio de traducción llamado LibreTranslate, con su texto. No se envía nada hasta que haga clic en **Aceptar**. Puede cambiar la dirección del servicio en el cuadro, por ejemplo por una que usted mismo gestione. Solo se usan direcciones web seguras (https), y direcciones http de este equipo. Se envían como máximo 5000 letras.

## Comprobar la accesibilidad

**Comprobar accesibilidad** abre una ventana pequeña que enumera lo que puede resultar difícil de usar para algunas personas: una imagen o una forma sin texto alternativo, una diapositiva sin título, una tabla sin fila de encabezado, un vídeo sin títulos, texto difícil de leer sobre su fondo, y objetos que un lector de pantalla lee en un orden distinto al que sugiere su posición. El texto necesita un contraste de 4,5 a 1 con su fondo (3 a 1 para texto de 18 puntos o más, o de 14 puntos o más en negrita); cuando el texto está sobre una imagen, un consejo le pide que compruebe el contraste a simple vista. Un lector de pantalla lee los objetos de una diapositiva desde el fondo de la pila hasta el frente. Haga clic en **Orden de lectura** para verlos en ese orden, y use **Leer antes** y **Leer después** (o arrastre) para cambiarlo; cada movimiento es un paso de Deshacer. Un lector de pantalla usa la primera fila de una tabla para nombrar sus columnas: seleccione un problema de tabla y haga clic en **Usar la primera fila como encabezado** para activar la fila de encabezado (un paso de Deshacer). Un vídeo sin títulos aparece como consejo: los vídeos necesitan títulos para las personas que no pueden oírlos. No hay una solución con un solo clic; seleccione el vídeo y use **Insertar títulos** en la pestaña **Reproducción**, y el consejo desaparece. Haga clic en un problema para ir a su diapositiva y seleccionar el objeto. Dé a una imagen texto alternativo en el panel **Formato de imagen**, o márquela como decorativa, y después haga clic en **Volver a comprobar**. También puede iniciar la comprobación desde **Archivo > Información**.

## Temas relacionados

- [Comparar presentaciones](compare)
- [Comentarios](comments)
- [Dar formato al texto](text)
