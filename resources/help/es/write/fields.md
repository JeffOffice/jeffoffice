# Campos

<!-- keywords: campo, campos, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, actualizar campo, F9, desvincular, bloquear, códigos de campo, número de página, fecha, NUMPAGES, SEQ, REF, PAGEREF, fórmula, Insertar campo, editar campo, clic derecho -->

Un **campo** es un fragmento de texto que el programa calcula por usted, como un número de página, la fecha de hoy o el número de una figura. Tiene dos partes: un **código**, que indica qué calcular, y un **resultado**, que es el texto que ve. El código de un número de página es `PAGE`, y su resultado en la tercera página es `3`.

## Insertar un campo en el texto

1. Haga clic donde debe ir el campo.
2. Haga clic en **Insertar > Campo**.
3. Elija un campo de la lista. Su código aparece en el cuadro de abajo, y puede cambiarlo.
4. Haga clic en **Aceptar**.

Por ejemplo, elija **Fecha**, cambie el código a `DATE \@ "MMMM d, yyyy"` y haga clic en **Aceptar**. El texto muestra la fecha como `2 de noviembre de 2026`.

## Cambiar el código de un campo

Haga clic con el botón derecho en un campo y elija **Editar campo**. Un cuadro muestra el código. Cámbielo y haga clic en **Aceptar**, y el campo se vuelve a calcular. **Deshacer** restaura el campo anterior. Un campo bloqueado no se puede editar hasta que lo desbloquee. El menú del botón derecho también incluye **Actualizar campo**, **Alternar código de campo**, **Bloquear campo**, **Desbloquear campo** y **Desvincular campo**.

## Actualizar campos

Un campo muestra el resultado que tenía la última vez que se calculó. Pulse **F9** para volver a calcular el campo donde está el cursor. Para actualizar muchos campos a la vez, seleccione el texto que los contiene y pulse **F9**. Pulse **Ctrl+A** y después **F9** para actualizar todos los campos del documento.

Actualizar es un solo paso: **Deshacer** devuelve todos los resultados anteriores. Actualizar no es un cambio registrado, aunque el control de cambios esté activado, porque el resultado no es algo que usted haya escrito.

Un campo de un encabezado o pie de página que muestra el número de página, el número total de páginas o un título siempre está al día y no necesita actualizarse.

## Mostrar los códigos

Pulse **Mayús+F9** para mostrar el código del campo donde está el cursor, entre llaves, como `{ PAGE }`, en lugar de su resultado. Pulse **Alt+F9** para mostrar los códigos de todos los campos, y púlselo de nuevo para mostrar los resultados. Esto solo cambia lo que ve: el archivo se guarda tal como estaba y el documento no se marca como modificado.

## Desvincular o bloquear un campo

- **Ctrl+Mayús+F9** convierte el campo en texto normal. El resultado se conserva, pero ya no es un campo.
- **Ctrl+F11** bloquea un campo, para que **F9** no lo modifique. **Ctrl+Mayús+F11** lo desbloquea.

## Formatos

Añada modificadores al código para cambiar el aspecto de un resultado:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` y `\* Caps` cambian las mayúsculas o el tipo de número. `{ = 21 \* CardText }` muestra `veintiuno`.
- `\# "0.00"` define el aspecto de un número. `{ = 1234.5 \# "#,##0.00" }` muestra `1.234,50`.
- `\@ "dddd, MMMM d"` define el aspecto de una fecha. `M` es el mes, y `m` son los minutos.

## Campos que JeffWrite calcula

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, una fórmula que empieza por `=`, TOC (también una tabla de ilustraciones), INDEX, TOA, CITATION y BIBLIOGRAPHY. Las marcas ocultas TC, XE y TA no tienen resultado que calcular: indican a una tabla, un índice o una tabla de autoridades qué elementos incluir. Un campo de cualquier otro tipo conserva el resultado que tiene y se guarda tal como estaba.

DATE, TIME, FILENAME y AUTHOR dependen del equipo y del momento, así que pueden cambiar cada vez que actualice.
