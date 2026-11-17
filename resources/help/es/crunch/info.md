# Info: propiedades, contraseñas e Inspeccionar

<!-- keywords: información, propiedades, título, autor, asunto, palabras clave, categorías, comentarios, empresa, administrador, propiedades personalizadas, marcar como final, final, solo lectura, contraseña, cifrar, proteger, inspeccionar libro, información personal, hojas ocultas, notas, vínculos externos, XML personalizado, mostrar en carpeta, copiar ruta, carpeta, ruta, historial de versiones, versiones, volver -->

Haga clic en **Archivo > Información** para ver los datos del libro y protegerlo antes de compartirlo. Presione **Esc** para volver al libro.

## Dónde está el archivo

En la parte superior, la página muestra dónde está guardado el archivo. Haga clic en **Mostrar en carpeta** para abrir esa carpeta. Haga clic en **Copiar ruta** para copiar la ruta y pegarla en un correo electrónico o en un chat.

## Propiedades

Las propiedades son datos del libro, como su título y quién lo escribió. La página tiene un cuadro para cada una: **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Empresa** y **Administrador**. Escriba en un cuadro y presione **Tab** o **Entrar**. JeffCrunch conserva el cambio y lo escribe en el archivo la próxima vez que guarde el libro. La página también muestra cuándo se creó, se guardó y se imprimió el libro.

Haga clic en **Propiedades personalizadas** para crear propiedades propias, como **Proyecto** o **Cliente**. Haga clic en **Agregar**, escriba un nombre, elija el tipo de valor (texto, un número entero, un número, sí o no, o una fecha) y escriba el valor. JeffCrunch las guarda tal como las escribió.

## Marcar como final

Haga clic en **Marcar como final** para indicar a los lectores que el libro está terminado. JeffCrunch guarda el libro y después lo mantiene solo para lectura. Cuando alguien lo abre, la barra de título indica que es de solo lectura. Es una forma suave de decir «por favor, no cambie esto». No es un bloqueo, porque cualquiera puede elegir **Marcado como final** de nuevo para editarlo.

## Abrir siempre como solo lectura

Haga clic en **Abrir siempre como solo lectura** para que el libro se abra cada vez solo para lectura, de modo que nadie lo cambie por error. La opción se guarda con el archivo al guardarlo. La próxima vez que alguien abra el archivo, la barra de estado indicará que está abierto solo para lectura. Para editarlo, abra **Archivo > Información** y haga clic en **Se abre siempre como solo lectura**. Basta un clic, así que es un recordatorio y no un bloqueo. Otros programas no leen esta opción. Un archivo de texto, un archivo .xls y un archivo .ods no pueden guardarla, así que guarde primero el libro como archivo .xlsx.

## Cifrar con contraseña

Haga clic en **Cifrar con contraseña** para pedir una contraseña cada vez que se abra el archivo. Es lo mismo que **Proteger con contraseña**. Consulte [Abrir libros protegidos con contraseña](open-password). Una contraseña protege un archivo **.xlsx** con una protección sólida. Si olvida la contraseña, **no se puede recuperar**. Las versiones que guardó y la copia de seguridad siguen la contraseña.

Para impedir cambios en una hoja o en su diseño, consulte [Proteger una hoja o el libro](protect).

## Proteger la hoja o las hojas

**Proteger hoja actual** le permite elegir qué pueden cambiar los usuarios en la hoja en la que se encuentra, con una contraseña si lo desea. **Proteger estructura del libro** impide agregar, eliminar, mover, ocultar o cambiar el nombre de las hojas. Ambas opciones también están en la ficha **Revisar**.

## Historial de versiones

**Historial de versiones** guarda copias de su libro en este equipo. No se envía nada a ningún lugar. Cada copia se llama versión. Guarde una antes de un cambio importante.

- **Guardar una versión** guarda el libro tal como está ahora, con el nombre que elija.
- **Abrir una copia** abre una versión en una ventana propia, para que pueda verla junto al libro.
- **Volver a esta versión** restaura una versión como su libro. El libro tal como está ahora se guarda primero como versión, así que no se pierde nada.
- **Eliminar** descarta una versión.

El libro debe estar guardado antes, porque las versiones pertenecen a un archivo. Volver a una versión funciona con un archivo .xlsx.

## Inspeccionar libro

Haga clic en **Inspeccionar libro** antes de enviar un libro a alguien. Muestra qué contiene el archivo que quizá no quiera compartir, y cuántos elementos de cada tipo encontró:

- **Notas** son las notas que otros dejaron en las celdas. **Quitar todo** las elimina.
- **Propiedades del documento e información personal** son el autor, la persona que lo guardó por última vez, la empresa y el administrador. **Quitar todo** los borra.
- **Hojas ocultas** son hojas que no aparecen como pestañas. **Mostrar todo** las muestra, para que vea su contenido y elimine las que no quiera.
- **Filas y columnas ocultas** son líneas que ocultó manualmente. Las filas ocultas por un filtro no se cuentan. **Mostrar todo** las muestra, para que vea su contenido.
- **Encabezados y pies de página** son el texto que se imprime en la parte superior e inferior de cada página. **Quitar todo** los borra en todas las hojas.
- **Vínculos externos** son vínculos a valores de otros libros. **Quitar todo** convierte cada celda que lee uno en el número o texto que muestra ahora, y omite los vínculos al guardar. Un nombre que representaba un lugar de otro libro se convierte en **#REF!**, un error que indica que el lugar ya no existe. Esto no se puede deshacer, así que guarde antes una copia si quizá necesite los vínculos.
- **Datos XML personalizados** son datos que otros programas agregaron al archivo y que no se ven en una hoja. **Quitar todo** los omite al guardar.

Guarde el libro después para conservar la copia limpia.

**Quitar información personal de este archivo al guardarlo**, debajo de las tarjetas, lo hace cada vez que guarda, en cualquier archivo, sin cambiar el documento que tiene abierto. Es la misma casilla que en **Opciones**, y es igual en todas las aplicaciones de JeffOffice.

## Comprobar compatibilidad con .csv

Haga clic en **Comprobar compatibilidad con .csv** para ver qué se perdería de su libro en un archivo .csv. Un archivo .csv solo guarda los valores de una hoja. La lista indica las demás hojas, las fórmulas (el archivo guarda sus resultados), las imágenes, los gráficos, las notas, las tablas dinámicas y el resto de elementos del libro, y siempre los formatos de número, las fuentes, los colores y los anchos de columna. También enumera las opciones guardadas en las propiedades del archivo, como Marcar como final y Abrir siempre como solo lectura. No guarda nada. Elija **Guardar como .xlsx** al guardar para conservar todo.

## Comprobar compatibilidad

Haga clic en **Comprobar compatibilidad** para ver qué no conservaría un archivo más antiguo, un archivo .xls. Enumera las partes del libro que se omitirían o se recortarían para ajustarse, como gráficos, notas o filas que superan el límite antiguo. No guarda nada y su libro permanece como está.
