# Información: propiedades, contraseñas e Inspeccionar documento
<!-- keywords: comprobar accesibilidad, información, propiedades, título, autor, asunto, palabras clave, categorías, comentarios, empresa, administrador, propiedades personalizadas, docproperty, marcar como final, final, solo lectura, contraseña, cifrar, proteger, contraseña de apertura, inspeccionar documento, información personal, texto oculto, XML personalizado, comprobar compatibilidad, convertir, modo de compatibilidad, historial de versiones, versiones, guardar una versión, volver, restaurar, comparar versiones, mostrar en la carpeta, copiar ruta, carpeta, ruta, información del documento, metadatos -->

Haga clic en **Archivo > Información** para ver los datos del documento y protegerlo antes de compartirlo. Pulse **Esc** para volver al documento.

## Dónde está el fichero

En la parte superior, la página muestra dónde está guardado el fichero. Haga clic en **Mostrar en la carpeta** para abrir esa carpeta. Haga clic en **Copiar ruta** para copiar la ruta y pegarla en un correo electrónico o en un chat.

## Propiedades

Las propiedades son datos del documento, como su título y quién lo escribió. La página tiene un cuadro para cada una: **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Empresa** y **Administrador**. Escriba en un cuadro y pulse **Tab** o **Entrar**. JeffWrite conserva el cambio y lo guarda la próxima vez que guarde el documento.

Debajo de los cuadros, la página muestra lo que JeffWrite calcula: cuándo se creó, guardó e imprimió el documento, y cuántas páginas, palabras, caracteres y párrafos tiene.

### Sus propias propiedades

Haga clic en **Propiedades personalizadas** para crear propiedades propias, como **Proyecto** o **Cliente**. Haga clic en **Agregar**, escriba un nombre, elija el tipo de valor (texto, un número entero, un número, sí o no, o una fecha) y escriba el valor.

Un campo puede mostrar una propiedad personalizada. Haga clic en **Insertar > Campo** y elija un campo. Cambie su código a `DOCPROPERTY "Client"` y haga clic en **Aceptar**. El texto muestra el valor. Pulse **F9** para actualizarlo después de cambiar la propiedad. Consulte [Campos](fields).

## Marcar como final

Haga clic en **Marcar como final** para indicar a los lectores que el documento está terminado. JeffWrite guarda el documento y lo deja solo para lectura. Cuando alguien lo abre, la barra de título indica que es de solo lectura, y una nota le informa de que se marcó como final. Es una forma suave de decir «por favor, no cambie esto». No es un bloqueo, porque cualquiera puede elegir **Marcado como final** de nuevo para editarlo.

## Abrir siempre como solo lectura

Haga clic en **Abrir siempre como solo lectura** para que el documento se abra solo para lectura cada vez, de modo que nadie lo cambie por accidente. La opción se guarda con el fichero al guardarlo. La próxima vez que alguien abra el fichero, la barra de estado indica que se abre solo para lectura. Para editarlo, abra **Archivo > Información** y haga clic en **Se abre siempre como solo lectura**. Es un solo clic, así que es un recordatorio y no un bloqueo. Otros programas no leen esta opción, y un fichero .odt no puede guardarla, por lo que primero debe guardar el documento como fichero .docx. Comprobar compatibilidad la indica para un fichero .odt.

## Cifrar con contraseña

Haga clic en **Cifrar con contraseña** para pedir una contraseña cada vez que se abra el fichero. Escriba la contraseña dos veces y haga clic en **Proteger**. El fichero se cifra al guardarlo, de modo que nadie puede leerlo sin la contraseña. JeffWrite usa el tipo de protección fuerte que emplean otros procesadores de texto, así que también podrán abrir el fichero con la contraseña.

- Una contraseña **no se puede recuperar** si la olvida. Guárdela en un lugar seguro.
- Las versiones que guardó y la copia de seguridad siguen a la contraseña: quedan protegidas con ella y cambian cuando cambie o quite la contraseña.
- Para cambiar la contraseña, haga clic de nuevo en la misma tarjeta. Haga clic en **Quitar contraseña** para eliminarla. El cambio se guarda la próxima vez que guarde.
- Las contraseñas funcionan con ficheros **.docx** y con ficheros **.odt**. Un fichero .odt se protege del modo que describe el estándar OpenDocument, de modo que otros programas que sigan el estándar también podrán abrirlo con la contraseña. Un fichero de texto sin formato o .rtf no puede guardar una contraseña, por lo que JeffWrite no guardará un documento protegido en esos formatos. En lugar de guardarlo sin contraseña, le avisará.

Para abrir un fichero protegido, ábralo como de costumbre. JeffWrite pedirá la contraseña. Si no es correcta, puede intentarlo de nuevo o hacer clic en **Cancelar**.

JeffWrite también abre un fichero **.doc** antiguo que pide una contraseña, y la pide de la misma forma. JeffWrite nunca guarda ficheros .doc, así que **Guardar como** pide un nombre nuevo, y la copia .docx conserva la misma contraseña.

## Inspeccionar documento

Haga clic en **Inspeccionar documento** antes de enviar un documento a alguien. Muestra lo que contiene el fichero y que quizá no quiera compartir, y cuántos elementos de cada tipo encontró:

- **Comentarios** son notas que las personas dejaron en el margen.
- **Cambios registrados** son ediciones que aún no se han aceptado ni rechazado. **Aceptar todo** conserva cada cambio como parte del texto.
- **Propiedades del documento e información personal** son el autor, la persona que lo guardó por última vez, la empresa y el administrador.
- **Texto oculto** es texto configurado para ocultarse. No se muestra ni se imprime, pero está en el fichero.
- **Encabezados y pies de página** son el texto de la parte superior e inferior de las páginas.
- **Datos XML personalizados** son datos que otros programas añadieron al fichero. No se ven en la página.
- **Vínculos externos** son vínculos a otros ficheros, no vínculos web: una imagen o un objeto que solo está vinculado (muestra un fichero que no está en el documento), y un campo que toma su texto o su imagen de otro fichero (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE y DDEAUTO). **Quitar todo** convierte cada campo en el texto que muestra ahora, quita cada imagen u objeto vinculado y deja el vínculo fuera del fichero al guardarlo. Los demás ficheros no se modifican. Los vínculos de encabezados, pies de página, notas al pie y notas al final también se cuentan y se quitan. Los vínculos web se conservan. También se buscan los vínculos dentro de comentarios, cuadros de texto y las partes que el programa conserva tal cual (como el texto de los dibujos). Una imagen vinculada que además contiene su propia copia de la imagen se conserva, como imagen sin vínculo, y el vínculo queda fuera del fichero al guardarlo. Quitar los vínculos dentro de comentarios y de las demás partes que se conservan tal cual no se puede deshacer; los cuadros de texto sí son un paso de Deshacer. La plantilla a la que está adjunto el documento (su ruta suele contener un nombre de usuario) también cuenta como vínculo externo: **Quitar todo** la desvincula, y el documento conserva sus propios estilos. Los demás vínculos a ficheros no se quitan, y una línea bajo el recuento los nombra: el origen de datos de la combinación de correspondencia, los marcos, los subdocumentos dentro de otras partes, los datos externos de un gráfico, los vídeos y sonidos vinculados, y las hojas de estilo vinculadas. Esa misma línea cuenta cualquier otro vínculo en un campo, un comentario, un cuadro de texto o la lista de vínculos del fichero que nada del documento usa, y lo indica cuando la dirección de una imagen vinculada no está en el fichero o cuando una parte está en una codificación que la comprobación no puede leer. En ese caso, la fila dice «No revisado por completo» en lugar de «No encontrado». Una sola acción de **Quitar todo** elimina como máximo 100 000 vínculos de las partes conservadas; si quedan más, la línea indica cuántos, y puede volver a ejecutar **Quitar todo**.
- **Imágenes originales conservadas bajo efectos de imagen** son las imágenes a partir de las que se creó un efecto, como Desenfoque o Mosaico. El fichero guarda el original junto a la imagen terminada, y quien reciba el fichero puede extraerlo. **Quitar originales** conserva solo la imagen terminada, y **Restablecer imagen** ya no podrá recuperar el original.
- **Partes recortadas de imágenes** son las partes que recortó y dejó fuera de la vista. Siguen en la imagen, y quien reciba el fichero puede verlas. **Quitar partes recortadas** las elimina, y la página se ve exactamente igual. Una imagen con un efecto primero se convierte en su imagen terminada.

Haga clic en **Quitar todo** junto a un tipo para eliminarlo. Puede deshacer con **Ctrl+Z** los cambios que modifican el texto. Los vínculos quitados de comentarios y de otras partes conservadas, y los datos XML personalizados, no se pueden deshacer. Guarde el documento después para conservar la copia limpia.

**Quitar la información personal de este fichero al guardarlo**, debajo de las tarjetas, hace esto cada vez que guarda, en cualquier fichero, sin cambiar el documento abierto. Es la misma casilla que en **Opciones**, y es igual en todas las aplicaciones de JeffOffice.

## Convertir

Un fichero .docx que indica que se creó con la forma de un programa anterior muestra **[Modo de compatibilidad]** después de su nombre en la barra de título. Eso significa que el documento se diseña con las reglas del programa anterior. Un documento nuevo, un fichero que no lo indica, y un .odt, .rtf, .doc o un fichero de texto no muestran nada. Abra **Archivo > Información** y haga clic en **Convertir** para usar las reglas más recientes. Antes pregunta, y no se puede deshacer con Ctrl+Z. Las páginas se ven bien de inmediato, tal como se verán al guardar el fichero y volver a abrirlo. Convertir cambia estos aspectos:

- **Tablas.** En el modo antiguo, la sangría de una tabla se mide hasta el texto de su primera celda, por lo que el borde izquierdo de la tabla queda un poco a la izquierda de ese texto. En el modo más reciente, la sangría se mide hasta el borde de la tabla, de modo que las tablas con sangría se desplazan a la derecha el ancho del margen de la celda.
- **Saltos de página.** En el modo antiguo, un párrafo que termina con un salto de página permite que el párrafo siguiente empiece la nueva página. En el modo más reciente, el final de ese párrafo pasa a la nueva página y deja una línea vacía en su parte superior.
- **Alturas de línea.** Un fichero creado con el modo más antiguo puede pedir líneas de puntos enteros. Después de Convertir, las líneas tienen su altura natural, así que las líneas y las páginas pueden cortarse en otros puntos.

La próxima vez que guarde el fichero, indicará que está en el modo más reciente, y la barra de título deja de mostrar la etiqueta. Las opciones que el fichero establece una por una, como la división de palabras y las páginas enfrentadas, se mantienen tal como están. Un documento que ya está en el modo más reciente no tiene el botón **Convertir**.

## Comprobar compatibilidad

Haga clic en **Comprobar compatibilidad** y elija un tipo de fichero para ver lo que no puede guardar. JeffWrite guarda ficheros **.docx**, **.odt**, **.rtf** y **.txt**. Un fichero .docx guarda todo el contenido del documento. Los demás tipos no pueden guardar algunas cosas, y esta lista indica cuáles. También nombra las opciones que el propio fichero guarda, Marcar como final y Abrir siempre como solo lectura, cuando están activadas, y la contraseña para un tipo que no puede guardarla. Si no aparece nada en la lista, no se pierde nada.

Al guardar como fichero .odt, JeffWrite enumera lo que el fichero no puede guardar **antes** de escribir nada. Puede elegir **Guardar de todos modos**, **Guardar como .docx** (el fichero .docx se crea junto a él y no se pierde nada) o **Cancelar**. Marque **No volver a preguntar para este fichero** y JeffWrite dejará de preguntar por ese fichero hasta que cierre el programa.

## Comprobar accesibilidad

Haga clic en **Comprobar accesibilidad** para abrir la lista de cosas que pueden resultar difíciles de leer o usar para algunas personas, como una imagen sin descripción. Es la misma comprobación que **Revisar > Comprobar accesibilidad**. Consulte [Contar palabras, establecer el idioma, comprobar la accesibilidad y buscar palabras](proofing-tools).

## Historial de versiones

Haga clic en **Historial de versiones** para guardar copias con nombre del documento en su equipo. No se envía nada a ningún sitio. Una versión es una copia completa del documento tal como está en ese momento. Guarde una antes de un cambio importante, para poder volver a ella. El documento debe estar guardado primero, porque las versiones se guardan con el nombre del fichero.

- **Guardar una versión** pide un nombre, como «Antes de la edición», y guarda una copia.
- **Abrir una copia** abre la versión elegida en una nueva ventana. La versión en sí no cambia.
- **Comparar** muestra el documento y la versión elegida uno junto a otro, como un solo documento, con las diferencias marcadas como cambios registrados. Consulte [Control de cambios](track-changes).
- **Volver a esta versión** hace que el documento sea igual a la versión elegida. JeffWrite guarda antes el documento tal como estaba, como una nueva versión, así que no se pierde nada. Guarde el documento para conservar el cambio.
- **Eliminar** borra la versión elegida.

JeffWrite conserva hasta 50 versiones de un fichero y elimina las más antiguas cuando hay más.
