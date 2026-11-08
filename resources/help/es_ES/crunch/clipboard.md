# Cortar, copiar y pegar
<!-- keywords: cortar, copiar, pegar, portapapeles, mover, duplicar, especial, valores, fórmulas, formatos, transponer, omitir espacios en blanco, sumar, restar, multiplicar, dividir, operación, combinada, texto, tabulación, html, página web, otro programa, Escape, hormigas marchantes, borde, anchos de columna, notas, comentarios, formatos de número, todo excepto bordes, imagen, mapa de bits, copiar como imagen, imagen impresa, como se imprime, pegar vínculo, hipervínculo, pegar importación de texto, dividir texto, separador, establecer pegado predeterminado, pegado predeterminado, ancho fijo -->

**Copiar** (**{key:Copy}**) pone una copia de las celdas seleccionadas en el Portapapeles, un lugar donde el equipo guarda una cosa a la vez. **Pegar** (**{key:Paste}**) coloca lo que hay en el Portapapeles en las celdas, empezando por la celda seleccionada. **Cortar** (**{key:Cut}**) es como copiar, pero al pegar, las celdas se mueven y el lugar original queda vacío.

Un borde discontinuo que se mueve muestra las celdas copiadas o cortadas. Pulse **Esc** para quitar el borde; entonces las celdas ya no están listas para pegarse. Las celdas cortadas también se liberan si hace otro cambio antes de pegarlas (escribir, insertar, deshacer): córtelas de nuevo si todavía quiere moverlas.

## Pegar dentro de JeffCrunch

Todo se traslada: los valores, las fórmulas, los formatos y las celdas combinadas. Una fórmula sigue apuntando a las mismas posiciones *relativas a donde está*. Por ejemplo: copie **=A1*2** desde B1 y péguela en B5, y se convertirá en **=A5*2**. Una parte con signo de dólar, como **$A$1**, se mantiene igual. Si selecciona un bloque que es un número entero de veces el tamaño de lo copiado, la copia se repite para llenarlo: copie una celda, seleccione veinte celdas, pegue, y las veinte recibirán el contenido.

Un número pegado que es demasiado ancho para una columna que todavía tiene su ancho inicial amplía la columna.

## Cortar mueve celdas

Cuando corta celdas y las pega, las celdas se mueven de verdad. Toda fórmula que apuntaba a ellas apunta a su nueva posición. Por ejemplo: B1 tiene **=A1+A2**. Corte A1:A2 y pegue en E4, y B1 se convertirá en **=E4+E5**. Lo que había en las celdas sobre las que pegó se pierde (**Deshacer** lo recupera), y una fórmula que leía una de esas celdas muestra **#REF!**, porque la celda que leía ya no está. Si las celdas cortadas son solo parte de una celda combinada, o si pega sobre solo una parte de una, no se mueve nada: seleccione la celda combinada completa. Cuando corta celdas y las pega en otra hoja, las fórmulas siguen leyendo las celdas que leían antes (indican la hoja). El Portapapeles se vacía después de un corte y pegado.

## Pegado especial

**Ctrl+Alt+V**, o la flecha debajo de **Pegar** en la pestaña Inicio, le permite elegir qué pegar:

- **Todo**, **Fórmulas**, **Valores** o **Formatos**. *Valores* convierte una fórmula en el número o texto que muestra.
- **Todo excepto bordes**: todo, pero las celdas conservan los bordes que tenían.
- **Fórmulas y formatos de número**, o **Valores y formatos de número**: las fórmulas (o los valores) llegan con la forma en que se muestran los números, como porcentaje o fecha, pero no los colores, las fuentes ni los bordes. Las celdas conservan el aspecto que tenían.
- **Anchos de columna**: las columnas en las que pega reciben los anchos de las columnas que copió.
- **Comentarios y notas**: las notas de las celdas copiadas se colocan en las celdas en las que pega. Ningún valor de celda cambia.
- Una **operación**: **Sumar**, **Restar**, **Multiplicar** o **Dividir**. Los números copiados se combinan con los números que ya hay en las celdas. Ejemplo: las celdas tienen 10 y 20, copió 5 y eligió *Multiplicar*: pasan a ser 50 y 100.
- **Omitir espacios en blanco**: las celdas vacías de la copia dejan como estaban las celdas en las que caen.
- **Transponer**: las filas se convierten en columnas y las columnas en filas.

## Pegar vínculo, Pegar como hipervínculo e Importación de texto al pegar

En la flecha debajo de **Pegar** hay otras tres opciones. Funcionan con celdas copiadas en el mismo libro, excepto la última.

- **Pegar vínculo** coloca en cada celda una fórmula que apunta a la celda copiada. Ejemplo: copie A1:B2, seleccione D4 y use **Pegar vínculo**: D4 pasa a ser **=Hoja1!$A$1**. Cuando cambie A1, D4 cambiará con ella. Una celda copiada que está vacía muestra 0.
- **Pegar como hipervínculo** coloca en cada celda las mismas palabras que muestra la celda copiada, y convierte cada una en un vínculo a la celda copiada. Mantenga pulsado **Ctrl** y haga clic en una para ir a esa celda. Funciona con hasta 2000 celdas a la vez.
- **Pegar importación de texto** sirve para texto proveniente de otro programa. Abre la misma ventana que al abrir un fichero de texto: indica qué separa los fragmentos (una coma, un tabulador, un espacio, un punto y coma, una barra vertical u otro carácter), qué encierra un fragmento entre comillas, la primera fila que se usará y cómo leer cada columna (tal como está escrita, como texto, como fecha u omitida). Parte del texto alinea sus fragmentos en columnas de igual anchura, con espacios entre ellas. Marque **Las columnas tienen un ancho fijo** e indique dónde empieza cada columna: cada línea se corta en esos puntos y se quitan los espacios alrededor de cada fragmento. Elija *Texto* para una columna de códigos como 007, para que se conserven los ceros. Los fragmentos se colocan en celdas, empezando por la celda seleccionada, como un solo paso que puede deshacer.

## Establecer pegado predeterminado

**Establecer pegado predeterminado** es la última opción de la flecha debajo de **Pegar**. Decide qué pegan el botón **Pegar** y **{key:Paste}** cuando ha copiado celdas: todo (al principio), o solo los valores, solo las fórmulas, solo el formato, etc. Por ejemplo: elija *Valores*, y una fórmula copiada aparecerá como el número que muestra. La elección se conserva para la próxima vez. No cambia **Pegado especial** ni las opciones del menú, y el texto de otros programas se pega tal cual. Elija *Todo* para volver a la configuración original.

## El menú Pegar e imágenes

La flecha debajo de **Pegar** muestra las opciones habituales y también **Sin bordes**, **Fórmulas y formato de número**, **Valores y formato de número**, **Mantener anchos de columna de origen** y **Comentarios y notas**. Son las mismas opciones de Pegado especial, con un solo clic cada una.

La flecha junto a **Copiar** tiene **Copiar como imagen** y **Copiar como mapa de bits**. Cada una pone en el Portapapeles una imagen de las celdas seleccionadas, dibujada tal como las ve, con las líneas de cuadrícula. Un mapa de bits tiene el tamaño de la pantalla. Una imagen tiene el doble de puntos, así que se ve nítida al ampliarla. **Copiar como imagen impresa** dibuja las celdas tal como las mostraría el papel: las líneas de cuadrícula solo aparecen si la configuración de página las imprime. Péguela en JeffWrite o JeffDeck como imagen. Si las celdas no caben en una sola imagen, seleccione menos.

## Entre programas

Al copiar, JeffCrunch también pone las celdas en el Portapapeles como texto (celdas separadas por tabulaciones, una fila por línea) y como tabla para páginas web, para que otros programas puedan usarlas. Cuando pega algo que no procede de JeffCrunch, texto con tabulaciones o una tabla de una página web u otra hoja de cálculo, cada fragmento va a su propia celda y se interpreta como si lo hubiera escrito: números, fechas y fórmulas que empiezan por un signo igual. Al copiar desde otro libro abierto en JeffCrunch se traen valores, fórmulas y formatos de número, pero no el resto del formato.

## Lo que no se puede pegar

JeffCrunch lo indica y no cambia nada cuando el pegado llegaría a una parte de una **fórmula matricial**, sobre la fila de nombres de columna de una **tabla**, a celdas bloqueadas de una **hoja protegida**, más allá del final de la hoja o a más de 100 000 celdas. Una **fórmula matricial** completa puede cortarse y moverse (seleccione todas sus celdas); una parte de ella, una tabla de datos o la fila de nombres de una tabla no.
