# Abrir y guardar presentaciones
<!-- keywords: macros, con macros, pptm, potm, ppsm, abrir, guardar, guardar como, otp, LibreOffice, omitir, cerrar, nuevo, archivos recientes, anclados, tipos de archivo, pptx, potx, ppsx, ppt, odp, plantilla, presentación con diapositivas, cambiar nombre, Ctrl+G, copia de seguridad, formatos de archivo, archivos antiguos, documento abierto, conservado, no se muestra, fuentes, insertar fuentes, fuentes incrustadas, fuente que falta, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, reparar, abrir y reparar, dañado -->

JeffDeck guarda su trabajo en archivos .pptx y en archivos .odp, y abre varios tipos de archivos de presentación.

## Crear una presentación nueva

Presione Ctrl+N. Se abre una ventana nueva con una presentación en blanco. También puede hacer clic en **Archivo > Nuevo** y luego en **Presentación en blanco**. Si la ventana en la que está contiene una presentación nueva que nadie ha tocado, la presentación en blanco la reemplaza. Si no, se abre en una ventana propia.

## Guardar

Presione Ctrl+S, o haga clic en **Archivo > Guardar**. La primera vez, JeffDeck pide un nombre y una ubicación. Después, Guardar reemplaza el archivo con su trabajo más reciente. En la parte inferior aparece un mensaje que dice «Guardado» y el nombre del archivo.

Un asterisco (*) después del nombre en la barra de título indica que hay cambios sin guardar. Guarde con frecuencia. Si JeffDeck se detiene inesperadamente, consulte [Recuperar trabajo sin guardar](recover).

Si un mensaje indica que algunos objetos «no se pueden guardar en este tipo de archivo todavía», el archivo se guarda, pero JeffDeck no tenía forma de escribir esos objetos. Conserve también el archivo original. Al guardar un archivo .odp, el mensaje indica lo que ese tipo de archivo no puede contener. La sección «Guardar como archivo .odp» más abajo explica lo que no puede contener.

## Guardar con otro nombre o tipo

Haga clic en **Archivo > Guardar como**, o presione F12. Escriba un nombre, elija el tipo y haga clic en **Guardar**. Las opciones son:

- **Presentación (*.pptx)** es la opción habitual.
- **Plantilla (*.potx)** guarda las diapositivas como un modelo para crear nuevas presentaciones.
- **Presentación con diapositivas (*.ppsx)** guarda una presentación para reproducirla.
- **Presentación habilitada para macros (*.pptm)**, **Plantilla habilitada para macros (*.potm)** y **Presentación con diapositivas habilitada para macros (*.ppsm)** son los mismos tres tipos para una presentación que contiene **macros**. Una macro es un pequeño programa que acompaña a un archivo. JeffDeck nunca ejecuta macros, y tampoco las lee ni las muestra. Si abre uno de estos archivos y lo guarda con el mismo tipo, las macros se conservan tal como estaban. Si lo guarda como .pptx, .potx o .ppsx, las macros se omiten y JeffDeck se lo indica. Una presentación sin macros también puede guardarse con estos tipos, pero no tiene macros que conservar. Si las macros de un archivo son demasiado grandes para conservarse (más de 32 MB en una parte, o 64 MB en total), JeffDeck las omite y se lo indica al abrirlo.
- **Presentación OpenDocument (*.odp)** guarda las diapositivas en el formato que usan LibreOffice y otros programas libres.
- **Plantilla de presentación OpenDocument (*.otp)** es la forma de plantilla de ese tipo.

Si no escribe la extensión, JeffDeck añade la que corresponde al tipo elegido. Después de guardar, la barra de título muestra el nuevo nombre y usted sigue trabajando en el archivo nuevo.

Una presentación abierta desde una plantilla se guarda como presentación nueva: **Guardar** pide un nombre, para que la plantilla no cambie. Lo mismo ocurre con una presentación abierta desde un archivo .ppt, .pps o .pot antiguo. Una presentación abierta desde un archivo .odp se vuelve a guardar como archivo .odp, sobre el mismo archivo.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffDeck escribe la presentación en ese archivo, y usted sigue trabajando en la que tenía. Su nombre y sus cambios sin guardar siguen como estaban.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones** y, en la pestaña **General**, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffDeck copia primero el archivo anterior en otro llamado «Backup of» más su nombre, en la misma carpeta. Si no se puede crear la copia, la aplicación pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el archivo queda como estaba.

## Guardar como plantilla

Una plantilla es una presentación que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla (un archivo **.potx**) se guarda en **Mis plantillas**, y su presentación no cambia. Para crear una presentación nueva a partir de ella, haga clic en **Archivo > Nuevo** y, en **Mis plantillas**, haga clic en la plantilla. La presentación nueva todavía no tiene archivo propio, así que la plantilla no cambia. Para quitar una plantilla de la lista, haga clic con el botón derecho sobre ella y elija **Quitar de Mis plantillas**.

## Abrir

Presione Ctrl+O para elegir un archivo de inmediato. También puede hacer clic en **Archivo > Abrir** y, después, en **Examinar**, o en un nombre de **Anclados** o de **Presentaciones recientes**.

El archivo se abre en esta ventana si contiene una presentación nueva que nadie ha tocado. Si no, se abre en una ventana propia.

Haga clic con el botón derecho en un nombre de la lista para **Anclar a la lista** (queda arriba), **Desanclar de la lista** o **Quitar de la lista**. Un nombre en gris indica que JeffDeck no encuentra ese archivo en este momento.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de recientes.

- **Abrir como solo lectura** abre la presentación solo para leerla. Puede ver las diapositivas, desplazarse por ellas y copiar, pero no cambiar nada. La barra de título dice «Solo lectura». Si presiona **Guardar**, JeffDeck pide un nombre nuevo, para que el archivo abierto no cambie.
- **Abrir como copia** abre las diapositivas como una presentación nueva sin archivo propio. **Guardar** pregunta dónde colocarla.
- **Abrir en nueva ventana** abre el archivo en una ventana propia, incluso cuando la ventana en la que está está vacía.
- **Abrir y reparar** sirve para un archivo .pptx o .ppt dañado, por ejemplo uno que se cortó al copiarlo. JeffDeck recupera todo lo que puede: lee cada parte que puede, conserva una parte que se interrumpió hasta el punto donde se cortó y omite una parte que no puede leer. En un archivo .ppt antiguo, encuentra las diapositivas según lo que contienen cuando las listas que indican dónde se guardan están dañadas o perdidas. Cuando las listas que indican cómo encajan las piezas del archivo están dañadas, el programa toma cada pieza siguiendo a la anterior y muestra lo que encuentra allí. Esa parte podría pertenecer a una copia anterior de la presentación o a otra parte del archivo, así que el mensaje indica la diapositiva donde empieza y le pide que la compare con una copia de seguridad, si tiene una. Si la suposición llega antes de la primera diapositiva, el mensaje indica que toda la presentación se reconstruyó a partir de piezas. Si no se pudieron recuperar por completo las imágenes, el mensaje lo indica. Si se pierde la lista que da el orden de las diapositivas, estas se muestran en el orden en que se guardaron, que puede no ser el orden original. Un archivo con contraseña no se puede reparar. Las diapositivas reparadas se abren como una copia llamada «Reparada» más el nombre, y un mensaje le indica lo que se hizo. El archivo dañado no cambia.

## Archivos con contraseña

Un archivo **.pptx** o **.odp** que pide una contraseña se abre como de costumbre: JeffDeck pide la contraseña, y puede intentarlo de nuevo si no es correcta, o hacer clic en **Cancelar**. El archivo sigue protegido al guardarlo. Para cambiar o quitar la contraseña, use **Archivo > Información > Cifrar con contraseña** (consulte La pantalla Archivo). Una copia guardada como archivo .odp también conserva la contraseña.

## Fuentes en el archivo

Una fuente es la forma de las letras. No todos los equipos tienen todas las fuentes. Si a un equipo le falta una fuente que usan sus diapositivas, dibuja las letras con otra fuente. Entonces las diapositivas pueden verse diferentes.

JeffDeck puede incluir dentro del archivo las fuentes que usan sus diapositivas. Así, las diapositivas se ven igual en cualquier equipo. Esto se llama insertar fuentes. JeffDeck lo hace cada vez que guarda. Para desactivarlo, haga clic en **Archivo > Opciones**, abra la pestaña **Guardar** y desactive **Insertar fuentes en el archivo**.

- JeffDeck incluye solo las fuentes que vienen con él, como Fraunces, Lato y Open Sans. Otros equipos no las tienen. Nunca incluye una fuente que tienen todos los equipos, como Calibri o Arial. Omite una fuente cuya licencia indica que no se puede copiar en un archivo.
- JeffDeck incluye cada estilo que usan las diapositivas: normal, negrita, cursiva y negrita cursiva. Si las diapositivas usan solo el estilo normal, solo ese se incluye.
- JeffDeck incluye todas las letras de cada fuente, no solo las que usó. Así, quien abra el archivo puede seguir cambiando las palabras.
- El archivo crece. Cada estilo de una fuente agrega entre 40 y 700 KB (un KB es mil bytes). El archivo se comprime al guardarlo, así que crece aproximadamente la mitad de esa cantidad.

Al abrir un archivo que contiene fuentes, JeffDeck las usa mientras la presentación está abierta. Las diapositivas se ven como se vieron donde se crearon. JeffDeck no instala las fuentes en su equipo ni las copia en ningún otro lugar. Pertenecen solo a esa presentación: no aparecen en sus listas de fuentes y ninguna otra presentación las usa. Desaparecen al cerrar la presentación. Si su equipo ya tiene una fuente, JeffDeck usa su propia copia.

JeffDeck lee las fuentes de los archivos .pptx creados por otros programas de presentaciones, aunque el programa las haya comprimido. También lee las fuentes de los archivos .odp.

Un archivo puede contener una fuente que JeffDeck no tiene. Al guardar, JeffDeck vuelve a incluir esa fuente en el archivo, siempre que las diapositivas sigan usándola. Lo hace tanto si **Insertar fuentes en el archivo** está activado como si no. Si una fuente del archivo está dañada o no se puede leer, el mensaje que aparece al abrir el archivo lo indica.

## Tipos de archivo que JeffDeck lee

- **.pptx, .potx y .ppsx** son un mismo formato en tres versiones: presentación, plantilla y presentación con diapositivas. JeffDeck lee y escribe las tres.
- **.ppt, .pps y .pot** son el formato antiguo. JeffDeck los lee, pero no puede escribirlos, así que **Guardar** pide un nombre nuevo y guarda un .pptx (un .ppsx en el caso de .pps). Nunca sobrescribe el original.
- **.odp y .otp** son presentaciones de documento abierto y sus plantillas. JeffDeck lee y escribe ambos. Un .odp abierto aquí se guarda como .odp. Un .otp es una plantilla, así que **Guardar** pide un nombre nuevo y sugiere un .odp.

Si una parte de un archivo no se puede leer, JeffDeck abre el resto y enumera lo que omitió. Si el archivo entero no se puede leer, indica el motivo.

## Guardar como archivo .odp

Un archivo .odp se abre en LibreOffice Impress con sus diapositivas, patrones, imágenes, notas y comentarios, y con el aspecto que muestra JeffDeck. Conserva animaciones, transiciones de diapositiva, presentaciones personalizadas y la configuración de la presentación con diapositivas. La fuente de cada palabra se guarda por nombre, así que un equipo que no tenga la fuente muestra otra.

JeffDeck todavía no lee de vuelta desde un archivo .odp las animaciones, los comentarios, las presentaciones personalizadas ni la configuración de la presentación. Están en el archivo y LibreOffice los reproduce, pero al abrir el archivo .odp en JeffDeck, los enumera como omitidos. Conserve su archivo .pptx si quiere seguir editándolos aquí.

Un archivo .odp no tiene lugar para algunas cosas. JeffDeck las enumera después de guardar, en el mensaje de la parte inferior de la ventana:

- **Los gráficos, diagramas y ecuaciones** se guardan como imágenes. Puede verlos, pero no puede editar sus números ni sus partes en el archivo .odp.
- **Las secciones** no se guardan.
- **Los biseles y las sombras interiores** se omiten, y también los resplandores y reflejos en el texto. Se conserva una sombra en el texto. Un reflejo en una forma, una línea con degradado, una línea de dos o tres trazos y el tamaño de una sombra se conservan de una forma que solo JeffDeck lee. Otro programa muestra la forma sin el reflejo, la línea en su primer color, un solo trazo y la sombra con el tamaño del objeto.
- **Los efectos de animación** que se inician al hacer clic en un objeto, y los efectos que cambian un color, se omiten. Un clic en un vídeo, para pausarlo o reproducirlo de nuevo, sí se conserva.
- **Un vídeo o sonido vinculado** desde otro archivo, y no incluido en la presentación, conserva su dirección y su imagen en el archivo .odp. LibreOffice lo reproduce si encuentra el archivo. Los vídeos y sonidos incluidos en la presentación se conservan en el archivo .odp y se reproducen.

Conserve también su archivo .pptx si necesita recuperar estas cosas.

## Lo que se conserva pero no se muestra

Algunas partes de un archivo son cosas que JeffDeck todavía no puede mostrar ni cambiar. Las conserva junto con sus diapositivas y las vuelve a escribir al guardar, para que no se pierdan. Entre ellas están las animaciones, que se reproducen en la presentación; los vídeos y sonidos, que todavía no se reproducen; y un efecto de transición para el que JeffDeck no tiene forma, que se reproduce como un desvanecimiento. Los gráficos y diagramas se muestran como se guardaron. Consulte [Bienvenida a JeffDeck](start).

## Temas relacionados

- [Recuperar trabajo sin guardar](recover)
- [La pantalla Archivo](file-screen)
- [Reutilizar diapositivas de otra presentación](reuse)
- [Imprimir, PDF e imágenes de diapositivas](printing)
