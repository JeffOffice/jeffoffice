# Cortar, copiar y pegar

<!-- keywords: cortar, copiar, pegar, portapapeles, mover, duplicar, pegado especial, valores, fórmulas, formatos, trasponer, saltar blancos, sumar, restar, multiplicar, dividir, operación, combinadas, texto, tabulación, html, página web, otro programa, Escape, hormigueo, borde, anchos de columna, notas, comentarios, formatos de número, todo excepto bordes, imagen, mapa de bits, copiar como imagen, imagen impresa, como se imprime, pegar vínculo, hipervínculo, importación de texto, dividir texto, separador -->

**Copiar** (**{key:Copy}**) pone una copia de las celdas seleccionadas en el Portapapeles, un lugar en el que el equipo guarda una cosa a la vez. **Pegar** (**{key:Paste}**) coloca lo que hay en el Portapapeles en las celdas, empezando por la celda seleccionada. **Cortar** (**{key:Cut}**) es como copiar, pero al pegar, las celdas se mueven y el lugar original queda vacío.

Un borde discontinuo animado muestra las celdas que se copiaron o cortaron. Presione **Esc** para quitar el borde; entonces las celdas ya no están listas para pegarse. Las celdas cortadas también se liberan si hace cualquier otro cambio antes de pegarlas (escribir, insertar, deshacer): córtelas de nuevo si todavía quiere moverlas.

## Pegar dentro de JeffCrunch

Se traslada todo: los valores, las fórmulas, los formatos y las celdas combinadas. Una fórmula sigue apuntando a los mismos lugares *en relación con su posición*. Por ejemplo: copie **=A1*2** desde B1 y péguela en B5, y pasará a ser **=A5*2**. Una parte con un signo de dólar, como **$A$1**, se mantiene tal cual. Si selecciona un bloque que es un número entero de veces el tamaño de lo que copió, la copia se repite para llenarlo: copie una celda, seleccione veinte celdas, pegue, y las veinte recibirán el contenido.

Un número pegado que es demasiado ancho para una columna que todavía tiene su primer ancho la ensancha.

## Cortar mueve celdas

Cuando corta celdas y las pega, las celdas se mueven de verdad. Cada fórmula que apuntaba a ellas apunta a su nueva posición. Por ejemplo: B1 contiene **=A1+A2**. Corte A1:A2 y péguelas en E4, y B1 pasará a ser **=E4+E5**. Lo que había en las celdas sobre las que pegó se pierde (**Deshacer** lo recupera), y una fórmula que leía una de esas celdas muestra **#¡REF!**, porque la celda que leía ya no está. Si las celdas que corta son solo una parte de una celda combinada, o si pega sobre solo una parte de una, no se mueve nada: seleccione toda la celda combinada. Cuando corta celdas a otra hoja, las fórmulas siguen leyendo las celdas que leían antes (indican la hoja). El Portapapeles se vacía después de cortar y pegar.

## Pegado especial

**Ctrl+Alt+V**, o la flecha debajo de **Pegar** en la pestaña Inicio, le permite elegir qué pegar:

- **Todo**, **Fórmulas**, **Valores** o **Formatos**. *Valores* convierte una fórmula en el número o texto que muestra.
- **Todo excepto bordes**: todo, pero las celdas conservan los bordes que tenían.
- **Fórmulas y formatos de número**, o **Valores y formatos de número**: las fórmulas (o los valores) se llevan junto con la forma en que se muestran los números, como porcentaje o fecha, pero no los colores, las fuentes ni los bordes. Las celdas conservan el aspecto que tenían.
- **Anchos de columna**: las columnas en las que pega reciben los anchos de las columnas que copió.
- **Comentarios y notas**: las notas de las celdas copiadas se colocan en las celdas en las que pega. Ningún valor de celda cambia.
- Una **operación**: **Sumar**, **Restar**, **Multiplicar** o **Dividir**. Los números copiados se combinan con los números que ya hay en las celdas. Por ejemplo: las celdas contienen 10 y 20, copió 5 y eligió *Multiplicar*: pasan a ser 50 y 100.
- **Saltar blancos**: las celdas vacías de la copia dejan como estaban las celdas en las que caen.
- **Trasponer**: las filas se convierten en columnas y las columnas en filas.

## Pegar vínculo, Pegar como hipervínculo e Importación de texto al pegar

Otras tres opciones están en la flecha debajo de **Pegar**. Funcionan con celdas copiadas en el mismo libro, excepto la última.

- **Pegar vínculo** pone en cada celda una fórmula que apunta a la celda copiada. Por ejemplo: copie A1:B2, seleccione D4 y use **Pegar vínculo**: D4 pasa a ser **=Hoja1!$A$1**. Cuando cambie A1, D4 cambiará con ella. Una celda copiada que está vacía muestra 0.
- **Pegar como hipervínculo** pone en cada celda las mismas palabras que muestra la celda copiada, y convierte cada una en un vínculo a la celda copiada. Mantenga presionada **Ctrl** y haga clic en una para ir a esa celda. Funciona con hasta 2000 celdas a la vez.
- **Importación de texto al pegar** sirve para texto procedente de otro programa. Abre la misma ventana que al abrir un archivo de texto: indica qué separa los datos (una coma, una tabulación, un espacio, un punto y coma, una barra vertical u otro carácter), qué encierra un dato entre comillas, la primera fila que se va a usar y cómo leer cada columna (tal como se escribió, como texto, como fecha o se omite). Elija *Texto* para una columna de códigos como 007, para que se conserven los ceros. Los datos se colocan en las celdas, empezando por la celda seleccionada, como un único paso que puede deshacer.

## El menú Pegar e imágenes

La flecha debajo de **Pegar** muestra las opciones habituales, y también **Sin bordes**, **Fórmulas y formato de número**, **Valores y formato de número**, **Mantener anchos de columna de origen** y **Comentarios y notas**. Son las mismas opciones que en Pegado especial, con un solo clic cada una.

La flecha junto a **Copiar** tiene **Copiar como imagen** y **Copiar como mapa de bits**. Cada una pone en el Portapapeles una imagen de las celdas seleccionadas, dibujada tal como se ven en pantalla, con las líneas de cuadrícula. Un mapa de bits tiene el tamaño de la pantalla. Una imagen tiene el doble de puntos, así que se ve nítida al ampliarla. **Copiar como imagen impresa** dibuja las celdas tal como las mostraría el papel: las líneas de cuadrícula solo aparecen si la configuración de página las imprime. Péguela en JeffWrite o JeffDeck como imagen. Si las celdas no caben en una sola imagen, seleccione menos.

## Entre programas

Al copiar, JeffCrunch también pone las celdas en el Portapapeles como texto (celdas separadas por tabulaciones, una fila por línea) y como tabla para páginas web, para que otros programas puedan usarlas. Cuando pega algo que no procede de JeffCrunch, texto con tabulaciones o una tabla de una página web u otra hoja de cálculo, cada dato va a su propia celda y se interpreta como si lo hubiera escrito: números, fechas y fórmulas que empiezan por un signo igual. Al copiar desde otro libro abierto en JeffCrunch se traen valores, fórmulas y formatos de número, pero no el resto del formato.

## Lo que no se puede pegar

JeffCrunch se lo indica y no cambia nada cuando el pegado afectaría a parte de una **fórmula matricial**, sobre la fila de nombres de columna de una **tabla**, a celdas bloqueadas de una **hoja protegida**, más allá del final de la hoja o a más de 100 000 celdas. Una **fórmula matricial** completa se puede cortar y mover (seleccione todas sus celdas); una parte de una, una tabla de datos o la fila de nombres de una tabla no.
