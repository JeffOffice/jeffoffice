# Buscar, reemplazar e ir a

<!-- keywords: buscar, reemplazar, cambiar, búsqueda, comodín, asterisco, signo de interrogación, tilde, mismas mayúsculas, celda completa, ir a, referencia, nombre, seleccionar por tipo, celdas vacías, constantes, fórmulas, errores, región actual, matriz actual, diferencias de fila, diferencias de columna, última celda, celdas visibles, formatos condicionales, validación, Ctrl+F, Ctrl+H, F5, notas -->

## Buscar

Pulse **Ctrl+F** (o elija **Buscar** en la pestaña Inicio y, después, **Buscar**). Escriba el texto y pulse **Entrar** para ir a la siguiente celda que lo contenga, o elija **Buscar todos** para obtener una lista de todas las celdas que coinciden. Haga clic en una línea de la lista para ir a esa celda.

| Opción | Qué hace |
| --- | --- |
| **Dónde** | Busca en esta hoja o en todas las hojas visibles. Si hay más de una celda seleccionada, una búsqueda en una hoja solo mira en ellas. |
| **Orden** | Recorre las celdas por filas (de izquierda a derecha y, después, hacia abajo) o por columnas (de arriba abajo y, después, hacia la derecha). **Siguiente** se detiene en la primera coincidencia según ese orden. |
| **Buscar en** | **Fórmulas** busca en lo escrito en cada celda, **Valores** en lo que muestra, y **Notas** en las notas de las celdas. |
| **Coincidir mayúsculas y minúsculas** | Las letras mayúsculas y minúsculas deben ser iguales. |
| **Coincidir con el contenido de toda la celda** | El contenido completo de la celda debe ser el texto, no solo una parte de él. |

En el texto que busca, **?** representa cualquier carácter y **\*** cualquier secuencia de caracteres (incluida ninguna). Escriba **~** delante de ?, \* o ~ para buscar el propio signo: **a~\*b** coincide con a\*b.

La búsqueda se ejecuta poco a poco, para que pueda seguir trabajando. **Esc** (o **Cancelar**) la detiene, y también la detiene cambiar una celda. Un texto muy largo cuesta más en cada paso que uno corto, así que una hoja con textos largos deja igualmente libre la ventana.

Las notas de las celdas siguen a las celdas: después de ordenar, o después de insertar o eliminar filas o columnas, una búsqueda en notas encuentra cada nota en la celda en la que está ahora. Una nota de una celda que se eliminó deja de aparecer en la búsqueda (y vuelve al deshacer). Si un libro se ha modificado de tantas formas que las notas no pueden seguirse por todas ellas, se le pedirá que lo guarde primero; un libro abierto desde un tipo de fichero más antiguo o desde un fichero de texto tiene sus notas en un formato que JeffCrunch no busca, y se le indicará.

## Reemplazar

Pulse **Ctrl+H**. **Cambiar** pone el nuevo texto en la celda con el cursor si la contiene y pasa a la siguiente. **Cambiar todo** lo hace en todas las celdas que lo contienen, como un único paso que puede deshacer; un cambio grande se aplica poco a poco. El nuevo texto se introduce como si lo hubiera escrito usted: un número se convierte en número, y una fórmula modificada sigue siendo una fórmula.

No se cambia nada, y se le indica el motivo, si una celda que se cambiaría está bloqueada en una hoja protegida, forma parte de una matriz o de una tabla de datos, está en la primera fila de una tabla o dejaría de ser una fórmula.

## Ir a

Pulse **F5** (o **Ctrl+G**), o escriba en el cuadro de nombres. Puede indicar una celda, como B7, un rango, como B3:D9, columnas o filas completas (C:E, 4:9), una celda de otra hoja (Hoja2!A1, o 'Mi hoja'!A1), o el nombre de un rango.

## Seleccionar por tipo

Elija **Seleccionar por tipo** en el menú **Buscar** para seleccionar celdas según lo que contienen:

- **Celdas en blanco**, **Constantes** (escritas), **Fórmulas** y **Celdas que muestran un error**. Para constantes y fórmulas puede excluir números, texto, VERDADERO y FALSO, o errores.
- **El bloque alrededor de la celda con el cursor** y **La última celda utilizada**.
- **La fórmula matricial en la que está el cursor** selecciona todas las celdas de esa fórmula matricial.
- **Celdas que difieren de la celda del cursor en su fila** y **en su columna** encuentran las celdas que no son iguales que la celda con el cursor. Seleccione primero un bloque y coloque el cursor en la celda con la que deben coincidir las demás. En cada fila (o columna) del bloque se elige una celda cuando contiene un número o texto distinto, o cuando contiene una fórmula que no es la misma una vez desplazadas sus referencias de celda hasta la posición de esa celda. También se elige una celda con fórmula cuando la celda del cursor no tiene ninguna. Si selecciona solo una celda, JeffCrunch mira el bloque alrededor de ella.
- **Celdas que muestran** (no en filas o columnas ocultas), **Celdas con formato condicional** y **Celdas con una regla sobre lo que se puede introducir** (todas, o solo las que tienen la misma regla que la celda con el cursor).

El primer bloque encontrado queda seleccionado y los demás se tiñen; el tinte desaparece al hacer clic en una celda.
