# Cortar, copiar y pegar
<!-- keywords: cortar, copiar, pegar, portapapeles, mover, duplicar, pegado especial, valores, fórmulas, formatos, transponer, omitir espacios en blanco, sumar, restar, multiplicar, dividir, operación, combinadas, texto, tabulación, html, página web, otro programa, Esc, hormigas marchantes, borde, anchos de columna, notas, comentarios, formatos de número, todo excepto bordes, imagen, mapa de bits, copiar como imagen, imagen impresa, como se imprime, pegar vínculo, hipervínculo, importar texto, texto delimitado, soltar texto, coma, punto y coma, csv, separador, establecer pegado predeterminado, pegado predeterminado, ancho fijo -->

**Copiar** (**{key:Copy}**) pone una copia de las celdas seleccionadas en el Portapapeles, un espacio que guarda el equipo para una cosa a la vez. **Pegar** (**{key:Paste}**) coloca lo que hay en el Portapapeles en las celdas, empezando por la celda seleccionada. **Cortar** (**{key:Cut}**) es como copiar, pero al pegar, las celdas se mueven y el lugar anterior queda vacío.

Un borde discontinuo animado muestra las celdas que se copiaron o cortaron. Pulse **Esc** para quitar el borde; entonces las celdas ya no están listas para pegarse. Las celdas cortadas también se liberan si hace cualquier otro cambio antes de pegarlas (escribir, insertar, deshacer): córtelas de nuevo si todavía quiere moverlas.

## Pegar dentro de JeffCrunch

Se traslada todo: los valores, las fórmulas, los formatos y las celdas combinadas. Una fórmula sigue apuntando a las mismas celdas *en relación con su posición*. Ejemplo: copie **=A1*2** de B1 y péguela en B5, y pasará a ser **=A5*2**. Una parte con signo de dólar, como **$A$1**, se mantiene tal cual. Si selecciona un bloque que es un número entero de veces el tamaño de lo que copió, la copia se repite para llenarlo: copie una celda, seleccione veinte celdas, pegue, y las veinte recibirán el contenido.

Un número pegado que es demasiado ancho para una columna que todavía tiene su primer ancho la ensancha.

## Cortar mueve las celdas

Cuando corta celdas y las pega, las celdas se mueven de verdad. Cada fórmula que apuntaba a ellas apunta a su nueva posición. Ejemplo: B1 contiene **=A1+A2**. Corte A1:A2 y pegue en E4, y B1 pasará a ser **=E4+E5**. Lo que había en las celdas sobre las que pegó se pierde (**Deshacer** lo recupera), y una fórmula que leía una de esas celdas muestra **#¡REF!**, porque la celda que leía ya no está. Si las celdas que corta son solo parte de una celda combinada, o pega sobre solo parte de una, no se mueve nada: seleccione toda la celda combinada. Cuando corta celdas a otra hoja, las fórmulas siguen leyendo las celdas que leían antes (indican la hoja). El Portapapeles se vacía después de cortar y pegar.

## Pegado especial

**Ctrl+Alt+V**, o la flecha debajo de **Pegar** en la pestaña Inicio, permite elegir qué pegar:

- **Todo**, **Fórmulas**, **Valores** o **Formatos**. *Valores* convierte una fórmula en el número o texto que muestra.
- **Todo excepto bordes**: todo, pero las celdas conservan los bordes que tenían.
- **Fórmulas y formatos de número**, o **Valores y formatos de número**: las fórmulas (o los valores) se acompañan del modo en que se muestran los números, como porcentaje o fecha, pero no de los colores, las fuentes ni los bordes. Las celdas conservan el aspecto que tenían.
- **Anchos de columna**: las columnas en las que pega reciben los anchos de las columnas que copió.
- **Comentarios y notas**: las notas de las celdas copiadas se colocan en las celdas donde pega. Ningún valor de celda cambia.
- Una **operación**: **Sumar**, **Restar**, **Multiplicar** o **Dividir**. Los números copiados se combinan con los números que ya están en las celdas. Ejemplo: las celdas contienen 10 y 20, usted copió 5 y eligió *Multiplicar*: pasan a ser 50 y 100.
- **Omitir espacios en blanco**: las celdas vacías de la copia dejan como estaban las celdas en las que caen.
- **Transponer**: las filas se convierten en columnas y las columnas en filas.

## El botón Opciones de pegado

Justo después de pegar celdas copiadas en JeffCrunch, aparece un pequeño botón **Opciones de pegado** junto a la última celda pegada. Haga clic en él para elegir otra forma de colocar las mismas celdas: **Mantener formato de origen** (todo), **Valores**, **Fórmulas**, **Formato**, **Sin bordes**, **Fórmulas y formato de número**, **Valores y formato de número**, **Mantener anchos de columna de origen** o **Transponer**. JeffCrunch deshace el pegado y lo vuelve a hacer de esa manera. El cambio es un solo paso: un único **Deshacer** quita todo el pegado, por muchas veces que cambie de opinión. Ejemplo: copie A1:B2, donde B1 es **=A1*2**, y pegue en F5. Elija **Valores** y G5 contendrá el número 2 en lugar de **=F5*2**; elija **Mantener formato de origen** y la fórmula vuelve.

Desde el teclado, pulse la tecla **Ctrl** (la tecla **Control** en un Mac) después de pegar para abrir el menú del botón, y luego use las teclas de flecha e **Intro** para elegir. El botón desaparece cuando cambia una celda u otra cosa del libro, copia otra cosa, pulsa **Esc**, o desplaza la última celda pegada fuera de la vista (vuelve al desplazarse de nuevo). No se ofrece después de cortar y pegar, ni para texto de otro programa.

## Pegar vínculo, Pegar como hipervínculo y Pegar importación de texto

Hay tres opciones más en la flecha debajo de **Pegar**. Funcionan con celdas copiadas en el mismo libro, salvo la última.

- **Pegar vínculo** pone en cada celda una fórmula que apunta a la celda copiada. Ejemplo: copie A1:B2, seleccione D4 y use **Pegar vínculo**: D4 pasa a ser **=Hoja1!$A$1**. Cuando cambia A1, D4 cambia con ella. Una celda copiada que está vacía muestra 0.
- **Pegar como hipervínculo** pone en cada celda las mismas palabras que muestra la celda copiada, y convierte cada una en un vínculo a la celda copiada. Mantenga pulsada **Ctrl** y haga clic en una para ir a esa celda. Funciona con hasta 2000 celdas a la vez.
- **Pegar importación de texto** sirve para texto de otro programa. Abre la misma ventana que al abrir un archivo de texto: indica qué separa las partes (una coma, una tabulación, un espacio, un punto y coma, una barra u otro carácter), qué encierra una parte entre comillas, la primera fila que se usará y cómo leer cada columna (tal como se escribió, como texto, como fecha, u omitida). Algunos textos alinean sus partes en columnas del mismo ancho, con espacios entre ellas. Marque **Las columnas tienen un ancho fijo** e indique dónde empieza cada columna: cada línea se corta en esos puntos y se quitan los espacios alrededor de cada parte. Elija *Texto* para una columna de códigos como 007, para que se conserven los ceros. Las partes se colocan en las celdas, empezando por la celda seleccionada, como un solo paso que puede deshacer.

## Texto con comas, punto y coma o columnas

Cuando el texto que pega desde otro programa es claramente una tabla, JeffCrunch abre la misma ventana que al abrir un archivo de texto, para que pueda comprobar cómo se divide antes de que nada llegue a las celdas. Esto ocurre solo con un **Pegar** normal ({key:Paste}), nunca con **Pegado especial** ni con una de sus opciones. Ocurre cuando el texto tiene al menos dos líneas y ninguna tabulación, y se cumple una de estas condiciones: todas las líneas tienen el mismo número de partes, al menos tres, separadas por comas, puntos y comas o barras, con algo en cada columna (una parte entre comillas puede contener una coma); o bien las partes están alineadas en columnas del mismo ancho, con los mismos espacios entre ellas en todas las líneas (al menos tres líneas y al menos dos separaciones). Los números con comas de miles, como 1,234, y las líneas de código o listas que terminan en coma o punto y coma no se toman como tabla. La ventana empieza con la forma de división que se haya detectado en el texto. Si los números tienen comas y las partes se separan por punto y coma, la coma es el separador decimal. Haga clic en **Aceptar** para colocar las partes, o en **Cancelar** para no colocar nada.

Ejemplo: copie estas líneas de un correo electrónico.

    nombre,cant,precio
    Manzana,12,1.50
    Pera,3,0.25

Péguelas con una tercera columna, como un precio, y la ventana muestra tres columnas, con **Coma** marcada. Una frase en una sola línea, o líneas con distinto número de comas, o solo dos partes por línea, se pegan como palabras, sin ventana. El texto con tabulaciones va directamente a las celdas, como siempre. Para abrir la ventana con cualquier texto, elija **Pegar importación de texto** (arriba).

Lo mismo ocurre cuando arrastra texto desde otro programa y lo suelta sobre la cuadrícula: se coloca en la celda seleccionada. Soltarlo en cualquier otro lugar de la ventana no hace nada.

## Establecer pegado predeterminado

**Establecer pegado predeterminado** es la última opción de la flecha debajo de **Pegar**. Decide lo que el botón **Pegar** y **{key:Paste}** colocan cuando ha copiado celdas: todo (al principio), o solo los valores, solo las fórmulas, solo el formato, etc. Ejemplo: elija *Valores*, y una fórmula copiada se colocará como el número que muestra. La elección se conserva para la próxima vez. No cambia **Pegado especial** ni las opciones del menú, y el texto de otros programas se pega tal cual. Elija *Todo* para volver a la configuración original.

La misma opción está en la página **Edición** de **Opciones**, en dos cuadros: uno para las celdas copiadas en este libro, y otro para las copiadas en otro libro. Hasta que elija una para otro libro, sigue a la primera.

## El menú Pegar e imágenes

La flecha debajo de **Pegar** muestra las opciones habituales, y además **Sin bordes**, **Fórmulas y formato de número**, **Valores y formato de número**, **Mantener anchos de columna de origen** y **Comentarios y notas**. Son las mismas opciones de Pegado especial, con un solo clic cada una.

La flecha junto a **Copiar** tiene **Copiar como imagen** y **Copiar como mapa de bits**. Cada una pone en el Portapapeles una imagen de las celdas seleccionadas, dibujada tal como se ven, con las líneas de cuadrícula. Un mapa de bits tiene el tamaño de la pantalla. Una imagen tiene el doble de puntos, así que se ve nítida al ampliarla. **Copiar como imagen impresa** dibuja las celdas como las mostraría el papel: las líneas de cuadrícula aparecen solo si la configuración de página las imprime. Péguela en JeffWrite o JeffDeck como imagen. Si las celdas no caben en una sola imagen, seleccione menos.

## Entre programas

Al copiar, JeffCrunch también pone las celdas en el Portapapeles como texto (celdas separadas por tabulaciones, una fila por línea) y como tabla para páginas web, para que otros programas puedan usarlas. Cuando pega algo que no proviene de JeffCrunch, texto con tabulaciones o una tabla de una página web u otra hoja de cálculo, cada parte va a su propia celda y se interpreta como si la hubiera escrito: números, fechas y fórmulas que empiezan por un signo igual. Copiar desde otro libro abierto en JeffCrunch trae valores, fórmulas y formatos de número, pero no el resto del formato. Si las pega en JeffDeck o JeffPub, se colocan como una tabla que conserva las fuentes, los sombreados, los bordes y los números tal como se muestran (véase **Pegar celdas como tabla** en su Ayuda).

## Lo que no se puede pegar

JeffCrunch le avisa y no cambia nada cuando el pegado afectaría a parte de una **fórmula matricial**, a la fila de nombres de columna de una **tabla**, a celdas bloqueadas de una **hoja protegida**, más allá del final de la hoja, o a más de 100 000 celdas. Una **fórmula matricial** completa puede cortarse y moverse (seleccione todas sus celdas); una parte de una, una tabla de datos o la fila de nombres de una tabla no pueden.
