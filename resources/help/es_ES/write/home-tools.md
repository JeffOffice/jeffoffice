# Cambiar mayúsculas, ordenar y pegar de otras formas

<!-- keywords: botón opciones de pegado, cortar y pegar inteligente, espacios al pegar, línea horizontal, regla, mostrar formato, inspector de estilos, mayús+f1, tabulaciones, tabulación, tabulación de barra, barra de tabulación, tabulaciones predeterminadas, distancia de tabulación, carácter de relleno, guía, conservar con el siguiente, mantener líneas juntas, salto de página antes, viuda, huérfana, nivel de esquema, saltos de línea y de página, cambiar mayúsculas y minúsculas, mayúsculas, minúsculas, oración, mayúscula inicial, alternar mayúsculas, mayús+f3, ordenar, ordenar párrafos, alfabético, orden, pegar, pegado especial, continuar lista, nueva lista, combinar lista, anidar tabla, celdas de hoja de cálculo, mantener solo texto, combinar formato, rtf, texto con formato, arrastrar texto, texto sin formato, imagen, portapapeles -->

Estas herramientas están en la ficha **Inicio**. Actúan sobre el texto seleccionado.

## Cambiar las mayúsculas de las letras

La **capitalización** de una letra indica si es una letra mayúscula (A) o minúscula (a).

1. Seleccione el texto. Si no hay nada seleccionado, JeffWrite cambia la palabra en la que está el cursor.
2. En el grupo **Fuente**, haga clic en **Cambiar mayúsculas y minúsculas**.
3. Elija una opción:
   - **Oración con mayúscula inicial** pone en mayúscula la primera letra de cada oración. El resto queda en minúscula.
   - **minúsculas** pone todas las letras en minúscula.
   - **MAYÚSCULAS** pone todas las letras en mayúscula.
   - **Poner En Mayúsculas Cada Palabra** pone en mayúscula la primera letra de cada palabra.
   - **tIPO iNVERSO** convierte cada mayúscula en minúscula, y cada minúscula en mayúscula.

Pulse **Mayús+F3** para recorrer tres de ellas en orden: **Poner En Mayúsculas Cada Palabra**, después **MAYÚSCULAS** y después **minúsculas**.

El aspecto de las letras se mantiene. La negrita sigue siendo negrita. **Ctrl+Z** deshace el cambio en un solo paso. Si el control de cambios está activado, el cambio aparece como una eliminación y una inserción.

## Ordenar párrafos

Ordenar coloca los párrafos en orden, por ejemplo de la A a la Z. Úselo para una lista de nombres o para las líneas de una tabla hecha con tabulaciones o comas.

1. Seleccione dos o más párrafos.
2. En el grupo **Párrafo**, haga clic en **Ordenar**.
3. Elija el criterio de ordenación. Cada opción tiene un **campo**, un **tipo** y un **orden**.
   - El **campo** es una parte del párrafo. El campo 1 es la primera parte. Si las partes están separadas por comas o tabulaciones, indíquelo en el cuadro que hay debajo de las claves. Si deja **Párrafos**, todo el párrafo es el único campo.
   - El **tipo** es **Texto**, **Número** o **Fecha**. Un número se ordena por su valor, así que 9 va antes que 10.
   - El **orden** es **Ascendente** (de la A a la Z, de menor a mayor, de antiguo a reciente) o **Descendente**.
4. Use **Luego por** para desempatar.
5. Marque **La primera fila contiene encabezados** para dejarla en su sitio.
6. Haga clic en **Aceptar**.

Cada párrafo conserva su propio aspecto. Mientras el control de cambios está activado, la ordenación se hace como de costumbre, sin registro, y la barra de estado indica que no se registró como cambio controlado. No se puede ordenar las filas de una tabla que tenga una fila insertada o eliminada: acepte o rechace antes esos cambios.

## Pegar de otras formas

Haga clic en la pequeña flecha bajo **Pegar** para ver más opciones:

- Cuando el texto procede de otro programa que pone texto con formato (RTF) en el portapapeles, **Ctrl+V** conserva su aspecto: fuentes, tamaños, colores, negrita, listas, tablas, vínculos e imágenes. Si el programa no proporcionó texto con formato, JeffWrite usa su forma de página web (HTML) y, después, solo las palabras. El texto con formato de más de 16 MB no se lee, y el pegado usa la siguiente forma. Arrastrar texto con formato desde otro programa a la página hace lo mismo.
- **Combinar formato** pega el texto con el aspecto del lugar al que va. Solo se conservan la negrita, la cursiva, el subrayado y el tachado.
- **Mantener solo texto** pega las palabras sin ningún formato.
- **Imagen** pega una imagen del portapapeles. Aparece atenuado cuando el portapapeles no contiene ninguna imagen.
- **Pegado especial** (**Ctrl+Alt+V**) muestra todas las formas en que se puede pegar lo que hay en el portapapeles en ese momento, y le permite elegir una, como **Texto con formato (RTF)**, **Texto sin formato** o **HTML**.

Una opción aparece atenuada cuando el portapapeles no tiene nada que pueda usar.

Justo después de pegar, aparece junto al cursor un pequeño botón **Opciones de pegado**. Haga clic en él para elegir **Conservar formato de origen**, **Combinar formato** o **Mantener solo texto**. JeffWrite deshace el pegado y lo vuelve a hacer de esa manera. El cambio es un solo paso: un solo **Deshacer** deshace todo el pegado, por muchas veces que cambie de opinión. Desde el teclado, pulse la tecla **Ctrl** (la tecla **Control** en un Mac) después de pegar para abrir el menú del botón, y luego use las teclas de flecha y **Intro** para elegir. El botón desaparece en cuanto escribe, mueve el cursor o pulsa **Esc**, y no se ofrece en un documento que no se puede modificar.

**Establecer pegado predeterminado** está en la página **Avanzado** de **Opciones**. Tiene un cuadro para pegar dentro del mismo documento, otro para pegar entre documentos y otro para pegar desde otros programas. Cada uno puede ser **Conservar formato de origen** (lo que era antes), **Combinar formato** o **Mantener solo texto**, y **Pegar** (**{key:Paste}**) hace lo que se haya configurado según el origen del contenido. Por ejemplo: configure el pegado desde otros programas como **Mantener solo texto**, y las palabras copiadas de una página web tomarán el formato del lugar donde se pegan. Aun así, puede elegir otra forma para cualquier pegado concreto con el botón **Opciones de pegado**.

Otros dos cuadros de esa página determinan qué hace **Pegar** con los elementos de lista pegados junto a una lista (**Tal como aparecen**, **Continuar lista**, **Nueva lista** o **Combinar lista**) y con las celdas de hoja de cálculo (**Como palabras**, **Tabla**, **Anidar tabla**, **Combinar tabla**, **Insertar como nuevas filas** o **Sobrescribir celdas**). Una opción solo se usa donde encaja: **Anidar tabla** con el cursor fuera de una tabla, por ejemplo, pega las celdas como palabras.

Cuando el pegado trae párrafos de lista junto a una lista, el botón también ofrece **Continuar lista** (la numeración sigue la de la lista contigua), **Nueva lista** (la numeración empieza de nuevo en 1) y **Combinar lista** (los elementos se unen a la lista contigua y toman su aspecto). El **Ctrl+V** normal pone los elementos tal como llegaron, y una opción cambia eso. Las mismas opciones son comandos en el menú Pegar de la ficha Inicio y en **Pegado especial**, de modo que también están disponibles desde el teclado.

Cuando el portapapeles contiene celdas de hoja de cálculo (copiadas desde JeffCrunch u otro programa como tabla o como líneas con tabulaciones entre las celdas), el **Ctrl+V** normal las pega como las palabras que son. Para pegarlas como celdas, elija **Tabla** en el botón, en el menú Pegar o en **Pegado especial**. Con el cursor dentro de una tabla, el botón ofrece **Anidar tabla** (una tabla dentro de la celda), **Combinar tabla** (las celdas se unen a la tabla, que obtiene las columnas que necesita), **Insertar como nuevas filas**, **Insertar como nuevas columnas** (las celdas se colocan en columnas nuevas, a partir de la celda del cursor, desde su fila hacia abajo; una tabla puede tener como máximo 256 columnas y 50 000 celdas, y si una celda combinada atraviesa el lugar de las columnas, se rechaza con un mensaje) y **Sobrescribir celdas** (las celdas sustituyen a las de la tabla, desde la celda del cursor en adelante). Una tabla pegada tiene como máximo 500 filas y 63 columnas, y una celda contiene como máximo 4000 caracteres; lo que sobrepase se omite y un mensaje lo indica. Una tabla anidada dentro de otra puede tener como máximo 8 niveles de profundidad, y una tabla anidada cuyas columnas serían demasiado estrechas para la celda se rechaza. Cada pegado es un solo paso: **Deshacer** lo deshace todo, y **Rehacer** lo vuelve a poner.

**Cortar y pegar inteligente** mantiene bien los espacios al mover palabras. Si corta palabras completas que están entre dos espacios, JeffWrite se lleva también uno de los espacios, para que no quede un espacio doble. Si pega una palabra pegada a otra, añade un espacio entre ellas. No modifica el interior de una palabra. Para desactivarlo, haga clic en **Archivo > Opciones > Avanzado** y desmarque **Cortar y pegar inteligente**.

## Saltos de línea y de página, y tabulaciones

Haga clic en el pequeño botón de la esquina del grupo **Párrafo** para abrir el cuadro **Párrafo**. Bajo las opciones de espaciado están estas:

- **Control de líneas viudas y huérfanas** evita que una línea suelta de un párrafo quede sola al principio o al final de una página.
- **Conservar con el siguiente** mantiene el párrafo en la misma página que el siguiente. Úselo en los títulos.
- **Mantener líneas juntas** mantiene todas las líneas del párrafo en una misma página.
- **Suprimir números de línea** excluye este párrafo cuando la página numera sus líneas (vea Diseño > Números de línea).
- **Salto de página antes** hace que el párrafo empiece en una página nueva.
- **No dividir palabras** impide que las palabras del párrafo se separen con un guion al final de una línea.
- **No añadir espacio entre párrafos del mismo estilo** elimina el espacio entre dos párrafos que tienen el mismo estilo.
- **Nivel de esquema** indica dónde está el párrafo en el esquema del documento: **Texto del cuerpo**, o un nivel del 1 al 9.

Haga clic en **Tabulaciones** en el cuadro para fijar las tabulaciones. Una tabulación es un lugar al que salta el texto cuando pulsa **Tab**. Escriba su posición, elija cómo se alinea el texto allí (izquierda, centro, derecha o decimal) y elija un carácter de relleno. Un carácter de relleno es la fila de puntos, guiones o una línea que llena el espacio antes de la tabulación. Haga clic en **Establecer** para añadirla. Para quitar una, selecciónela en la lista y haga clic en **Borrar**. **Borrar todas** las quita todas.

Elija **Barra** para fijar una tabulación de barra. Una tabulación de barra no detiene el texto. Dibuja una línea fina y recta, de arriba abajo, en ese lugar de cada línea del párrafo. Sirve para poner un filete entre columnas de texto. Una tabulación de barra no tiene carácter de relleno. La regla no muestra las tabulaciones de barra. Un párrafo admite hasta 64. Pendiente: en un párrafo que se lee de derecha a izquierda, las barras todavía se miden desde el borde izquierdo. Fíjelas y bórrelas en el cuadro **Tabulaciones**.

**Tabulaciones predeterminadas** es la distancia entre las tabulaciones que tiene un párrafo cuando no tiene ninguna propia, y más allá de la última que sí tiene. La distancia habitual es de 1,25 cm. Esta opción afecta a todo el documento, no solo al párrafo en el que está. La nueva distancia se aplica a todos los párrafos en cuanto hace clic en **Aceptar**.

## Bordes y sombreado de párrafos

Seleccione uno o más párrafos. En el grupo **Párrafo**, el botón **Bordes** tiene las opciones rápidas: **Borde inferior**, **Borde superior**, **Borde izquierdo**, **Borde derecho**, **Todos los bordes** y **Sin bordes**. Una opción rápida dibuja una sola línea, de medio punto de grosor, a un punto del texto. Una opción para un solo lado deja los demás lados como estaban.

**Bordes y sombreado**, al final de esa lista, abre un cuadro. Marque los lados que llevarán una línea, o haga clic en **Caja** o **Ninguno**. Marque **Entre** para poner una línea entre los párrafos seleccionados que tienen los mismos bordes. Después elija el **Estilo** (simple, doble, grueso, punteado, discontinuo, guiones cortos, guion-punto o guion-punto-punto), el **Ancho**, el **Color** y la **Distancia del texto**. **Sombreado** rellena los párrafos con un color. El botón **Sombreado**, junto a **Bordes**, establece solo el relleno, con los mismos colores que **Color de fuente**.

Las líneas y el relleno son un solo paso que **Deshacer** revierte, y se conservan al guardar en un fichero .docx o .odt. Si un párrafo procede de un fichero con su propio borde o relleno, su elección ocupa su lugar.

## Dibujar una línea horizontal

Haga clic en **Línea horizontal** en el grupo **Párrafo**. JeffWrite añade un párrafo propio con una línea fina debajo, después del párrafo en el que está el cursor. El cursor pasa a un párrafo normal debajo de la línea. Para quitar la línea, seleccione su párrafo y elimínelo.

## Ver lo que está establecido donde está el cursor

Pulse **Mayús+F1**, o use **Mostrar formato**. Un cuadro muestra los estilos, la fuente, el tamaño, el aspecto de las letras, el color, y la alineación, las sangrías y el espaciado del párrafo en el cursor. Solo los muestra. Para cambiarlos, use los cuadros **Fuente** y **Párrafo**.

## El panel Portapapeles

El panel Portapapeles recuerda las últimas 24 cosas que copió o cortó en el programa de esta ventana, para que pueda pegar una anterior sin volver a copiarla. Haga clic en la pequeña flecha de la esquina del grupo **Portapapeles** de la ficha **Inicio** (o pulse **Alt**, después **H** y luego **FO**) para abrirlo. Aparece un panel junto a la página.

Cada elemento muestra una pequeña vista previa y el tipo de contenido: texto, texto con formato, celdas, una imagen, formas o diapositivas. El más reciente está arriba. Haga clic en un elemento para pegarlo donde esté el cursor o la selección. Se pega como cualquier otro pegado: un paso de **Deshacer** lo revierte, y el botón Opciones de pegado aparece como de costumbre.

- **Pegar todo** pega todos los elementos, del más antiguo al más reciente, de modo que aparezcan en el orden en que los copió.
- **Eliminar** quita el elemento que eligió. **Borrar todo** quita todos.
- El panel solo recoge elementos mientras está abierto. Para seguir recogiéndolos después de cerrarlo, marque **Seguir recopilando cuando se cierre este panel**. La opción queda guardada, y la recogida empieza al iniciarse el programa, antes de abrir el panel.
- Para recoger también lo que copia en otros programas, marque **Recopilar también lo que se copie en otros programas**. Solo se conservan el texto sin formato y las imágenes, y esta opción se desactiva de nuevo cada vez que abre el programa. Con esta opción activada, una copia hecha en otro programa se lee completa antes de que este programa decida si la conserva, de modo que una copia muy grande puede hacer que el programa se detenga un momento y use tanta memoria como tamaño tenga la copia. Solo se lee una forma: su texto cuando tiene texto, y si no, una imagen. Con esta opción desactivada, no se lee nada copiado en otros programas.

Los elementos se guardan solo en la memoria del programa. No se guarda nada en ningún fichero, y la lista se vacía al cerrar la última ventana del programa. Un elemento permanece en la lista aunque se cierre el documento del que procede. Un elemento de más de 8 MB no se conserva, y el panel se lo indica; una imagen cuenta 4 bytes por cada punto (píxel), así que una imagen de más de unos 2 millones de píxeles es demasiado grande. Todos los elementos juntos no pueden superar los 64 MB, y se eliminan primero los más antiguos. Nunca se conserva lo que un gestor de contraseñas marque como secreto.
