# Abrir y guardar presentaciones
<!-- keywords: macros, con macros, pptm, potm, ppsm, abrir, guardar, guardar como, otp, libreoffice, omitidos, cerrar, nuevo, archivos recientes, anclado, tipos de archivo, pptx, potx, ppsx, ppt, odp, plantilla, presentación, renombrar, Ctrl+S, copia de seguridad, formatos de archivo, archivos antiguos, documento de OpenDocument, conservado, no mostrado, fuentes, insertar fuentes, fuentes insertadas, fuente que falta, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, reparar, abrir y reparar, dañado, ficheros recientes, tipos de fichero, formatos de fichero, ficheros antiguos -->

JeffDeck guarda su trabajo en ficheros .pptx y en ficheros .odp, y abre varios tipos de ficheros de presentación.

## Crear una presentación nueva

Pulse Ctrl+N. Se abre una ventana nueva con una presentación en blanco. También puede hacer clic en **Archivo > Nuevo** y luego en **Presentación en blanco**. Si la ventana en la que está contiene una presentación nueva que nadie ha modificado, la presentación en blanco la reemplaza. Si no, se abre en una ventana propia.

## Guardar

Pulse Ctrl+S, o haga clic en **Archivo > Guardar**. La primera vez, JeffDeck le pide un nombre y una ubicación. Después, Guardar reemplaza el fichero con su trabajo más reciente. En la parte inferior aparece un mensaje que dice "Guardado" y el nombre del fichero.

Un asterisco (*) después del nombre en la barra de título indica que hay cambios sin guardar. Guarde con frecuencia. Si JeffDeck se detiene inesperadamente, consulte [Recuperar trabajo sin guardar](recover).

Si un mensaje indica que algunos objetos "no se pueden guardar en este tipo de fichero todavía", el fichero se guarda, pero JeffDeck no tenía forma de escribir esos objetos. Conserve también el fichero original. Al guardar un fichero .odp, el mensaje enumera lo que ese tipo de fichero no puede contener. La sección "Guardar como fichero .odp" más abajo explica qué no puede contener.

## Guardar con otro nombre o tipo

Haga clic en **Archivo > Guardar como**, o pulse F12. Escriba un nombre, elija el tipo y haga clic en **Guardar**. Las opciones son:

- **Presentación (*.pptx)** es la opción habitual.
- **Plantilla (*.potx)** guarda las diapositivas como un modelo para crear presentaciones nuevas.
- **Presentación con diapositivas (*.ppsx)** guarda una presentación para reproducirla.
- **Presentación con macros (*.pptm)**, **Plantilla con macros (*.potm)** y **Presentación con diapositivas con macros (*.ppsm)** son los mismos tres tipos para una presentación que contiene **macros**. Una macro es un pequeño programa que acompaña a un fichero. JeffDeck nunca ejecuta macros, y tampoco las lee ni las muestra. Si abre uno de estos ficheros y lo guarda con el mismo tipo, las macros se conservan tal como estaban. Si lo guarda como .pptx, .potx o .ppsx, las macros se omiten y JeffDeck se lo indica. Una presentación sin macros también puede guardarse con estos tipos, pero no tiene macros que conservar. Si las macros de un fichero son demasiado grandes para conservarlas (más de 32 MB en una parte, o 64 MB en total), JeffDeck las omite y se lo indica al abrirlo.
- **Presentación OpenDocument (*.odp)** guarda las diapositivas en el formato que usan LibreOffice y otros programas libres.
- **Plantilla de presentación OpenDocument (*.otp)** es la forma de plantilla de ese tipo.

Si omite la extensión, JeffDeck añade la que corresponde al tipo. Después de guardar, la barra de título muestra el nombre nuevo y usted sigue trabajando en el fichero nuevo.

Una presentación que abrió a partir de una plantilla se guarda como presentación nueva: **Guardar** pide un nombre, para que la plantilla no cambie. Lo mismo ocurre con una que abrió desde un fichero .ppt, .pps o .pot antiguo. Una presentación que abrió desde un fichero .odp se vuelve a guardar como fichero .odp, sobre el mismo fichero.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffDeck escribe la presentación en ese fichero, y usted sigue trabajando en la que tenía. Su nombre y sus cambios sin guardar se mantienen como estaban.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones** y, en la pestaña **General**, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffDeck copia primero el fichero anterior en otro llamado "Copia de seguridad de" seguido de su nombre, en la misma carpeta. Si no se puede crear la copia, la aplicación pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el fichero se queda como estaba.

## Guardar como plantilla

Una plantilla es una presentación que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla (un fichero **.potx**) se guarda en **Mis plantillas**, y su presentación no cambia. Para crear una presentación nueva a partir de ella, haga clic en **Archivo > Nuevo** y luego en la plantilla de **Mis plantillas**. La presentación nueva todavía no tiene fichero propio, así que la plantilla no cambia. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Abrir

Pulse Ctrl+O para elegir un fichero de inmediato. O haga clic en **Archivo > Abrir** y luego en **Examinar**, o en un nombre de **Anclado** o **Presentaciones recientes**.

El fichero se abre en esta ventana si contiene una presentación nueva que nadie ha modificado. Si no, se abre en una ventana propia.

Haga clic con el botón derecho en un nombre de la lista para **Anclar a la lista** (se mantiene arriba), **Desanclar de la lista** o **Quitar de la lista**. Un nombre en gris indica que JeffDeck no encuentra ese fichero en este momento.

## Otras formas de abrir un fichero

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de recientes.

- **Abrir como solo lectura** abre la presentación solo para leerla. Puede mirarla, desplazarse por las diapositivas y copiar, pero no puede cambiar nada. La barra de título indica "Solo lectura". Si pulsa **Guardar**, JeffDeck pide un nombre nuevo, de modo que el fichero que abrió no cambia.
- **Abrir como copia** abre las diapositivas como una presentación nueva sin fichero propio. **Guardar** pregunta dónde colocarla.
- **Abrir en nueva ventana** abre el fichero en una ventana propia, aunque la ventana en la que está esté vacía.
- **Abrir y reparar** sirve para un fichero .pptx o .ppt dañado, por ejemplo uno que quedó incompleto al copiarlo. JeffDeck recupera todo lo que puede: lee cada parte que puede leer, conserva una parte que se interrumpió hasta el punto donde se cortó y omite una parte que no puede leer. En un fichero .ppt antiguo, busca las diapositivas según su contenido cuando las listas que indican dónde están guardadas están dañadas o perdidas. Cuando las listas que indican cómo encajan las partes del fichero están dañadas, el programa toma cada parte como continuación de la anterior y muestra lo que encuentra allí. Esa parte podría pertenecer a una copia anterior de la presentación o a otra parte del fichero, así que el mensaje indica la diapositiva donde empieza y le pide que la compare con una copia de seguridad si tiene una. Si la suposición llega antes de la primera diapositiva, el mensaje indica que toda la presentación se reconstruyó a partir de fragmentos. Si las imágenes no pudieron recuperarse por completo, el mensaje lo indica. Si se pierde la lista que da el orden de las diapositivas, estas aparecen en el orden en que se guardaron, que podría no ser el orden original. Un fichero que tiene contraseña no se puede reparar. Las diapositivas reparadas se abren como una copia llamada "Reparada" seguido del nombre, y un mensaje le indica lo que se hizo. El fichero dañado no se modifica.

## Ficheros con contraseña

Un fichero **.pptx** u **.odp** que pide una contraseña se abre como de costumbre: JeffDeck le pide la contraseña, y puede intentarlo de nuevo si no es correcta, o hacer clic en **Cancelar**. El fichero sigue protegido al guardarlo. Para cambiar o quitar la contraseña, use **Archivo > Información > Cifrar con contraseña** (consulte La pantalla Archivo). Una copia guardada como fichero .odp también conserva la contraseña.

## Fuentes en el fichero

Una fuente es el diseño de las letras. No todos los equipos tienen todas las fuentes. Si a un equipo le falta una fuente que usan sus diapositivas, dibuja las letras con otra fuente. Entonces las diapositivas pueden verse distintas.

JeffDeck puede incluir dentro del fichero las fuentes que usan sus diapositivas. Así, las diapositivas se ven igual en cualquier equipo. Esto se llama insertar fuentes. JeffDeck lo hace cada vez que guarda. Para desactivarlo, haga clic en **Archivo > Opciones**, abra la pestaña **Guardar** y desactive **Insertar fuentes en el fichero**.

- JeffDeck incluye solo las fuentes que vienen con él, como Fraunces, Lato y Open Sans. Otros equipos no tienen esas fuentes. Nunca incluye una fuente que tienen todos los equipos, como Calibri o Arial. Omite una fuente cuya licencia indica que no puede copiarse dentro de un fichero.
- JeffDeck incluye cada estilo que usan las diapositivas: normal, negrita, cursiva y negrita cursiva. Si las diapositivas usan solo el estilo normal, solo se incluye ese.
- JeffDeck incluye todas las letras de cada fuente, no solo las que usó. Así, quien abra el fichero puede seguir cambiando las palabras.
- El fichero aumenta de tamaño. Cada estilo de una fuente añade entre 40 y 700 KB (un KB son mil bytes). El fichero se comprime al guardarlo, así que crece alrededor de la mitad de esa cantidad.

Al abrir un fichero que tiene fuentes insertadas, JeffDeck las usa mientras la presentación está abierta. Las diapositivas se ven como se vieron donde se crearon. JeffDeck no instala las fuentes en su equipo ni las copia en ningún lugar. Pertenecen solo a esa presentación: no aparecen en sus listas de fuentes, y ninguna otra presentación las usa. Desaparecen al cerrar la presentación. Si su equipo ya tiene una fuente, JeffDeck usa su propia copia.

JeffDeck lee las fuentes de ficheros .pptx creados por otros programas de presentaciones, incluso cuando el programa las comprimió. También lee las fuentes de ficheros .odp.

Un fichero puede contener una fuente que JeffDeck no tiene. Al guardar, JeffDeck vuelve a incluir esa fuente en el fichero, siempre que las diapositivas sigan usándola. Lo hace tanto si **Insertar fuentes en el fichero** está activado como si no. Si una fuente del fichero está dañada o no puede leerse, el mensaje que aparece al abrir el fichero lo indica.

## Tipos de fichero que JeffDeck lee

- **.pptx, .potx y .ppsx** son un mismo formato en tres versiones: presentación, plantilla y presentación con diapositivas. JeffDeck lee y escribe las tres.
- **.ppt, .pps y .pot** son el tipo antiguo. JeffDeck los lee, pero no puede escribirlos, así que **Guardar** pide un nombre nuevo y guarda un .pptx (un .ppsx en el caso de .pps). Nunca sobrescribe el original.
- **.odp y .otp** son presentaciones de documento abierto y sus plantillas. JeffDeck lee y escribe ambos. Un .odp abierto aquí se guarda como .odp. Un .otp es una plantilla, así que **Guardar** pide un nombre nuevo y sugiere un .odp.

Si una parte de un fichero no se puede leer, JeffDeck abre el resto y enumera lo que omitió. Si el fichero entero no se puede leer, indica el motivo.

## Guardar como fichero .odp

Un fichero .odp se abre en LibreOffice Impress con sus diapositivas, patrones, imágenes, notas y comentarios, y con el aspecto que muestra JeffDeck. Contiene animaciones, transiciones de diapositiva, presentaciones personalizadas y la configuración de la presentación con diapositivas. El tipo de letra de cada palabra se guarda por nombre, así que un equipo que no tenga esa fuente muestra otra.

JeffDeck todavía no lee de vuelta las animaciones, los comentarios, las presentaciones personalizadas ni la configuración de la presentación desde un fichero .odp. Están en el fichero, y LibreOffice los reproduce, pero cuando abre el fichero .odp en JeffDeck, los enumera como omitidos. Conserve su fichero .pptx si quiere seguir editándolos aquí.

Un fichero .odp no tiene lugar para algunas cosas. JeffDeck las enumera después de guardar, en el mensaje de la parte inferior de la ventana:

- **Gráficos, diagramas y ecuaciones** se guardan como imágenes. Puede verlos, pero no puede editar sus números ni sus partes en el fichero .odp.
- **Las secciones** no se guardan.
- **Los biseles y las sombras interiores** se omiten, y lo mismo ocurre con los resplandores y reflejos en el texto. Una sombra en el texto se conserva. Un reflejo en una forma, una línea con degradado, una línea de dos o tres trazos y el tamaño de una sombra se conservan de una forma que solo JeffDeck interpreta. Otros programas muestran la forma sin el reflejo, la línea en su primer color, un trazo simple y la sombra con el tamaño del objeto.
- **Los efectos de animación** que se inician al hacer clic en un objeto, y los efectos que cambian un color, se omiten. El color al que se atenúa un objeto después de su efecto también se omite: el efecto se mantiene y el objeto conserva su propio color. El texto animado por palabra o por letra se guarda como una sola animación de todo el texto. Un clic en un vídeo, para pausarlo o reproducirlo de nuevo, se conserva.
- **Un vídeo o sonido vinculado** desde otro fichero, que no se guarda dentro de la presentación, conserva su dirección y su imagen en el fichero .odp. LibreOffice lo reproduce si encuentra el fichero. Los vídeos y sonidos que están dentro de la presentación se guardan en el fichero .odp y se reproducen.

Conserve también su fichero .pptx si necesita recuperar estas cosas.

## Lo que se conserva pero no se muestra

Algunas partes de un fichero son cosas que JeffDeck todavía no puede mostrar ni cambiar. Las conserva junto con sus diapositivas y las vuelve a escribir al guardar, así que no se pierden. Incluyen las animaciones, que se reproducen en la presentación; los vídeos y sonidos, que todavía no se reproducen; y un efecto de transición para el que JeffDeck no tiene forma, que se reproduce como un desvanecimiento. Los gráficos y diagramas se muestran tal como se guardaron. Consulte [Bienvenido a JeffDeck](start).

## Temas relacionados

- [Recuperar trabajo sin guardar](recover)
- [La pantalla Archivo](file-screen)
- [Reutilizar diapositivas de otra presentación](reuse)
- [Imprimir, PDF e imágenes de diapositivas](printing)
