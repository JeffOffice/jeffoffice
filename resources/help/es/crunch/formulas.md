# Cómo se muestran y se calculan las fórmulas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, volver a calcular, función, funciones, valor, resultado, almacenado, guardado, F9, circular, referencia circular, mostrar fórmulas, barra de fórmulas, error -->

Una **fórmula** es una celda que calcula su valor a partir de otras celdas, como =SUMA(B3:B8). La celda muestra el valor y la **barra de fórmulas** muestra la fórmula.

## Qué hace JeffCrunch con ellas

Al abrir un libro, el archivo contiene el último valor de cada fórmula, de modo que la hoja se puede mostrar de inmediato. Después, JeffCrunch vuelve a calcular todas las fórmulas, unas pocas cada vez, para que la ventana siga respondiendo con rapidez. Mientras tanto, la barra de estado muestra **Calculando** con un porcentaje. Cuando termina, las celdas contienen los resultados calculados por JeffCrunch. Para volver a calcularlo todo, haga clic en **Volver a calcular todo** en la pestaña **Fórmulas** o presione **F9**.

## Cuando JeffCrunch no puede calcular una fórmula

JeffCrunch tiene más de 200 funciones, y el número sigue creciendo. Una fórmula que usa una función que JeffCrunch aún no tiene conserva el valor que el archivo guardó para ella. Lo mismo ocurre con una fórmula que hace referencia a otro libro. Las celdas que dependen de ellas se calculan a partir de los valores guardados.

## Mostrar las fórmulas

Haga clic en **Mostrar fórmulas** en la pestaña **Fórmulas** o presione **Ctrl+`** (en un Mac, la tecla Control, no la tecla Comando) para ver la fórmula de cada celda en lugar de su valor.

## Cambiar fórmulas

Para cambiar una fórmula, seleccione su celda, presione **F2**, modifíquela y presione **Entrar**. También puede modificarla en la barra de fórmulas. Cuando cambia una celda, las fórmulas que dependen de ella se vuelven a calcular en segundo plano. Consulte [Escribir en las celdas](edit-cells).

## Escribir una fórmula

Escriba **=** y las primeras letras de una función, y aparecerá una breve lista con las funciones y los [nombres](names) que empiezan así, con una frase sobre cada uno. Presione **Tab** para elegir el que está marcado. Use las flechas **Abajo** y **Arriba** para marcar otro y, a continuación, presione **Tab** o **Entrar**. **Esc** cierra la lista. Cuando esté dentro de los paréntesis de una función, sus argumentos aparecen debajo de la celda, con el que está escribiendo en negrita. La lista y los argumentos son los mismos que muestra **Elegir una función**. Los argumentos se separan con comas, o con punto y coma en los idiomas que usan la coma como separador decimal.

## Calcular manualmente

Haga clic en **Modo de cálculo** en la pestaña **Fórmulas** para elegir cómo calcula el libro sus fórmulas:

- **Automático** las calcula a medida que cambia las celdas.
- **Automático, excepto tablas** hace lo mismo y se guarda en el archivo para los programas que tratan las tablas de datos por separado.
- **Manual** no calcula nada hasta que usted lo pida. Después de un cambio, la barra de estado muestra **Desactualizado**. Presione **F9** (**Volver a calcular todo**) para calcularlo todo, o **Mayús+F9** (**Recalcular esta hoja**) para calcular las fórmulas de la hoja en la que se encuentra y todo lo demás que estuviera pendiente. Un libro configurado en Manual se abre con los resultados que contiene su archivo.

La opción se guarda con el libro y es un paso que se puede deshacer.

## Fórmulas que hacen referencia a sí mismas

Una fórmula que hace referencia a su propio resultado, directamente o a través de otras celdas, no se puede calcular de la forma habitual. JeffCrunch le avisa una vez, indica las celdas y las marca con un contorno en la hoja; estas conservan los valores que tenían. Haga clic en **Referencias circulares** en la pestaña **Fórmulas** para ver la lista, y haga doble clic en una celda de la lista para ir a ella.

Algunos modelos necesitan un ciclo así a propósito. Elija **Iterar fórmulas circulares** en el menú **Modo de cálculo**, y las fórmulas se calcularán una y otra vez, cada una con los valores más recientes de las demás, hasta que ningún valor cambie más de la cantidad que indique o se alcance el número de iteraciones permitido.

## Ver qué lee una fórmula

Seleccione una celda y haga clic en **Mostrar lo que alimenta esta celda** para dibujar flechas desde las celdas que lee su fórmula. Haga clic de nuevo para retroceder un paso más. **Mostrar lo que usa esta celda** dibuja flechas hacia las fórmulas que leen la celda, y cada clic avanza un paso. Una pequeña página representa una celda de otra hoja. Se dibujan como máximo 2000 flechas, y la barra de estado indica si había más. **Quitar flechas** las borra, al igual que cualquier cambio en el libro.
