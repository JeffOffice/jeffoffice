# Buscar, reemplazar e ir a

<!-- keywords: buscar, reemplazar, cambiar, búsqueda, buscar qué, reemplazar con, reemplazar todo, buscar siguiente, buscar, cambiar a, cambiar todo, mismas mayúsculas, comodín, asterisco, signo de interrogación, tilde, coincidir mayúsculas y minúsculas, celda completa, ir a, referencia, nombre, seleccionar por tipo, espacios en blanco, constantes, fórmulas, errores, región actual, matriz actual, diferencias de fila, diferencias de columna, última celda, celdas visibles, formatos condicionales, validación, Ctrl+F, Ctrl+H, F5, notas -->

## Buscar

Pulse **Ctrl+F** (o elija **Buscar** en la ficha Inicio y, después, **Buscar**). Escriba el texto en **Buscar qué** y pulse **Entrar** para ir a la siguiente celda que lo contenga, o elija **Buscar todos** para obtener una lista de todas las celdas que coinciden. Haga clic en una línea de la lista para ir a esa celda.

| Opción | Qué hace |
| --- | --- |
| **Dónde** | Buscar en esta hoja o en todas las hojas visibles. Si hay más de una celda seleccionada, una búsqueda en una hoja solo mira en ellas. |
| **Orden** | Recorrer las celdas por filas (de izquierda a derecha y luego hacia abajo) o por columnas (de arriba abajo y luego hacia la derecha). **Buscar siguiente** se detiene en la primera coincidencia en ese orden. |
| **Buscar en** | **Fórmulas** busca en lo escrito en cada celda, **Valores** en lo que muestra, y **Notas** en las notas de las celdas. |
| **Coincidir mayúsculas y minúsculas** | Las letras mayúsculas y minúsculas deben ser iguales. |
| **Coincidir con el contenido de toda la celda** | El contenido completo de la celda debe ser el texto, no solo una parte de él. |

En el texto que escriba en **Buscar qué**, **?** representa cualquier carácter y **\*** cualquier secuencia de caracteres (incluida una secuencia vacía). Escriba **~** delante de ?, \* o ~ para buscar el propio signo: **a~\*b** coincide con a\*b.

La búsqueda se ejecuta poco a poco, para que pueda seguir trabajando. **Esc** (o **Cancelar**) la detiene, y también la detiene cambiar una celda. Un texto muy largo consume más en cada paso que uno corto, así que una hoja con textos largos sigue dejando libre la ventana.

Las notas de las celdas siguen a las celdas: después de ordenar, o después de insertar o eliminar filas o columnas, una búsqueda en notas encuentra cada nota en la celda donde está ahora. Una nota de una celda que se eliminó deja de aparecer en la búsqueda (y vuelve con Deshacer). Si un libro se ha modificado de tantas formas que no es posible seguir las notas en todas ellas, se le pedirá que lo guarde primero. Un libro abierto desde un tipo de fichero más antiguo o desde un fichero de texto tiene sus notas en un formato que JeffCrunch no busca, y así se le indicará.

## Reemplazar

Pulse **Ctrl+H**, escriba el nuevo texto en **Reemplazar con** y elija un botón. **Reemplazar** pone el nuevo texto en la celda con el cursor si contiene el texto, y pasa a la siguiente. **Reemplazar todo** lo hace en todas las celdas que lo contienen, como un único paso que puede deshacer; un cambio grande se aplica poco a poco. El nuevo texto se introduce como si lo hubiera escrito usted: un número se convierte en número, y una fórmula modificada sigue siendo una fórmula.

No se cambia nada, y se le indica el motivo, si una celda que se cambiaría está bloqueada en una hoja protegida, forma parte de una matriz o de una tabla de datos, está en la primera fila de una tabla, o dejaría de ser una fórmula.

## Ir a

Pulse **F5** (o **Ctrl+G**), o escriba en el cuadro de nombres. Puede indicar una celda, como B7; un rango, como B3:D9; columnas o filas completas (C:E, 4:9); una celda de otra hoja (Hoja2!A1, o 'Mi hoja'!A1), o el nombre de un rango.

## Seleccionar por tipo

Elija **Seleccionar por tipo** en el menú **Buscar** para seleccionar celdas según lo que contienen:

- **Celdas en blanco**, **Constantes** (escritas), **Fórmulas** y **Celdas que muestran un error**. Para las constantes y las fórmulas puede excluir números, texto, VERDADERO y FALSO, o errores.
- **El bloque alrededor de la celda con el cursor** y **La última celda utilizada**.
- **La fórmula matricial en la que está el cursor** selecciona todas las celdas de esa fórmula matricial.
- **Celdas que difieren de la celda del cursor en su fila** y **en su columna** encuentran las celdas que no son iguales que la celda con el cursor. Seleccione primero un bloque y coloque el cursor en la celda con la que deben coincidir las demás. En cada fila (o columna) del bloque se elige una celda cuando contiene un número o texto distinto, o cuando contiene una fórmula que no es la misma al desplazar sus referencias de celda hasta la posición de esa celda. Una celda con fórmula también se elige cuando la celda del cursor no tiene ninguna. Si selecciona solo una celda, JeffCrunch examina el bloque alrededor de ella.
- **Celdas que muestran** (no en filas o columnas ocultas), **Celdas con formato condicional** y **Celdas con una regla de entrada de datos** (todas, o solo las que tienen la misma regla que la celda con el cursor).

El primer bloque encontrado se selecciona y los demás se tiñen; el tinte desaparece al hacer clic en una celda.
