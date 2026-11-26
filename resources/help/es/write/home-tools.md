# Cambiar mayúsculas, ordenar y pegar de otras formas
<!-- keywords: botón opciones de pegado, cortar y pegar inteligente, espacios al pegar, línea horizontal, regla, mostrar formato, inspector de estilos, mayús+f1, tabulaciones, tabuladores, tabulación de barra, barra de tabulación, tabulaciones predeterminadas, distancia de tabulación, carácter de relleno, líneas, conservar con el siguiente, mantener líneas juntas, salto de página antes, viuda, huérfana, nivel de esquema, saltos de línea y de página, cambiar mayúsculas, mayúsculas, minúsculas, oración con mayúscula inicial, poner en mayúscula, alternar mayúsculas, mayús+f3, ordenar, ordenar párrafos, alfabético, orden, pegar, pegado especial, continuar lista, nueva lista, combinar lista, anidar tabla, celdas de hoja de cálculo, conservar solo texto, combinar formato, rtf, texto con formato, formato de texto, arrastrar texto, texto sin formato, imagen, portapapeles -->

Estas herramientas están en la pestaña **Inicio**. Actúan sobre el texto seleccionado.

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

Pulse **Mayús+F3** para recorrer tres de ellas en orden: **Poner En Mayúsculas Cada Palabra**, luego **MAYÚSCULAS** y después **minúsculas**.

El aspecto de las letras no cambia. La negrita se mantiene. **Ctrl+Z** deshace el cambio de una sola vez. Si el control de cambios está activado, el cambio aparece como una eliminación y una inserción.

## Ordenar párrafos

Ordenar coloca los párrafos en orden, por ejemplo de la A a la Z. Úselo para una lista de nombres o para líneas de una tabla hecha con tabulaciones o comas.

1. Seleccione dos o más párrafos.
2. En el grupo **Párrafo**, haga clic en **Ordenar**.
3. Elija por qué ordenar. Cada opción tiene un **campo**, un **tipo** y un **orden**.
   - El **campo** es una parte del párrafo. El campo 1 es la primera parte. Si las partes están separadas por comas o tabulaciones, indíquelo en el cuadro que aparece debajo de las claves. Si deja **Párrafos**, todo el párrafo es el único campo.
   - El **tipo** es **Texto**, **Número** o **Fecha**. Un número se ordena por su valor, así que 9 va antes que 10.
   - El **orden** es **Ascendente** (de la A a la Z, de menor a mayor, de más antiguo a más reciente) o **Descendente**.
4. Use **Luego por** para desempatar.
5. Marque **El primer párrafo es un encabezado** para dejarlo en su sitio.
6. Haga clic en **Aceptar**.

Cada párrafo conserva su propio aspecto de párrafo. No se puede ordenar mientras el control de cambios está activado, porque una ordenación no es un cambio que mostrar. Desactive el control de cambios, ordene y vuelva a activarlo.

## Pegar de otras formas

Haga clic en la pequeña flecha bajo **Pegar** para ver más opciones:

- Cuando el texto procede de otro programa que copia texto con formato (RTF) al portapapeles, **Ctrl+V** conserva su aspecto: tipos de letra, tamaños, colores, negrita, listas, tablas, vínculos e imágenes. Si el programa no dio texto con formato, JeffWrite usa su forma de página web (HTML) y, después, solo las palabras. El texto con formato de más de 16 MB no se lee, y el pegado usa la forma siguiente. Arrastrar a la página texto con formato de otro programa hace lo mismo.
- **Combinar formato** pega el texto con el aspecto del lugar al que va. Solo se conservan la negrita, la cursiva, el subrayado y el tachado.
- **Conservar solo texto** pega las palabras sin ningún formato.
- **Imagen** pega una imagen del portapapeles. Aparece atenuada cuando el portapapeles no contiene ninguna imagen.
- **Pegado especial** (**Ctrl+Alt+V**) muestra todas las formas en que se puede pegar lo que hay ahora en el portapapeles, y le permite elegir una, como **Texto con formato (RTF)**, **Texto sin formato** o **HTML**.

Una opción aparece atenuada cuando el portapapeles no contiene nada que pueda usar.

Justo después de pegar, aparece junto al cursor un pequeño botón **Opciones de pegado**. Haga clic en él para elegir **Conservar formato de origen**, **Combinar formato** o **Conservar solo texto**. JeffWrite deshace el pegado y lo vuelve a hacer de esa forma. El cambio es un solo paso: un único **Deshacer** deshace todo el pegado, por muchas veces que haya cambiado de opinión. Desde el teclado, pulse la tecla **Ctrl** (la tecla **Control** en un Mac) después de pegar para abrir el menú del botón, y luego use las teclas de flecha e **Intro** para elegir. El botón desaparece en cuanto escribe, mueve el cursor o pulsa **Esc**, y no se ofrece en un documento que no se puede modificar.

**Establecer pegado predeterminado** está en la página **Avanzadas** de **Opciones**. Tiene un cuadro para pegar dentro del mismo documento, otro para pegar entre documentos y otro para pegar desde otros programas. Cada uno puede estar en **Conservar formato de origen** (como antes), **Combinar formato** o **Conservar solo texto**, y **Pegar** (**{key:Paste}**) hace lo que se haya configurado según el origen del contenido. Por ejemplo: si configura pegar desde otros programas como **Conservar solo texto**, las palabras copiadas de una página web tomarán el formato del lugar donde se pegan. Aun así, puede elegir otra forma para un pegado concreto con el botón **Opciones de pegado**.

Otras dos casillas de esa página determinan qué hace **Pegar** con los elementos de lista pegados junto a una lista (**Según lo copiado**, **Continuar lista**, **Nueva lista** o **Combinar lista**) y con las celdas de hoja de cálculo (**Como palabras**, **Tabla**, **Anidar tabla**, **Combinar tabla**, **Insertar como nuevas filas** o **Sobrescribir celdas**). Cada opción se usa solo donde corresponde: **Anidar tabla** con el cursor fuera de una tabla, por ejemplo, pega las celdas como palabras.

Cuando el pegado trae párrafos de lista junto a una lista, el botón también ofrece **Continuar lista** (la numeración sigue la de la lista contigua), **Nueva lista** (la numeración empieza de nuevo en 1) y **Combinar lista** (los elementos se unen a la lista contigua y toman su aspecto). **Ctrl+V** normal pega los elementos tal como vienen, y una opción cambia ese comportamiento. Las mismas opciones son comandos del menú Pegar de la pestaña Inicio y de **Pegado especial**, así que también se pueden usar desde el teclado.

Cuando el portapapeles contiene celdas de hoja de cálculo (copiadas desde JeffCrunch u otro programa como tabla o como líneas con tabulaciones entre las celdas), **Ctrl+V** normal las pega como las palabras que son. Para pegarlas como celdas, elija **Tabla** en el botón, en el menú Pegar o en **Pegado especial**. Con el cursor dentro de una tabla, el botón ofrece **Anidar tabla** (una tabla dentro de la celda), **Combinar tabla** (las celdas se unen a la tabla, que obtiene las columnas que necesite), **Insertar como nuevas filas**, **Insertar como nuevas columnas** (las celdas se colocan en columnas nuevas, después de la celda del cursor, desde su fila hacia abajo; una tabla puede tener como máximo 256 columnas y 50 000 celdas, y si una celda combinada se extiende sobre el lugar de las columnas, el pegado se rechaza con un mensaje) y **Sobrescribir celdas** (las celdas sustituyen a las de la tabla, desde la celda del cursor en adelante). Una tabla pegada tiene como máximo 500 filas y 63 columnas, y una celda admite como máximo 4000 caracteres; lo que exceda se omite y un mensaje lo indica. Una tabla anidada dentro de otra puede tener como máximo 8 niveles de profundidad, y se rechaza una tabla anidada cuyas columnas serían demasiado estrechas para la celda. Cada pegado es un solo paso: **Deshacer** lo deshace todo, y **Rehacer** lo vuelve a poner.

**Cortar y pegar inteligente** mantiene bien los espacios al mover palabras. Si corta palabras completas que están entre dos espacios, JeffWrite también elimina uno de esos espacios, para que no queden dos espacios seguidos. Si pega una palabra pegada a otra, añade un espacio entre ellas. No modifica el interior de una palabra. Para desactivarlo, haga clic en **Archivo > Opciones > Avanzadas** y desmarque **Cortar y pegar inteligente**.

## Saltos de línea y de página, y tabulaciones

Haga clic en el pequeño botón de la esquina del grupo **Párrafo** para abrir el cuadro **Párrafo**. Debajo de las opciones de espaciado están estas:

- **Control de viudas y huérfanas** evita que una línea suelta del párrafo quede sola al principio o al final de una página.
- **Conservar con el siguiente** mantiene el párrafo en la misma página que el siguiente. Úselo en los encabezados.
- **Mantener líneas juntas** mantiene todas las líneas del párrafo en una misma página.
- **Suprimir números de línea** excluye este párrafo cuando se numeran las líneas de la página (vea Diseño > Números de línea).
- **Salto de página antes** hace que el párrafo empiece en una página nueva.
- **No dividir palabras** evita que las palabras del párrafo se partan con un guion al final de una línea.
- **No añadir espacio entre párrafos del mismo estilo** elimina el espacio entre dos párrafos que tienen el mismo estilo.
- **Nivel de esquema** indica dónde está el párrafo en el esquema del documento: **Texto del cuerpo**, o un nivel del 1 al 9.

Haga clic en **Tabulaciones** en el cuadro para fijar las tabulaciones. Una tabulación es un lugar al que salta el texto cuando pulsa **Tab**. Escriba su posición, elija cómo se alinea el texto en ese punto (izquierda, centro, derecha o decimal) y elija un carácter de relleno. Un carácter de relleno es la fila de puntos, guiones o la línea que llena el espacio antes de la tabulación. Haga clic en **Establecer** para añadirla. Para quitar una, selecciónela en la lista y haga clic en **Borrar**. **Borrar todas** las elimina todas.

Elija **Barra** para fijar una barra de tabulación. Una barra no detiene el texto. Dibuja una línea vertical fina en ese punto en cada línea del párrafo. Sirve para poner un filete entre columnas de texto. Una barra no tiene carácter de relleno. La regla no muestra las barras. Un párrafo admite hasta 64. Pendiente: en un párrafo que se lee de derecha a izquierda, las barras todavía se miden desde el borde izquierdo. Puede establecerlas y borrarlas en el cuadro **Tabulaciones**.

**Tabulaciones predeterminadas** es la distancia entre las tabulaciones que tiene un párrafo cuando no tiene las suyas propias, y más allá de la última que sí tiene. La distancia habitual es de 1,3 cm. Este valor se aplica a todo el documento, no solo al párrafo en el que está. La nueva distancia se aplica a todos los párrafos en cuanto hace clic en **Aceptar**.

## Bordes y sombreado de párrafos

Seleccione uno o más párrafos. En el grupo **Párrafo**, el botón **Bordes** tiene opciones rápidas: **Borde inferior**, **Borde superior**, **Borde izquierdo**, **Borde derecho**, **Todos los bordes** y **Sin bordes**. Una opción rápida dibuja una sola línea de medio punto de grosor, a un punto del texto. Una opción para un lado deja los demás lados como estaban.

**Bordes y sombreado**, al final de esa lista, abre un cuadro. Marque los lados que recibirán una línea, o haga clic en **Cuadro** o **Ninguno**. Marque **Entre** para poner una línea entre los párrafos seleccionados que tienen los mismos bordes. Después elija el **Estilo** (simple, doble, grueso, punteado, discontinuo, guiones cortos, guion-punto o guion-punto-punto), el **Ancho**, el **Color** y la **Distancia del texto**. **Sombreado** rellena los párrafos con un color. El botón **Sombreado**, junto a **Bordes**, establece solo el relleno, con los mismos colores que **Color de fuente**.

Las líneas y el relleno forman un solo paso que **Deshacer** revierte, y se conservan al guardar un archivo .docx o .odt. Si un párrafo procede de un archivo que tenía su propio borde o relleno, su elección lo sustituye.

## Dibujar una línea horizontal

Haga clic en **Línea horizontal** en el grupo **Párrafo**. JeffWrite añade un párrafo propio con una línea fina debajo, después del párrafo en el que está el cursor. El cursor pasa a un párrafo normal debajo de la línea. Para quitar la línea, seleccione su párrafo y elimínelo.

## Ver lo que hay aplicado en la posición del cursor

Pulse **Mayús+F1**, o use **Mostrar formato**. Un cuadro muestra los estilos, la fuente, el tamaño, el aspecto de las letras, el color, y la alineación, las sangrías y el espaciado del párrafo en el cursor. Solo los muestra. Para cambiarlos, use los cuadros **Fuente** y **Párrafo**.

## El panel Portapapeles

El panel Portapapeles guarda los últimos 24 elementos que copió o cortó en el programa de esta ventana, para que pueda pegar uno anterior sin volver a copiarlo. Haga clic en la pequeña flecha de la esquina del grupo **Portapapeles** de la pestaña **Inicio** (o pulse **Alt**, luego **H** y luego **FO**) para abrirlo. Aparece un panel junto a la página.

Cada elemento muestra una vista previa y el tipo de contenido: texto, texto con formato, celdas, una imagen, formas o diapositivas. El más reciente aparece arriba. Haga clic en un elemento para pegarlo donde esté el cursor o la selección. Se pega como cualquier otro pegado: un paso de **Deshacer** lo revierte, y el botón Opciones de pegado aparece como de costumbre.

- **Pegar todo** pega todos los elementos, del más antiguo al más reciente, para que se lean en el orden en que los copió.
- **Eliminar** quita el elemento elegido. **Borrar todo** los quita todos.
- El panel solo recoge elementos mientras está abierto. Para seguir recogiéndolos después de cerrarlo, marque **Seguir recopilando cuando se cierre este panel**. La opción se recuerda, y la recogida empieza al iniciar el programa, antes de abrir el panel.
- Para recoger también lo que copie en otros programas, marque **Recopilar también lo que se copie en otros programas**. Solo se guardan texto sin formato e imágenes, y la opción vuelve a desactivarse cada vez que se abre el programa. Con esta opción activada, una copia hecha en otro programa se lee por completo antes de que este programa decida si la conserva, así que una copia muy grande puede hacer que el programa se detenga un momento y use tanta memoria como tamaño tenga la copia. Solo se lee una forma: su texto, si lo tiene, o, si no, una imagen. Con la opción desactivada, no se lee nada copiado en otros programas.

Los elementos se guardan solo en la memoria del programa. No se guarda nada en ningún archivo, y la lista se vacía al cerrar la última ventana del programa. Un elemento sigue en la lista aunque se cierre el documento del que procede. Un elemento de más de 8 MB no se guarda, y el panel se lo indica; una imagen ocupa 4 bytes por cada punto (píxel), así que una imagen de más de unos 2 millones de píxeles es demasiado grande. Todos los elementos juntos no pueden superar los 64 MB, y se eliminan primero los más antiguos. Nunca se guarda nada que un gestor de contraseñas marque como secreto.
