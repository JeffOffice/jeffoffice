# Cortar, copiar y pegar
<!-- keywords: cortar, copiar, pegar, portapapeles, mover, duplicar, pegado especial, valores, fórmulas, formatos, trasponer, transponer, saltar blancos, sumar, restar, multiplicar, dividir, operación, combinadas, texto, tabulación, html, página web, otro programa, Esc, borde intermitente, hormigas, borde -->

**Copiar** (**{key:Copy}**) coloca una copia de las celdas seleccionadas en el Portapapeles, un espacio donde el equipo guarda un solo elemento cada vez. **Pegar** (**{key:Paste}**) coloca el contenido del Portapapeles en las celdas a partir de la celda seleccionada. **Cortar** (**{key:Cut}**) es como copiar, pero al pegar, las celdas se mueven y su ubicación original queda vacía.

Un borde discontinuo en movimiento indica las celdas que se han copiado o cortado. Presione **Esc** para quitar el borde; a partir de ese momento, las celdas ya no están listas para pegarse. Las celdas cortadas también se liberan si realiza cualquier otro cambio antes de pegarlas (escribir, insertar, deshacer): vuelva a cortarlas si todavía quiere moverlas.

## Pegar dentro de JeffCrunch

Se pega todo: los valores, las fórmulas, los formatos y las celdas combinadas. Una fórmula sigue haciendo referencia a las mismas posiciones *en relación con el lugar donde se encuentra*. Ejemplo: si copia **=A1*2** de B1 y la pega en B5, se convierte en **=A5*2**. Una parte con el signo de dólar, como **$A$1**, no cambia. Si selecciona un bloque cuyo tamaño es un múltiplo exacto de lo que copió, la copia se repite hasta rellenarlo: copie una celda, seleccione veinte celdas, pegue y las veinte la recibirán.

Si se pega un número demasiado ancho para una columna que conserva su ancho original, la columna se ensancha.

## Cortar mueve las celdas

Al cortar y pegar celdas, estas se mueven realmente. Todas las fórmulas que hacían referencia a ellas pasan a hacer referencia a su nueva ubicación. Ejemplo: B1 contiene **=A1+A2**. Si corta A1:A2 y lo pega en E4, B1 se convierte en **=E4+E5**. El contenido de las celdas sobre las que pega se pierde (**Deshacer** lo recupera), y una fórmula que leía una de esas celdas muestra **#¡REF!**, porque la celda que leía ya no existe. Si las celdas que corta son solo una parte de una celda combinada, o si pega solo sobre una parte de una, no se mueve nada: seleccione la celda combinada completa. Al cortar celdas y pegarlas en otra hoja, las fórmulas siguen leyendo las mismas celdas que antes (indican de qué hoja se trata). El Portapapeles se vacía después de cortar y pegar.

## Pegado especial

**Ctrl+Alt+V**, o la flecha situada debajo de **Pegar** en la pestaña **Inicio**, le permite elegir qué pegar:

- **Todo**, **Fórmulas**, **Valores** o **Formatos**. *Valores* convierte una fórmula en el número o texto que muestra.
- Una **operación**: **Sumar**, **Restar**, **Multiplicar** o **Dividir**. Los números copiados se combinan con los números que ya contienen las celdas. Ejemplo: las celdas contienen 10 y 20, ha copiado 5 y ha elegido *Multiplicar*: pasan a ser 50 y 100.
- **Saltar blancos**: las celdas vacías de la copia dejan sin cambios las celdas de destino.
- **Trasponer**: las filas se convierten en columnas y las columnas en filas.

## Entre programas

Al copiar, JeffCrunch también coloca las celdas en el Portapapeles como texto (celdas separadas por tabulaciones, una fila por línea) y como tabla para páginas web, de modo que otros programas puedan usarlas. Al pegar algo que no procede de JeffCrunch, como texto con tabulaciones o una tabla de una página web o de otra hoja de cálculo, cada elemento se coloca en su propia celda y se interpreta como si lo hubiera escrito usted: números, fechas y fórmulas que empiezan por el signo igual. Al copiar desde otro libro abierto en JeffCrunch se pegan los valores, las fórmulas y los formatos de número, pero no el resto del formato.

## Qué no se puede pegar

JeffCrunch le avisa y no cambia nada si el pegado afectaría a una parte de una **fórmula de matriz**, a la fila de encabezado de una **tabla**, a celdas bloqueadas de una **hoja protegida**, a zonas más allá del final de la hoja o a más de 100.000 celdas. Una **fórmula de matriz** completa se puede cortar y mover (seleccione todas sus celdas); en cambio, no se puede hacer con una parte de una fórmula de matriz, con una tabla de datos ni con la fila de encabezado de una tabla.
