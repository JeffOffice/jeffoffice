# Escribir en celdas
<!-- keywords: autocorrección, comillas tipográficas, guion, fracción, vínculo, dirección, palabra en negrita, parte de una celda, texto enriquecido, formato de parte del texto, color de una palabra, tramos, escribir, escritura, introducir, entrar, editar, cambiar, celda, entrada, barra de fórmulas, F2, doble clic, Tab, Esc, cancelar, deshacer, rehacer, eliminar, borrar, porcentaje, moneda, dinero, fecha, hora, número, texto, apóstrofo, fórmula, igual, reemplazar, límite, caracteres -->

Para escribir algo en una celda, seleccione la celda y empiece a escribir. Lo que escriba sustituye al contenido de la celda. Pulse **Entrar** para conservarlo, o **Esc** para cancelar la entrada y dejar la celda como estaba.

## Tres formas de empezar

- **Escribir sobre la celda.** Seleccione una celda y escriba. El primer carácter que escriba inicia la entrada, y la celda muestra lo que escribe a medida que lo hace.
- **Editar el contenido de la celda.** Pulse **F2** o haga doble clic en la celda. La celda se abre con su contenido, y el cursor queda al final. Use las teclas de flecha para moverse por el texto.
- **Usar la barra de fórmulas.** Haga clic en la barra de fórmulas, que está junto al cuadro de nombres, y escriba allí. La celda también muestra lo que escribe.

Al escribir sobre una celda, una tecla de flecha termina la entrada y mueve la selección. Al editar el contenido de la celda, las teclas de flecha se mueven por el texto. Pulse **F2** para cambiar de un modo a otro.

## Terminar la entrada

| Teclas | Qué hacen |
| --- | --- |
| Entrar | Conserva la entrada y baja (o hace lo que indique **Archivo > Opciones > Edición**) |
| Mayús+Entrar | Conserva la entrada y se mueve en sentido contrario |
| Ctrl+Entrar | Pone la entrada en todas las celdas seleccionadas, y usted se queda donde está |
| Tab | Conserva la entrada y se mueve a la derecha |
| Mayús+Tab | Conserva la entrada y se mueve a la izquierda |
| Teclas de flecha | Conservan la entrada y se mueven una celda (solo cuando escribió sobre la celda) |
| Esc | Cancela la entrada |
| Alt+Entrar | Inicia una línea nueva dentro de la celda |
| Ctrl+; | Escribe la fecha de hoy (dentro de la entrada, si la está escribiendo) |
| Ctrl+Mayús+; | Escribe la hora actual |
| Alt+Flecha abajo | Muestra las palabras que ya hay en la columna, para elegir una |

**Ctrl+Entrar** con varias celdas seleccionadas pone lo que escribió en todas ellas. Una fórmula se escribe para la celda en la que la introdujo, y en cada una de las demás se lee como si fuera una copia de ella: si escribe **=A1** con el bloque E6:F7 seleccionado, en F7 aparece **=B2**. Es un solo paso de Deshacer.

**Alt+Flecha abajo** en una celda muestra, en orden y una sola vez, las palabras de las celdas situadas encima y debajo de ella que no estén separadas por una celda vacía. Al elegir una, se escribe. Si la celda tiene una lista propia, se muestra esa lista.

En **Archivo > Opciones > Edición** puede elegir si **Entrar** baja, va a la derecha, sube, va a la izquierda o no se mueve.

Si escribe en varias celdas de una fila con **Tab** y luego pulsa **Entrar**, la selección baja a la fila siguiente, en la columna donde empezó. Si antes selecciona un bloque de celdas, **Entrar** y **Tab** se mueven por el bloque.

## Autocorrección mientras escribe

Cuando escribe un espacio o un signo como un punto después de una palabra, JeffCrunch puede corregirla al terminarla. Corrige errores de escritura frecuentes (como «teh» por «the»), dos mayúsculas al principio de una palabra, una minúscula que inicia una oración o un día de la semana, y una comilla recta (la convierte en una tipográfica). También cambia dos guiones entre palabras por un guion largo, y 1/2, 1/4 y 3/4 por ½, ¼ y ¾ cuando la entrada contiene palabras. Un número como 1/2 escrito solo sigue siendo una fecha o una fracción, como siempre.

Cada corrección es un paso propio. Pulse **{key:Undo}** justo después de una corrección para deshacer solo esa corrección y conservar lo que escribió. No se cambia nada en una fórmula: una entrada que empieza por un signo igual, o por un signo más o menos seguido de una fórmula, se queda tal como la escribió. Una dirección web (como https://example.org) o una dirección de correo electrónico que sea toda la entrada se convierte en un vínculo en su celda al terminar la entrada, si está activada la opción **Convertir las direcciones web en vínculos mientras escribe**.

Active o desactive estas opciones en **Archivo > Opciones > Revisión**. Una palabra mal escrita se marca mientras escribe: vea [Ortografía](notes-links).

## Dar formato a una parte del texto

Algunas palabras de una celda pueden tener un aspecto distinto del resto. Por ejemplo, una palabra puede ir en negrita o en rojo.

1. Pulse F2 para editar la celda (o haga doble clic en ella).
2. Seleccione las palabras que desea cambiar, con Mayús y las teclas de flecha, o con el ratón.
3. Use **Negrita**, **Cursiva**, **Subrayado**, **Tachado**, **Fuente**, **Tamaño de fuente** o **Color de fuente** en la ficha **Inicio**, o bien Ctrl+N, Ctrl+K y Ctrl+S.
4. Pulse Entrar.

Solo cambian las palabras seleccionadas. El resto de la celda conserva el aspecto propio de la celda. El cuadro de entrada muestra las palabras con un aspecto mientras edita, y la celda muestra cada parte con su propio aspecto al pulsar Entrar. Si más tarde escribe o borra palabras, el formato se mueve con ellas. Elegir una fuente o un tamaño sin palabras seleccionadas cambia toda la celda, como siempre.

Las celdas que tienen formato dentro del texto procedente de otro programa lo muestran, lo conservan y lo guardan de nuevo. Una celda que cambia conserva su formato. El formato dentro de un texto se aplica al texto, no a los números ni a las fórmulas. Una copia guardada en el formato antiguo .xls o en .ods conserva las palabras, y al guardarla se avisa de que no se conserva el formato interior.

## Lo que escribe se convierte en

JeffCrunch interpreta lo que escribe según la configuración de su equipo. El separador decimal, el separador de miles, el símbolo monetario y el orden del día y del mes en una fecha proceden de esa configuración. Un número que escribe en una celda que no tiene formato propio recibe el formato que lo muestra tal como lo escribió.

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

Una celda que ya tiene un formato propio, como porcentaje o fecha, lo conserva. Una celda cuyo formato es **Texto** conserva todo lo que escribe como texto, salvo una fórmula.

Para guardar como texto algo que parece un número, como un código **007**, escriba antes un apóstrofo: **'007**. El apóstrofo no forma parte del texto. Cuando edite la celda, volverá a verlo.

## Fórmulas

Una fórmula empieza por un signo igual. Por ejemplo, **=B2*2** duplica el número de B2. Escriba los nombres de las funciones en mayúsculas o en minúsculas; JeffCrunch los escribe en mayúsculas.

Los nombres de las funciones son siempre los ingleses, en todos los idiomas: escriba **SUM**, no el nombre que su idioma da a la función. Los signos que hay entre ellos siguen la configuración de su equipo. Cuando el separador decimal es la coma, como en alemán y francés, los argumentos de una función se separan con punto y coma, y en un número se escribe una coma: **=SUM(A1;A2)*0,5**. La barra de fórmulas y **F2** muestran las fórmulas del mismo modo. Cuando el separador decimal es el punto, los argumentos se separan con coma: **=SUM(A1,A2)*0.5**. El fichero guarda siempre la forma que leen todos los programas, con comas entre argumentos y puntos en los números, de modo que se abre igual en cualquier sitio.

Si una fórmula tiene un error, como un paréntesis que nunca se cierra, JeffCrunch le indica qué está mal, y la entrada queda tal como la escribió, con el cursor cerca del error, para que pueda corregirla. No se cambia nada hasta que la fórmula sea correcta.

Una celda puede contener hasta 32 767 caracteres, y una fórmula hasta 8 192. JeffCrunch le avisa cuando una entrada es más larga.

## Borrar celdas

Seleccione las celdas y pulse **Supr**. Esto borra lo que contienen las celdas y conserva sus formatos, y es un paso que puede deshacer. JeffCrunch borra hasta 100 000 celdas a la vez.

En la ficha **Inicio**, el botón **Borrar** tiene un menú. **Borrar todo** vacía las celdas, sus formatos, sus notas, sus comentarios (los hilos con respuestas) y sus vínculos. Un hilo con más de 500 comentarios se conserva, y JeffCrunch lo indica, porque Deshacer no podría recuperar los comentarios que no llegó a leer. **Borrar formatos** conserva el contenido de las celdas y les devuelve el formato normal. **Borrar contenido** es lo mismo que **Supr**. **Borrar comentarios y notas** quita solo las notas y los comentarios de las celdas seleccionadas. **Borrar hipervínculos** quita solo los vínculos, y las celdas conservan su texto. Una opción aparece atenuada cuando las celdas seleccionadas no tienen nada que borrar con ella.

## Deshacer y rehacer

**{key:Undo}** deshace el último cambio, y **{key:Redo}** lo vuelve a hacer. También están en la barra de herramientas de acceso rápido, en la parte superior. Cada celda en la que escribe es un paso, y también cada borrado. Deshacer devuelve la celda a su estado anterior: su valor, su fórmula, su formato y las fórmulas que dependen de ella. JeffCrunch recuerda hasta 1000 pasos, menos cuando los cambios afectan a muchísimas celdas, para no ocupar mucha memoria.

## Mientras JeffCrunch calcula las fórmulas

Cuando cambia una celda, JeffCrunch calcula en segundo plano las fórmulas que dependen de ella. La celda que cambió muestra su nuevo contenido al instante, y las demás celdas se actualizan un momento después; la barra de estado muestra **Calculando** hasta que termina. Mientras tanto, puede seguir escribiendo.

## Lo que no se puede cambiar

- Una celda que forma parte de una **fórmula matricial** o de una tabla de datos no se puede cambiar por sí sola.
- En una **hoja protegida**, no se puede cambiar una celda bloqueada. Todas las celdas están bloqueadas, salvo que el libro indique lo contrario. JeffCrunch le avisa cuando lo intenta. Para cambiar la celda, elija Desproteger hoja en la ficha Revisar.
- La primera fila de una **tabla** contiene los nombres de sus columnas. Escriba un nombre nuevo en una de esas celdas para cambiar el nombre de la columna: las fórmulas que usan el nombre anterior se actualizan, y la tabla del fichero sigue siendo válida. Un nombre que ya tiene otra columna recibe un número al final, un nombre no puede quedar vacío, y **Supr** no borra esa fila.
- Un libro que no se pudo leer completo puede consultarse, pero no cambiarse ni guardarse.
- Una función que JeffCrunch todavía no tiene deja en la celda una fórmula nueva sin valor; el libro pide al programa que lo abra que calcule la fórmula.
- El formato, la ordenación y buscar y reemplazar llegarán en una versión posterior. Insertar y eliminar, cortar y pegar, rellenar y el tamaño de filas y columnas se explican en [Insertar y eliminar filas, columnas y celdas](insert-delete), [Cortar, copiar y pegar](clipboard), [Rellenar celdas y sumarlas](fill-sum) y [Cambiar el ancho de las columnas y el alto de las filas](sizes).

## Libros nuevos

En **Archivo > Opciones > Libros nuevos** puede elegir la fuente, el tamaño de fuente y el número de hojas con que empieza cada libro nuevo. Los libros que ya tiene no se modifican.
