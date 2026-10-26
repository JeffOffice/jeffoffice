# Insertar y eliminar filas, columnas y celdas
<!-- keywords: insertar, eliminar, borrar, quitar, fila, columna, celda, desplazar, mover, agregar, añadir, en blanco, vacía, Ctrl+más, Ctrl+menos, clic derecho, menú contextual, encabezado, deshacer, referencia, #REF!, fórmula, nombre, combinadas, tabla, matriz, borde -->

Puede agregar filas, columnas o celdas vacías en medio de los datos y quitar filas, columnas o celdas. Todo lo que está detrás se desplaza para hacer sitio o para cerrar el hueco, y las fórmulas que hacen referencia a las celdas desplazadas siguen haciendo referencia a ellas.

## Insertar

1. Seleccione los números de fila, las letras de columna o las celdas donde quiere las nuevas. Para agregar tres filas, seleccione tres filas.
2. Presione **Ctrl y la tecla más** (**Ctrl y la tecla menos** elimina), o use **Inicio > Insertar**, o haga clic con el botón derecho en la selección y elija **Insertar**.

Si hay filas o columnas completas seleccionadas, las nuevas se insertan de inmediato, encima de las filas o a la izquierda de las columnas. Si hay celdas seleccionadas, una pequeña ventana pregunta cómo deben desplazarse las celdas que van detrás:

- **Desplazar las celdas hacia la derecha** o **Desplazar las celdas hacia abajo** desplaza solo las celdas de las columnas (o filas) de la selección y deja el resto de la hoja donde está.
- **Toda la fila** o **Toda la columna** agrega filas o columnas completas.

## Eliminar

Seleccione lo que quiere quitar y presione **Ctrl y la tecla menos**, o use **Inicio > Eliminar**. Si hay celdas seleccionadas, la ventana pregunta si desea **desplazar las celdas hacia la izquierda**, **desplazar las celdas hacia arriba** o eliminar **toda la fila** o **toda la columna**. Eliminar es un solo paso, y **Deshacer** restaura todo lo que se quitó, incluidas las fórmulas.

## Qué ocurre después

Una fórmula que hace referencia a celdas que se desplazan pasa a hacer referencia a ellas en su nueva posición. Ejemplo: en B1 tiene **=SUMA(A1:A5)**. Si inserta una fila encima de la fila 3, la fórmula pasa a ser **=SUMA(A1:A6)**, porque el rango se amplió para incluir la nueva fila. Si elimina la fila 3, pasa a ser **=SUMA(A1:A4)**. Una fórmula que hacía referencia a una celda eliminada muestra **#REF!**, lo que significa que la celda que necesita ya no existe. **Deshacer** recupera la celda y la fórmula.

También siguen a las celdas: las fórmulas de todas las hojas, los nombres (como las áreas de impresión y los títulos de impresión), las celdas combinadas, las celdas a las que se aplican las reglas de validación y los formatos condicionales, las tablas, los gráficos, las imágenes, las notas, los hipervínculos y los saltos de página.

## Qué no se puede hacer

- No se pueden insertar ni eliminar celdas que atraviesen parte de una **fórmula de matriz** o de una tabla de datos. Muévalas completas o no las toque. Tampoco se puede eliminar una celda que una tabla de datos lee como entrada.
- Una **tabla dinámica** todavía no se puede mover ni cortar: no se pueden insertar ni eliminar filas, columnas o celdas donde eso la movería o la modificaría, y no se pueden cortar celdas de ella ni pegarlas encima. El resto de la hoja se puede modificar como siempre.
- Una **tabla** no se puede cortar transversalmente: no se puede insertar una columna dentro de ella ni eliminar su fila de encabezado o todas sus filas. Inserte o elimine filas dentro de ella, o modifique las celdas que la rodean.
- En una **hoja protegida** no se pueden insertar ni eliminar filas, columnas o celdas. Para hacerlo, elija Desproteger hoja en la pestaña Revisar.
- Las celdas con contenido situadas al final de la hoja no se pueden empujar fuera del borde. Elimínelas primero.
- Después de guardar, **Deshacer** sigue pudiendo revertir lo que hizo antes, incluidas las filas y columnas insertadas (el siguiente guardado las vuelve a quitar). Lo único que no puede recuperar son las filas, columnas o celdas **eliminadas**: el archivo guardado ha perdido su contenido, por lo que **Deshacer** se detiene en la última eliminación guardada, y la línea de la parte inferior de la ventana lo indica al guardar.
- Un paso que mueve o elimina tantas celdas que **Deshacer** no podría almacenarlo (más de unos 128 MB) pide confirmación primero, como hacen las hojas de cálculo habituales: "Esta acción no se puede deshacer". Si continúa, se lleva a cabo y **Deshacer** no puede revertirlo ni revertir nada de lo anterior.
