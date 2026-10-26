# Escribir en celdas
<!-- keywords: escribir, escritura, introducir, entrada, editar, modificar, cambiar, celda, barra de fórmulas, F2, doble clic, Tab, Esc, cancelar, deshacer, rehacer, eliminar, borrar, suprimir, porcentaje, moneda, dinero, fecha, hora, número, texto, apóstrofo, fórmula, signo igual, reemplazar, límite, caracteres -->

Para introducir algo en una celda, selecciónela y empiece a escribir. Lo que escriba reemplaza el contenido de la celda. Presione **Entrar** para conservarlo o **Esc** para cancelarlo y dejar la celda como estaba.

## Tres formas de empezar

- **Escribir sobre la celda.** Seleccione una celda y escriba. El primer carácter que escriba inicia la entrada, y la celda muestra lo que escribe a medida que lo hace.
- **Modificar el contenido de la celda.** Presione **F2** o haga doble clic en la celda. La celda se abre con su contenido y el cursor al final. Use las teclas de dirección para moverse por el texto.
- **Usar la barra de fórmulas.** Haga clic en la barra de fórmulas, que está junto al cuadro de nombres, y escriba allí. La celda también muestra lo que escribe.

Cuando escribe sobre una celda, una tecla de dirección finaliza la entrada y mueve la selección. Cuando modifica el contenido de la celda, las teclas de dirección se mueven por el texto. Presione **F2** para pasar de un modo a otro.

## Finalizar la entrada

| Teclas | Qué hacen |
| --- | --- |
| Entrar | Conservar la entrada y bajar |
| Mayús+Entrar | Conservar la entrada y subir |
| Tab | Conservar la entrada y moverse a la derecha |
| Mayús+Tab | Conservar la entrada y moverse a la izquierda |
| Teclas de dirección | Conservar la entrada y moverse una celda (solo cuando escribió sobre la celda) |
| Esc | Cancelar la entrada |
| Alt+Entrar | Iniciar una nueva línea dentro de la celda |

Si escribe en varias celdas de una fila con **Tab** y luego presiona **Entrar**, la selección baja a la fila siguiente, en la columna donde empezó. Si antes selecciona un bloque de celdas, **Entrar** y **Tab** se mueven dentro del bloque.

## En qué se convierte lo que escribe

JeffCrunch interpreta lo que escribe según la configuración del equipo. El separador decimal, el separador de miles, el símbolo de moneda y el orden del día y el mes en una fecha dependen de esa configuración. A un número que escriba en una celda sin formato propio se le asigna el formato que lo muestra tal como lo escribió.

| Escribe | La celda contiene | Y muestra |
| --- | --- | --- |
| 1234.5 | el número 1234,5 | 1234.5 |
| 5% | el número 0,05 | 5% |
| $12.50 | el número 12,5 | $12.50 |
| 3/4/2025 | la fecha, como un número de días | 3/4/2025 |
| 10:30 PM | la hora, como una fracción de día | 10:30 PM |
| TRUE | el valor lógico verdadero | TRUE |
| #N/A | el error #N/A | #N/A |
| =B2*2 | una fórmula | el resultado de la fórmula |
| cualquier otra cosa | texto | el texto |

Una celda que ya tiene un formato propio, como un porcentaje o una fecha, lo conserva. Una celda con formato **Texto** guarda como texto todo lo que escriba, excepto una fórmula.

Para conservar como texto algo que parece un número, como el código **007**, escriba primero un apóstrofo: **'007**. El apóstrofo no forma parte del texto. Cuando modifique la celda, lo volverá a ver.

## Fórmulas

Una fórmula empieza por un signo igual. Por ejemplo, **=B2*2** duplica el número de B2. Escriba los nombres de las funciones en mayúsculas o minúsculas; JeffCrunch los escribe en mayúsculas.

Los nombres de las funciones son siempre los ingleses, en todos los idiomas: escriba **SUM**, no el nombre que le da su idioma. Los signos que los separan dependen de la configuración del equipo. Donde el separador decimal es la coma, como en alemán y en francés, los argumentos de una función se separan con punto y coma y los números se escriben con coma: **=SUM(A1;A2)*0,5**. La barra de fórmulas y **F2** muestran las fórmulas de la misma forma. Donde el separador decimal es el punto, los argumentos se separan con coma: **=SUM(A1,A2)*0.5**. El archivo siempre guarda la forma que leen todos los programas, con comas entre los argumentos y puntos en los números, para que se abra igual en todas partes.

Si una fórmula tiene un error, como un paréntesis que no se cierra, JeffCrunch le indica qué está mal, y la entrada se queda tal como la escribió, con el cursor cerca del error, para que pueda corregirlo. No se cambia nada hasta que la fórmula sea correcta.

Una celda admite hasta 32.767 caracteres, y una fórmula hasta 8192. JeffCrunch le avisa cuando una entrada es más larga.

## Borrar celdas

Seleccione las celdas y presione **Supr**. Así se borra el contenido de las celdas y se conservan sus formatos, y es un solo paso que puede deshacer. JeffCrunch borra hasta 100.000 celdas a la vez.

## Deshacer y Rehacer

**{key:Undo}** deshace el último cambio y **{key:Redo}** lo vuelve a hacer. También están en la barra de herramientas de acceso rápido, en la parte superior. Cada celda en la que escribe es un paso, y cada borrado también. Deshacer deja la celda como estaba: su valor, su fórmula, su formato y las fórmulas que dependen de ella. JeffCrunch recuerda hasta 1000 pasos, menos cuando cambiaron muchísimas celdas, para no ocupar nunca mucha memoria.

## Mientras JeffCrunch calcula las fórmulas

Cuando cambia una celda, JeffCrunch calcula en segundo plano las fórmulas que dependen de ella. La celda que cambió muestra enseguida su nuevo contenido y las demás celdas se actualizan un momento después; la barra de estado muestra **Calculando** hasta que terminan. Mientras tanto, puede seguir escribiendo.

## Lo que no se puede cambiar

- Una celda que forma parte de una **fórmula matricial** o de una tabla de datos no se puede cambiar por separado.
- En una **hoja protegida**, una celda bloqueada no se puede cambiar. Todas las celdas están bloqueadas, salvo que el libro indique lo contrario. JeffCrunch le avisa cuando lo intenta. Para cambiar la celda, elija Desproteger hoja en la pestaña Revisar.
- La primera fila de una **tabla** contiene los nombres de sus columnas. Escriba un nombre nuevo en una de esas celdas para cambiar el nombre de la columna: las fórmulas que usan el nombre anterior se actualizan y la tabla del archivo sigue siendo válida. Un nombre que ya tiene otra columna recibe un número detrás, un nombre no puede estar vacío y **Supr** no borra esa fila.
- Un libro que no se pudo leer completo se puede consultar, pero no cambiar ni guardar.
- Una función que JeffCrunch aún no tiene deja la nueva fórmula sin valor en la celda; el libro pide al programa que lo abra a continuación que calcule la fórmula.
- El formato, la ordenación y la búsqueda y reemplazo llegarán en una versión posterior. Insertar y eliminar, cortar y pegar, rellenar, y el tamaño de filas y columnas se explican en [Insertar y eliminar filas, columnas y celdas](insert-delete), [Cortar, copiar y pegar](clipboard), [Rellenar celdas y sumarlas](fill-sum) y [Cambiar el ancho de las columnas y el alto de las filas](sizes).
