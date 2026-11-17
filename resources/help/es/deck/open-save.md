# Abrir y guardar presentaciones
<!-- keywords: macros, con macros, pptm, potm, ppsm, abrir, guardar, guardar como, otp, libreoffice, omitir, cerrar, nuevo, archivos recientes, anclado, tipos de archivo, pptx, potx, ppsx, ppt, odp, plantilla, presentación con diapositivas, cambiar nombre, Ctrl+S, copia de seguridad, formatos de archivo, archivos antiguos, documento abierto, conservado, no se muestra, fuentes, insertar fuentes, fuentes insertadas, fuente que falta, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, reparar, abrir y reparar, dañado, incrustar fuentes -->

JeffDeck guarda tu trabajo en archivos .pptx y en archivos .odp, y abre varios tipos de archivos de presentación.

## Crear una presentación nueva

Pulse Ctrl+N. Se abre una ventana nueva con una presentación en blanco. También puede hacer clic en **Archivo > Nuevo** y luego en **Presentación en blanco**. Si la ventana en la que está contiene una presentación nueva que nadie ha tocado, la presentación en blanco la reemplaza. Si no, se abre en una ventana propia.

## Guardar

Pulse Ctrl+S, o haga clic en **Archivo > Guardar**. La primera vez, JeffDeck pide un nombre y una ubicación. Después, Guardar reemplaza el archivo con su trabajo más reciente. Un mensaje en la parte inferior dice "Guardado" y el nombre del archivo.

Un asterisco (*) después del nombre en la barra de título indica que tiene cambios sin guardar. Guarde con frecuencia. Si JeffDeck se detiene inesperadamente, consulte [Recuperar trabajo sin guardar](recover).

Si un mensaje dice que algunos objetos "no se pueden guardar en este tipo de archivo todavía", el archivo se guarda, pero JeffDeck no tenía forma de escribir esos objetos. Conserve también el archivo original. Al guardar un archivo .odp, el mensaje indica lo que ese tipo de archivo no puede contener. La sección «Guardar como archivo .odp» más abajo explica qué no puede contener.

## Guardar con otro nombre o tipo

Haga clic en **Archivo > Guardar como**, o pulse F12. Escriba un nombre, elija el tipo y haga clic en **Guardar**. Las opciones son:

- **Presentación (*.pptx)** es la opción habitual.
- **Plantilla (*.potx)** guarda las diapositivas como modelo para crear presentaciones nuevas.
- **Presentación con diapositivas (*.ppsx)** guarda una presentación para reproducirla.
- **Presentación con macros (*.pptm)**, **Plantilla con macros (*.potm)** y **Presentación con diapositivas con macros (*.ppsm)** son los mismos tres tipos para una presentación que contiene **macros**. Una macro es un pequeño programa que acompaña a un archivo. JeffDeck nunca ejecuta macros, y no las lee ni las muestra. Si abre uno de estos archivos y lo guarda con el mismo tipo, las macros se conservan tal como estaban. Si lo guarda como .pptx, .potx o .ppsx, las macros se omiten, y JeffDeck se lo indica. Una presentación sin macros también se puede guardar con estos tipos, pero no tiene macros que conservar. Si las macros de un archivo son demasiado grandes para conservarlas (más de 32 MB en una parte, o 64 MB en total), JeffDeck las omite y se lo indica al abrir el archivo.
- **Presentación OpenDocument (*.odp)** guarda las diapositivas en el tipo de archivo que usan LibreOffice y otros programas libres.
- **Plantilla de presentación OpenDocument (*.otp)** es la forma de plantilla de ese tipo.

Si no escribe la extensión, JeffDeck añade la que corresponde al tipo. Después de guardar, la barra de título muestra el nombre nuevo, y sigue trabajando en el archivo nuevo.

Una presentación que abrió desde una plantilla se guarda como presentación nueva: **Guardar** pide un nombre, para que la plantilla no cambie. Lo mismo ocurre con una abierta desde un archivo .ppt, .pps o .pot antiguo. Una presentación que abrió desde un archivo .odp se guarda de nuevo como .odp, sobre el mismo archivo.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffDeck escribe la presentación en ese archivo, y usted sigue trabajando en la que tenía. Su nombre y sus cambios sin guardar siguen como estaban.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones**, y en la ficha **General** active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffDeck copia primero el archivo anterior en otro llamado "Copia de seguridad de" más su nombre, en la misma carpeta. Si no se puede crear la copia, la aplicación pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el archivo se queda como estaba.

## Guardar como plantilla

Una plantilla es una presentación que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla (un archivo **.potx**) se guarda en **Mis plantillas**, y su presentación no cambia. Para crear una presentación nueva a partir de ella, haga clic en **Archivo > Nuevo** y en la plantilla que aparece en **Mis plantillas**. La presentación nueva no tiene aún archivo propio, así que la plantilla no cambia. Para quitar una plantilla de la lista, haga clic con el botón derecho sobre ella y elija **Quitar de Mis plantillas**.

## Abrir

Pulse Ctrl+O para elegir un archivo de inmediato. También puede hacer clic en **Archivo > Abrir**, y luego en **Examinar**, o en un nombre bajo **Anclado** o **Presentaciones recientes**.

El archivo se abre en esta ventana si contiene una presentación nueva que nadie ha tocado. Si no, se abre en una ventana propia.

Haga clic con el botón derecho sobre un nombre de la lista para **Anclar a la lista** (queda arriba), **Desanclar de la lista** o **Quitar de la lista**. Un nombre en gris indica que JeffDeck no encuentra ese archivo en este momento.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho sobre un nombre de la lista de recientes.

- **Abrir como solo lectura** abre la presentación solo para leerla. Puede ver, desplazarse por las diapositivas y copiar, pero no puede cambiar nada. La barra de título dice "Solo lectura". Si pulsa **Guardar**, JeffDeck pide un nombre nuevo, para que el archivo que abrió no cambie.
- **Abrir como copia** abre las diapositivas como una presentación nueva sin archivo propio. **Guardar** pide dónde ubicarla.
- **Abrir en una nueva ventana** abre el archivo en una ventana propia, aunque la ventana en la que está esté vacía.
- **Abrir y reparar** sirve para un archivo .pptx o .ppt dañado, por ejemplo uno que se cortó al copiarlo. JeffDeck lee cada parte que puede, conserva una parte que se interrumpió hasta el punto donde se cortó, y omite una parte que no puede leer. En un archivo .ppt antiguo, busca las diapositivas según lo que contienen cuando las listas que indican dónde están guardadas están dañadas o perdidas. Si se pierde la lista que indica el orden de las diapositivas, estas se incorporan en el orden en que se guardaron, que puede no ser el orden original. Un archivo que tiene contraseña no se puede reparar. Las diapositivas reparadas se abren como una copia llamada "Reparada" más el nombre, y un mensaje le indica qué se omitió. El archivo dañado no se modifica.

## Archivos con contraseña

Un archivo **.pptx** u **.odp** que pide contraseña se abre como de costumbre: JeffDeck pide la contraseña, y puede volver a intentarlo si no es correcta, o hacer clic en **Cancelar**. El archivo sigue protegido al guardarlo. Para cambiar o quitar la contraseña, use **Archivo > Información > Cifrar con contraseña** (véase La pantalla Archivo). Una copia guardada como archivo .odp también conserva la contraseña.

## Fuentes en el archivo

Una fuente es la forma de las letras. No todos los equipos tienen todas las fuentes. Si a un equipo le falta una fuente que usan sus diapositivas, dibuja las letras con otra fuente. Entonces las diapositivas pueden verse distintas.

JeffDeck puede incluir dentro del archivo las fuentes que usan sus diapositivas. Así, las diapositivas se ven igual en cualquier equipo. Esto se llama incrustar. JeffDeck lo hace cada vez que guarda. Para desactivarlo, haga clic en **Archivo > Opciones**, abra la ficha **Guardar** y desmarque **Insertar fuentes en el archivo**.

- JeffDeck incluye solo las fuentes que vienen con él, como Fraunces, Lato y Open Sans. Otros equipos no las tienen. Nunca incluye una fuente que todos los equipos tienen, como Calibri o Arial. Omite una fuente cuya licencia indica que no puede copiarse en un archivo.
- JeffDeck incluye cada estilo que usan las diapositivas: normal, negrita, cursiva y negrita cursiva. Si las diapositivas usan solo el estilo normal, solo ese se incluye.
- JeffDeck incluye todas las letras de cada fuente, no solo las que usó. Así, quien abra el archivo puede seguir cambiando las palabras.
- El archivo aumenta de tamaño. Cada estilo de una fuente añade entre 40 y 700 KB (un KB es mil bytes). El archivo se comprime al guardarse, así que crece alrededor de la mitad de esa cantidad.

Al abrir un archivo que tiene fuentes incluidas, JeffDeck las usa mientras la presentación está abierta. Las diapositivas se ven como se vieron al crearlas. JeffDeck no instala las fuentes en su equipo, y no las copia en ningún lugar. Pertenecen solo a esa presentación: no aparecen en sus listas de fuentes, y ninguna otra presentación las usa. Desaparecen al cerrar la presentación. Si su equipo ya tiene una fuente, JeffDeck usa su propia copia.

JeffDeck lee las fuentes de archivos .pptx creados por otros programas de presentaciones, incluso cuando el programa las comprimió. También lee las fuentes de archivos .odp.

Un archivo puede contener una fuente que JeffDeck no tiene. Al guardar, JeffDeck vuelve a incluir esa fuente en el archivo, siempre que las diapositivas sigan usándola. Lo hace tanto si **Insertar fuentes en el archivo** está activado como si no. Si una fuente del archivo está dañada o no se puede leer, el mensaje que ve al abrir el archivo lo indica.

## Tipos de archivo que JeffDeck lee

- **.pptx, .potx y .ppsx** son un mismo formato en tres versiones: presentación, plantilla y presentación con diapositivas. JeffDeck lee y escribe las tres.
- **.ppt, .pps y .pot** son el tipo antiguo. JeffDeck los lee, pero no puede escribirlos, así que **Guardar** pide un nombre nuevo y guarda un .pptx (un .ppsx en el caso de .pps). Nunca sobrescribe el original.
- **.odp y .otp** son presentaciones de documento abierto y sus plantillas. JeffDeck lee y escribe ambos. Un .odp abierto aquí se guarda como .odp. Un .otp es una plantilla, así que **Guardar** pide un nombre nuevo y sugiere un .odp.

Si parte de un archivo no se puede leer, JeffDeck abre el resto y enumera lo que omitió. Si el archivo entero no se puede leer, indica el motivo.

## Guardar como archivo .odp

Un archivo .odp se abre en LibreOffice Impress con sus diapositivas, patrones, imágenes, notas y comentarios, y con el aspecto que muestra JeffDeck. Conserva las animaciones, las transiciones entre diapositivas, las presentaciones personalizadas y la configuración de la presentación con diapositivas. La fuente de cada palabra se guarda por su nombre, así que un equipo que no tenga la fuente muestra otra.

JeffDeck todavía no lee de vuelta desde un archivo .odp las animaciones, los comentarios, las presentaciones personalizadas ni la configuración de la presentación. Están en el archivo, y LibreOffice los reproduce, pero cuando abre el archivo .odp en JeffDeck, los enumera como omitidos. Conserve su archivo .pptx si quiere seguir editándolos aquí.

Un archivo .odp no tiene lugar para algunas cosas. JeffDeck las enumera después de guardar, en el mensaje de la parte inferior de la ventana:

- **Gráficos, diagramas y ecuaciones** se guardan como imágenes. Puede verlos, pero no puede editar sus números ni sus partes en el archivo .odp.
- **Las secciones** no se guardan.
- **Los biseles y las sombras interiores** se omiten, y también los iluminados y reflejos en las palabras. Una sombra en las palabras se conserva. Un reflejo en una forma, una línea con degradado, una línea de dos o tres trazos y el tamaño de una sombra se conservan de una forma que solo JeffDeck lee. Otro programa muestra la forma sin el reflejo, la línea con su primer color, un solo trazo y la sombra con el tamaño del objeto.
- **Los efectos de animación** que se inician al hacer clic en un objeto, y los efectos que cambian un color, se omiten. Un clic en un vídeo, para pausarlo o reproducirlo otra vez, se conserva.
- **Un vídeo o sonido vinculado** desde otro archivo, no guardado en la presentación, conserva su dirección y su imagen en el archivo .odp. LibreOffice lo reproduce si encuentra el archivo. Los vídeos y sonidos que están en la presentación se conservan en el archivo .odp y se reproducen.

Conserve también su archivo .pptx si necesita recuperar estas cosas.

## Lo que se conserva pero no se muestra

Algunas partes de un archivo son cosas que JeffDeck todavía no puede mostrar ni cambiar. Las conserva junto a sus diapositivas y las vuelve a escribir al guardar, para que no se pierdan. Entre ellas están las animaciones, que se reproducen en la presentación; los vídeos y sonidos, que todavía no se reproducen; y un efecto de transición para el que JeffDeck no tiene forma, que se reproduce como un desvanecido. Los gráficos y diagramas se muestran tal como se guardaron. Consulte [Bienvenido a JeffDeck](start).

## Temas relacionados

- [Recuperar trabajo sin guardar](recover)
- [La pantalla Archivo](file-screen)
- [Reutilizar diapositivas de otra presentación](reuse)
- [Imprimir, PDF e imágenes de diapositivas](printing)
