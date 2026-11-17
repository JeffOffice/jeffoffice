# Cambiar mayúsculas, ordenar y pegar de otras formas

<!-- keywords: pegar opciones, botón opciones de pegado, cortar y pegar inteligente, espacios al pegar, línea horizontal, regla, mostrar formato, inspector de estilos, mayús+f1, tabulaciones, tabuladores, carácter de relleno, mantener con el siguiente, mantener líneas juntas, salto de página antes, líneas viudas, huérfanas, nivel de esquema, saltos de línea y de página, cambiar mayúsculas y minúsculas, mayúsculas, minúsculas, oración con mayúscula inicial, poner en mayúscula, tipo inverso, mayús+f3, ordenar, ordenar párrafos, alfabético, orden, pegar, pegado especial, continuar lista, nueva lista, combinar lista, anidar tabla, celdas de hoja de cálculo, conservar solo texto, combinar formato, rtf, texto con formato, arrastrar texto, texto sin formato, imagen, portapapeles -->

Estas herramientas están en la ficha **Inicio**. Actúan sobre el texto que seleccione.

## Cambiar las mayúsculas y minúsculas de las letras

La **capitalización** de una letra indica si es una letra mayúscula (A) o minúscula (a).

1. Seleccione el texto. Si no hay nada seleccionado, JeffWrite cambia la palabra en la que está el cursor.
2. En el grupo **Fuente**, haga clic en **Cambiar mayúsculas y minúsculas**.
3. Elija una opción:
   - **Oración con mayúscula inicial** pone en mayúscula la primera letra de cada oración. El resto va en minúscula.
   - **minúsculas** pone todas las letras en minúscula.
   - **MAYÚSCULAS** pone todas las letras en mayúscula.
   - **Poner En Mayúsculas Cada Palabra** pone en mayúscula la primera letra de cada palabra.
   - **tIPO iNVERSO** convierte cada mayúscula en minúscula, y cada minúscula en mayúscula.

Pulse **Mayús+F3** para recorrer tres de ellas en orden: **Poner En Mayúsculas Cada Palabra**, luego **MAYÚSCULAS** y después **minúsculas**.

El aspecto de las letras no cambia. La negrita se mantiene. **Ctrl+Z** deshace el cambio de una sola vez. Si el control de cambios está activado, el cambio aparece como una eliminación y una inserción.

## Ordenar párrafos

Ordenar coloca los párrafos en orden, por ejemplo de la A a la Z. Úselo para una lista de nombres o para las líneas de una tabla hecha con tabulaciones o comas.

1. Seleccione dos o más párrafos.
2. En el grupo **Párrafo**, haga clic en **Ordenar**.
3. Elija el criterio de ordenación. Cada criterio tiene un **campo**, un **tipo** y un **orden**.
   - El **campo** es una parte del párrafo. El campo 1 es la primera parte. Si las partes están separadas por comas o tabulaciones, indíquelo en el cuadro situado debajo de las claves. Si deja **Párrafos**, todo el párrafo es el único campo.
   - El **tipo** es **Texto**, **Número** o **Fecha**. Un número se ordena por su valor, así que 9 va antes que 10.
   - El **orden** es **Ascendente** (de la A a la Z, de menor a mayor, de más antiguo a más reciente) o **Descendente**.
4. Use **Luego por** para desempatar.
5. Marque **La primera fila es un encabezado** para dejarla en su sitio.
6. Haga clic en **Aceptar**.

Cada lugar conserva el aspecto de su párrafo. Ordenar no se puede usar mientras el control de cambios está activado, porque una ordenación no es un cambio que se pueda mostrar. Desactive el control de cambios, ordene y vuelva a activarlo.

## Pegar de otras formas

Haga clic en la pequeña flecha debajo de **Pegar** para ver más opciones:

- Cuando el texto procede de otro programa que pone texto con formato (RTF) en el portapapeles, **Ctrl+V** conserva su aspecto: tipos de letra, tamaños, colores, negrita, listas, tablas, vínculos e imágenes. Si el programa no dio texto con formato, JeffWrite usa su versión para páginas web (HTML) y, después, solo las palabras. El texto con formato de más de 16 MB no se lee, y el pegado usa la forma siguiente. Arrastrar texto con formato desde otro programa a la página hace lo mismo.
- **Combinar formato** pega el texto con el aspecto del lugar al que va. Solo se conservan la negrita, la cursiva, el subrayado y el tachado.
- **Conservar solo texto** pega las palabras sin ningún formato.
- **Imagen** pega una imagen del portapapeles. Aparece atenuada cuando el portapapeles no contiene ninguna imagen.
- **Pegado especial** (**Ctrl+Alt+V**) muestra todas las formas en que se puede pegar lo que hay en el portapapeles en ese momento, y le permite elegir una, como **Texto con formato (RTF)**, **Texto sin formato** o **HTML**.

Una opción aparece atenuada cuando el portapapeles no contiene nada que pueda usar.

Justo después de pegar, aparece junto al cursor un pequeño botón **Opciones de pegado**. Haga clic en él para elegir **Conservar formato de origen**, **Combinar formato** o **Conservar solo texto**. JeffWrite deshace el pegado y lo vuelve a hacer de esa manera. El botón desaparece en cuanto escribe o mueve el cursor.

Cuando el pegado trae párrafos de lista junto a una lista, el botón también ofrece **Continuar lista** (la numeración continúa a partir de la lista contigua), **Nueva lista** (la numeración empieza de nuevo en 1) y **Combinar lista** (los elementos se unen a la lista contigua y adoptan su aspecto). **Ctrl+V** normal inserta los elementos tal como vienen, y cada opción cambia ese resultado. Las mismas opciones son comandos del menú Pegar de la ficha Inicio y de **Pegado especial**, así que también se pueden usar desde el teclado.

Cuando el portapapeles contiene celdas de hoja de cálculo (copiadas de JeffCrunch u otro programa como tabla o como líneas con tabulaciones entre las celdas), **Ctrl+V** normal las pega como las palabras que son. Para pegarlas como celdas, elija **Tabla** en el botón, en el menú Pegar o en **Pegado especial**. Con el cursor dentro de una tabla, el botón ofrece **Anidar tabla** (una tabla dentro de la celda), **Combinar tabla** (las celdas se unen a la tabla, que obtiene las columnas que necesita), **Insertar como filas nuevas** y **Sobrescribir celdas** (las celdas sustituyen a las de la tabla, desde la celda del cursor en adelante). Una tabla pegada tiene como máximo 500 filas y 63 columnas, y una celda contiene como máximo 4000 caracteres; lo que exceda de ese límite se omite y aparece un mensaje que lo indica. Una tabla anidada dentro de otra puede tener como máximo 8 niveles de profundidad, y no se acepta una tabla anidada cuyas columnas serían demasiado estrechas para la celda. Cada pegado es un solo paso: **Deshacer** lo quita todo y **Rehacer** lo vuelve a poner.

**Cortar y pegar inteligente** mantiene bien los espacios al mover palabras. Si corta palabras completas que están entre dos espacios, JeffWrite elimina también uno de los espacios, para que no queden dos espacios seguidos. Si pega una palabra pegada a otra, añade un espacio entre ellas. No modifica el interior de una palabra. Para desactivarlo, haga clic en **Archivo > Opciones > Avanzadas** y desmarque **Cortar y pegar inteligente**.

## Saltos de línea y de página, y tabulaciones

Haga clic en el pequeño botón de la esquina del grupo **Párrafo** para abrir el cuadro **Párrafo**. Debajo de las opciones de espaciado hay estas:

- **Control de líneas viudas y huérfanas** evita que una línea suelta de un párrafo quede sola al principio o al final de una página.
- **Mantener con el siguiente** mantiene el párrafo en la misma página que el siguiente. Úselo en los títulos.
- **Mantener líneas juntas** mantiene todas las líneas del párrafo en una misma página.
- **Suprimir números de línea** excluye este párrafo de la numeración de líneas de la página (vea Diseño > Números de línea).
- **Salto de página antes** empieza el párrafo en una página nueva.
- **No dividir palabras** evita que las palabras del párrafo se partan con un guion al final de una línea.
- **No añadir espacio entre párrafos del mismo estilo** quita el espacio entre dos párrafos que tienen el mismo estilo.
- **Nivel de esquema** indica el lugar del párrafo en el esquema del documento: **Texto del cuerpo**, o un nivel del 1 al 9.

Haga clic en **Tabulaciones** en el cuadro para fijar las tabulaciones. Una tabulación es un punto al que salta el texto cuando pulsa **Tab**. Escriba su posición, elija cómo se alinea el texto en ese punto (izquierda, centro, derecha o decimal) y elija un carácter de relleno. El carácter de relleno es la fila de puntos, guiones o la línea que ocupa el espacio antes de la tabulación. Haga clic en **Establecer** para añadirla. Para quitar una, selecciónela en la lista y haga clic en **Borrar**. **Borrar todas** las elimina todas.

## Dibujar una línea horizontal

Haga clic en **Línea horizontal** en el grupo **Párrafo**. JeffWrite añade un párrafo propio con una línea fina debajo, después del párrafo en el que está el cursor. El cursor pasa a un párrafo normal situado debajo de la línea. Para quitar la línea, seleccione su párrafo y elimínelo.

## Ver qué formato hay en la posición del cursor

Pulse **Mayús+F1** o use **Mostrar formato**. Un cuadro muestra los estilos, la fuente, el tamaño, el aspecto de las letras, el color, y la alineación, las sangrías y el espaciado del párrafo en el cursor. Solo muestra la información. Para cambiarla, use los cuadros **Fuente** y **Párrafo**.
