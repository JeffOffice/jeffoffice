# Escribir en celdas
<!-- keywords: palabra en negrita, parte de una celda, texto con formato, dar formato a parte del texto, color de una palabra, escribir, escritura, introducir, editar, modificar, cambiar, celda, entrada, barra de fórmulas, F2, doble clic, Tab, Esc, cancelar, deshacer, rehacer, eliminar, borrar, porcentaje, moneda, dinero, fecha, hora, número, texto, apóstrofo, fórmula, igual, reemplazar, límite, caracteres -->

Para introducir algo en una celda, seleccione la celda y empiece a escribir. Lo que escriba reemplaza el contenido de la celda. Pulse **Entrar** para confirmarlo o **Esc** para cancelarlo y dejar la celda como estaba.

## Tres formas de empezar

- **Escribir sobre la celda.** Seleccione una celda y escriba. El primer carácter que escriba inicia la entrada, y la celda muestra lo que escribe mientras lo hace.
- **Editar el contenido de la celda.** Pulse **F2** o haga doble clic en la celda. La celda se abre con su contenido, y el cursor se coloca al final. Use las teclas de flecha para moverse por el texto.
- **Usar la barra de fórmulas.** Haga clic en la barra de fórmulas, que está junto al cuadro de nombres, y escriba allí. La celda también muestra lo que escribe.

Cuando escribe sobre una celda, una tecla de flecha termina la entrada y mueve la selección. Cuando edita el contenido de la celda, las teclas de flecha se mueven dentro del texto. Pulse **F2** para cambiar de un modo a otro.

## Terminar la entrada

| Teclas | Qué hacen |
| --- | --- |
| Entrar | Confirma la entrada y baja a la celda siguiente (o hacia donde indique **Archivo > Opciones > Edición**) |
| Mayús+Entrar | Confirma la entrada y se mueve en sentido contrario |
| Ctrl+Entrar | Coloca la entrada en todas las celdas seleccionadas y se queda donde está |
| Tab | Confirma la entrada y se mueve a la derecha |
| Mayús+Tab | Confirma la entrada y se mueve a la izquierda |
| Teclas de flecha | Confirman la entrada y se mueven una celda (solo cuando escribió sobre la celda) |
| Esc | Cancela la entrada |
| Alt+Entrar | Inicia una línea nueva dentro de la celda |
| Ctrl+; | Introduce la fecha de hoy (dentro de la entrada, cuando la está escribiendo) |
| Ctrl+Mayús+; | Introduce la hora actual |
| Alt+Abajo | Muestra las palabras que ya hay en la columna y permite elegir una |

**Ctrl+Entrar** con varias celdas seleccionadas coloca lo que escribió en todas ellas. Una fórmula se escribe para la celda en la que la introdujo, y en las demás se comporta como si fuera una copia: al escribir **=A1** con el bloque E6:F7 seleccionado, en F7 aparece **=B2**. Es un solo paso de Deshacer.

**Alt+Abajo** en una celda muestra, en orden y una sola vez, las palabras de las celdas situadas encima y debajo de ella que no estén separadas por una celda vacía. Al elegir una, se escribe. Si la celda tiene una lista propia, se muestra esa lista.

En **Archivo > Opciones > Edición** puede elegir si **Entrar** baja, va a la derecha, sube, va a la izquierda o no se mueve.

Si escribe en varias celdas de una fila con **Tab** y luego pulsa **Entrar**, la selección baja a la fila siguiente, en la columna donde empezó. Si antes selecciona un bloque de celdas, **Entrar** y **Tab** recorren el bloque.

## Dar formato a parte del texto

Algunas palabras de una celda pueden tener un aspecto distinto del resto. Por ejemplo, una palabra puede estar en negrita o en rojo.

1. Pulse F2 para editar la celda (o haga doble clic en ella).
2. Seleccione las palabras que desea cambiar, con Mayús y las teclas de flecha o con el ratón.
3. Use **Negrita**, **Cursiva**, **Subrayado**, **Tachado**, **Fuente**, **Tamaño de fuente** o **Color de fuente** en la pestaña **Inicio**, o bien Ctrl+N, Ctrl+K y Ctrl+S.
4. Pulse Entrar.

Solo cambian las palabras que seleccionó. El resto de la celda conserva el aspecto propio de la celda. El cuadro de entrada muestra las palabras con un aspecto único mientras edita, y la celda muestra cada parte con su propio aspecto al pulsar Entrar. Si más adelante escribe o elimina palabras, el formato se desplaza con las palabras. Si elige una fuente o un tamaño sin tener palabras seleccionadas, cambia toda la celda, como siempre.

Las celdas que tienen formato dentro del texto procedente de otro programa lo muestran, lo conservan y lo guardan. Una celda que modifique conserva su formato. El formato dentro de un texto se aplica al texto, no a los números ni a las fórmulas. Una copia guardada como archivo .xls antiguo o como .ods conserva las palabras, y al guardar se avisa de que no se conserva el formato dentro de ellas.

## Lo que escribe y cómo se interpreta

JeffCrunch interpreta lo que escribe según la configuración de su equipo. El separador decimal, el separador de miles, el símbolo de moneda y el orden del día y del mes en una fecha proceden de esa configuración. Un número que escribe en una celda que no tiene formato propio recibe el formato que lo muestra tal como lo escribió.

| Escribe | La celda contiene | Y muestra |
| --- | --- | --- |
| 1234.5 | el número 1234.5 | 1234.5 |
| 5% | el número 0.05 | 5% |
| $12.50 | el número 12.5 | $12.50 |
| 1 1/2 | el número 1.5 | 1 1/2 |
| 3/4/2025 | la fecha, como número de días | 3/4/2025 |
| 10:30 PM | la hora, como parte de un día | 10:30 PM |
| TRUE | el valor lógico verdadero | TRUE |
| #N/A | el error #N/A | #N/A |
| =B2*2 | una fórmula | el resultado de la fórmula |
| cualquier otra cosa | texto | el texto |

Una celda que ya tiene un formato propio, como porcentaje o fecha, lo conserva. Una celda cuyo formato es **Texto** conserva todo lo que escriba como texto, excepto una fórmula.

Para conservar como texto algo que parece un número, como un código **007**, escriba primero un apóstrofo: **'007**. El apóstrofo no forma parte del texto. Al editar la celda, vuelve a verlo.

## Fórmulas

Una fórmula empieza por un signo igual. Por ejemplo, **=B2*2** duplica el número de B2. Escriba los nombres de las funciones en mayúsculas o minúsculas; JeffCrunch los escribe en mayúsculas.

Los nombres de las funciones son siempre los del inglés, en todos los idiomas: escriba **SUM**, no el nombre que le dé su idioma. Los signos que las separan siguen la configuración de su equipo. Cuando el separador decimal es la coma, como en alemán y francés, separa los argumentos de una función con punto y coma, y escribe la coma en los números: **=SUM(A1;A2)*0,5**. La barra de fórmulas y F2 muestran las fórmulas de la misma manera. Cuando el separador decimal es el punto, separa los argumentos con coma: **=SUM(A1,A2)*0.5**. El archivo guarda siempre la forma que leen todos los programas, con comas entre argumentos y puntos en los números, de modo que se abre igual en todas partes.

Si una fórmula tiene un error, como un paréntesis que no se cierra, JeffCrunch le indica qué ocurre, y la entrada se mantiene tal como la escribió, con el cursor cerca del error, para que pueda corregirla. No se cambia nada hasta que la fórmula sea correcta.

Una celda admite hasta 32 767 caracteres, y una fórmula, hasta 8 192. JeffCrunch le avisa cuando una entrada es más larga.

## Borrar celdas

Seleccione las celdas y pulse **Supr**. Esto borra lo que contienen y conserva su formato, y es un paso que puede deshacer. JeffCrunch borra hasta 100 000 celdas a la vez.

En la pestaña **Inicio**, el botón **Borrar** tiene un menú. **Borrar todo** vacía las celdas, sus formatos, sus notas, sus comentarios (los hilos con respuestas) y sus vínculos. Un hilo con más de 500 comentarios se conserva, y JeffCrunch lo indica, porque Deshacer no podría recuperar los comentarios que no leyó. **Borrar formatos** conserva lo que contienen las celdas y restablece el formato normal. **Borrar contenido** equivale a **Supr**. **Borrar comentarios y notas** quita solo las notas y los comentarios de las celdas seleccionadas. **Quitar hipervínculos** quita solo los vínculos, y las celdas conservan su texto. Una opción aparece atenuada cuando las celdas seleccionadas no tienen nada que borrar con ella.

## Deshacer y rehacer

**{key:Undo}** deshace el último cambio, y **{key:Redo}** lo vuelve a hacer. También están en la barra de herramientas de acceso rápido, en la parte superior. Cada celda en la que escribe es un paso, y también lo es cada borrado. Deshacer devuelve la celda a su estado anterior: su valor, su fórmula, su formato y las fórmulas que dependen de ella. JeffCrunch recuerda hasta 1 000 pasos, menos cuando los cambios afectan a muchísimas celdas, para no ocupar mucha memoria.

## Mientras JeffCrunch calcula las fórmulas

Cuando cambia una celda, JeffCrunch calcula en segundo plano las fórmulas que dependen de ella. La celda que cambió muestra su nuevo contenido de inmediato, y las demás se actualizan un momento después; la barra de estado indica **Calculando** hasta que termina. Mientras tanto puede seguir escribiendo.

## Lo que no se puede cambiar

- Una celda que forma parte de una **fórmula matricial** o de una tabla de datos no se puede cambiar por separado.
- En una **hoja protegida**, no se puede cambiar una celda bloqueada. Todas las celdas están bloqueadas salvo que el libro indique lo contrario. JeffCrunch le avisa cuando lo intenta. Para cambiar la celda, elija Desproteger hoja en la pestaña Revisar.
- La primera fila de una **tabla** contiene los nombres de sus columnas. Escriba un nombre nuevo en una de esas celdas para cambiar el nombre de la columna: las fórmulas que usan el nombre anterior se actualizan, y la tabla del archivo sigue siendo válida. Un nombre que ya tenga otra columna recibe un número al final, un nombre no puede quedar vacío, y **Supr** no borra esa fila.
- Un libro que no se pudo leer completo puede consultarse, pero no cambiarse ni guardarse.
- Una función que JeffCrunch todavía no tiene deja una fórmula nueva sin valor en la celda; el libro pide al programa que lo abra que calcule la fórmula.
- El formato, la ordenación, y buscar y reemplazar llegarán en una versión posterior. Insertar y eliminar, cortar y pegar, rellenar, y los tamaños de filas y columnas se explican en [Insertar y eliminar filas, columnas y celdas](insert-delete), [Cortar, copiar y pegar](clipboard), [Rellenar celdas y sumarlas](fill-sum) y [Cambiar el ancho de las columnas y el alto de las filas](sizes).

## Libros nuevos

En **Archivo > Opciones > Libros nuevos**, elija la fuente, el tamaño de fuente y el número de hojas con que empieza todo libro nuevo. Los libros que ya tiene no se modifican.
