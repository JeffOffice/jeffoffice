# Cómo se muestran y calculan las fórmulas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, volver a calcular, función, funciones, valor, resultado, almacenado, F9, circular, referencia circular, mostrar fórmulas, barra de fórmulas, error, señalar, apuntar, hacer clic en una celda, referencia, F4 -->

Una **fórmula** es una celda que calcula su valor a partir de otras celdas, como =SUMA(B3:B8). La celda muestra el valor y la **barra de fórmulas** muestra la fórmula.

## Qué hace JeffCrunch con ellas

Al abrir un libro, el archivo contiene el último valor de cada fórmula, de modo que la hoja se puede mostrar al instante. Después, JeffCrunch vuelve a calcular todas las fórmulas, unas pocas cada vez, para que la ventana siga respondiendo con rapidez. Mientras lo hace, la barra de estado muestra **Calculando** con un porcentaje. Cuando termina, las celdas contienen los resultados calculados por JeffCrunch. Para volver a calcularlo todo, haga clic en **Volver a calcular todo** en la pestaña **Fórmulas** o presione **F9**.

## Cuando JeffCrunch no puede calcular una fórmula

JeffCrunch tiene más de 200 funciones, y siguen aumentando. Una fórmula que usa una función que JeffCrunch aún no tiene conserva el valor que el archivo almacenó para ella. Lo mismo ocurre con una fórmula que hace referencia a otro libro. Las celdas que dependen de ellas se calculan a partir de los valores almacenados.

## RESIDUO: el resto

**RESIDUO(n; d)** devuelve lo que queda de **n** después de quitar el mayor número posible de veces enteras **d**: es decir, n - d * ENTERO(n / d). Ejemplo: **=RESIDUO(10; 3)** es 1, y **=RESIDUO(-10; 3)** es 2, porque ENTERO(-10 / 3) es -4 y -10 - 3 * -4 es 2. El resultado tiene el signo de **d**. Si **d** es 0, el resultado es #¡DIV/0!.

## Mostrar las fórmulas

Haga clic en **Mostrar fórmulas** en la pestaña **Fórmulas**, o presione **Ctrl+`** (en un Mac, la tecla Control, no la tecla Comando), para ver la fórmula de cada celda en lugar de su valor. Las columnas que contienen fórmulas se ensanchan, hasta un límite, para que las fórmulas se vean completas; recuperan su ancho al desactivar esta opción, y los anchos del archivo no se modifican.

## Modificar fórmulas

Para modificar una fórmula, seleccione su celda, presione **F2**, edítela y presione **Entrar**. También puede editarla en la barra de fórmulas. Cuando cambia una celda, las fórmulas que dependen de ella se vuelven a calcular en segundo plano. Consulte [Escribir en las celdas](edit-cells).

## Escribir una fórmula

Escriba **=** y las primeras letras de una función, y aparecerá una lista breve con las funciones y los [nombres](names) que empiezan así, con una frase sobre cada uno. Presione **Tab** para elegir el que está marcado. Use las flechas **Abajo** y **Arriba** para marcar otro y, después, presione **Tab** o **Entrar**. **Esc** cierra la lista. Cuando el cursor está dentro de los paréntesis de una función, sus argumentos se muestran debajo de la celda, con el que está escribiendo en negrita. La lista y los argumentos son los mismos que muestra **Elegir una función**. Los argumentos se separan con comas, o con punto y coma en los idiomas que usan la coma como separador decimal.

## Señalar celdas

En lugar de escribir la dirección de una celda, puede señalarla. Mientras escribe una fórmula, coloque el cursor donde puede ir una celda (justo después de **=**, de un operador como **+**, de un paréntesis de apertura o de un separador) y haga clic en la celda. Su dirección se escribe automáticamente. Arrastre sobre varias celdas para escribir un rango como B2:D5. Si vuelve a hacer clic antes de escribir nada más, la nueva celda reemplaza a la que se acaba de escribir. Mantenga presionada **Ctrl** y haga clic para agregar otra dirección después de un separador. Haga clic en la letra de una columna para escribir una columna entera, como A:A, o en el número de una fila para escribir una fila entera, como 3:3, y arrastre por las letras o los números para obtener B:D o 2:5.

También funcionan las teclas de dirección. Presione una flecha y un puntero se aleja de la celda en la que escribe y se escribe su dirección; presione más flechas para moverlo, o mantenga presionada **Mayús** para extender el puntero sobre un rango. La barra de estado muestra **Señalar** mientras las teclas de dirección señalan celdas, **Introducir** mientras escribe y **Modificar** mientras las teclas de dirección mueven el cursor por el texto. Presione **F2** para cambiar entre señalar y editar el texto.

Para señalar una celda de otra hoja, haga clic en la pestaña de esa hoja mientras el cursor está donde puede ir una celda y, después, haga clic en la celda. El nombre de la hoja se escribe con ella, entre comillas si es necesario, como en 'Mi hoja'!B3. **Entrar** le devuelve a la hoja en la que empezó y coloca la fórmula en su celda. **Esc** lo cancela todo, incluido el cambio de hoja.

También puede arrastrar el borde de un rango resaltado en la hoja para mover esa dirección, o arrastrar su cuadrado de la esquina para ampliarlo; la fórmula cambia mientras arrastra.

Presione **F4** con el cursor dentro o justo después de una dirección para fijar partes de ella con el signo de dólar: A1 pasa a ser $A$1, luego A$1, luego $A1 y de nuevo A1.

Cada dirección de la fórmula aparece coloreada en el texto, y las celdas a las que hace referencia se resaltan en la hoja con el mismo color, para que vea de un vistazo qué lee la fórmula. Un clic después de una dirección completa, como =A1, finaliza la entrada y selecciona la celda en la que hizo clic, como ocurre con cualquier otro texto.

## Tablas de datos

Una tabla de datos (tabla de hipótesis) muestra el resultado de una fórmula para muchos valores de una o dos celdas. Si abre un libro que tiene una, JeffCrunch calcula sus celdas: en una tabla con dos variables, la fórmula está en la celda de la esquina superior izquierda de la tabla, los valores horizontales se colocan en la celda de entrada de fila y los verticales en la celda de entrada de columna. En una tabla con una variable, los valores van hacia abajo por la columna de su izquierda (y las fórmulas están en la fila superior) o a lo largo de la fila superior (y las fórmulas están en la columna de su izquierda). Cada celda de la tabla es el resultado de la fórmula, y de todas las fórmulas que dependen de la celda de entrada, con la entrada establecida en ese valor; no se modifica nada más del libro. En el modo **Automático**, las tablas se calculan junto con las fórmulas, y después se calculan las fórmulas que leen las celdas de una tabla. Todavía no se pueden crear tablas de datos nuevas. Una tabla que no se puede calcular (porque una fórmula que depende de la entrada usa una función que JeffCrunch no tiene, o cambia por sí sola, como ALEATORIO o AHORA) conserva los valores que contiene su archivo.

## Cálculo manual

Haga clic en **Modo de cálculo** en la pestaña **Fórmulas** para elegir cómo calcula el libro sus fórmulas:

- **Automático** las calcula a medida que cambia las celdas.
- **Automático, excepto tablas** hace lo mismo, pero deja las celdas de las tablas de datos como están hasta que presiona **F9**.
- **Manual** no calcula nada, ni siquiera las tablas de datos, hasta que usted lo pida. Después de un cambio, la barra de estado muestra **Desactualizado**. Presione **F9** (**Volver a calcular todo**) para calcularlo todo, o **Mayús+F9** (**Recalcular esta hoja**) para calcular las fórmulas de la hoja en la que está y todo lo que estuviera pendiente. Un libro configurado en Manual se abre con los resultados que contiene su archivo.

La opción se guarda con el libro y se puede deshacer en un solo paso.

## Fórmulas que hacen referencia a sí mismas

Una fórmula que lee su propio resultado, directamente o a través de otras celdas, no se puede calcular de la forma habitual. JeffCrunch se lo indica una vez, nombra las celdas y las resalta en la hoja; estas conservan los valores que tenían. Haga clic en **Referencias circulares** en la pestaña **Fórmulas** para ver la lista, y haga doble clic en una celda de la lista para ir a ella.

Algunos modelos necesitan este tipo de círculo a propósito. Elija **Iterar fórmulas circulares** en el menú **Modo de cálculo**, y las fórmulas se calcularán una y otra vez, cada una leyendo los últimos valores de las demás, hasta que ningún valor cambie más de la cantidad que indique o se alcance el número de iteraciones que permita.

## Ver qué lee una fórmula

Seleccione una celda y haga clic en **Mostrar lo que alimenta esta celda** para dibujar flechas desde las celdas que lee su fórmula. Haga clic de nuevo para retroceder un paso más. **Mostrar lo que usa esta celda** dibuja flechas hacia las fórmulas que leen la celda, y cada clic avanza un paso. Una pequeña página representa una celda de otra hoja. Se dibujan como máximo 2.000 flechas, y la barra de estado indica si había más. **Quitar flechas** las quita, igual que cualquier cambio en el libro.
