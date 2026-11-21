# Buscar, reemplazar e ir a

<!-- keywords: buscar, reemplazar, cambiar, encontrar, buscar qué, reemplazar con, reemplazar todo, buscar siguiente, buscar, cambiar a, cambiar todo, mismas mayúsculas, comodín, asterisco, signo de interrogación, tilde, coincidir mayúsculas y minúsculas, celda completa, buscar por formato, formato, reemplazar en notas, ir a, referencia, nombre, seleccionar por tipo, espacios en blanco, constantes, fórmulas, errores, región actual, matriz actual, diferencias de fila, diferencias de columna, última celda, celdas visibles, formatos condicionales, validación, Ctrl+F, Ctrl+H, F5, notas -->

## Buscar

Presione **Ctrl+F** (o elija **Buscar** en la ficha Inicio y, después, **Buscar**). Escriba el texto en **Buscar qué** y presione **Entrar** para ir a la siguiente celda que lo contenga, o elija **Buscar todos** para obtener una lista de todas las celdas que coinciden. Haga clic en una línea de la lista para ir a esa celda.

| Opción | Qué hace |
| --- | --- |
| **Dónde** | Busca en esta hoja o en todas las hojas visibles. Si hay más de una celda seleccionada, una búsqueda en una hoja solo busca en ellas. |
| **Orden** | Recorre las celdas por filas (de izquierda a derecha y luego hacia abajo) o por columnas (de arriba abajo y luego hacia la derecha). **Buscar siguiente** se detiene en la primera coincidencia según ese orden. |
| **Buscar en** | **Fórmulas** busca en lo que se escribe en cada celda, **Valores** en lo que muestra, y **Notas** en las notas de las celdas. |
| **Formato…** | Busca solo las celdas que tienen un formato. **Formato…** abre el cuadro de diálogo Formato de celdas, iniciado desde la celda con el cursor: lo que cambie allí es lo que se busca (una fuente en negrita, un color de relleno, un formato de número). **Formato desde celda** toma todas las partes del formato de la celda con el cursor, y **Borrar formato** vuelve a buscar solo en el texto. El cuadro de diálogo indica qué se busca y avisa si el formato elegido no tiene nada. El aspecto que un formato condicional da a una celda no se busca, solo el formato que la celda tiene por sí misma. Si **Buscar qué** está vacío, **Buscar todos** y **Buscar siguiente** encuentran las celdas que tienen el formato; si hay texto, la celda debe tener ambas cosas. |
| **Coincidir mayúsculas y minúsculas** | Las letras mayúsculas y minúsculas deben coincidir. |
| **Celda completa** | La celda completa debe ser el texto, no solo una parte de él. |

En el texto que escribe en **Buscar qué**, **?** representa cualquier carácter y **\*** cualquier secuencia de caracteres (incluida ninguna). Escriba **~** delante de ?, \* o ~ para buscar el propio signo: **a~\*b** coincide con a\*b.

La búsqueda avanza poco a poco, para que pueda seguir trabajando. **Esc** (o **Cancelar**) la detiene, y también la detiene cambiar una celda. Un texto muy largo cuesta más en cada paso que uno corto, así que una hoja con textos largos deja igualmente libre la ventana.

Las notas de las celdas siguen a las celdas: después de ordenar, o de insertar o eliminar filas o columnas, una búsqueda en notas encuentra cada nota en la celda donde está ahora. Una nota de una celda eliminada deja de aparecer en la búsqueda (y vuelve al deshacer). Si un libro se ha modificado de tantas maneras que no se pueden seguir las notas, se le pedirá que lo guarde primero; un libro abierto desde un tipo de archivo más antiguo o desde un archivo de texto tiene sus notas en un formato que JeffCrunch no busca, y se le avisará.

## Reemplazar

Presione **Ctrl+H**, escriba el nuevo texto en **Reemplazar con** y elija un botón. **Reemplazar** pone el nuevo texto en la celda con el cursor si contiene el texto, y pasa a la siguiente. **Reemplazar todo** lo hace en todas las celdas que lo contienen, como un solo paso que puede deshacer; un cambio grande se aplica poco a poco. El nuevo texto se introduce como si lo escribiera usted: un número se convierte en número, y una fórmula modificada sigue siendo una fórmula.

Para reemplazar en notas, elija **Notas** en **Buscar en** de la página Reemplazar. **Reemplazar todo** cambia entonces las palabras de todas las notas que contengan el texto, en esta hoja o en todas, como un solo paso que puede deshacer; **Reemplazar** cambia la nota de la celda con el cursor y pasa a la siguiente. Una nota que es una conversación de comentarios se deja como está (cámbielos en la ficha Revisar), y se le indica cuántas se dejaron. No se cambia nada, y se le indica el motivo, si una hoja con una nota que cambiar está protegida, o si una nota quedaría sin palabras o con más de 32 767 caracteres. Una nota cuyas palabras se reemplazan pierde la fecha en que se escribió, como ocurre al editar sus palabras. Las notas se guardan con el libro, en todos los formatos que admiten notas. **Valores** se puede buscar, pero no reemplazar, porque un valor es el resultado de una fórmula.

No se cambia nada, y se le indica el motivo, si una celda que se cambiaría está bloqueada en una hoja protegida, forma parte de una matriz o de una tabla de datos, está en la primera fila de una tabla, o dejaría de ser una fórmula.

## Ir a

Presione **F5** (o **Ctrl+G**), o escriba en el cuadro de nombres. Puede indicar una celda, como B7; un rango, como B3:D9; columnas o filas completas (C:E, 4:9); una celda de otra hoja (Hoja2!A1, o 'Mi hoja'!A1), o el nombre de un rango.

## Seleccionar por tipo

Elija **Seleccionar por tipo** en el menú **Buscar** para seleccionar celdas según lo que contienen:

- **Celdas en blanco**, **Constantes** (escritas), **Fórmulas** y **Celdas que muestran un error**. En el caso de las constantes y las fórmulas, puede excluir números, texto, VERDADERO y FALSO, o errores.
- **El bloque alrededor de la celda con el cursor** y **La última celda utilizada**.
- **La fórmula matricial en la que está el cursor** selecciona todas las celdas de esa fórmula matricial.
- **Celdas que difieren de la celda del cursor en su fila** y **en su columna** encuentran las celdas que no son iguales a la celda con el cursor. Seleccione primero un bloque y coloque el cursor en la celda con la que deben coincidir las demás. En cada fila (o columna) del bloque, se toma una celda cuando contiene un número o texto distinto, o cuando contiene una fórmula que no es la misma una vez desplazadas sus referencias a la posición de esa celda. También se toma una celda que tiene una fórmula cuando la celda del cursor no tiene ninguna. Si selecciona una sola celda, JeffCrunch analiza el bloque alrededor de ella.
- **Celdas que muestran** (no en filas o columnas ocultas), **Celdas con formato condicional** y **Celdas con una regla de lo que se puede introducir** (todas, o solo las que tienen la misma regla que la celda con el cursor).

Se selecciona el primer bloque encontrado y los demás se resaltan con un tono; el tono desaparece al hacer clic en una celda.
