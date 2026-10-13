# Abrir y guardar presentaciones
<!-- keywords: abrir, guardar, guardar como, otp, libreoffice, omitido, cerrar, nuevo, nueva presentación, archivos recientes, anclado, tipos de archivo, formatos, pptx, potx, ppsx, ppt, odp, plantilla, presentación con diapositivas, cambiar nombre, Ctrl+S, copia de seguridad, formatos de archivo, archivos antiguos, OpenDocument, conservado, no se muestra, fuentes, tipos de letra, insertar fuentes, incrustar fuentes, fuentes insertadas, fuente que falta -->

JeffDeck guarda su trabajo en archivos .pptx y en archivos .odp, y abre varios tipos de archivos de presentación.

## Empezar una presentación nueva

Presione Ctrl+N. Se abre una ventana nueva con una presentación en blanco. También puede hacer clic en **Archivo > Nuevo** y, después, en **Presentación en blanco**. Si la ventana en la que está contiene una presentación nueva que nadie ha modificado, la presentación en blanco ocupa su lugar. De lo contrario, se abre en una ventana propia.

## Guardar

Presione Ctrl+S o haga clic en **Archivo > Guardar**. La primera vez, JeffDeck le pide un nombre y una ubicación. A partir de entonces, Guardar reemplaza el archivo con su trabajo más reciente. Un mensaje en la parte inferior indica "Guardado" y el nombre del archivo.

Un asterisco (*) después del nombre en la barra de título indica que hay cambios sin guardar. Guarde a menudo. Si JeffDeck se cierra inesperadamente, consulte [Recuperar trabajo no guardado](recover).

Si un mensaje indica que algunos objetos "todavía no se pueden guardar en este tipo de archivo", el archivo se ha guardado, pero JeffDeck no ha podido escribir esos objetos. Conserve también el archivo original. Al guardar un archivo .odp, el mensaje enumera lo que ese tipo de archivo no puede contener. La sección "Guardar como archivo .odp", más abajo, explica qué es lo que no puede contener.

## Guardar con otro nombre o en otro tipo de archivo

Haga clic en **Archivo > Guardar como** o presione F12. Escriba un nombre, elija el tipo y haga clic en **Guardar**. Las opciones son:

- **Presentación (*.pptx)** es la opción habitual.
- **Plantilla (*.potx)** guarda las diapositivas como modelo a partir del cual crear presentaciones nuevas.
- **Presentación con diapositivas (*.ppsx)** guarda una presentación para reproducirla.
- **Presentación OpenDocument (*.odp)** guarda las diapositivas en el tipo de archivo que usan LibreOffice y otros programas libres.
- **Plantilla de presentación OpenDocument (*.otp)** es la versión de plantilla de ese tipo de archivo.

Si no escribe la extensión, JeffDeck agrega la que corresponde al tipo. Después de guardar, la barra de título muestra el nombre nuevo y usted sigue trabajando en el archivo nuevo.

Una presentación abierta a partir de una plantilla se guarda como presentación nueva: **Guardar** le pide un nombre, de modo que la plantilla no cambia. Lo mismo ocurre con una presentación abierta desde un archivo antiguo .ppt, .pps o .pot. Una presentación abierta desde un archivo .odp se vuelve a guardar como archivo .odp, sobre el mismo archivo.

## Abrir

Presione Ctrl+O para elegir un archivo directamente. También puede hacer clic en **Archivo > Abrir** y, después, en **Examinar** o en un nombre de **Anclado** o **Presentaciones recientes**.

El archivo se abre en esta ventana si contiene una presentación nueva que nadie ha modificado. De lo contrario, se abre en una ventana propia.

Haga clic con el botón derecho en un nombre de la lista para elegir **Anclar a la lista** (se queda en la parte superior), **Desanclar de la lista** o **Quitar de la lista**. Un nombre en gris indica que JeffDeck no encuentra ese archivo en este momento.

## Fuentes en el archivo

Una fuente es la forma de las letras. No todos los equipos tienen todas las fuentes. Si a un equipo le falta una fuente que usan sus diapositivas, dibuja las letras con otra fuente, y las diapositivas pueden verse distintas.

JeffDeck puede incluir dentro del archivo las fuentes que usan sus diapositivas. Así, las diapositivas se ven igual en cualquier equipo. Esto se llama insertar fuentes. JeffDeck lo hace cada vez que guarda. Para desactivarlo, haga clic en **Archivo > Opciones**, abra la pestaña **Guardar** y desactive **Insertar fuentes en el archivo**.

- JeffDeck solo inserta las fuentes que incluye el propio programa, como Fraunces, Lato y Open Sans, que otros equipos no tienen. Nunca inserta una fuente que tienen todos los equipos, como Calibri o Arial. Omite las fuentes cuya licencia no permite copiarlas en un archivo.
- JeffDeck inserta cada estilo que usan las diapositivas: normal, negrita, cursiva y negrita cursiva. Si las diapositivas solo usan el estilo normal, solo se inserta ese.
- JeffDeck inserta todas las letras de cada fuente, no solo las que ha usado. Así, quien abra el archivo podrá seguir modificando el texto.
- El archivo aumenta de tamaño. Cada estilo de una fuente agrega entre 40 y 700 KB aproximadamente (un KB son mil bytes). El archivo se comprime al guardarlo, por lo que crece más o menos la mitad de esa cantidad.

Al abrir un archivo que contiene fuentes, JeffDeck las usa mientras la presentación está abierta. Las diapositivas se ven como en el equipo donde se crearon. JeffDeck no instala las fuentes en su equipo ni las copia en ninguna parte. Solo pertenecen a esa presentación: no aparecen en sus listas de fuentes y ninguna otra presentación las usa. Desaparecen al cerrar la presentación. Si su equipo ya tiene una fuente, JeffDeck usa su propia copia.

JeffDeck lee las fuentes de archivos .pptx creados con otros programas de presentaciones, incluso si el programa las comprimió. También lee fuentes de archivos .odp.

Un archivo puede contener una fuente que JeffDeck no tiene. Al guardar, JeffDeck vuelve a incluir esa fuente en el archivo, siempre que las diapositivas la sigan usando. Lo hace tanto si **Insertar fuentes en el archivo** está activado como si no. Si una fuente de un archivo está dañada o no se puede leer, el mensaje que aparece al abrir el archivo lo indica.

## Tipos de archivo que lee JeffDeck

- **.pptx, .potx y .ppsx** son un mismo formato en tres variantes: presentación, plantilla y presentación con diapositivas. JeffDeck lee y escribe las tres.
- **.ppt, .pps y .pot** son el tipo antiguo. JeffDeck los lee pero no puede escribirlos, así que **Guardar** pide un nombre nuevo y guarda un archivo .pptx (o .ppsx si era un .pps). Nunca sobrescribe el original.
- **.odp y .otp** son presentaciones OpenDocument y sus plantillas. JeffDeck lee y escribe ambos. Un archivo .odp abierto aquí se guarda como .odp. Un archivo .otp es una plantilla, así que **Guardar** pide un nombre nuevo y sugiere un .odp.

Si no se puede leer una parte de un archivo, JeffDeck abre el resto y enumera lo que ha omitido. Si no se puede leer el archivo completo, indica el motivo.

## Guardar como archivo .odp

Un archivo .odp se abre en LibreOffice Impress con sus diapositivas, patrones, imágenes, notas y comentarios, y con el aspecto que muestra JeffDeck. También contiene las animaciones, las transiciones, las presentaciones personalizadas y la configuración de la presentación con diapositivas. La fuente de cada palabra se guarda por su nombre, de modo que un equipo que no tenga la fuente mostrará otra.

JeffDeck todavía no lee las animaciones, los comentarios, las presentaciones personalizadas ni la configuración de la presentación de un archivo .odp. Están en el archivo y LibreOffice los reproduce, pero al abrir el archivo .odp en JeffDeck, el programa los indica como omitidos. Conserve el archivo .pptx si quiere seguir modificándolos aquí.

Un archivo .odp no tiene lugar para algunos elementos. JeffDeck los enumera después de guardar, en el mensaje de la parte inferior de la ventana:

- **Los gráficos, los diagramas SmartArt y las ecuaciones** se guardan como imágenes. Puede verlos, pero no modificar sus datos ni sus partes en el archivo .odp.
- **Las secciones** no se guardan.
- **Las reflexiones, los biseles y las sombras interiores** se omiten.
- **Los efectos de animación** que se inician al hacer clic en un objeto y los efectos que cambian un color se omiten. Se conserva el clic en un vídeo para pausarlo o volver a reproducirlo.
- **Un vídeo o un sonido vinculado** desde otro archivo, en lugar de incluido en la presentación, conserva su dirección y su imagen en el archivo .odp. LibreOffice lo reproduce si encuentra el archivo. Los vídeos y sonidos incluidos en la presentación se conservan en el archivo .odp y se reproducen.

Conserve también el archivo .pptx si necesita recuperar estos elementos.

## Qué se conserva pero no se muestra

Algunas partes de un archivo son elementos que JeffDeck todavía no puede mostrar ni modificar. Los conserva con sus diapositivas y los vuelve a escribir al guardar, para que no se pierdan. Entre ellos están las animaciones, que se reproducen en la presentación; los vídeos y sonidos, que todavía no se reproducen, y los efectos de transición que JeffDeck no admite, que se reproducen como un desvanecimiento. Los gráficos y los diagramas SmartArt se muestran tal como se guardaron. Consulte [Bienvenido a JeffDeck](start).

## Temas relacionados

- [Recuperar trabajo no guardado](recover)
- [La pantalla Archivo](file-screen)
- [Reutilizar diapositivas de otra presentación](reuse)
- [Imprimir, PDF e imágenes de diapositivas](printing)
