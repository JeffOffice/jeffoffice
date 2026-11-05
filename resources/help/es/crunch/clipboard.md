# Cortar, copiar y pegar
<!-- keywords: cortar, copiar, pegar, portapapeles, mover, duplicar, pegado especial, valores, fórmulas, formatos, trasponer, omitir blancos, sumar, restar, multiplicar, dividir, operación, combinada, texto, tabulación, html, página web, otro programa, Esc, hormigas marchantes, borde, anchos de columna, notas, comentarios, formatos de número, todo excepto bordes, imagen, mapa de bits, copiar como imagen -->

**Copiar** (**{key:Copy}**) pone una copia de las celdas seleccionadas en el Portapapeles, un lugar donde el equipo guarda una cosa cada vez. **Pegar** (**{key:Paste}**) pone lo que está en el Portapapeles en las celdas, a partir de la celda seleccionada. **Cortar** (**{key:Cut}**) es como copiar, pero al pegar, las celdas se mueven y el lugar original queda vacío.

Un borde discontinuo animado muestra las celdas que se copiaron o cortaron. Presione **Esc** para quitar el borde; entonces las celdas ya no están listas para pegarse. Las celdas cortadas también se liberan si hace cualquier otro cambio antes de pegarlas (escribir, insertar, deshacer): córtelas de nuevo si todavía quiere moverlas.

## Pegar dentro de JeffCrunch

Se pega todo: los valores, las fórmulas, los formatos y las celdas combinadas. Una fórmula sigue apuntando a los mismos lugares *en relación con su posición*. Ejemplo: copie **=A1*2** desde B1 y péguela en B5, y pasará a ser **=A5*2**. Una parte con signo de dólar, como **$A$1**, no cambia. Si selecciona un bloque que es un número entero de veces el tamaño de lo que copió, la copia se repite para llenarlo: copie una celda, seleccione veinte celdas, pegue, y las veinte recibirán el contenido.

Si un número pegado es demasiado ancho para una columna que todavía tiene su ancho original, la columna se ensancha.

## Cortar mueve las celdas

Al cortar celdas y pegarlas, las celdas se mueven de verdad. Toda fórmula que apuntaba a ellas pasa a apuntar a su nueva posición. Ejemplo: B1 contiene **=A1+A2**. Corte A1:A2 y péguelo en E4, y B1 pasará a ser **=E4+E5**. Lo que había en las celdas sobre las que pegó se pierde (**Deshacer** lo recupera), y una fórmula que leía una de esas celdas muestra **#¡REF!**, porque la celda que leía ya no está. Si las celdas que corta son solo parte de una celda combinada, o si pega sobre solo una parte de una, no se mueve nada: seleccione la celda combinada completa. Al cortar celdas y pegarlas en otra hoja, las fórmulas siguen leyendo las celdas que leían antes (indican la hoja). El Portapapeles se vacía después de un corte y pegado.

## Pegado especial

**Ctrl+Alt+V**, o la flecha situada debajo de **Pegar** en la pestaña Inicio, permite elegir qué pegar:

- **Todo**, **Fórmulas**, **Valores** o **Formatos**. *Valores* convierte una fórmula en el número o el texto que muestra.
- **Todo excepto bordes**: todo, pero las celdas conservan los bordes que tenían.
- **Fórmulas y formatos de número**, o **Valores y formatos de número**: las fórmulas (o los valores) se pegan con la forma en que se muestran los números, como porcentaje o fecha, pero no los colores, las fuentes ni los bordes. Las celdas conservan el aspecto que tenían.
- **Anchos de columna**: las columnas en las que pega reciben los anchos de las columnas que copió.
- **Comentarios y notas**: las notas de las celdas copiadas se colocan en las celdas en las que pega. Ningún valor de celda cambia.
- Una **operación**: **Sumar**, **Restar**, **Multiplicar** o **Dividir**. Los números que copió se combinan con los números que ya hay en las celdas. Ejemplo: las celdas contienen 10 y 20, copió 5 y eligió *Multiplicar*: pasan a ser 50 y 100.
- **Omitir blancos**: las celdas vacías de la copia dejan como estaban las celdas en las que caen.
- **Trasponer**: las filas se convierten en columnas y las columnas en filas.

## El menú Pegar e imágenes

La flecha situada debajo de **Pegar** muestra las opciones habituales, y además **Sin bordes**, **Fórmulas y formato de número**, **Valores y formato de número**, **Mantener anchos de columna de origen** y **Comentarios y notas**. Son las mismas opciones de Pegado especial, con un solo clic cada una.

La flecha situada junto a **Copiar** tiene **Copiar como imagen** y **Copiar como mapa de bits**. Cada una pone en el Portapapeles una imagen de las celdas seleccionadas, dibujada tal como se ven, con las líneas de cuadrícula. Un mapa de bits tiene el tamaño de la pantalla. Una imagen tiene el doble de puntos, así que se mantiene nítida al ampliarla. Péguela en JeffWrite o JeffDeck como imagen. Si las celdas no caben en una sola imagen, seleccione menos.

## Entre programas

Al copiar, JeffCrunch también pone las celdas en el Portapapeles como texto (celdas separadas por tabulaciones, una fila por línea) y como tabla para páginas web, para que otros programas puedan usarlas. Cuando pega algo que no procede de JeffCrunch, como texto con tabulaciones o una tabla de una página web u otra hoja de cálculo, cada elemento va a su propia celda y se interpreta como si lo hubiera escrito: números, fechas y fórmulas que empiezan con un signo igual. Al copiar desde otro libro abierto en JeffCrunch se traen los valores, las fórmulas y los formatos de número, pero no el resto del formato.

## Lo que no se puede pegar

JeffCrunch le avisa y no cambia nada cuando el pegado afectaría a parte de una **fórmula matricial**, a la fila de nombres de columna de una **tabla**, a celdas bloqueadas de una **hoja protegida**, más allá del final de la hoja, o a más de 100 000 celdas. Una **fórmula matricial** completa se puede cortar y mover (seleccione todas sus celdas); una parte de ella, una tabla de datos o la fila de nombres de una tabla no.
