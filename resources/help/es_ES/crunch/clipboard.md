# Cortar, copiar y pegar

<!-- keywords: cortar, copiar, pegar, portapapeles, mover, duplicar, pegado especial, valores, fórmulas, formatos, transponer, omitir blancos, sumar, restar, multiplicar, dividir, operación, combinadas, texto, tabulación, html, página web, otro programa, Esc, hormigas marchantes, bordes, anchos de columna, notas, comentarios, formatos de número, todo excepto bordes, imagen, mapa de bits, copiar como imagen, imagen impresa, como se imprime, pegar vínculo, hipervínculo, importar texto, texto delimitado, soltar texto, coma, punto y coma, csv, separador, establecer pegado predeterminado, pegado predeterminado, ancho fijo, copy, paste -->

**Copiar** (**{key:Copy}**) pone una copia de las celdas seleccionadas en el Portapapeles, un espacio que el equipo guarda para una cosa a la vez. **Pegar** (**{key:Paste}**) coloca lo que hay en el Portapapeles en las celdas, a partir de la celda seleccionada. **Cortar** (**{key:Cut}**) funciona como copiar, pero al pegar, las celdas se mueven y el lugar original queda vacío.

Un borde discontinuo animado muestra las celdas que se copiaron o cortaron. Pulse **Esc** para quitar el borde; entonces las celdas ya no están listas para pegarse. Las celdas cortadas también se liberan si hace cualquier otro cambio antes de pegarlas (escribir, insertar, deshacer): córtelas de nuevo si todavía quiere moverlas.

## Pegar dentro de JeffCrunch

Se traslada todo: los valores, las fórmulas, los formatos y las celdas combinadas. Una fórmula sigue apuntando a las mismas celdas *en relación con su posición*. Por ejemplo: copie **=A1*2** desde B1 y péguela en B5, y se convertirá en **=A5*2**. Una parte con signo de dólar, como **$A$1**, se mantiene tal cual. Si selecciona un bloque que es un múltiplo exacto del tamaño de lo copiado, la copia se repite para llenarlo: copie una celda, seleccione veinte celdas, pegue, y las veinte recibirán el contenido.

Si pega un número demasiado ancho para una columna que conserva su ancho inicial, la columna se ensancha.

## Cortar mueve las celdas

Cuando corta celdas y las pega, las celdas se mueven de verdad. Toda fórmula que apuntaba a ellas pasa a apuntar a su nueva posición. Por ejemplo: B1 contiene **=A1+A2**. Corte A1:A2 y pegue en E4, y B1 pasará a ser **=E4+E5**. Lo que hubiera en las celdas sobre las que pegó se pierde (**Deshacer** lo recupera), y una fórmula que leía alguna de esas celdas muestra **#¡REF!**, porque la celda que leía ya no está. Si las celdas cortadas son solo una parte de una celda combinada, o si pega sobre solo una parte de una, no se mueve nada: seleccione la celda combinada completa. Al cortar celdas y pegarlas en otra hoja, las fórmulas siguen leyendo las celdas que leían antes (indican la hoja). El Portapapeles se vacía después de un corte y pegado.

## Pegado especial

**Ctrl+Alt+V**, o la flecha situada debajo de **Pegar** en la pestaña Inicio, permite elegir qué pegar:

- **Todo**, **Fórmulas**, **Valores** o **Formatos**. *Valores* convierte una fórmula en el número o el texto que muestra.
- **Todo excepto bordes**: todo, pero las celdas conservan los bordes que tenían.
- **Fórmulas y formatos de número**, o **Valores y formatos de número**: las fórmulas (o los valores) llegan con el modo en que se muestran los números, como porcentaje o fecha, pero no los colores, las fuentes ni los bordes. Las celdas conservan el aspecto que tenían.
- **Anchos de columna**: las columnas en las que pega reciben los anchos de las columnas copiadas.
- **Comentarios y notas**: las notas de las celdas copiadas se colocan en las celdas donde pega. No cambia ningún valor de celda.
- Una **operación**: **Sumar**, **Restar**, **Multiplicar** o **Dividir**. Los números copiados se combinan con los que ya hay en las celdas. Por ejemplo: las celdas contienen 10 y 20, copió 5 y eligió *Multiplicar*: pasarán a ser 50 y 100.
- **Omitir blancos**: las celdas vacías de la copia dejan como estaban las celdas en las que caen.
- **Transponer**: las filas pasan a ser columnas, y las columnas, filas.

## El botón Opciones de pegado

Justo después de pegar celdas copiadas en JeffCrunch, aparece un pequeño botón **Opciones de pegado** junto a la última celda pegada. Haga clic en él para elegir otra forma de colocar las mismas celdas: **Mantener formato de origen** (todo), **Valores**, **Fórmulas**, **Formato**, **Sin bordes**, **Fórmulas y formato de número**, **Valores y formato de número**, **Mantener anchos de columna de origen** o **Transponer**. JeffCrunch deshace el pegado y lo vuelve a hacer de esa manera. El cambio cuenta como un solo paso: un único **Deshacer** quita todo el pegado, sin importar cuántas veces haya cambiado de opinión. Por ejemplo: copie A1:B2, donde B1 es **=A1*2**, y pegue en F5. Elija **Valores** y G5 contendrá el número 2 en lugar de **=F5*2**; elija **Mantener formato de origen** y la fórmula vuelve.

Desde el teclado, pulse la tecla **Ctrl** (la tecla **Control** en un Mac) después de pegar para abrir el menú del botón, y luego use las teclas de flecha y **Entrar** para elegir. El botón desaparece cuando cambia una celda o cualquier otra cosa del libro, copia algo distinto, pulsa **Esc**, o desplaza la última celda pegada fuera de la vista (vuelve al desplazarse de nuevo). No aparece después de un corte y pegado, ni para texto procedente de otro programa.

## Pegar vínculo, Pegar como hipervínculo e Importar texto al pegar

En la flecha situada debajo de **Pegar** hay tres opciones más. Funcionan con celdas copiadas en el mismo libro, salvo la última.

- **Pegar vínculo** coloca en cada celda una fórmula que apunta a la celda copiada. Por ejemplo: copie A1:B2, seleccione D4 y use **Pegar vínculo**: D4 pasará a ser **=Hoja1!$A$1**. Cuando cambie A1, D4 cambiará con él. Una celda copiada que está vacía muestra 0.
- **Pegar como hipervínculo** coloca en cada celda las mismas palabras que muestra la celda copiada, y convierte cada una en un vínculo a la celda copiada. Mantenga pulsada **Ctrl** y haga clic en una para ir a esa celda. Funciona con hasta 2000 celdas a la vez.
- **Importar texto al pegar** sirve para texto procedente de otro programa. Abre la misma ventana que al abrir un fichero de texto: indica qué separa los datos (una coma, un tabulador, un espacio, un punto y coma, una barra u otro carácter), qué encierra un dato entre comillas, la primera fila que se usará y cómo leer cada columna (tal como está escrita, como texto, como fecha o se omite). Algunos textos alinean sus datos en columnas de igual anchura, con espacios entre ellas. Marque **Las columnas tienen un ancho fijo** e indique dónde empieza cada columna: cada línea se corta en esos puntos y se eliminan los espacios alrededor de cada dato. Elija *texto* para una columna de códigos como 007, para que se conserven los ceros. Los datos se colocan en las celdas, a partir de la celda seleccionada, como un solo paso que puede deshacer.

## Texto con comas, punto y coma o columnas

Cuando el texto que pega desde otro programa es claramente una tabla, JeffCrunch abre la misma ventana que al abrir un fichero de texto, para que pueda comprobar cómo se separa antes de que algo llegue a las celdas. Esto ocurre solo con **Pegar** normal (**{key:Paste}**), nunca con **Pegado especial** ni con una de sus opciones. Sucede cuando el texto tiene al menos dos líneas y ningún tabulador, y, o bien todas las líneas tienen el mismo número de datos, al menos tres, separados por comas, punto y coma o barras, con algo en cada columna (un dato entre comillas puede contener una coma); o bien los datos están alineados en columnas de igual anchura, con los mismos espacios entre ellas en todas las líneas (al menos tres líneas y al menos dos separaciones). Los números con separador de miles, como 1,234, y las líneas de código o las listas que terminan en coma o punto y coma no se toman como tabla. La ventana empieza con la separación que se haya detectado en el texto. Si los números tienen comas y los datos están separados por punto y coma, la coma es el separador decimal. Haga clic en **Aceptar** para colocar los datos, o en **Cancelar** para no colocar nada.

Por ejemplo: copie estas líneas de un correo electrónico.

    nombre,cant,precio
    Manzana,12,1.50
    Pera,3,0.25

Si las pega con una tercera columna, como un precio, la ventana muestra tres columnas, con **Coma** marcada. Una frase en una sola línea, líneas con distinto número de comas o con solo dos datos por línea se pegan como palabras, sin ventana. El texto con tabuladores va directamente a las celdas, como siempre. Para abrir la ventana con cualquier texto, elija **Importar texto al pegar** (arriba).

Lo mismo ocurre cuando arrastra texto desde otro programa y lo suelta sobre la cuadrícula: se coloca en la celda seleccionada. Soltarlo en cualquier otro lugar de la ventana no hace nada.

## Establecer pegado predeterminado

**Establecer pegado predeterminado** es la última opción de la flecha situada debajo de **Pegar**. Decide qué colocan el botón **Pegar** y **{key:Paste}** cuando ha copiado celdas: todo (al principio), o solo los valores, solo las fórmulas, solo el formato, etc. Por ejemplo: elija *Valores*, y una fórmula copiada aparecerá como el número que muestra. La elección se conserva para la próxima vez. No cambia **Pegado especial** ni las opciones del menú, y el texto de otros programas se pega tal cual. Elija *Todo* para volver a la situación inicial.

La misma opción está en la página **Edición** de **Opciones**, en dos cuadros: uno para las celdas copiadas en este libro, y otro para las copiadas en otro libro. Hasta que elija una para otro libro, seguirá la primera.

## El menú Pegar e imágenes

La flecha situada debajo de **Pegar** muestra las opciones habituales, y también **Sin bordes**, **Fórmulas y formato de número**, **Valores y formato de número**, **Mantener anchos de columna de origen** y **Comentarios y notas**. Son las mismas opciones que en Pegado especial, con un solo clic cada una.

La flecha junto a **Copiar** tiene **Copiar como imagen** y **Copiar como mapa de bits**. Cada una coloca en el Portapapeles una imagen de las celdas seleccionadas, dibujada tal como se ven, con las líneas de cuadrícula. Un mapa de bits tiene el tamaño de la pantalla. Una imagen tiene el doble de puntos, así que se mantiene nítida al ampliarla. **Copiar como imagen impresa** dibuja las celdas tal como las mostraría el papel: las líneas de cuadrícula solo aparecen si la configuración de página las imprime. Péguela en JeffWrite o JeffDeck como imagen. Si las celdas no caben en una sola imagen, seleccione menos.

## Entre programas

Al copiar, JeffCrunch también pone las celdas en el Portapapeles como texto (celdas separadas por tabuladores, una fila por línea) y como tabla para páginas web, para que otros programas puedan usarlas. Cuando pega algo que no procede de JeffCrunch, como texto con tabuladores o una tabla de una página web u otra hoja de cálculo, cada dato va a su propia celda y se interpreta como si lo hubiera escrito: números, fechas y fórmulas que empiezan por un signo igual. Copiar desde otro libro abierto en JeffCrunch trae valores, fórmulas y formatos de número, pero no el resto del formato. Si los pega en JeffDeck o JeffPub, se colocan como una tabla que conserva las fuentes, los sombreados, los bordes y los números tal como se muestran (consulte **Pegar celdas como tabla** en su Ayuda).

## Lo que no se puede pegar

JeffCrunch le avisa y no cambia nada cuando el pegado afectaría a parte de una **fórmula matricial**, a la fila de nombres de columna de una **tabla**, a celdas bloqueadas de una **hoja protegida**, más allá del final de la hoja, o a más de 100 000 celdas. Una **fórmula matricial** completa puede cortarse y moverse (seleccione todas sus celdas); una parte de ella, una tabla de datos o la fila de nombres de una tabla no pueden.

## El panel Portapapeles

El panel Portapapeles recuerda las últimas 24 cosas copiadas o cortadas en el programa de esta ventana, para que pueda pegar una anterior sin volver a copiarla. Haga clic en la pequeña flecha de la esquina del grupo **Portapapeles** de la pestaña **Inicio** (o pulse **Alt**, después **H** y después **FO**) para abrirlo. Aparece un panel junto a la página.

Cada elemento muestra una vista previa y el tipo de contenido: texto, texto con formato, celdas, una imagen, formas o diapositivas. El más reciente está arriba. Haga clic en un elemento para pegarlo donde esté el cursor o la selección. Se pega como cualquier otro pegado: un paso de **Deshacer** lo quita, y el botón Opciones de pegado aparece como de costumbre.

- **Pegar todo** pega todos los elementos, del más antiguo al más reciente, para que aparezcan en el orden en que los copió.
- **Eliminar** quita el elemento elegido. **Borrar todo** quita todos.
- El panel solo recoge elementos mientras está abierto. Para seguir recogiéndolos después de cerrarlo, marque **Seguir recopilando cuando se cierre este panel**. La opción se recuerda, y la recopilación empieza al iniciar el programa, antes de abrir el panel.
- Para recoger también lo que copie en otros programas, marque **Recopilar también lo que se copie en otros programas**. Solo se conservan el texto sin formato y las imágenes, y la opción se desactiva de nuevo cada vez que abre el programa. Con esta opción activada, una copia hecha en otro programa se lee completa antes de que este programa decida si la conserva, así que una copia muy grande puede hacer que este programa se detenga un momento y use tanta memoria como ocupe la copia. Solo se lee una forma: su texto, si lo tiene; si no, una imagen. Con la opción desactivada, no se lee nada de lo copiado en otros programas.

Los elementos se guardan solo en la memoria del programa. No se guarda nada en ningún fichero, y la lista se vacía al cerrar la última ventana del programa. Un elemento permanece en la lista aunque se cierre el documento del que procede. No se conserva ningún elemento de más de 8 MB, y el panel avisa cuando ocurre; una imagen ocupa 4 bytes por cada punto (píxel), así que una imagen de más de unos 2 millones de píxeles es demasiado grande. Todos los elementos juntos ocupan como máximo 64 MB, y los más antiguos se eliminan primero. Nunca se conserva nada que un gestor de contraseñas marque como secreto.
