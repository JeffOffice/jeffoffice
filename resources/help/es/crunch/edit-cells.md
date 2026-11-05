# Escribir en celdas
<!-- keywords: escribir, escritura, introducir, editar, modificar, cambiar, celda, entrada, barra de fórmulas, F2, doble clic, Tab, Esc, cancelar, deshacer, rehacer, eliminar, borrar, porcentaje, moneda, dinero, fecha, hora, número, texto, apóstrofo, fórmula, igual, reemplazar, límite, caracteres -->

Para escribir algo en una celda, seleccione la celda y empiece a escribir. Lo que escriba sustituye el contenido que tenía la celda. Pulse **Entrar** para conservarlo, o **Esc** para cancelarlo y dejar la celda como estaba.

## Tres formas de empezar

- **Escribir sobre la celda.** Seleccione una celda y escriba. El primer carácter que escriba inicia la entrada, y la celda muestra lo que escribe a medida que lo hace.
- **Editar lo que hay en la celda.** Pulse **F2** o haga doble clic en la celda. La celda se abre con su contenido, y el cursor se coloca al final. Use las teclas de flecha para moverse por el texto.
- **Usar la barra de fórmulas.** Haga clic en la barra de fórmulas, que está junto al cuadro de nombres, y escriba allí. La celda muestra también lo que escribe.

Al escribir sobre una celda, una tecla de flecha termina la entrada y mueve la selección. Al editar lo que hay en la celda, las teclas de flecha se mueven por el texto. Pulse **F2** para cambiar de un modo a otro.

## Terminar la entrada

| Teclas | Qué hacen |
| --- | --- |
| Entrar | Conserva la entrada y baja (o donde indique **Archivo > Opciones > Edición**) |
| Mayús+Entrar | Conserva la entrada y se mueve en sentido contrario |
| Ctrl+Entrar | Pone la entrada en todas las celdas seleccionadas y se queda donde está |
| Tab | Conserva la entrada y se mueve a la derecha |
| Mayús+Tab | Conserva la entrada y se mueve a la izquierda |
| Teclas de flecha | Conservan la entrada y se mueven una celda (solo al escribir sobre la celda) |
| Esc | Cancela la entrada |
| Alt+Entrar | Inicia una línea nueva dentro de la celda |
| Ctrl+; | Escribe la fecha de hoy (dentro de la entrada, si la está escribiendo) |
| Ctrl+Mayús+; | Escribe la hora actual |
| Alt+Flecha abajo | Muestra las palabras que ya hay en la columna, y permite elegir una |

**Ctrl+Entrar** con varias celdas seleccionadas pone lo que escribió en todas ellas. Una fórmula se escribe para la celda en la que la introdujo, y en las demás se comporta como si fuera una copia de ella: al escribir **=A1** con el bloque E6:F7 seleccionado, en F7 aparece **=B2**. Es un solo paso de Deshacer.

**Alt+Flecha abajo** en una celda muestra, en orden y una sola vez, las palabras de las celdas situadas encima y debajo de ella que no estén separadas por una celda vacía. Al elegir una, se escribe. Si la celda tiene una lista propia, se muestra esa lista.

En **Archivo > Opciones > Edición** puede elegir si **Entrar** baja, va a la derecha, sube o a la izquierda, o no se mueve.

Si escribe en varias celdas de una fila con **Tab** y después pulsa **Entrar**, la selección baja a la fila siguiente, en la columna en la que empezó. Si antes selecciona un bloque de celdas, **Entrar** y **Tab** se mueven por el bloque.

## Lo que escribe se convierte en

JeffCrunch interpreta lo que escribe según la configuración de su equipo. El separador decimal, el separador de miles, el símbolo de moneda y el orden del día y del mes en una fecha dependen de esa configuración. Un número escrito en una celda que no tiene formato propio recibe el formato que lo muestra tal como lo escribió.

| Escribe | La celda contiene | Y muestra |
| --- | --- | --- |
| 1234,5 | el número 1234,5 | 1234,5 |
| 5 % | el número 0,05 | 5 % |
| 12,50 € | el número 12,5 | 12,50 € |
| 1 1/2 | el número 1,5 | 1 1/2 |
| 4/3/2025 | la fecha, como número de días | 4/3/2025 |
| 22:30 | la hora, como parte de un día | 22:30 |
| VERDADERO | el valor lógico verdadero | VERDADERO |
| #N/A | el error #N/A | #N/A |
| =B2*2 | una fórmula | el resultado de la fórmula |
| cualquier otra cosa | texto | el texto |

Una celda que ya tiene un formato propio, como porcentaje o fecha, lo conserva. Una celda cuyo formato es **Texto** conserva todo lo que escriba como texto, excepto una fórmula.

Para conservar como texto algo que parece un número, como un código **007**, escriba antes un apóstrofo: **'007**. El apóstrofo no forma parte del texto. Al editar la celda, vuelve a verlo.

## Fórmulas

Una fórmula empieza por un signo igual. Por ejemplo, **=B2*2** duplica el número de B2. Escriba los nombres de las funciones en mayúsculas o en minúsculas; JeffCrunch los escribe en mayúsculas.

Los nombres de las funciones son siempre los inglesos, en todos los idiomas: escriba **SUMA**, no el nombre que le dé su idioma. Los signos que las separan siguen la configuración de su equipo. Cuando el separador decimal es la coma, como en español, los argumentos de una función se separan con punto y coma, y en un número se escribe la coma: **=SUMA(A1;A2)*0,5**. La barra de fórmulas y **F2** muestran las fórmulas de la misma forma. Cuando el separador decimal es el punto, los argumentos se separan con coma: **=SUMA(A1,A2)*0.5**. El archivo guarda siempre la forma que leen todos los programas, con comas entre argumentos y puntos en los números, para que se abra igual en cualquier parte.

Si una fórmula tiene un error, como un paréntesis que nunca se cierra, JeffCrunch le indica qué está mal, y la entrada se queda tal como la escribió, con el cursor cerca del error, para que pueda corregirla. No se cambia nada hasta que la fórmula sea correcta.

Una celda admite hasta 32 767 caracteres, y una fórmula hasta 8 192. JeffCrunch le avisa cuando una entrada es más larga.

## Borrar celdas

Seleccione las celdas y pulse **Suprimir**. Así se borra lo que contienen las celdas y se conserva su formato, y es un paso que puede deshacer. JeffCrunch borra hasta 100 000 celdas a la vez.

En la ficha **Inicio**, el botón **Borrar** tiene un menú. **Borrar todo** vacía las celdas, y también borra su formato, sus notas y sus vínculos. **Borrar formatos** conserva lo que contienen las celdas y restablece el formato normal. **Borrar contenido** es lo mismo que **Suprimir**. **Borrar comentarios y notas** quita solo las notas de las celdas seleccionadas. **Borrar hipervínculos** quita solo los vínculos, y las celdas conservan su texto. Una opción aparece atenuada cuando las celdas seleccionadas no tienen nada que borrar.

## Deshacer y rehacer

**{key:Deshacer}** deshace el último cambio, y **{key:Rehacer}** lo vuelve a hacer. También están en la barra de herramientas rápida, en la parte superior. Cada celda en la que escribe es un paso, y también lo es cada borrado. Deshacer devuelve la celda a su estado anterior: su valor, su fórmula, su formato y las fórmulas que dependen de ella. JeffCrunch recuerda hasta 1000 pasos, menos cuando los cambios afectan a muchísimas celdas, para no ocupar mucha memoria.

## Mientras JeffCrunch calcula las fórmulas

Cuando cambia una celda, JeffCrunch calcula en segundo plano las fórmulas que dependen de ella. La celda que cambió muestra su nuevo contenido al instante, y las demás se actualizan un momento después; la barra de estado indica **Calculando** hasta que termina. Mientras tanto, puede seguir escribiendo.

## Lo que no se puede cambiar

- Una celda que forma parte de una **fórmula matricial** o de una tabla de datos no se puede cambiar por separado.
- En una **hoja protegida**, no se puede cambiar una celda bloqueada. Todas las celdas están bloqueadas, salvo que el libro indique lo contrario. JeffCrunch le avisa cuando lo intenta. Para cambiar la celda, elija Desproteger hoja en la ficha **Revisar**.
- La primera fila de una **tabla** contiene los nombres de sus columnas. Escriba un nombre nuevo en una de esas celdas para cambiar el nombre de la columna: las fórmulas que usan el nombre anterior se actualizan, y la tabla del archivo sigue siendo válida. Un nombre que ya tiene otra columna recibe un número al final, un nombre no puede estar vacío, y **Suprimir** no borra esa fila.
- Un libro que no pudo leerse completo se puede consultar, pero no cambiar ni guardar.
- Si JeffCrunch todavía no tiene una función, la nueva fórmula queda sin valor en la celda; el libro pide al programa que lo abra que calcule la fórmula.
- El formato, la ordenación y buscar y reemplazar llegarán en una versión posterior. Insertar y eliminar, cortar y pegar, rellenar y el tamaño de filas y columnas se explican en [Insertar y eliminar filas, columnas y celdas](insert-delete), [Cortar, copiar y pegar](clipboard), [Rellenar celdas y sumarlas](fill-sum) y [Cambiar el ancho de las columnas y el alto de las filas](sizes).

## Libros nuevos

En **Archivo > Opciones > Libros nuevos**, elija la fuente, el tamaño de fuente y el número de hojas con los que empieza cada libro nuevo. Los libros que ya tiene no se modifican.
