# Campos
<!-- keywords: campo, campos, actualizar campo, F9, desvincular, bloquear, códigos de campo, número de página, fecha, NUMPAGES, SEQ, REF, PAGEREF, fórmula, Insertar campo, campo calculado -->

Un **campo** es un fragmento de texto que el programa calcula por usted, como un número de página, la fecha de hoy o el número de una figura. Tiene dos partes: un **código**, que indica qué se calcula, y un **resultado**, que es el texto que ve. El código de un número de página es `PAGE`, y su resultado en la tercera página es `3`.

## Insertar un campo en el texto

1. Haga clic donde debe ir el campo.
2. Haga clic en **Insertar > Campo**.
3. Elija un campo de la lista. Su código aparece en el cuadro de abajo, y puede modificarlo.
4. Haga clic en **Aceptar**.

Por ejemplo, elija **Fecha**, cambie el código a `DATE \@ "d 'de' MMMM 'de' yyyy"` y haga clic en **Aceptar**. El texto muestra la fecha como `2 de noviembre de 2026`.

## Actualizar campos

Un campo muestra el resultado que tenía la última vez que se calculó. Presione **F9** para volver a calcular el campo en el que está el cursor. Para actualizar muchos campos a la vez, seleccione el texto que los contiene y presione **F9**. Presione **Ctrl+A** y luego **F9** para actualizar todos los campos del documento.

La actualización es un solo paso: **Deshacer** restablece todos los resultados. La actualización no es un cambio controlado, aunque el control de cambios esté activado, porque el resultado no es algo que haya escrito usted.

Un campo de un encabezado o pie de página que muestra el número de página, el número total de páginas o un título siempre es correcto y no necesita actualizarse.

## Mostrar los códigos

Presione **Mayús+F9** para mostrar el código del campo en el que está el cursor, entre llaves, como `{ PAGE }`, en lugar de su resultado. Presione **Alt+F9** para mostrar los códigos de todos los campos, y vuelva a presionarlo para mostrar los resultados. Esto solo cambia lo que ve: el archivo se guarda tal como estaba y el documento no se marca como modificado.

## Desvincular o bloquear un campo

- **Ctrl+Mayús+F9** convierte el campo en texto normal. El resultado se mantiene, pero ya no es un campo.
- **Ctrl+F11** bloquea un campo, de modo que **F9** no lo modifica. **Ctrl+Mayús+F11** lo desbloquea.

## Formatos

Agregue modificadores al código para cambiar el aspecto del resultado:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` y `\* Caps` cambian las mayúsculas y minúsculas o el tipo de número. `{ = 21 \* CardText }` muestra `veintiuno`.
- `\# "0.00"` define el aspecto de un número. `{ = 1234.5 \# "#,##0.00" }` muestra `1.234,50`.
- `\@ "dddd, d 'de' MMMM"` define el aspecto de una fecha. `M` es el mes y `m` es el minuto.

## Campos que calcula JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, una fórmula que empiece por `=` y TOC. Un campo de cualquier otro tipo conserva el resultado que tiene y se guarda tal como estaba.

DATE, TIME, FILENAME y AUTHOR dependen del equipo y del momento, por lo que pueden cambiar cada vez que actualice.
