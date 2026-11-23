# Info: propiedades, contraseñas e Inspeccionar

<!-- keywords: comprobar accesibilidad, información, propiedades, título, autor, asunto, palabras clave, categorías, comentarios, empresa, administrador, propiedades personalizadas, docproperty, marcar como final, final, solo lectura, contraseña, cifrar, proteger, contraseña de apertura, inspeccionar documento, información personal, texto oculto, XML personalizado, comprobar compatibilidad, historial de versiones, versiones, guardar una versión, volver, restaurar, comparar versiones, mostrar en carpeta, copiar ruta, carpeta, ruta -->

Haga clic en **Archivo > Información** para ver los datos del documento y protegerlo antes de compartirlo. Pulse **Esc** para volver al documento.

## Dónde está el fichero

En la parte superior, la página muestra dónde está guardado el fichero. Haga clic en **Mostrar en carpeta** para abrir esa carpeta. Haga clic en **Copiar ruta** para copiar la ruta y pegarla en un correo electrónico o en un chat.

## Propiedades

Las propiedades son datos del documento, como su título y quién lo escribió. La página tiene un cuadro para cada una: **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Empresa** y **Administrador**. Escriba en un cuadro y pulse **Tab** o **Enter**. JeffWrite conserva el cambio y lo guarda la próxima vez que guarde el documento.

Debajo de los cuadros, la página muestra lo que JeffWrite calcula: cuándo se creó, se guardó y se imprimió el documento, y cuántas páginas, palabras, caracteres y párrafos tiene.

### Propiedades personalizadas

Haga clic en **Propiedades personalizadas** para crear propiedades propias, como **Proyecto** o **Cliente**. Haga clic en **Agregar**, escriba un nombre, elija el tipo de valor (texto, un número entero, un número, sí o no, o una fecha) y escriba el valor.

Un campo puede mostrar una propiedad personalizada. Haga clic en **Insertar > Campo** y elija un campo. Cambie su código a `DOCPROPERTY "Cliente"` y haga clic en **Aceptar**. El texto mostrará el valor. Pulse **F9** para actualizarlo después de cambiar la propiedad. Consulte [Campos](fields).

## Marcar como final

Haga clic en **Marcar como final** para indicar a los lectores que el documento está terminado. JeffWrite guarda el documento y lo deja solo para lectura. Cuando alguien lo abre, la barra de título indica que es de solo lectura, y una nota le informa de que se marcó como final. Es una forma suave de decir «por favor, no lo cambie». No es un bloqueo, porque cualquiera puede elegir **Marcado como final** de nuevo para editarlo.

## Abrir siempre como solo lectura

Haga clic en **Abrir siempre como solo lectura** para que el documento se abra solo para lectura cada vez, de modo que nadie lo cambie por accidente. La opción se guarda con el fichero al guardarlo. La próxima vez que alguien abra el fichero, la barra de estado indicará que se abre solo para lectura. Para editarlo, abra **Archivo > Información** y haga clic en **Se abre siempre como solo lectura**. Basta un clic, así que es un recordatorio y no un bloqueo. Otros programas no leen esta opción, y un fichero .odt no puede guardarla, así que guarde primero el documento como .docx. Comprobar compatibilidad la indica para un fichero .odt.

## Cifrar con contraseña

Haga clic en **Cifrar con contraseña** para pedir una contraseña cada vez que se abra el fichero. Escriba la contraseña dos veces y haga clic en **Proteger**. El fichero se cifra al guardarlo, de modo que nadie puede leerlo sin la contraseña. JeffWrite usa el tipo de protección fuerte que emplean otros procesadores de texto, así que también podrán abrir el fichero con la contraseña.

- Una contraseña **no se puede recuperar** si la olvida. Guárdela en un lugar seguro.
- Las versiones guardadas y la copia de seguridad siguen a la contraseña: están protegidas con ella y cambian cuando usted la cambia o la quita.
- Para cambiar la contraseña, haga clic de nuevo en la misma tarjeta. Haga clic en **Quitar contraseña** para eliminarla. El cambio se guarda la próxima vez que guarde.
- Las contraseñas funcionan con ficheros **.docx** y con ficheros **.odt**. Un fichero .odt se protege de la forma que describe el estándar OpenDocument, así que otros programas que sigan el estándar también podrán abrirlo con la contraseña. Un fichero de texto sin formato o .rtf no puede guardar una contraseña, así que JeffWrite no guardará un documento protegido en esos formatos. En lugar de guardarlo sin la contraseña, le avisará.

Para abrir un fichero protegido, ábralo como de costumbre. JeffWrite le pedirá la contraseña. Si no es correcta, puede volver a intentarlo o hacer clic en **Cancelar**.

JeffWrite también abre un fichero **.doc** antiguo que pide una contraseña, y la pide de la misma forma. JeffWrite nunca guarda ficheros .doc, así que **Guardar como** pide un nombre nuevo, y la copia .docx conserva la misma contraseña.

## Inspeccionar documento

Haga clic en **Inspeccionar documento** antes de enviar un documento a alguien. Muestra lo que contiene el fichero que quizá no quiera compartir, y cuántos elementos de cada tipo encontró:

- **Comentarios** son notas que las personas dejaron en el margen.
- **Cambios registrados** son ediciones que todavía no se han aceptado ni rechazado. **Aceptar todo** conserva cada cambio como parte del texto.
- **Propiedades del documento e información personal** son el autor, la persona que lo guardó por última vez, la empresa y el administrador.
- **Texto oculto** es texto configurado para ocultarse. No se muestra ni se imprime, pero está en el fichero.
- **Encabezados y pies de página** son el texto de la parte superior e inferior de las páginas.
- **Datos XML personalizados** son datos que otros programas añadieron al fichero. No se ven en la página.
- **Vínculos externos** son vínculos a otros ficheros, no vínculos web: una imagen o un objeto que solo está vinculado (muestra un fichero que no está en el documento), y un campo que toma su texto o su imagen de otro fichero (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE y DDEAUTO). **Quitar todo** convierte cada campo en el texto que muestra ahora, quita cada imagen u objeto vinculado y deja el vínculo fuera del fichero al guardarlo. Los demás ficheros no se tocan. Los vínculos de encabezados, pies de página, notas al pie y notas al final también se cuentan y se quitan. Los vínculos web se mantienen. También se buscan los vínculos dentro de comentarios, cuadros de texto y las partes que el programa conserva tal cual (como el texto de los dibujos). Una imagen vinculada que además contiene su propia copia de la imagen se conserva, como imagen sin vínculo, y el vínculo se deja fuera del fichero al guardarlo. Quitar vínculos dentro de comentarios y de las demás partes que se conservan tal cual no se puede deshacer; los cuadros de texto sí son un paso de Deshacer. La plantilla a la que está adjunto el documento (su ruta suele contener un nombre de usuario) también cuenta como vínculo externo: **Quitar todo** la desvincula, y el documento conserva sus propios estilos. Otros vínculos a ficheros no se quitan, y una línea debajo del recuento los nombra: el origen de datos de la combinación de correspondencia, los marcos, los subdocumentos dentro de otras partes, los datos externos de un gráfico, los vídeos y sonidos vinculados, y las hojas de estilos vinculadas. La misma línea cuenta cualquier otro vínculo en un campo, un comentario, un cuadro de texto o la lista de vínculos del fichero que nada del documento usa, e indica cuándo la dirección de una imagen vinculada no está en el fichero o cuando una parte está en una codificación que la comprobación no puede leer. Entonces la fila dirá «No revisado por completo» en lugar de «No se encontró ninguno». Una sola acción de **Quitar todo** elimina como máximo 100 000 vínculos en las partes conservadas; si quedan más, la línea indica cuántos y puede volver a ejecutar **Quitar todo**.

Haga clic en **Quitar todo** junto a un tipo para eliminarlo. Puede deshacer con **Ctrl+Z** los cambios que modifican el texto. Los vínculos quitados de comentarios y de otras partes conservadas, y los datos XML personalizados, no se pueden deshacer. Guarde el documento después para conservar la copia limpia.

**Quitar la información personal de este fichero al guardarlo**, debajo de las tarjetas, lo hace cada vez que guarda, en cualquier fichero, sin cambiar el documento abierto. Es el mismo cuadro que aparece en **Opciones**, y es igual en todas las aplicaciones de JeffOffice.

## Comprobar compatibilidad

Haga clic en **Comprobar compatibilidad** y elija un tipo de fichero para ver lo que no puede contener. JeffWrite guarda ficheros **.docx**, **.odt**, **.rtf** y **.txt**. Un fichero .docx contiene todo lo que hay en el documento. Los demás tipos no pueden contener algunas cosas, y esta lista indica cuáles. También nombra las opciones guardadas en el propio fichero, Marcar como final y Abrir siempre como solo lectura, cuando están activadas, y la contraseña para un tipo que no puede guardarla. Si no aparece nada en la lista, no se pierde nada.

Al guardar como fichero .odt, JeffWrite muestra lo que el fichero no puede contener **antes** de escribir nada. Puede elegir **Guardar de todos modos**, **Guardar como .docx** (el fichero .docx se crea junto a él y no se pierde nada) o **Cancelar**. Marque **No volver a preguntar para este fichero** y JeffWrite dejará de preguntar sobre ese fichero hasta que cierre el programa.

## Comprobar accesibilidad

Haga clic en **Comprobar accesibilidad** para abrir la lista de elementos que pueden ser difíciles de leer o usar para algunas personas, como una imagen sin descripción. Es la misma comprobación que **Revisar > Comprobar accesibilidad**. Consulte [Contar palabras, definir el idioma, comprobar la accesibilidad y buscar palabras](proofing-tools).

## Historial de versiones

Haga clic en **Historial de versiones** para guardar copias con nombre del documento en su equipo. No se envía nada a ningún sitio. Una versión es una copia completa del documento tal como está en ese momento. Guarde una antes de un cambio importante, para poder volver a ella. El documento debe guardarse primero, porque las versiones se guardan con el nombre del fichero.

- **Guardar una versión** pide un nombre, como «Antes de la edición», y guarda una copia.
- **Abrir una copia** abre la versión elegida en una ventana nueva. La versión en sí no cambia.
- **Comparar** muestra el documento y la versión elegida uno junto a otro, como un solo documento, con las diferencias marcadas como cambios registrados. Consulte [Control de cambios](track-changes).
- **Volver a esta versión** hace que el documento sea igual a la versión elegida. JeffWrite guarda antes el documento tal como estaba como una versión nueva, para que no se pierda nada. Guarde el documento para conservar el cambio.
- **Eliminar** borra la versión elegida.

JeffWrite conserva hasta 50 versiones de un fichero y elimina la más antigua cuando hay más.
