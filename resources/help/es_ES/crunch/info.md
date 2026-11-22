# Info: propiedades, contraseñas e Inspeccionar

<!-- keywords: información, propiedades, título, autor, asunto, palabras clave, categorías, comentarios, empresa, administrador, propiedades personalizadas, marcar como final, final, solo lectura, contraseña, cifrar, proteger, inspeccionar libro, información personal, hojas ocultas, notas, vínculos externos, XML personalizado, mostrar en la carpeta, copiar ruta, carpeta, ruta, historial de versiones, versiones, volver -->

Haga clic en **Archivo > Información** para ver los datos del libro y protegerlo antes de compartirlo. Pulse **Esc** para volver al libro.

## Dónde está el fichero

En la parte superior, la página muestra dónde está guardado el fichero. Haga clic en **Mostrar en la carpeta** para abrir esa carpeta. Haga clic en **Copiar ruta** para copiar la ruta y pegarla en un correo electrónico o en un chat.

## Propiedades

Las propiedades son datos del libro, como su título o quién lo escribió. La página tiene un cuadro para cada una: **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Empresa** y **Administrador**. Escriba en un cuadro y pulse **Tab** o **Entrar**. JeffCrunch conserva el cambio y lo escribe en el fichero la próxima vez que guarde el libro. La página también muestra cuándo se creó, se guardó y se imprimió el libro.

Haga clic en **Propiedades personalizadas** para crear propiedades propias, como **Proyecto** o **Cliente**. Haga clic en **Agregar**, escriba un nombre, elija el tipo de valor (texto, número entero, número, sí o no, o fecha) y escriba el valor. JeffCrunch las guarda tal como las escribió.

## Marcar como final

Haga clic en **Marcar como final** para indicar a los lectores que el libro está terminado. JeffCrunch guarda el libro y lo deja abierto solo para lectura. Cuando alguien lo abre, la barra de título indica que es de solo lectura. Es una forma suave de decir «por favor, no cambie esto». No es un bloqueo, porque cualquiera puede elegir **Marcado como final** de nuevo para editarlo.

## Abrir siempre como solo lectura

Haga clic en **Abrir siempre como solo lectura** para que el libro se abra solo para lectura cada vez, de modo que nadie lo cambie por error. La opción se guarda con el fichero al guardarlo. La próxima vez que alguien abra el fichero, la barra de estado indicará que está abierto solo para lectura. Para editarlo, abra **Archivo > Información** y haga clic en **Se abre siempre como solo lectura**. Basta un clic, así que es un recordatorio y no un bloqueo. Otros programas no leen esta opción. Un fichero de texto, un fichero .xls y un fichero .ods no pueden guardarla, así que guarde primero el libro como fichero .xlsx.

## Cifrar con contraseña

Haga clic en **Cifrar con contraseña** para pedir una contraseña cada vez que se abra el fichero. Es lo mismo que **Proteger con contraseña**. Consulte [Abrir libros protegidos con contraseña](open-password). Una contraseña protege un fichero **.xlsx** con una protección sólida. Si olvida una contraseña, **no se puede recuperar**. Las versiones que conservó y la copia de seguridad siguen la contraseña.

Para impedir cambios en una hoja o en su diseño, consulte [Proteger una hoja o el libro](protect).

## Proteger la hoja o las hojas

**Proteger hoja actual** le permite elegir qué pueden cambiar los usuarios en la hoja en la que está, con una contraseña si lo desea. **Proteger estructura del libro** impide agregar, eliminar, mover, ocultar o cambiar el nombre de las hojas. Ambas opciones también están en la ficha **Revisar**.

## Historial de versiones

**Historial de versiones** guarda copias del libro en este equipo. No se envía nada a ningún sitio. Cada copia se llama versión. Conserve una antes de hacer un cambio importante.

- **Guardar una versión** guarda el libro tal como está ahora, con el nombre que elija.
- **Abrir una copia** abre una versión en una ventana propia, para que pueda verla junto al libro.
- **Volver a esta versión** restaura una versión como su libro. El libro tal como está ahora se guarda antes como versión, para que no se pierda nada.
- **Eliminar** descarta una versión.

El libro debe estar guardado antes, porque las versiones pertenecen a un fichero. Volver a una versión funciona con un fichero .xlsx.

## Inspeccionar libro

Haga clic en **Inspeccionar libro** antes de enviar un libro a alguien. Muestra lo que contiene el fichero y que quizá no desee compartir, y cuántos elementos de cada tipo encontró:

- **Notas** son las notas que dejaron personas en las celdas. **Quitar todo** las elimina.
- **Propiedades del documento e información personal** son el autor, la persona que lo guardó por última vez, la empresa y el administrador. **Quitar todo** las borra.
- **Hojas ocultas** son hojas que no aparecen como pestañas. **Mostrar todo** las muestra para que vea su contenido y elimine las que no quiera.
- **Filas y columnas ocultas** son líneas que ocultó a mano. Las filas ocultas por un filtro no se cuentan. **Mostrar todo** las muestra para que vea su contenido.
- **Encabezados y pies de página** son el texto que se imprime en la parte superior e inferior de cada página. **Quitar todo** los borra en todas las hojas.
- **Vínculos externos** son vínculos a valores de otros libros. **Quitar todo** convierte cada celda que lee uno en el número o texto que muestra ahora, y omite los vínculos al guardar. Un nombre que representaba un lugar de otro libro pasa a ser **#REF!**, un error que indica que ese lugar ya no existe. Esta acción no se puede deshacer, así que guarde una copia antes si quizá necesite los vínculos después.
- **Datos XML personalizados** son datos que otros programas añadieron al fichero y que no se ven en una hoja. **Quitar todo** los omite al guardar.

Guarde el libro después para conservar la copia limpia.

**Quitar información personal de este fichero al guardarlo**, debajo de las tarjetas, lo hace cada vez que guarda, en cualquier fichero, sin cambiar el documento que tiene abierto. Es la misma casilla que aparece en **Opciones**, y es igual en todas las aplicaciones de JeffOffice.

## Comprobar compatibilidad para .csv

Elija **.csv** en **Comprobar compatibilidad** para ver qué perdería un fichero .csv de su libro. Un fichero .csv contiene solo los valores de una hoja. La lista indica las otras hojas, las fórmulas (el fichero guarda sus resultados), las imágenes, los gráficos, las notas, las tablas dinámicas y el resto de elementos del libro, los nombres definidos y las tablas, y siempre todo lo que no sea un valor, como formatos de número, fuentes, colores, bordes, anchos de columna, celdas combinadas, paneles inmovilizados, filtros, formato condicional, validación de datos, vínculos y opciones de impresión. También indica las opciones guardadas en las propiedades del fichero, como Marcar como final y Abrir siempre como solo lectura. No guarda nada. Elija **Guardar como .xlsx** al guardar para conservarlo todo.

## Comprobar compatibilidad

Haga clic en **Comprobar compatibilidad** y elija **.xls**, **.ods** o **.csv** para ver qué no conservaría ese tipo de fichero. Muestra las partes del libro que se omitirían o se recortarían para caber, como gráficos, notas o filas que superan el límite antiguo, y las opciones guardadas en el propio fichero, Marcar como final y Abrir siempre como solo lectura, cuando estén activadas. No guarda nada y su libro sigue igual.
