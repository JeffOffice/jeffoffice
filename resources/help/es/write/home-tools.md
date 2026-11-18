# Cambiar mayúsculas y minúsculas, ordenar y pegar de otras formas
<!-- keywords: pegar opciones, botón opciones de pegado, cortar y pegar inteligente, espacios al pegar, línea horizontal, regla, mostrar formato, inspector de estilos, mayús+f1, tabulaciones, tabuladores, tabulación de barra, barra de tabulación, tabulaciones predeterminadas, distancia de tabulación, carácter de relleno, mantener con el siguiente, mantener líneas juntas, salto de página antes, líneas viudas, huérfanas, nivel de esquema, saltos de línea y de página, cambiar mayúsculas, mayúsculas, minúsculas, oración con mayúscula inicial, poner en mayúscula, alternar mayúsculas, mayús+f3, ordenar, ordenar párrafos, alfabético, orden, pegar, pegado especial, continuar lista, nueva lista, combinar lista, anidar tabla, celdas de hoja de cálculo, mantener solo texto, combinar formato, rtf, texto con formato, arrastrar texto, texto sin formato, imagen, portapapeles -->

Estas herramientas están en la ficha **Inicio**. Actúan sobre el texto seleccionado.

## Cambiar las mayúsculas y minúsculas de las letras

Una letra está en **mayúscula** (A) o en **minúscula** (a).

1. Seleccione el texto. Si no hay nada seleccionado, JeffWrite cambia la palabra en la que está el cursor.
2. En el grupo **Fuente**, haga clic en **Cambiar mayúsculas y minúsculas**.
3. Elija una opción:
   - **Oración con mayúscula inicial** pone en mayúscula la primera letra de cada oración. El resto queda en minúscula.
   - **minúsculas** convierte todas las letras en minúsculas.
   - **MAYÚSCULAS** convierte todas las letras en mayúsculas.
   - **Poner En Mayúsculas Cada Palabra** pone en mayúscula la primera letra de cada palabra.
   - **tIPO iNVERSO** convierte cada mayúscula en minúscula, y cada minúscula en mayúscula.

Pulse **Mayús+F3** para recorrer tres de estas opciones en orden: **Poner En Mayúsculas Cada Palabra**, luego **MAYÚSCULAS** y después **minúsculas**.

El aspecto de las letras no cambia. La negrita sigue en negrita. **Ctrl+Z** deshace el cambio de una sola vez. Si el control de cambios está activado, el cambio aparece como una eliminación y una inserción.

## Ordenar párrafos

Ordenar pone los párrafos en orden, por ejemplo de la A a la Z. Úselo para una lista de nombres o para las líneas de una tabla hecha con tabulaciones o comas.

1. Seleccione dos o más párrafos.
2. En el grupo **Párrafo**, haga clic en **Ordenar**.
3. Elija el criterio de ordenación. Cada criterio tiene un **campo**, un **tipo** y un **orden**.
   - El **campo** es una parte del párrafo. El campo 1 es la primera parte. Si las partes están separadas por comas o tabulaciones, use el cuadro de abajo de las claves. Si deja **Párrafos**, todo el párrafo es el único campo.
   - El **tipo** es **Texto**, **Número** o **Fecha**. Un número se ordena por su valor, así que 9 va antes que 10.
   - El **orden** es **Ascendente** (de la A a la Z, de menor a mayor, de más antiguo a más reciente) o **Descendente**.
4. Use **Luego por** para desempatar.
5. Marque **La primera línea del párrafo es un encabezado** para dejarla en su sitio.
6. Haga clic en **Aceptar**.

Cada párrafo conserva su propio aspecto. Ordenar no se puede usar mientras el control de cambios está activado, porque una ordenación no es un cambio que mostrar. Desactive el control de cambios, ordene y vuelva a activarlo.

## Pegar de otras formas

Haga clic en la pequeña flecha debajo de **Pegar** para ver más opciones:

- Cuando el texto procede de otro programa que pone texto con formato (RTF) en el portapapeles, **Ctrl+V** conserva su aspecto: tipos de letra, tamaños, colores, negrita, listas, tablas, vínculos e imágenes. Si el programa no dio texto con formato, JeffWrite usa su forma de página web (HTML) y, después, solo las palabras. El texto con formato de más de 16 MB no se lee, y el pegado usa la siguiente forma. Arrastrar texto con formato desde otro programa a la página hace lo mismo.
- **Combinar formato** pega el texto con el aspecto del lugar al que va. Solo se conservan la negrita, la cursiva, el subrayado y el tachado.
- **Mantener solo texto** pega las palabras sin ningún formato.
- **Imagen** pega una imagen del portapapeles. Aparece atenuada cuando el portapapeles no contiene ninguna imagen.
- **Pegado especial** (**Ctrl+Alt+V**) muestra todas las formas en que se puede pegar lo que hay ahora en el portapapeles, y permite elegir una, como **Texto con formato (RTF)**, **Texto sin formato** o **HTML**.

Una opción aparece atenuada cuando el portapapeles no tiene nada que pueda usar.

Justo después de pegar, aparece junto al cursor un pequeño botón **Opciones de pegado**. Haga clic en él para elegir **Mantener formato de origen**, **Combinar formato** o **Mantener solo texto**. JeffWrite deshace el pegado y lo vuelve a hacer de ese modo. El botón desaparece en cuanto escribe o mueve el cursor.

Cuando el pegado trae párrafos de lista junto a una lista, el botón también ofrece **Continuar lista** (la numeración sigue la de la lista contigua), **Nueva lista** (la numeración empieza de nuevo en 1) y **Combinar lista** (los elementos se unen a la lista contigua y adoptan su aspecto). **Ctrl+V** normal inserta los elementos tal como vienen, y cada opción cambia ese resultado. Las mismas opciones son comandos del menú Pegar de la ficha Inicio y de **Pegado especial**, así que también se pueden usar con el teclado.

Cuando el portapapeles contiene celdas de hoja de cálculo (copiadas desde JeffCrunch u otro programa, como tabla o como líneas con tabulaciones entre las celdas), **Ctrl+V** normal las pega como las palabras que son. Para pegarlas como celdas, elija **Tabla** desde el botón, el menú Pegar o **Pegado especial**. Con el cursor dentro de una tabla, el botón ofrece **Anidar tabla** (una tabla dentro de la celda), **Combinar tabla** (las celdas se unen a la tabla, que recibe las columnas que necesite), **Insertar como filas nuevas** y **Sobrescribir celdas** (las celdas sustituyen a las de la tabla, desde la celda del cursor). Una tabla pegada tiene como máximo 500 filas y 63 columnas, y una celda contiene como máximo 4000 caracteres; lo que sobrepase se omite y aparece un mensaje que lo indica. Una tabla anidada dentro de otra puede tener como máximo 8 niveles de profundidad, y se rechaza una tabla anidada cuyas columnas serían demasiado estrechas para la celda. Cada pegado es un solo paso: **Deshacer** lo quita todo, y **Rehacer** lo vuelve a poner.

**Cortar y pegar inteligente** mantiene bien los espacios al mover palabras. Si corta palabras completas que estaban entre dos espacios, JeffWrite se lleva también uno de los espacios, para que no queden dos espacios seguidos. Si pega una palabra pegada a otra, añade un espacio entre ellas. No modifica el interior de una palabra. Para desactivarlo, haga clic en **Archivo > Opciones > Avanzadas** y desmarque **Cortar y pegar inteligente**.

## Saltos de línea y de página, y tabulaciones

Haga clic en el pequeño botón de la esquina del grupo **Párrafo** para abrir el cuadro **Párrafo**. Debajo de las opciones de espaciado hay estas:

- **Control de líneas viudas y huérfanas** evita que una línea suelta del párrafo quede sola al principio o al final de una página.
- **Mantener con el siguiente** mantiene el párrafo en la misma página que el siguiente. Úselo en los títulos.
- **Mantener líneas juntas** mantiene todas las líneas del párrafo en una misma página.
- **Suprimir números de línea** excluye este párrafo de la numeración de líneas de la página (vea Diseño > Números de línea).
- **Salto de página antes** empieza el párrafo en una página nueva.
- **No dividir palabras** evita que las palabras del párrafo se separen con un guion al final de una línea.
- **No añadir espacio entre párrafos del mismo estilo** quita el espacio entre dos párrafos que tienen el mismo estilo.
- **Nivel de esquema** indica dónde está el párrafo en el esquema del documento: **Texto del cuerpo**, o un nivel del 1 al 9.

Haga clic en **Tabulaciones** en el cuadro para fijar las tabulaciones. Una tabulación es un lugar al que salta el texto cuando pulsa **Tab**. Escriba su posición, elija cómo se alinea el texto en ese punto (izquierda, centro, derecha o decimal) y elija un carácter de relleno. El carácter de relleno es la fila de puntos, guiones o la línea que llena el espacio antes de la tabulación. Haga clic en **Establecer** para añadirla. Para quitar una, selecciónela en la lista y haga clic en **Borrar**. **Borrar todo** las quita todas.

Elija **Barra** para fijar una tabulación de barra. Una tabulación de barra no detiene el texto. Dibuja una línea fina y recta, de arriba abajo, en ese punto de cada línea del párrafo. Sirve para poner un filete entre columnas de texto. Una tabulación de barra no tiene carácter de relleno. La regla no muestra las tabulaciones de barra. Un párrafo admite hasta 64. Pendiente: en un párrafo que se lee de derecha a izquierda, las tabulaciones de barra todavía se miden desde el borde izquierdo. Se fijan y se borran en el cuadro **Tabulaciones**.

**Tabulaciones predeterminadas** es la distancia entre las tabulaciones que tiene un párrafo cuando no tiene ninguna propia, y más allá de la última que sí tiene. La distancia habitual es de 1,3 cm. Este valor se aplica a todo el documento, no solo al párrafo en el que está. La nueva distancia se aplica a todos los párrafos en cuanto hace clic en **Aceptar**.

## Dibujar una línea horizontal

Haga clic en **Línea horizontal** en el grupo **Párrafo**. JeffWrite añade un párrafo propio con una línea fina debajo, después del párrafo en el que está el cursor. El cursor pasa a un párrafo normal situado debajo de la línea. Para quitar la línea, seleccione su párrafo y elimínelo.

## Ver lo que hay definido en la posición del cursor

Pulse **Mayús+F1**, o use **Mostrar formato**. Un cuadro muestra los estilos, la fuente, el tamaño, el aspecto de las letras, el color, y la alineación, las sangrías y el espaciado del párrafo en el cursor. Solo los muestra. Para cambiarlos, use los cuadros **Fuente** y **Párrafo**.
