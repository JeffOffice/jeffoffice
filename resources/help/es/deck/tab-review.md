# Pestaña Revisar

<!-- keywords: pestaña revisar, cinta de opciones, revisión, ortografía, sinónimos, recuento de palabras, comentarios, nuevo comentario, eliminar comentario, anterior, siguiente, mostrar comentarios, idioma, idioma de revisión, establecer idioma, no comprobar ortografía, traducir, traducción, libretranslate, gramática, comprobar gramática, estilo, línea azul, línea punteada, comprobar gramática mientras escribe -->

La pestaña **Revisar** comprueba sus palabras, las traduce y administra los comentarios.

- **Revisión** incluye **Ortografía** (F7), **Sinónimos** (Mayús+F7), **Recuento de palabras** y **Comprobar accesibilidad**. Consulte [Escribir y dar formato al texto](text).
- **Idioma** incluye **Traducir**, **Establecer idioma de revisión** y **No comprobar ortografía**. Consulte más abajo.
- **Comentarios** incluye **Nuevo comentario** (Ctrl+Alt+M), **Eliminar** con su menú, **Anterior**, **Siguiente** y **Mostrar comentarios**. Consulte [Comentarios](comments).

## Comprobar la gramática mientras escribe

JeffDeck también busca errores claros de gramática y de estilo en el texto de las diapositivas y de las notas del orador. Un problema aparece subrayado con una línea azul de puntos, y una palabra mal escrita conserva su línea roja ondulada. Haga clic con el botón derecho en el problema para ver qué está mal y elegir una sugerencia, o elija **Omitir este problema** u **Omitir todo**. **Ortografía** (F7) revisa los problemas de gramática de las diapositivas después de la ortografía. El texto en un idioma que no se revisa, y el texto marcado como «no comprobar», se omite. Para desactivarla, haga clic en **Archivo > Opciones > Revisión** y desmarque **Comprobar gramática mientras escribe**. Es una única opción para todas las aplicaciones de JeffOffice.

**Qué idiomas se revisan.** Inglés, francés, alemán y español, cada uno con sus propias reglas. El francés revisa el espacio sin separación antes de ; : ! ? y dentro de « », el apóstrofo en palabras como «l'ami» y «qu'il», y confusiones como «a» y «à». El alemán revisa «das» y «dass», «seid» y «seit», «wider» y «wieder», la coma antes de «dass» y «weil», y las mayúsculas en sustantivos comunes. El español revisa la falta de ¿ o ¡, las tildes en palabras interrogativas, «a ver» y «haber», y «del» y «al». Un párrafo se revisa en el idioma con que está marcado, y el texto sin idioma se revisa como inglés. En un párrafo con más de un idioma se usa el primer idioma revisable y se omite el resto.

**Elegir qué se busca.** Haga clic en **Archivo > Opciones > Revisión > Configuración de gramática** para elegir los tipos de problemas: **Gramática** (verbos que no concuerdan, forma incorrecta de un verbo, «a» y «an»), **Puntuación** (espacios, comas y guiones sobrantes o que faltan), **Mayúsculas**, **Palabras confundidas** (palabras que suenan igual), **Repetición** y **Estilo**. **Estilo** está desactivado hasta que lo active. Sirve para sugerir, no para corregir errores: oraciones de más de 40 palabras, la voz pasiva, tres oraciones seguidas que empiezan con la misma palabra, y frases redundantes («in order to», «due to the fact that»). La elección se conserva al hacer clic en **Aceptar** en Opciones. Se aplica de inmediato a las líneas azules y al panel Ortografía (F7) en las ventanas abiertas de esta aplicación, y a las demás aplicaciones de JeffOffice la próxima vez que se inicien.

## Establecer el idioma de un texto

La ortografía revisa cada fragmento de texto en su propio idioma. Para cambiar el idioma de algunas palabras, selecciónelas, o seleccione un cuadro de texto o un marcador de posición completo, y haga clic en **Establecer idioma de revisión** en el grupo **Idioma**. Elija el idioma en la lista y haga clic en **Aceptar**. Las palabras en ese idioma se revisan con su diccionario, y el archivo conserva la elección al guardarlo como presentación o como presentación OpenDocument. El texto que no ha marcado se revisa en inglés de Estados Unidos.

## No comprobar algunos textos

Algunos textos no deben revisarse: un código, un nombre o una línea en un idioma inventado. Selecciónelo y haga clic en **No comprobar ortografía** en el grupo **Idioma**. Las líneas rojas onduladas desaparecen. Vuelva a hacer clic para revisar el texto de nuevo. El cuadro de notas solo contiene texto sin formato, por lo que no tiene idioma propio.

## Traducir texto

1. Seleccione las palabras de un cuadro de texto o de las notas. Para traducir todo el contenido de un cuadro de texto, seleccione el cuadro.
2. Haga clic en **Traducir** en el grupo **Idioma**.
3. Elija el idioma al que desea traducir.
4. Haga clic en **Aceptar**.

A continuación, su navegador web abre un servicio de traducción llamado LibreTranslate, con su texto. No se envía nada hasta que haga clic en **Aceptar**. Puede cambiar la dirección del servicio en el cuadro, por ejemplo por una que usted mismo gestione. Solo se usan direcciones web seguras (https), y direcciones http de este equipo. Se envían como máximo 5000 letras.

## Comprobar accesibilidad

**Comprobar accesibilidad** abre una pequeña ventana que enumera lo que puede resultar difícil de usar para algunas personas: una imagen o una forma sin texto alternativo, una diapositiva sin título, una tabla sin fila de encabezado, un vídeo sin subtítulos, texto difícil de leer sobre su fondo, y objetos que un lector de pantalla lee en un orden distinto al de su posición. El texto necesita un contraste de 4,5 a 1 con su fondo (3 a 1 para texto de 18 puntos o más, o de 14 puntos o más en negrita); cuando el texto está sobre una imagen, un consejo le pide comprobar el contraste a simple vista. Un lector de pantalla lee los objetos de una diapositiva desde el fondo de la pila hasta el frente. Haga clic en **Orden de lectura** para verlos en ese orden, y use **Leer antes** y **Leer después** (o arrastre) para cambiarlo; cada movimiento es un paso de Deshacer. Un lector de pantalla usa la primera fila de una tabla para dar nombre a sus columnas: seleccione un problema de tabla y haga clic en **Usar la primera fila como encabezado** para activar la fila de encabezado (un paso de Deshacer). Un vídeo sin subtítulos aparece como consejo: los vídeos necesitan subtítulos para las personas que no pueden oírlos. No hay una corrección con un solo clic; seleccione el vídeo y use **Insertar títulos** en la pestaña **Reproducción**, y el consejo desaparece. Haga clic en un problema para ir a su diapositiva y seleccionar el objeto. Dé al texto alternativo de una imagen en el panel **Formato de imagen**, o márquela como decorativa, y después haga clic en **Volver a comprobar**. También puede iniciar la comprobación desde **Archivo > Información**.

## Temas relacionados

- [Comentarios](comments)
- [Escribir y dar formato al texto](text)
