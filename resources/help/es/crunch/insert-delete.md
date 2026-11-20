# Insertar y eliminar filas, columnas y celdas
<!-- keywords: insertar celdas cortadas, insertar celdas copiadas, desplazar a la derecha, desplazar hacia abajo, insertar, eliminar, quitar, fila, columna, celda, desplazar, mover, agregar, en blanco, Ctrl+más, Ctrl+menos, clic derecho, encabezado, deshacer, referencia, #REF!, fórmula, nombre, combinada, combinadas, tabla, matriz, borde -->

Puede agregar filas, columnas o celdas vacías en medio de sus datos, y quitar filas, columnas o celdas. Todo lo que hay debajo o a la derecha se desplaza para dejar espacio o para cerrar el hueco, y las fórmulas que apuntan a las celdas que se movieron siguen apuntando a ellas.

## Insertar

1. Seleccione los números de fila, las letras de columna o las celdas donde desea las nuevas. Para agregar tres filas, seleccione tres filas.
2. Presione **Ctrl y la tecla más** (**Ctrl y la tecla menos** elimina), o use **Inicio > Insertar**, o haga clic derecho en la selección y elija **Insertar**.

Si selecciona filas o columnas completas, las nuevas se insertan de una vez, encima de las filas o a la izquierda de las columnas. Si selecciona celdas, aparece una pequeña ventana que pregunta cómo deben desplazarse las celdas que están después:

- **Desplazar celdas hacia la derecha** o **Desplazar celdas hacia abajo** mueve solo las celdas de las columnas (o filas) de su selección, y deja el resto de la hoja donde está.
- **Fila completa** o **Columna completa** agrega filas o columnas enteras.

## Insertar celdas cortadas o copiadas

Para colocar celdas que cortó o copió entre otras celdas, en lugar de sobre ellas, primero córtelas o cópielas (**Ctrl+X** o **Ctrl+C**). Después seleccione la celda donde deben ir y use **Inicio > Insertar > Insertar celdas cortadas o copiadas** (el comando dice **Insertar celdas cortadas** o **Insertar celdas copiadas** cuando hay algo que colocar), o haga clic derecho en la celda y elija ese comando. Una pequeña ventana pregunta si las celdas que están ahí deben **desplazarse a la derecha** o **hacia abajo** para dejar espacio.

El botón **Insertar** y **Ctrl y la tecla más** (también **Ctrl+Mayús y la tecla más**) hacen lo mismo, pero solo cuando está claro lo que quiere hacer:

- cuando las celdas están **cortadas** y el corte sigue pendiente (se ve su borde de línea discontinua) en la hoja de origen, y
- cuando las celdas están **copiadas** y no ha cambiado nada desde entonces (sin escribir, sin otra edición, sin Deshacer ni Rehacer).

En cualquier otro caso, el botón y las teclas insertan celdas en blanco, como siempre. Después de cambiar algo, la copia sigue disponible para **Pegar** y para el comando del menú. **Esc** termina un corte o una copia: desaparece el borde, y el botón y las teclas vuelven a insertar celdas en blanco. Si corta celdas en una hoja y presiona las teclas en otra, se insertan celdas en blanco; el comando del menú le indica que las celdas cortadas solo pueden colocarse entre otras en su propia hoja. Si hay filas o columnas completas seleccionadas, el comando está desactivado y Insertar agrega filas o columnas como antes.

- Las celdas cortadas **se mueven**: dejan su lugar anterior, y lo que apunta a ellas las sigue. Es lo mismo que mantener **Mayús** mientras arrastra el borde de la selección, y es todo o nada: si se rechaza alguna parte, no cambia nada.
- Las celdas copiadas **se colocan** como copia, tal como lo haría **Pegar** (según indica **Establecer pegado predeterminado**, y con las fórmulas ajustadas a su nueva posición), y la copia queda lista para volver a colocarla en otro lugar.
- Las celdas se insertan una sola vez, en la celda superior izquierda de lo que seleccionó, aunque haya seleccionado más celdas de las que copió.
- Si la lista está filtrada, las celdas se colocan en las filas visibles, como lo haría un pegado, y el espacio creado incluye las filas ocultas por el filtro que quedan en medio. Una fila que ocultó a mano cuenta como cualquier otra fila.
- Cada operación es un paso que puede deshacer. Las celdas cortadas solo pueden colocarse entre otras en la hoja de la que proceden; para moverlas a otra hoja, use **Pegar**.
- Una celda no puede colocarse dentro de sí misma (las celdas cortadas de A3:A4 no pueden ir entre A3 y A4), y una hoja protegida no lo permite. JeffCrunch le indica el motivo y no cambia nada.

## Eliminar

Seleccione lo que desea quitar y presione **Ctrl y la tecla menos**, o use **Inicio > Eliminar**. Si hay celdas seleccionadas, la ventana pregunta si desea **desplazar celdas hacia la izquierda**, **desplazar celdas hacia arriba**, o eliminar la **fila completa** o la **columna completa**. Eliminar es un solo paso, y **Deshacer** devuelve todo lo que se quitó, incluidas las fórmulas.

## Qué sucede después

Una fórmula que apunta a celdas que se mueven apunta a ellas en sus nuevas posiciones. Ejemplo: en B1 tiene **=SUMA(A1:A5)**. Si inserta una fila encima de la fila 3, la fórmula pasa a ser **=SUMA(A1:A6)**, porque el rango creció para incluir la nueva fila. Si elimina la fila 3, pasa a ser **=SUMA(A1:A4)**. Una fórmula que apuntaba a una celda eliminada muestra **#REF!**, lo que significa que la celda que necesita ya no existe. **Deshacer** devuelve la celda y la fórmula.

También siguen a las celdas: las fórmulas de todas las hojas, los nombres (como las áreas de impresión y los títulos de impresión), las celdas combinadas, las celdas a las que se aplican las reglas de validación y los formatos condicionales, las tablas, los gráficos, las imágenes, las notas, los hipervínculos y los saltos de página.

## Lo que no se puede hacer

- No se pueden insertar ni eliminar celdas en una parte de una **fórmula matricial** o de una tabla de datos. Muévalas completas o no las toque. Tampoco se puede eliminar una celda que una tabla de datos lee como entrada.
- Una **tabla dinámica** no se puede mover ni cortar todavía: no se pueden insertar ni eliminar filas, columnas o celdas cuando eso la moviera o la cambiara, y no se pueden cortar celdas de ella ni colocar celdas sobre ella. El resto de la hoja se puede cambiar como de costumbre. Una tabla dinámica se modifica con los comandos de la pestaña **Tabla dinámica** (actualizar, cambiar el origen de datos, campos y diseño), no escribiendo en sus celdas.
- Una **tabla** no se puede cortar a lo ancho: no puede insertar una columna dentro de ella, ni eliminar su fila de encabezado o todas sus filas. Inserte o elimine filas dentro de ella, o cambie las celdas que la rodean.
- Una **hoja protegida** no permite insertar ni eliminar filas, columnas o celdas. Para hacerlo, elija Desproteger hoja en la pestaña Revisar.
- Las celdas al final de la hoja que tienen contenido no pueden empujarse fuera del borde. Elimínelas primero.
- Después de guardar, **Deshacer** sigue deshaciendo lo que hizo antes, incluidas las filas y columnas que insertó (el siguiente guardado vuelve a quitarlas). Lo único que no puede devolver son las filas, columnas o celdas que **eliminó**: el archivo guardado ya perdió lo que había en ellas, así que **Deshacer** se detiene en la última eliminación guardada, y la línea de la parte inferior de la ventana lo indica al guardar.
- Un paso que mueve o elimina tantas celdas que **Deshacer** no puede almacenarlo (más de unos 128 MB) pregunta antes, como hacen los programas de hojas de cálculo conocidos: «Esto no se puede deshacer». Si continúa, se realiza, y **Deshacer** no retrocede ni sobre esto ni sobre lo anterior.
