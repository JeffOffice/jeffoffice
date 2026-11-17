# Info: propiedades, contraseñas e Inspeccionar documento
<!-- keywords: comprobar accesibilidad, información, propiedades, título, autor, asunto, palabras clave, categorías, comentarios, empresa, administrador, propiedades personalizadas, docproperty, marcar como final, final, solo lectura, contraseña, cifrar, proteger, contraseña de apertura, inspeccionar documento, información personal, texto oculto, xml personalizado, comprobar compatibilidad, historial de versiones, versiones, guardar una versión, volver, restaurar, comparar versiones, mostrar en la carpeta, copiar ruta, carpeta, ruta, información del archivo -->

Haga clic en **Archivo > Información** para ver los datos de su documento y protegerlo antes de compartirlo. Presione **Esc** para volver a su documento.

## Dónde está el archivo

En la parte superior, la página muestra dónde está guardado el archivo. Haga clic en **Mostrar en la carpeta** para abrir esa carpeta. Haga clic en **Copiar ruta** para copiar la ruta y pegarla en un correo electrónico o en un chat.

## Propiedades

Las propiedades son datos del documento, como su título y quién lo escribió. La página tiene un cuadro para cada una: **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Empresa** y **Administrador**. Escriba en un cuadro y presione **Tab** o **Intro**. JeffWrite conserva el cambio y lo guarda la próxima vez que guarde el documento.

Debajo de los cuadros, la página muestra lo que JeffWrite calcula: cuándo se creó, guardó e imprimió el documento, y cuántas páginas, palabras, caracteres y párrafos tiene.

### Sus propias propiedades

Haga clic en **Propiedades personalizadas** para crear propiedades propias, como **Proyecto** o **Cliente**. Haga clic en **Agregar**, escriba un nombre, elija el tipo de valor (texto, un número entero, un número, sí o no, o una fecha) y escriba el valor.

Un campo puede mostrar una propiedad personalizada. Haga clic en **Insertar > Campo** y elija un campo. Cambie su código por `DOCPROPERTY "Cliente"` y haga clic en **Aceptar**. El texto muestra el valor. Presione **F9** para actualizarlo después de cambiar la propiedad. Consulte [Campos](fields).

## Marcar como final

Haga clic en **Marcar como final** para indicar a los lectores que el documento está terminado. JeffWrite guarda el documento y luego lo mantiene solo para lectura. Cuando alguien lo abre, la barra de título indica que es de solo lectura, y una nota le informa de que se marcó como final. Es una forma suave de decir «por favor, no cambie esto». No es un bloqueo, porque cualquiera puede elegir **Marcado como final** de nuevo para editarlo.

## Cifrar con contraseña

Haga clic en **Cifrar con contraseña** para pedir una contraseña cada vez que se abra el archivo. Escriba la contraseña dos veces y haga clic en **Proteger**. El archivo se cifra al guardarlo, así que nadie puede leerlo sin la contraseña. JeffWrite usa el tipo de protección fuerte que emplean otros procesadores de texto, de modo que también puedan abrir el archivo con la contraseña.

- Una contraseña **no se puede recuperar** si la olvida. Guárdela en un lugar seguro.
- Las versiones guardadas y la copia de seguridad siguen a la contraseña: están protegidas con ella y cambian cuando cambia o se quita.
- Para cambiar la contraseña, haga clic de nuevo en la misma tarjeta. Haga clic en **Quitar contraseña** para eliminarla. El cambio se guarda la próxima vez que guarde.
- Las contraseñas funcionan con archivos **.docx** y con archivos **.odt**. Un archivo .odt se protege según describe el estándar OpenDocument, de modo que otros programas que siguen el estándar también pueden abrirlo con la contraseña. Un archivo de texto sin formato o .rtf no puede contener una contraseña, así que JeffWrite no guardará un documento protegido en esos formatos. En su lugar, se lo indicará y no lo guardará sin la contraseña.

Para abrir un archivo protegido, ábralo como de costumbre. JeffWrite le pedirá la contraseña. Si no es correcta, puede intentarlo de nuevo o hacer clic en **Cancelar**.

JeffWrite también abre un archivo **.doc** antiguo que pide contraseña, y se la pide de la misma forma. JeffWrite nunca guarda archivos .doc, así que **Guardar como** le pedirá un nombre nuevo, y la copia .docx conserva la misma contraseña.

## Inspeccionar documento

Haga clic en **Inspeccionar documento** antes de enviar un documento a alguien. Muestra lo que contiene el archivo y que quizá no desee compartir, y cuántos elementos de cada tipo encontró:

- **Comentarios** son notas que las personas dejaron en el margen.
- **Cambios registrados** son ediciones que aún no se han aceptado ni rechazado. **Aceptar todo** conserva cada cambio como parte del texto.
- **Propiedades del documento e información personal** son el autor, la persona que lo guardó por última vez, la empresa y el administrador.
- **Texto oculto** es texto configurado para ocultarse. No se muestra ni se imprime, pero está en el archivo.
- **Encabezados y pies de página** son el texto de la parte superior e inferior de las páginas.
- **Datos XML personalizados** son datos que otros programas añadieron al archivo. No se ven en la página.

Haga clic en **Quitar todo** junto a un tipo para eliminarlo. Puede deshacer los cambios en el texto con **Ctrl+Z**. Guarde el documento después para conservar la copia limpia.

**Quitar la información personal de este archivo al guardarlo**, debajo de las tarjetas, hace esto cada vez que guarda, con cualquier archivo, sin cambiar el documento que tiene abierto. Es el mismo cuadro que aparece en **Opciones**, y es igual en todas las aplicaciones de JeffOffice.

## Comprobar compatibilidad

Haga clic en **Comprobar compatibilidad** para ver lo que un tipo de archivo más antiguo no puede contener. JeffWrite guarda archivos **.docx** y **.odt**. Un archivo .docx contiene todo lo que hay en su documento. Un archivo .odt no puede contener algunas cosas, y esta lista indica cuáles. Si no aparece nada, no se pierde nada.

Al guardar como archivo .odt, JeffWrite muestra lo que el archivo no puede contener **antes** de escribir nada. Puede elegir **Guardar de todos modos**, **Guardar como .docx** (el archivo .docx se guarda junto al otro y no se pierde nada) o **Cancelar**. Marque **No volver a preguntar para este archivo** y JeffWrite dejará de preguntar sobre ese archivo hasta que cierre el programa.

## Comprobar accesibilidad

Haga clic en **Comprobar accesibilidad** para abrir la lista de elementos que pueden resultar difíciles de leer o usar para algunas personas, como una imagen sin descripción. Es la misma comprobación que **Revisar > Comprobar accesibilidad**. Consulte [Contar palabras, definir el idioma, comprobar la accesibilidad y buscar palabras](proofing-tools).

## Historial de versiones

Haga clic en **Historial de versiones** para guardar copias con nombre del documento en su equipo. No se envía nada a ningún sitio. Una versión es una copia completa del documento tal como está en ese momento. Guarde una antes de un cambio importante, para poder volver a ella. El documento debe guardarse primero, porque las versiones se guardan con el nombre del archivo.

- **Guardar una versión** pide un nombre, como «Antes de la edición», y guarda una copia.
- **Abrir una copia** abre la versión elegida en una ventana nueva. La versión en sí no cambia.
- **Comparar** muestra el documento y la versión elegida juntos, en un solo documento, con las diferencias marcadas como cambios registrados. Consulte [Control de cambios](track-changes).
- **Volver a esta versión** hace que el documento sea igual a la versión elegida. JeffWrite guarda antes el documento tal como estaba como una versión nueva, así que no se pierde nada. Guarde el documento para conservar el cambio.
- **Eliminar** quita la versión elegida.

JeffWrite conserva hasta 50 versiones de un archivo y elimina la más antigua cuando hay más.
