# Buscar, reemplazar e ir a
<!-- keywords: buscar, reemplazar, cambiar, búsqueda, comodín, asterisco, signo de interrogación, tilde, coincidir mayúsculas y minúsculas, toda la celda, ir a, referencia, nombre, seleccionar por tipo, celdas en blanco, vacías, constantes, fórmulas, errores, región actual, última celda, celdas visibles, formatos condicionales, validación, Ctrl+B, Ctrl+L, F5, notas -->

## Buscar

Presione **Ctrl+B** (o elija **Buscar** en la pestaña Inicio y, a continuación, **Buscar** de nuevo). Escriba el texto y presione **Entrar** para ir a la siguiente celda que lo contiene, o elija **Buscar todos** para obtener una lista de todas las celdas que coinciden. Haga clic en una línea de la lista para ir a esa celda.

| Opción | Qué hace |
| --- | --- |
| **Dentro de** | Busca en esta hoja o en todas las hojas visibles. Si hay más de una celda seleccionada, la búsqueda en una hoja se limita a esas celdas. |
| **Orden** | Recorre las celdas por filas (de izquierda a derecha y luego hacia abajo) o por columnas (de arriba abajo y luego hacia la derecha). **Siguiente** se detiene en la primera coincidencia en ese orden. |
| **Buscar en** | **Fórmulas** busca en lo que está escrito en cada celda, **Valores** en lo que muestra y **Notas** en las notas de las celdas. |
| **Coincidir mayúsculas y minúsculas** | Las mayúsculas y minúsculas deben coincidir. |
| **Coincidir con el contenido de toda la celda** | Toda la celda debe ser el texto, no solo una parte. |

En el texto que busca, **?** representa un carácter cualquiera y **\*** cualquier secuencia de caracteres (incluso ninguno). Escriba **~** delante de ?, \* o ~ para buscar el propio signo: **a~\*b** coincide con a\*b.

La búsqueda se realiza poco a poco, para que pueda seguir trabajando. **Esc** (o **Cancelar**) la detiene, igual que modificar una celda. Un texto muy largo ocupa más de cada pequeño paso que uno corto, por lo que una hoja con textos largos sigue dejando libre la ventana.

Las notas de las celdas siguen a las celdas: después de ordenar, o de insertar o eliminar filas o columnas, una búsqueda en las notas encuentra cada nota en la celda en la que está ahora. Una nota de una celda eliminada desaparece de la búsqueda (y vuelve tras Deshacer). Si un libro se ha modificado de tantas maneras que no se pueden seguir las notas a través de todos los cambios, se le pedirá que lo guarde primero; un libro abierto desde un tipo de archivo más antiguo o desde un archivo de texto tiene sus notas en un formato en el que JeffCrunch no busca, y se le avisará de ello.

## Reemplazar

Presione **Ctrl+L**. **Reemplazar** pone el texto nuevo en la celda del cursor si contiene el texto y pasa a la siguiente. **Reemplazar todos** lo hace en todas las celdas que lo contienen, en un solo paso que puede deshacer; un cambio grande se aplica poco a poco. El texto nuevo se introduce como si lo escribiera: un número se convierte en número, y una fórmula modificada sigue siendo una fórmula.

No se cambia nada, y se le indica el motivo, si una celda que se cambiaría está bloqueada en una hoja protegida, forma parte de una matriz o de una tabla de datos, está en la primera fila de una tabla o dejaría de ser una fórmula.

## Ir a

Presione **F5** (o **Ctrl+I**), o escriba en el Cuadro de nombres. Puede indicar una celda como B7, un rango como B3:D9, columnas o filas completas (C:E, 4:9), una celda de otra hoja (Hoja2!A1, o 'Mi hoja'!A1) o el nombre de un rango.

## Seleccionar por tipo

Elija **Seleccionar por tipo** en el menú **Buscar** para seleccionar celdas según lo que son:

- **Celdas en blanco**, **Constantes** (escritas), **Fórmulas** y **Celdas que muestran un error**. Para constantes y fórmulas, puede omitir números, texto, VERDADERO y FALSO, o errores.
- **El bloque alrededor de la celda con el cursor** y **La última celda utilizada**.
- **Celdas visibles** (que no están en filas o columnas ocultas), **Celdas con formato condicional** y **Celdas con una regla sobre lo que se puede introducir** (todas, o solo las que tienen la misma regla que la celda del cursor).

Se selecciona el primer bloque encontrado y los demás se tiñen; el tinte desaparece al hacer clic en una celda.
