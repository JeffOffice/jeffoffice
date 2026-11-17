# Abrir y guardar presentaciones
<!-- keywords: macros, macroinstrucciones, con macros, pptm, potm, ppsm, abrir, guardar, guardar como, otp, libreoffice, omitido, omitidos, cerrar, nueva, archivos recientes, anclado, fijado, tipos de archivo, pptx, potx, ppsx, ppt, odp, plantilla, presentación, mostrar, cambiar nombre, Ctrl+S, copia de seguridad, formatos de archivo, archivos antiguos, documento abierto, conservado, no mostrado, fuentes, insertar fuentes, fuentes insertadas, fuente que falta, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, reparar, abrir y reparar, dañado, dañada -->

JeffDeck guarda su trabajo en archivos .pptx y en archivos .odp, y abre varios tipos de archivos de presentación.

## Crear una presentación nueva

Pulse Ctrl+N. Se abre una ventana nueva con una presentación en blanco. También puede hacer clic en **Archivo > Nuevo** y, después, en **Presentación en blanco**. Si la ventana en la que está contiene una presentación nueva que nadie ha modificado, la presentación en blanco la reemplaza. Si no, se abre en una ventana propia.

## Guardar

Pulse Ctrl+S o haga clic en **Archivo > Guardar**. La primera vez, JeffDeck le pide un nombre y una ubicación. Después, Guardar reemplaza el archivo con su trabajo más reciente. En la parte inferior aparece el mensaje "Guardado" y el nombre del archivo.

Un asterisco (*) después del nombre en la barra de título indica que tiene cambios sin guardar. Guarde con frecuencia. Si JeffDeck se detiene inesperadamente, consulte [Recuperar trabajo sin guardar](recover).

Si un mensaje indica que algunos objetos «no se pueden guardar en este tipo de archivo todavía», el archivo se guarda, pero JeffDeck no tenía forma de escribir esos objetos. Conserve también el archivo original. Al guardar un archivo .odp, el mensaje indica lo que ese tipo de archivo no puede contener. La sección «Guardar como archivo .odp» más abajo explica qué no puede contener.

## Guardar con otro nombre o tipo

Haga clic en **Archivo > Guardar como** o pulse F12. Escriba un nombre, elija el tipo y haga clic en **Guardar**. Las opciones son:

- **Presentación (*.pptx)** es la opción habitual.
- **Plantilla (*.potx)** guarda las diapositivas como un modelo para crear presentaciones nuevas.
- **Presentación con diapositivas (*.ppsx)** guarda una presentación para reproducirla.
- **Presentación habilitada para macros (*.pptm)**, **Plantilla habilitada para macros (*.potm)** y **Presentación con diapositivas habilitada para macros (*.ppsm)** son los mismos tres tipos para una presentación que contiene **macroinstrucciones**. Una macroinstrucción es un pequeño programa que acompaña a un archivo. JeffDeck nunca ejecuta macroinstrucciones, y tampoco las lee ni las muestra. Si abre uno de estos archivos y lo guarda con el mismo tipo, las macroinstrucciones se conservan tal como estaban. Si lo guarda como .pptx, .potx o .ppsx, las macroinstrucciones se omiten y JeffDeck se lo indica. Una presentación sin macroinstrucciones también se puede guardar con estos tipos, pero no tiene macroinstrucciones que conservar. Si las macroinstrucciones de un archivo son demasiado grandes para conservarlas (más de 32 MB en una parte, o más de 64 MB en total), JeffDeck las omite y se lo indica al abrirlo.
- **Presentación OpenDocument (*.odp)** guarda las diapositivas en el tipo de archivo que usan LibreOffice y otros programas libres.
- **Plantilla de presentación OpenDocument (*.otp)** es la forma de plantilla de ese tipo.

Si no escribe la extensión, JeffDeck añade la que corresponde al tipo. Después de guardar, la barra de título muestra el nombre nuevo, y usted sigue trabajando en el archivo nuevo.

Una presentación abierta desde una plantilla se guarda como presentación nueva: **Guardar** pide un nombre, de modo que la plantilla no cambia. Lo mismo ocurre con una presentación abierta desde un archivo .ppt, .pps o .pot antiguo. Una presentación abierta desde un archivo .odp se vuelve a guardar como archivo .odp, sobre el mismo archivo.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffDeck escribe la presentación en ese archivo, y usted sigue trabajando en la que tenía. Su nombre y sus cambios sin guardar siguen igual.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones** y, en la pestaña **General**, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffDeck copia primero el archivo anterior a otro llamado «Copia de seguridad de» seguido de su nombre, en la misma carpeta. Si no se puede crear la copia, la aplicación pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el archivo se mantiene como estaba.

## Guardar como plantilla

Una plantilla es una presentación que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla (un archivo **.potx**) se guarda en **Mis plantillas**, y su presentación no cambia. Para crear una presentación nueva a partir de ella, haga clic en **Archivo > Nuevo** y, en **Mis plantillas**, haga clic en la plantilla. La presentación nueva todavía no tiene archivo propio, así que la plantilla no cambia. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Abrir

Pulse Ctrl+O para elegir un archivo de inmediato. También puede hacer clic en **Archivo > Abrir** y, después, en **Examinar**, o en un nombre de **Anclados** o de **Presentaciones recientes**.

El archivo se abre en esta ventana si contiene una presentación nueva que nadie ha modificado. Si no, se abre en una ventana propia.

Haga clic con el botón derecho en un nombre de la lista para **Anclar a la lista** (se mantiene arriba), **Desanclar de la lista** o **Quitar de la lista**. Un nombre en gris indica que JeffDeck no encuentra ese archivo en este momento.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de recientes.

- **Abrir como solo lectura** abre la presentación solo para leerla. Puede mirar, desplazarse por las diapositivas y copiar, pero no puede cambiar nada. La barra de título indica «Solo lectura». Si pulsa **Guardar**, JeffDeck pide un nombre nuevo, así que el archivo que abrió no cambia.
- **Abrir como copia** abre las diapositivas como una presentación nueva sin archivo propio. **Guardar** pregunta dónde colocarla.
- **Abrir en una nueva ventana** abre el archivo en una ventana propia, incluso cuando la ventana en la que está está vacía.
- **Abrir y reparar** sirve para un archivo .pptx dañado, por ejemplo uno que se cortó al copiarlo. JeffDeck lee cada parte que puede, conserva una parte que se interrumpió hasta el punto donde se cortó y omite una parte que no puede leer. Las diapositivas reparadas se abren como una copia llamada «Reparada» seguido del nombre, y un mensaje indica qué se omitió. El archivo dañado no cambia.

## Archivos con contraseña

Un archivo **.pptx** u **.odp** que pide una contraseña se abre como de costumbre: JeffDeck la solicita, puede volver a intentarlo si no es correcta, o hacer clic en **Cancelar**. El archivo sigue protegido al guardarlo. Para cambiar o quitar la contraseña, use **Archivo > Información > Cifrar con contraseña** (consulte La pantalla Archivo). Una copia guardada como archivo .odp también conserva la contraseña.

## Fuentes en el archivo

Una fuente es la forma de las letras. No todos los equipos tienen todas las fuentes. Si a un equipo le falta una fuente que usan sus diapositivas, dibuja las letras con otra fuente. Entonces las diapositivas pueden verse diferentes.

JeffDeck puede incluir dentro del archivo las fuentes que usan sus diapositivas. Así, las diapositivas se ven igual en cualquier equipo. Esto se llama insertar. JeffDeck lo hace cada vez que guarda. Para desactivarlo, haga clic en **Archivo > Opciones**, abra la pestaña **Guardar** y desactive **Insertar fuentes en el archivo**.

- JeffDeck inserta solo las fuentes que vienen con él, como Fraunces, Lato y Open Sans. Otros equipos no las tienen. Nunca inserta una fuente que todos los equipos tienen, como Calibri o Arial. Omite una fuente cuya licencia indica que no se puede copiar en un archivo.
- JeffDeck inserta cada estilo que usan las diapositivas: normal, negrita, cursiva y negrita cursiva. Si las diapositivas usan solo el estilo normal, solo ese se inserta.
- JeffDeck inserta todas las letras de cada fuente, no solo las que usó. Así, quien abra el archivo puede seguir cambiando las palabras.
- El archivo se hace más grande. Cada estilo de una fuente añade entre 40 y 700 KB (un KB son mil bytes). El archivo se comprime al guardarlo, así que crece alrededor de la mitad de esa cantidad.

Al abrir un archivo que tiene fuentes insertadas, JeffDeck las usa mientras la presentación está abierta. Las diapositivas se ven como se vieron al crearlas. JeffDeck no instala las fuentes en su equipo ni las copia a ningún lugar. Pertenecen solo a esa presentación: no aparecen en sus listas de fuentes y ninguna otra presentación las usa. Desaparecen al cerrar la presentación. Si su equipo ya tiene una fuente, JeffDeck usa su propia copia.

JeffDeck lee las fuentes de archivos .pptx creados por otros programas de presentaciones, incluso cuando el programa las comprimió. También lee las fuentes de archivos .odp.

Un archivo puede contener una fuente que JeffDeck no tiene. Al guardar, JeffDeck vuelve a incluir esa fuente en el archivo, siempre que las diapositivas sigan usándola. Lo hace tanto si **Insertar fuentes en el archivo** está activada como si no. Si una fuente del archivo está dañada o no se puede leer, el mensaje que ve al abrir el archivo lo indica.

## Tipos de archivo que JeffDeck lee

- **.pptx, .potx y .ppsx** son un mismo formato en tres versiones: presentación, plantilla y presentación con diapositivas. JeffDeck lee y escribe las tres.
- **.ppt, .pps y .pot** son el tipo anterior. JeffDeck los lee, pero no puede escribirlos, así que **Guardar** pide un nombre nuevo y guarda un .pptx (un .ppsx en el caso de .pps). Nunca sobrescribe el original.
- **.odp y .otp** son presentaciones de documento abierto y sus plantillas. JeffDeck lee y escribe ambos. Un .odp abierto aquí se guarda como .odp. Un .otp es una plantilla, así que **Guardar** pide un nombre nuevo y sugiere un .odp.

Si parte de un archivo no se puede leer, JeffDeck abre el resto y enumera lo que omitió. Si el archivo entero no se puede leer, indica el motivo.

## Guardar como archivo .odp

Un archivo .odp se abre en LibreOffice Impress con sus diapositivas, patrones, imágenes, notas y comentarios, y con el aspecto que muestra JeffDeck. Contiene animaciones, transiciones entre diapositivas, presentaciones personalizadas y la configuración de la presentación con diapositivas. La fuente de cada palabra se guarda por su nombre, así que un equipo que no tenga la fuente muestra otra.

JeffDeck todavía no lee de vuelta desde un archivo .odp las animaciones, los comentarios, las presentaciones personalizadas ni la configuración de la presentación. Están en el archivo, y LibreOffice los reproduce, pero cuando abre el archivo .odp en JeffDeck, este los enumera como omitidos. Conserve su archivo .pptx si quiere seguir editándolos aquí.

Un archivo .odp no tiene lugar para algunas cosas. JeffDeck las enumera después de guardar, en el mensaje de la parte inferior de la ventana:

- **Gráficos, diagramas y ecuaciones** se guardan como imágenes. Puede verlos, pero no puede editar sus números ni sus partes en el archivo .odp.
- **Las secciones** no se guardan.
- **Los biselados y las sombras interiores** se omiten, y lo mismo ocurre con los resplandores y los reflejos en el texto. La sombra en el texto se conserva. El reflejo en una forma, una línea con degradado, una línea de dos o tres trazos y el tamaño de una sombra se conservan de una forma que solo JeffDeck lee. Otro programa muestra la forma sin el reflejo, la línea en su primer color, un solo trazo y la sombra con el tamaño del objeto.
- **Los efectos de animación** que se inician al hacer clic en un objeto, y los efectos que cambian un color, se omiten. Un clic en un vídeo, para pausarlo o reproducirlo de nuevo, se conserva.
- **Un vídeo o sonido vinculado** desde otro archivo, que no se guarda en la presentación, conserva su dirección y su imagen en el archivo .odp. LibreOffice lo reproduce si encuentra el archivo. Los vídeos y sonidos que están en la presentación se conservan en el archivo .odp y se reproducen.

Conserve también su archivo .pptx, si necesita recuperar estas cosas.

## Qué se conserva pero no se muestra

Algunas partes de un archivo son cosas que JeffDeck todavía no puede mostrar ni cambiar. Las conserva junto con sus diapositivas y las vuelve a escribir al guardar, de modo que no se pierden. Entre ellas están las animaciones, que se reproducen en la presentación; los vídeos y sonidos, que todavía no se reproducen; y un efecto de transición para el que JeffDeck no tiene forma, que se reproduce como un desvanecimiento. Los gráficos y diagramas se muestran tal como se guardaron. Consulte [Bienvenido a JeffDeck](start).

## Temas relacionados

- [Recuperar trabajo sin guardar](recover)
- [La pantalla Archivo](file-screen)
- [Reutilizar diapositivas de otra presentación](reuse)
- [Imprimir, PDF e imágenes de diapositivas](printing)
