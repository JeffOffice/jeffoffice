# Seleccionar celdas y moverse

<!-- keywords: seleccionar, selección, celda, rango, fila, columna, clic, arrastrar, mayús, ctrl, flecha, inicio, fin, re pág, av pág, tabulador, intro, fila completa, columna completa, cuadro de nombres, ir a, mover celdas, arrastrar borde, desplazar, barra de estado, suma, promedio, contar -->

**Seleccionar** celdas significa elegir aquellas a las que se aplicará el siguiente comando. Las celdas seleccionadas aparecen con un tono de color y un borde verde alrededor. La **celda activa** es la que tiene el cursor, y dentro de la selección permanece en blanco.

## Con el ratón

- **Haga clic** en una celda para seleccionarla. Si hace clic dentro de una celda combinada, se selecciona la celda combinada completa.
- **Arrastre** sobre las celdas para seleccionar un bloque. Si arrastra más allá del borde de la ventana, la hoja se desplaza.
- **Mayús+clic** en otra celda para seleccionar desde la celda activa hasta esa.
- Haga clic en una **letra de columna** o en un **número de fila** para seleccionar la columna o la fila completa. Arrastre sobre las letras o los números, o use Mayús+clic, para seleccionar varias.
- Haga clic en la **esquina** donde se cruzan las letras y los números para seleccionar todo.
- Mantenga pulsada **Ctrl** mientras hace clic o arrastra para **agregar un rango** a la selección. Los rangos que agregue aparecen con un tono de color, y el que tiene el cursor muestra el borde verde. **Suprimir** borra todos los rangos de una sola vez. Un comando que trabaja con un solo rango a la vez, como Copiar, Rellenar, Ordenar, Buscar en la selección, Reemplazar todo o un formato, se lo indica y no cambia nada; haga clic una vez para volver a un solo rango. Al cambiar a otra hoja, o al insertar o quitar filas o columnas, la selección también vuelve a ser un solo rango.
- **Arrastre el borde** de la selección para mover las celdas a otro lugar. Sitúe el puntero en el borde verde hasta que muestre cuatro flechas. Una línea de puntos indica dónde quedarán las celdas. Todo lo que apunte a las celdas, como una fórmula, las seguirá. Mantenga pulsada **Ctrl** mientras arrastra para copiar las celdas en lugar de moverlas. Mantenga pulsada **Mayús** mientras arrastra para insertar las celdas entre otras en lugar de sobrescribirlas. Para hacer lo mismo sin ratón, corte o copie las celdas y use **Insertar celdas cortadas o copiadas** (consulte [Insertar y eliminar](insert-delete)). Si solo hace clic en el borde sin mover, se selecciona la celda como de costumbre.
- Gire la **rueda del ratón** para desplazarse tres filas. Mantenga pulsada **Mayús** para desplazarse horizontalmente, o **Ctrl** para cambiar el zoom.

## Con el teclado

Las teclas de flecha mueven la celda activa, omitiendo las filas y columnas ocultas. Mantenga pulsada **Mayús** para ampliar la selección. **Ctrl** con una flecha lleva al borde de los datos: al final de una serie de celdas que contienen algo o, desde el final de una serie, a la siguiente celda que contenga algo. Consulte [Usar JeffCrunch con el teclado](keyboard) para ver todas las teclas.

**Ctrl+A** selecciona el bloque de celdas alrededor de la celda activa, delimitado por filas y columnas vacías; púlselo de nuevo para seleccionar toda la hoja. Cuando la selección ya es mayor que ese bloque, Ctrl+A selecciona de una vez toda la hoja. **Ctrl+Mayús+\*** selecciona solo el bloque. Pulse **Fin** y, a continuación, una flecha, para ir al borde de los datos como lo hace **Ctrl** con la flecha; la barra de estado muestra **Fin** hasta que pulse la flecha, otra tecla o haga clic.

## Ir a una celda

Escriba una dirección en el **Cuadro de nombres**, como **C5**, **A1:D10**, **B:D** o **3:7**, y pulse **Intro**. Pulse **F5** (o Ctrl+G) para abrir **Ir a** y escriba allí la dirección. Separe las direcciones con comas, como **A1:B2,D4**, para seleccionar varios rangos a la vez.

## Los números de la selección

Cuando selecciona dos o más celdas, la barra de estado muestra el **promedio**, el **recuento** y la **suma** de las celdas. El recuento es el número de celdas que contienen algo; el promedio y la suma solo usan los números. Las filas y columnas ocultas no se incluyen. Para copiar la selección como texto, pulse **Ctrl+C**.

## Qué indica la barra de estado

En la parte izquierda de la barra de estado aparece **Listo** cuando puede trabajar; **Introducir** mientras escribe en una celda; **Editar** después de pulsar F2; **Señalar** mientras puede hacer clic o usar las flechas para colocar la dirección de una celda en una fórmula; y **Fin** después de pulsar la tecla Fin. Cuando hay un filtro activo, también indica cuántas filas ha dejado el filtro, como **3 de 8 registros encontrados**.
