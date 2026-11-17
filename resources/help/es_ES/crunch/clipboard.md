# Cortar, copiar y pegar

<!-- keywords: cortar, copiar, pegar, portapapeles, mover, duplicar, pegado especial, valores, fórmulas, formatos, transponer, omitir blancos, sumar, restar, multiplicar, dividir, operación, combinadas, texto, tabulación, html, página web, otro programa, Esc, hormigas marchantes, bordes, anchos de columna, notas, comentarios, formatos de número, todo excepto bordes, imagen, mapa de bits, copiar como imagen, imagen impresa, como se imprime, pegar vínculo, hipervínculo, importación de texto, texto delimitado, soltar texto, coma, punto y coma, csv, separador, establecer pegado predeterminado, pegado predeterminado, ancho fijo -->

**Copiar** (**{key:Copy}**) pone una copia de las celdas seleccionadas en el Portapapeles, un espacio que guarda el equipo para una cosa a la vez. **Pegar** (**{key:Paste}**) pone lo que hay en el Portapapeles en las celdas, a partir de la celda seleccionada. **Cortar** (**{key:Cut}**) es como copiar, pero al pegar, las celdas se mueven y el lugar anterior queda vacío.

Un borde discontinuo animado muestra las celdas que se copiaron o cortaron. Pulse **Esc** para quitar el borde; entonces las celdas ya no están listas para pegarse. Las celdas cortadas también se desechan si hace cualquier otro cambio antes de pegarlas (escribir, insertar, deshacer): córtelas de nuevo si todavía quiere moverlas.

## Pegar dentro de JeffCrunch

Se traslada todo: los valores, las fórmulas, los formatos y las celdas combinadas. Una fórmula sigue apuntando a los mismos lugares *en relación con su posición*. Por ejemplo: copie **=A1*2** desde B1 y péguela en B5, y pasará a ser **=A5*2**. Una parte con signo de dólar, como **$A$1**, se mantiene tal cual. Si selecciona un bloque que es un número exacto de veces el tamaño de lo copiado, la copia se repite para llenarlo: copie una celda, seleccione veinte celdas, pegue, y las veinte recibirán el contenido.

Un número pegado que es demasiado ancho para una columna que todavía tiene su primer ancho amplía la columna.

## Cortar mueve las celdas

Cuando corta celdas y las pega, las celdas se mueven de verdad. Toda fórmula que apuntaba a ellas apunta a su nueva ubicación. Por ejemplo: B1 contiene **=A1+A2**. Corte A1:A2 y pegue en E4, y B1 pasará a ser **=E4+E5**. Lo que había en las celdas sobre las que pega se pierde (**Deshacer** lo recupera), y una fórmula que leía alguna de esas celdas muestra **#REF!**, porque la celda que leía ya no está. Si las celdas que corta son solo parte de una celda combinada, o pega sobre solo una parte de una, no se mueve nada: seleccione toda la celda combinada. Cuando corta celdas a otra hoja, las fórmulas siguen leyendo las celdas que leían antes (indican la hoja). El Portapapeles se vacía después de cortar y pegar.

## Pegado especial

**Ctrl+Alt+V**, o la flecha debajo de **Pegar** en la pestaña Inicio, le permite elegir qué pegar:

- **Todo**, **Fórmulas**, **Valores** o **Formatos**. *Valores* convierte una fórmula en el número o texto que muestra.
- **Todo excepto bordes**: todo, pero las celdas conservan los bordes que tenían.
- **Fórmulas y formatos de número**, o **Valores y formatos de número**: las fórmulas (o los valores) llegan con la forma de mostrar los números, como porcentaje o fecha, pero no los colores, las fuentes ni los bordes. Las celdas conservan el aspecto que tenían.
- **Anchos de columna**: las columnas en las que pega reciben los anchos de las columnas copiadas.
- **Comentarios y notas**: las notas de las celdas copiadas se colocan en las celdas donde pega. Ningún valor de celda cambia.
- Una **operación**: **Sumar**, **Restar**, **Multiplicar** o **Dividir**. Los números copiados se combinan con los números que ya están en las celdas. Por ejemplo: las celdas tienen 10 y 20, copió 5 y eligió *Multiplicar*: pasan a ser 50 y 100.
- **Omitir blancos**: las celdas vacías de la copia dejan como estaban las celdas en las que caen.
- **Transponer**: las filas se convierten en columnas y las columnas en filas.

## Pegar vínculo, Pegar como hipervínculo e Importación de texto al pegar

Otras tres opciones están en la flecha debajo de **Pegar**. Funcionan con celdas copiadas en el mismo libro, salvo la última.

- **Pegar vínculo** pone en cada celda una fórmula que apunta a la celda copiada. Por ejemplo: copie A1:B2, seleccione D4 y use **Pegar vínculo**: D4 pasa a ser **=Hoja1!$A$1**. Cuando cambie A1, D4 cambiará con él. Una celda copiada que está vacía muestra 0.
- **Pegar como hipervínculo** pone en cada celda las mismas palabras que muestra la celda copiada, y convierte cada una en un vínculo a la celda copiada. Mantenga pulsada **Ctrl** y haga clic en una para ir a esa celda. Funciona con hasta 2000 celdas a la vez.
- **Importación de texto al pegar** sirve para texto de otro programa. Abre la misma ventana que al abrir un fichero de texto: indica qué separa los datos (una coma, una tabulación, un espacio, un punto y coma, una barra u otro carácter), qué encierra un dato entre comillas, la primera fila que se usa y cómo leer cada columna (tal cual, como texto, como fecha u omitida). Algún texto alinea sus datos en columnas del mismo ancho, con espacios entre ellas. Marque **Las columnas tienen un ancho fijo** e indique dónde empieza cada columna: cada línea se corta en esos puntos y se quitan los espacios alrededor de cada dato. Elija *Texto* para una columna de códigos como 007, para que se conserven los ceros. Los datos se colocan en celdas, a partir de la celda seleccionada, como un solo paso que puede deshacer.

## Texto con comas, punto y coma o columnas

Cuando el texto que pega de otro programa es claramente una tabla, JeffCrunch abre la misma ventana que al abrir un fichero de texto, para que pueda comprobar cómo se divide antes de que nada llegue a las celdas. Esto ocurre solo con un **Pegar** normal (**{key:Paste}**), nunca con **Pegado especial** ni con una de sus opciones. Ocurre cuando el texto tiene al menos dos líneas y ninguna tabulación, y, o bien todas las líneas tienen el mismo número de datos, al menos tres, separados por comas, puntos y coma o barras, con algo en cada columna (un dato entre comillas puede contener una coma); o bien los datos se alinean en columnas del mismo ancho, con los mismos espacios entre ellas en todas las líneas (al menos tres líneas y al menos dos espacios). Los números con separador de miles, como 1,234, y las líneas de código o listas que terminan en coma o punto y coma no se toman por una tabla. La ventana empieza con la forma en que se ha detectado que está dividido el texto. Si los números tienen comas y los datos están separados por punto y coma, la coma es el separador decimal. Haga clic en **Aceptar** para colocar los datos, o en **Cancelar** para no poner nada.

Por ejemplo: copie estas líneas de un correo electrónico.

    nombre,cant,precio
    Manzana,12,1.50
    Pera,3,0.25

Péguelas con una tercera columna, como un precio, y la ventana muestra tres columnas, con **Coma** marcada. Una frase en una sola línea, o líneas con distinto número de comas, o solo dos datos por línea, se pegan como palabras, sin ventana. El texto con tabulaciones va directamente a las celdas, como siempre. Para abrir la ventana con cualquier texto, elija **Importación de texto al pegar** (arriba).

Lo mismo ocurre cuando arrastra texto de otro programa y lo suelta en la cuadrícula: entra en la celda seleccionada. Si lo suelta en cualquier otro punto de la ventana, no pasa nada.

## Establecer pegado predeterminado

**Establecer pegado predeterminado** es la última opción de la flecha debajo de **Pegar**. Decide qué colocan el botón **Pegar** y **{key:Paste}** cuando ha copiado celdas: todo (al principio), o solo los valores, solo las fórmulas, solo el formato, etc. Por ejemplo: elija *Valores*, y una fórmula copiada llegará como el número que muestra. La elección se conserva para la próxima vez. No cambia **Pegado especial** ni las opciones del menú, y el texto de otros programas se pega tal cual. Elija *Todo* para volver a la situación inicial.

## El menú Pegar e imágenes

La flecha debajo de **Pegar** muestra las opciones habituales, y además **Sin bordes**, **Fórmulas y formatos de número**, **Valores y formatos de número**, **Mantener anchos de columna de origen** y **Comentarios y notas**. Son las mismas opciones de Pegado especial, un clic cada una.

La flecha junto a **Copiar** tiene **Copiar como imagen** y **Copiar como mapa de bits**. Cada una pone en el Portapapeles una imagen de las celdas seleccionadas, dibujada tal como las ve, con las líneas de cuadrícula. Un mapa de bits tiene el tamaño de la pantalla. Una imagen tiene el doble de puntos, así que se ve nítida al ampliarla. **Copiar como imagen impresa** dibuja las celdas como las mostraría el papel: las líneas de cuadrícula aparecen solo si la configuración de página las imprime. Péguela en JeffWrite o JeffDeck como imagen. Si las celdas no caben en una sola imagen, seleccione menos.

## Entre programas

Al copiar, JeffCrunch también pone las celdas en el Portapapeles como texto (celdas separadas por tabulaciones, una fila por línea) y como tabla para páginas web, para que otros programas puedan usarlas. Cuando pega algo que no procede de JeffCrunch, texto con tabulaciones o una tabla de una página web u otra hoja de cálculo, cada dato va a su propia celda y se interpreta como si lo hubiera escrito: números, fechas y fórmulas que empiezan por un signo igual. Si copia desde otro libro abierto en JeffCrunch, se traen los valores, las fórmulas y los formatos de número, pero no el resto del formato. Péguelos en JeffDeck o JeffPub y llegarán como una tabla que conserva las fuentes, los sombreados, los bordes y los números tal como se muestran (consulte **Pegar celdas como tabla** en su Ayuda).

## Lo que no se puede pegar

JeffCrunch se lo indica y no cambia nada cuando el pegado llegaría a una parte de una **fórmula matricial**, sobre la fila de nombres de columna de una **tabla**, a celdas bloqueadas de una **hoja protegida**, más allá del final de la hoja o a más de 100 000 celdas. Una **fórmula matricial** completa puede cortarse y moverse (seleccione todas sus celdas); una parte de ella, una tabla de datos o la fila de nombres de una tabla no.
