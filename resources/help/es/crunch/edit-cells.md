# Escribir en celdas
<!-- keywords: símbolo, fuente de símbolos, caracteres especiales, tecla de acceso directo para un símbolo, autocorrección, comillas tipográficas, guion, fracción, vínculo, dirección, palabra en negrita, parte de una celda, texto con formato, dar formato a parte del texto, color de una palabra, escribir, escritura, introducir, editar, modificar, cambiar, celda, entrada, barra de fórmulas, F2, doble clic, Tab, Esc, cancelar, deshacer, rehacer, eliminar, borrar, porcentaje, moneda, dinero, fecha, hora, número, texto, apóstrofo, fórmula, igual, reemplazar, límite, caracteres -->

Para escribir algo en una celda, seleccione la celda y empiece a escribir. Lo que escriba reemplaza el contenido que tenía la celda. Presione **Entrar** para conservarlo, o **Esc** para cancelarlo y dejar la celda como estaba.

## Tres formas de empezar

- **Escribir sobre la celda.** Seleccione una celda y escriba. El primer carácter que escriba inicia la entrada, y la celda muestra lo que escribe sobre la marcha.
- **Editar el contenido de la celda.** Presione **F2** o haga doble clic en la celda. La celda se abre con su contenido, y el cursor queda al final. Use las teclas de flecha para moverse por el texto.
- **Usar la barra de fórmulas.** Haga clic en la barra de fórmulas, que está junto al cuadro de nombres, y escriba allí. La celda también muestra lo que escribe.

Al escribir sobre una celda, una tecla de flecha termina la entrada y mueve la selección. Al editar el contenido de la celda, las teclas de flecha se mueven dentro del texto. Presione **F2** para cambiar de un modo a otro.

## Terminar la entrada

| Teclas | Qué hacen |
| --- | --- |
| Entrar | Conserva la entrada y baja (o hacia donde indique **Archivo > Opciones > Edición**) |
| Mayús+Entrar | Conserva la entrada y sube |
| Ctrl+Entrar | Pone la entrada en todas las celdas seleccionadas, y usted se queda donde está |
| Tab | Conserva la entrada y va a la derecha |
| Mayús+Tab | Conserva la entrada y va a la izquierda |
| Teclas de flecha | Conservan la entrada y se mueven una celda (solo si escribió sobre la celda) |
| Esc | Cancela la entrada |
| Alt+Entrar | Inicia una línea nueva dentro de la celda |
| Ctrl+; | Escribe la fecha de hoy (dentro de la entrada, si está escribiendo una) |
| Ctrl+Mayús+; | Escribe la hora actual |
| Alt+Flecha abajo | Muestra las palabras que ya hay en la columna, y elija una |

**Ctrl+Entrar** con varias celdas seleccionadas pone lo escrito en todas ellas. Una fórmula se escribe para la celda en la que la escribió, y en cada una de las demás se lee como lo haría una copia de ella: si escribe **=A1** con el bloque E6:F7 seleccionado, en F7 aparece **=B2**. Es un solo paso de Deshacer.

**Alt+Flecha abajo** en una celda muestra, en orden y una sola vez, las palabras de las celdas situadas encima y debajo de ella que no están separadas por una celda vacía. Al elegir una, se escribe. Si una celda tiene una lista propia, muestra esa lista.

En **Archivo > Opciones > Edición**, elija si **Entrar** baja, va a la derecha, sube, va a la izquierda o no se mueve.

Si escribe en varias celdas de una fila con **Tab** y luego presiona **Entrar**, la selección baja a la fila siguiente, en la columna donde empezó. Si antes selecciona un bloque de celdas, **Entrar** y **Tab** se mueven por el bloque.

## Autocorrección mientras escribe

Al escribir un espacio o un signo, como un punto, después de una palabra, JeffCrunch puede corregir la palabra al terminarla. Corrige errores de escritura comunes (como «teh» por «the»), dos mayúsculas al principio de una palabra, una minúscula que inicia una oración o un día de la semana, y una comilla recta (la convierte en una tipográfica). También cambia dos guiones entre palabras por una raya, y 1/2, 1/4 y 3/4 por ½, ¼ y ¾ cuando la entrada tiene palabras. Un número como 1/2 escrito solo sigue siendo una fecha o una fracción, como siempre.

Cada corrección es un paso propio. Presione **Deshacer** justo después de una corrección para anular solo esa corrección y conservar lo que escribió. No se cambia nada en una fórmula: una entrada que empieza con signo igual, o con más o menos seguido de una fórmula, queda como la escribió. Una dirección web (como https://example.org) o una dirección de correo electrónico que sea toda la entrada se convierte en vínculo en su celda al terminar la entrada, si **Convertir las direcciones web en hipervínculos mientras escribe** está activada.

Active o desactive estas opciones en **Archivo > Opciones > Revisión**. Una palabra mal escrita se marca mientras escribe: consulte [Ortografía](notes-links).

## Dar formato a parte del texto

Algunas palabras de una celda pueden verse distintas del resto. Por ejemplo, una palabra puede estar en negrita o en rojo.

1. Presione F2 para editar la celda (o haga doble clic en ella).
2. Seleccione las palabras que desea cambiar, con Mayús y las teclas de flecha, o con el ratón.
3. Use **Negrita**, **Cursiva**, **Subrayado**, **Tachado**, **Fuente**, **Tamaño de fuente** o **Color de fuente** en la ficha **Inicio**, o bien Ctrl+N, Ctrl+K y Ctrl+S.
4. Presione Entrar.

Solo cambian las palabras seleccionadas. El resto de la celda conserva el aspecto propio de la celda. El cuadro de entrada muestra las palabras con un solo aspecto mientras edita, y la celda muestra cada parte con su propio aspecto al presionar Entrar. Si más adelante escribe o borra palabras, el formato se mueve con las palabras. Elegir una fuente o un tamaño sin palabras seleccionadas cambia toda la celda, como siempre.

## La minibarra de herramientas

Una pequeña barra con el formato que más usa aparece flotando donde trabaja, para que no tenga que subir a la cinta de opciones. Nunca le quita el teclado a la celda: lo que estaba escribiendo sigue donde estaba.

- **Texto seleccionado en una celda:** seleccione palabras con el ratón mientras edita una celda, y la barra aparece sobre ellas. Tiene los cuadros de **fuente** y **tamaño**, **Negrita**, **Cursiva**, **Subrayado**, **Aumentar tamaño de fuente**, **Disminuir tamaño de fuente** y **Color de fuente**. Cambian solo las palabras seleccionadas, igual que en la cinta de opciones.
- **Celdas seleccionadas:** haga clic con el botón derecho en las celdas, y la barra aparece en la parte superior del menú contextual. También tiene **Color de relleno**, **Bordes**, **Moneda**, **Estilo porcentual**, **Estilo millares**, **Aumentar decimales**, **Disminuir decimales**, **Combinar y centrar** y **Copiar formato**. Son los comandos propios de la cinta de opciones, y cada uno es un solo paso que puede deshacer.

La barra sobre el texto seleccionado se desvanece al alejar el puntero, y desaparece cuando el puntero está lejos. También desaparece al escribir, presionar Esc o desplazarse. Para desactivar la barra, desmarque **Mostrar minibarra de herramientas al seleccionar** en la página **General** de **Archivo > Opciones**; las demás aplicaciones de JeffOffice siguen también esa elección. El botón de Análisis rápido que aparece bajo un rango seleccionado no está en esta versión.

Las celdas que tienen formato dentro del texto procedente de otro programa lo muestran, lo conservan y lo guardan de nuevo. Una celda que cambie conserva su formato. El formato dentro de un texto se aplica al texto, no a los números ni a las fórmulas. Una copia guardada como archivo .xls antiguo o como .ods conserva las palabras, y al guardar se avisa de que el formato interno de las palabras no se conserva.

## Lo que escribe se convierte en

JeffCrunch interpreta lo que escribe según la configuración de su equipo. El separador decimal, el separador de miles, el signo de moneda y el orden del día y del mes en una fecha vienen de esa configuración. Un número que escribe en una celda sin formato propio recibe el formato que lo muestra tal como lo escribió.

| Escribe | La celda contiene | Y muestra |
| --- | --- | --- |
| 1234,5 | el número 1234,5 | 1234,5 |
| 5% | el número 0,05 | 5% |
| 12,50 $ | el número 12,5 | 12,50 $ |
| 1 1/2 | el número 1,5 | 1 1/2 |
| 4/3/2025 | la fecha, como un número de días | 4/3/2025 |
| 22:30 | la hora, como una parte del día | 22:30 |
| VERDADERO | el valor lógico verdadero | VERDADERO |
| #N/A | el error #N/A | #N/A |
| =B2*2 | una fórmula | el resultado de la fórmula |
| cualquier otra cosa | texto | el texto |

Una celda que ya tiene un formato propio, como un porcentaje o una fecha, lo conserva. Una celda cuyo formato es **Texto** conserva todo lo que escribe como texto, salvo una fórmula.

Para guardar como texto algo que parece un número, como un código **007**, escriba antes un apóstrofo: **'007**. El apóstrofo no forma parte del texto. Al editar la celda, vuelve a verlo.

## Fórmulas

Una fórmula empieza con un signo igual. Por ejemplo, **=B2*2** duplica el número de B2. Escriba los nombres de las funciones en mayúsculas o en minúsculas; JeffCrunch los escribe en mayúsculas.

Los nombres de las funciones son siempre los inglés, en todos los idiomas: escriba **SUM**, no el nombre que su idioma da a la función. Los signos entre ellos siguen la configuración de su equipo. Cuando el separador decimal es la coma, como en alemán y francés, los argumentos de una función se separan con punto y coma, y en un número se escribe la coma: **=SUM(A1;A2)*0,5**. La barra de fórmulas y **F2** muestran las fórmulas de la misma manera. Cuando el separador decimal es el punto, los argumentos se separan con coma: **=SUM(A1,A2)*0.5**. El archivo guarda siempre la forma que leen todos los programas, con comas entre argumentos y puntos en los números, para que se abra igual en todas partes.

Si una fórmula tiene un error, como un paréntesis que nunca se cierra, JeffCrunch le indica qué falla, y la entrada queda como la escribió, con el cursor cerca del error, para que pueda corregirla. No se cambia nada hasta que la fórmula esté bien.

Una celda admite hasta 32 767 caracteres, y una fórmula hasta 8 192. JeffCrunch le avisa cuando una entrada es más larga.

## Borrar celdas

Seleccione las celdas y presione **Supr**. Esto borra lo que contienen las celdas y conserva su formato, y es un paso que puede deshacer. JeffCrunch borra hasta 100 000 celdas a la vez.

En la ficha **Inicio**, el botón **Borrar** tiene un menú. **Borrar todo** vacía las celdas, y quita su formato, sus notas, sus comentarios (los hilos con respuestas) y sus vínculos. Un hilo con más de 500 comentarios se conserva, y JeffCrunch lo indica, porque al deshacer no podría recuperar los comentarios que no leyó. **Borrar formatos** conserva lo que contienen las celdas y vuelve a poner el formato normal. **Borrar contenido** es igual que **Supr**. **Borrar comentarios y notas** quita solo las notas y los comentarios de las celdas seleccionadas. **Borrar hipervínculos** quita solo los vínculos, y las celdas conservan su texto. Una opción aparece atenuada cuando las celdas seleccionadas no tienen nada que borrar.

## Deshacer y rehacer

**Deshacer** anula el último cambio, y **Rehacer** lo vuelve a hacer. También están en la barra de herramientas rápida, en la parte superior. Cada celda que escribe es un paso, y también lo es cada borrado. Deshacer devuelve la celda a su estado anterior: su valor, su fórmula, su formato y las fórmulas que dependen de ella. JeffCrunch recuerda hasta 1000 pasos, menos cuando cambiaron muchísimas celdas, para no ocupar demasiada memoria.

**Retroceder varios pasos a la vez.** En la barra de herramientas rápida, **Deshacer** y **Rehacer** tienen una flecha junto a ellos. Haga clic en la flecha para ver los pasos, del más reciente al más antiguo (se muestran hasta 100). Mueva el puntero hacia abajo en la lista para marcar cuántos pasos quiere (la línea bajo la lista indica, por ejemplo, «Deshacer 3 acciones»), y haga clic para ejecutarlos. Si elige el tercer paso, se deshacen los tres primeros. Los pasos se mantienen en orden, así que Rehacer los devuelve de la misma manera.

## Repetir

**F4** repite su último comando sobre las celdas que están seleccionadas ahora. Ejemplo: ponga en negrita **A1**, seleccione **C4:C9** y presione **F4**, y las celdas de C4:C9 quedan en negrita, en un solo paso que puede deshacer. Repetir funciona con el formato, los estilos de celda, el borrado y la inserción o eliminación de filas y columnas. La información del botón, o el comando **Repetir**, indica lo que haría («Repetir Negrita»). **Ctrl+Y** también repite, pero solo cuando no hay ningún paso que rehacer (mientras lo haya, **Ctrl+Y** es Rehacer). Mientras escribe una fórmula, **F4** sigue cambiando una referencia entre relativa y fija, como antes. Los comandos que abren un cuadro, y la escritura en una celda, no se repiten.

## Mientras JeffCrunch calcula las fórmulas

Cuando cambia una celda, JeffCrunch calcula en segundo plano las fórmulas que dependen de ella. La celda que cambió muestra su nuevo contenido al instante, y las demás celdas se actualizan un momento después; la barra de estado indica **Calculando** hasta que termina. Mientras tanto, puede seguir escribiendo.

## Lo que no se puede cambiar

- Una celda que forma parte de una **fórmula matricial** o de una tabla de datos no se puede cambiar por separado. Para crear una fórmula matricial, seleccione las celdas que debe llenar, escriba la fórmula y presione **Ctrl+Mayús+Entrar**. Entonces una sola fórmula llena todas las celdas, y la barra de fórmulas la muestra entre llaves, como **{=A1:A3*2}**. Para cambiarla, seleccione cualquier celda de la matriz, presione **F2**, modifique la fórmula y vuelva a presionar **Ctrl+Mayús+Entrar**. Para eliminarla, seleccione todas sus celdas y presione **Supr**. Ambas operaciones actúan sobre toda la matriz, en un solo paso de Deshacer. **F2** en cualquier celda de una matriz abre la fórmula de la matriz, y una hoja protegida o un rango con contraseña rechaza o pide la contraseña, como con cualquier otra celda. Al copiar una matriz, o parte de ella, y al rellenar desde una de sus celdas, se obtienen los valores que muestra, no una fórmula.
- En una **hoja protegida**, una celda bloqueada no se puede cambiar. Todas las celdas están bloqueadas, salvo que el libro indique lo contrario. JeffCrunch se lo indica cuando lo intenta. Para cambiar la celda, elija **Desproteger hoja** en la ficha **Revisar**.
- La primera fila de una **tabla** contiene los nombres de sus columnas. Escriba un nombre nuevo en una de esas celdas para cambiar el nombre de la columna: las fórmulas que usan el nombre anterior se actualizan, y la tabla del archivo sigue siendo válida. Si el nombre ya lo tiene otra columna, se le añade un número, un nombre no puede estar vacío, y **Supr** no borra esa fila.
- Un libro que no se pudo leer completo se puede consultar, pero no cambiar ni guardar.
- Una función que JeffCrunch todavía no tiene deja en una fórmula nueva una celda sin valor; el libro pide al programa que lo abra que calcule la fórmula.
- El formato, la ordenación, y buscar y reemplazar llegarán en una versión posterior. Insertar y eliminar, cortar y pegar, rellenar y el tamaño de filas y columnas se explican en [Insertar y eliminar filas, columnas y celdas](insert-delete), [Cortar, copiar y pegar](clipboard), [Rellenar celdas y sumarlas](fill-sum) y [Cambiar el ancho de columnas y el alto de filas](sizes).

## Insertar un símbolo

Haga clic en **Insertar > Símbolos > Símbolo** para abrir el cuadro Símbolo. Elija un carácter y haga clic en **Insertar**, o haga doble clic en él. Se inserta en el cursor de la entrada que está escribiendo. Si no está escribiendo, se abre la entrada de la propia celda con el símbolo añadido al final, y **Entrar** lo pone en la celda. Ejemplo: con una celda que contiene la palabra *Total*, al elegir el signo del euro la entrada queda en *Total€*. Los símbolos que usó por última vez aparecen primero en la lista.

**Tomar el carácter de una fuente.** Elija una fuente en el cuadro **Fuente** y, después, el carácter. El carácter se inserta en un tramo propio, escrito con esa fuente, y el resto del texto de la celda conserva la fuente de la celda. Ejemplo: en una celda que contiene *Total*, un tic tomado de una fuente de símbolos da *Total✓*, con solo el tic en esa fuente. Si elige **(texto normal)**, la fuente de la celda dibuja el símbolo, y un símbolo que la fuente no tiene se muestra como un cuadro. En una fórmula, solo se inserta el carácter.

**Caracteres especiales.** La ficha **Caracteres especiales** muestra el espacio y el guion de no separación, el guion opcional, la raya corta y la raya larga, los espacios en y em, las comillas y el corte opcional de ancho cero, cada uno con su nombre. Haga doble clic en uno para insertarlo.

**Asignar una tecla de acceso directo a un símbolo.** Elija un símbolo o un carácter especial, haga clic en **Tecla de acceso directo**, presione las teclas que desee, como **Ctrl+Alt+Mayús+J**, y haga clic en **Aceptar**. Las teclas insertan entonces ese símbolo, igual que el cuadro. Deje las teclas vacías y haga clic en **Aceptar** para quitarlas. JeffCrunch le avisa cuando otro comando ya usa esas teclas. El símbolo se convierte en un comando, así que también aparece en **Archivo > Opciones > Personalizar > Teclado**, donde puede cambiar sus teclas (consulte [Métodos abreviados de teclado](keyboard)).

## Libros nuevos

En **Archivo > Opciones > Libros nuevos**, elija la fuente, el tamaño de fuente y el número de hojas con que empieza cada libro nuevo. Los libros que ya tiene no se cambian.
