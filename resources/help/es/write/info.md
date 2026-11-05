# Información: propiedades, contraseñas e Inspeccionar

<!-- keywords: información, propiedades, título, autor, asunto, palabras clave, categorías, comentarios, empresa, administrador, propiedades personalizadas, docproperty, marcar como final, final, solo lectura, contraseña, cifrar, proteger, contraseña de apertura, inspeccionar documento, información personal, texto oculto, xml personalizado, comprobar compatibilidad, historial de versiones, versiones, guardar una versión, volver, restaurar, comparar versiones, mostrar en la carpeta, copiar ruta, carpeta, ruta -->

Haga clic en **Archivo > Información** para ver los datos de su documento y protegerlo antes de compartirlo. Presione **Esc** para volver al documento.

## Dónde está el archivo

En la parte superior, la página muestra dónde está guardado el archivo. Haga clic en **Mostrar en la carpeta** para abrir esa carpeta. Haga clic en **Copiar ruta** para copiar la ruta y pegarla en un correo electrónico o en un chat.

## Propiedades

Las propiedades son datos del documento, como su título y quién lo escribió. La página tiene un cuadro para cada una: **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Empresa** y **Administrador**. Escriba en un cuadro y presione **Tab** o **Entrar**. JeffWrite conserva el cambio y lo guarda la próxima vez que guarde el documento.

Debajo de los cuadros, la página muestra lo que JeffWrite calcula: cuándo se creó, guardó e imprimió el documento, y cuántas páginas, palabras, caracteres y párrafos tiene.

### Sus propias propiedades

Haga clic en **Propiedades personalizadas** para crear propiedades propias, como **Proyecto** o **Cliente**. Haga clic en **Agregar**, escriba un nombre, elija qué tipo de valor es (texto, número entero, número, sí o no, o fecha) y escriba el valor.

Un campo puede mostrar una propiedad personalizada. Haga clic en **Insertar > Campo** y elija un campo. Cambie su código a `DOCPROPERTY "Cliente"` y haga clic en **Aceptar**. El texto muestra el valor. Presione **F9** para actualizarlo después de cambiar la propiedad. Consulte [Campos](fields).

## Marcar como final

Haga clic en **Marcar como final** para indicar a los lectores que el documento está terminado. JeffWrite guarda el documento y luego lo abre solo para lectura. Cuando alguien lo abre, la barra de título indica que es de solo lectura, y una nota le informa de que se marcó como final. Es una forma suave de decir «por favor, no lo cambie». No es un bloqueo, porque cualquiera puede elegir **Marcado como final** de nuevo para editarlo.

## Cifrar con contraseña

Haga clic en **Cifrar con contraseña** para pedir una contraseña cada vez que se abra el archivo. Escriba la contraseña dos veces y haga clic en **Proteger**. El archivo se cifra al guardarlo, de modo que nadie puede leerlo sin la contraseña. JeffWrite usa el tipo de protección fuerte que emplean otros procesadores de texto, de modo que también pueden abrir el archivo con la contraseña.

- Una contraseña **no se puede recuperar** si la olvida. Guárdela en un lugar seguro.
- Para cambiar la contraseña, haga clic de nuevo en la misma tarjeta. Haga clic en **Quitar contraseña** para eliminarla. El cambio se guarda la próxima vez que guarde.
- Las contraseñas funcionan con archivos **.docx**. Un archivo **.odt** todavía no puede tener contraseña, así que JeffWrite no guardará un documento protegido como .odt. En su lugar, le avisará, en lugar de guardarlo sin contraseña.

Para abrir un archivo protegido, ábralo como de costumbre. JeffWrite le pedirá la contraseña. Si no es correcta, puede intentarlo de nuevo o hacer clic en **Cancelar**.

## Inspeccionar documento

Haga clic en **Inspeccionar documento** antes de enviar un documento a alguien. Muestra lo que contiene el archivo y que quizá no quiera compartir, y cuántos elementos de cada tipo encontró:

- **Comentarios** son notas que alguien dejó en el margen.
- **Cambios registrados** son ediciones que todavía no se han aceptado ni rechazado. **Aceptar todo** mantiene todos los cambios como parte del texto.
- **Propiedades del documento e información personal** son el autor, la persona que lo guardó por última vez, la empresa y el administrador.
- **Texto oculto** es texto configurado para ocultarse. No se muestra ni se imprime, pero está en el archivo.
- **Encabezados y pies de página** son el texto de la parte superior e inferior de las páginas.
- **Datos XML personalizados** son datos que otros programas añadieron al archivo. No se ven en la página.

Haga clic en **Quitar todo** junto a un tipo para eliminarlo. Puede deshacer los cambios que afectan al texto con **Ctrl+Z**. Guarde el documento después para conservar la copia limpia.

## Comprobar compatibilidad

Haga clic en **Comprobar compatibilidad** para ver qué no puede guardar un tipo de archivo más antiguo. JeffWrite guarda archivos **.docx** y **.odt**. Un archivo .docx contiene todo el documento. Un archivo .odt no puede contener algunas cosas, y esta lista indica cuáles. Si no aparece nada en la lista, no se pierde nada.

Al guardar como archivo .odt, JeffWrite muestra lo que el archivo no puede contener **antes** de escribir nada. Puede elegir **Guardar como .odt de todos modos**, **Guardar como .docx en su lugar** (el archivo .docx se guarda junto a él y no se pierde nada) o **Cancelar**.

## Historial de versiones

Haga clic en **Historial de versiones** para guardar copias con nombre del documento en su equipo. No se envía nada a ningún sitio. Una versión es una copia completa del documento tal como está en ese momento. Guarde una antes de un cambio importante para poder volver a ella. El documento debe guardarse primero, porque las versiones se guardan con el nombre del archivo.

- **Guardar una versión** pide un nombre, como «Antes de la revisión», y guarda una copia.
- **Abrir una copia** abre la versión elegida en una ventana nueva. La versión no cambia.
- **Comparar** muestra el documento y la versión elegida uno junto a otro, como un solo documento, con las diferencias marcadas como cambios registrados. Consulte [Control de cambios](track-changes).
- **Volver a esta versión** hace que el documento sea igual a la versión elegida. JeffWrite guarda antes el documento tal como estaba, como una versión nueva, para que no se pierda nada. Guarde el documento para conservar el cambio.
- **Eliminar** borra la versión elegida.

JeffWrite conserva hasta 50 versiones de un archivo y elimina la más antigua cuando hay más.
