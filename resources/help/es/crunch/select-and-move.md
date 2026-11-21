# Seleccionar celdas y desplazarse

<!-- keywords: seleccionar, selección, celda, rango, fila, columna, clic, arrastrar, mayús, ctrl, flecha, inicio, fin, re pág, av pág, tabulador, intro, fila completa, columna completa, cuadro de nombres, ir a, mover celdas, arrastrar borde, desplazar, barra de estado, suma, promedio, contar -->

**Seleccionar** celdas significa elegir aquellas a las que se aplicará el siguiente comando. Las celdas seleccionadas aparecen con un tono de color y un borde verde a su alrededor. La **celda activa** es la que tiene el cursor, y permanece en blanco dentro de la selección.

## Con el ratón

- **Haga clic** en una celda para seleccionarla. Haga clic dentro de una celda combinada para seleccionar toda la celda combinada.
- **Arrastre** sobre las celdas para seleccionar un bloque. Si arrastra más allá del borde de la ventana, la hoja se desplaza.
- **Mayús+clic** en otra celda para seleccionar desde la celda activa hasta esa.
- Haga clic en una **letra de columna** o en un **número de fila** para seleccionar la columna o la fila completa. Arrastre sobre las letras o los números, o use Mayús+clic, para seleccionar varias.
- Haga clic en la **esquina** donde se unen las letras y los números para seleccionar toda la hoja.
- Mantenga pulsada **Ctrl** mientras hace clic o arrastra para **agregar un rango** a la selección. Los rangos agregados aparecen con un tono de color, y el que contiene el cursor tiene el borde verde. **Suprimir** borra todos los rangos de una vez. Un comando que trabaja con un solo rango a la vez, como Copiar, Rellenar, Ordenar, Buscar en la selección, Reemplazar todo o un formato, se lo indica y no cambia nada; haga clic una vez para volver a un solo rango. Cambiar a otra hoja, o insertar o quitar filas o columnas, también deja un solo rango.
- **Arrastre el borde** de la selección para mover las celdas a otro lugar. Sitúe el puntero en el borde verde hasta que muestre cuatro flechas. Una línea discontinua indica dónde caerán las celdas. Todo lo que apunte a las celdas, como una fórmula, las seguirá. Mantenga pulsada **Ctrl** mientras arrastra para copiar las celdas en lugar de moverlas. Para mover las celdas a otra hoja, empiece a arrastrar el borde, mantenga pulsada **Alt** y suelte sobre la pestaña de esa hoja: las celdas se colocan en las mismas posiciones de esa hoja (si además mantiene pulsada **Ctrl**, se copian allí). Mantener **Alt** antes de pulsar el botón del ratón no inicia este tipo de arrastre. Las celdas que ya están en el lugar de destino se sobrescriben, igual que al mover celdas dentro de una misma hoja, y **Deshacer** devuelve ambas hojas en un solo paso. La hoja donde suelta debe permitirlo: una hoja protegida, o celdas que cortarían celdas combinadas, una tabla o una tabla dinámica, no se admiten, y no se cambia nada. Una hoja oculta no tiene pestaña, y el botón **Nueva hoja** no es una pestaña, así que ninguno de los dos recibe celdas. Pulsar y soltar **Alt** durante un arrastre no muestra las KeyTips. Para hacer lo mismo desde el teclado, corte las celdas, vaya a la otra hoja y pegue. Mantenga pulsada **Mayús** mientras arrastra para colocar las celdas entre otras en lugar de sobre ellas. Para hacer lo mismo sin ratón, corte o copie las celdas y use **Insertar celdas cortadas o copiadas** (consulte [Insertar y eliminar](insert-delete)). Si solo hace clic en el borde sin mover, se selecciona la celda como de costumbre.
- Gire la **rueda del ratón** para desplazarse tres filas. Mantenga pulsada **Mayús** para desplazarse en horizontal, o **Ctrl** para hacer zoom.

## Con el teclado

Las teclas de flecha mueven la celda activa, saltando las filas y columnas ocultas. Mantenga pulsada **Mayús** para ampliar la selección. **Ctrl** con una flecha lleva al borde de los datos: al final de una serie de celdas que contienen algo o, desde el final de una serie, a la siguiente celda que contenga algo. Consulte [Usar JeffCrunch con el teclado](keyboard) para ver todas las teclas.

**Ctrl+A** selecciona el bloque de celdas alrededor de la celda activa, delimitado por filas y columnas vacías; púlselo de nuevo para seleccionar toda la hoja. Si la selección ya es mayor que ese bloque, Ctrl+A selecciona toda la hoja de una vez. **Ctrl+Mayús+\*** selecciona solo el bloque. Pulse **Fin** y luego una flecha para ir al borde de los datos, igual que con Ctrl y la flecha; la barra de estado indica **Fin** hasta que pulse la flecha, otra tecla o haga clic.

## Ir a una celda

Escriba una dirección en el **Cuadro de nombres**, como **C5**, **A1:D10**, **B:D** o **3:7**, y pulse **Intro**. Pulse **F5** (o Ctrl+G) para abrir el cuadro **Ir a** y escriba la dirección allí. Separe las direcciones con comas, como **A1:B2,D4**, para seleccionar varios rangos a la vez.

## Los números de la selección

Cuando selecciona dos o más celdas, la barra de estado muestra el **promedio**, el **recuento** y la **suma** de las celdas. El recuento es el número de celdas que contienen algo; el promedio y la suma usan solo los números. Las filas y columnas ocultas no se incluyen. Para copiar la selección como texto, pulse **Ctrl+C**.

## Qué indica la barra de estado

En la parte izquierda de la barra de estado aparece **Listo** cuando puede trabajar, **Introducir** mientras escribe en una celda, **Editar** después de pulsar F2, **Señalar** mientras puede hacer clic o usar las flechas para situar una celda y poner su dirección en una fórmula, y **Fin** después de pulsar la tecla Fin. Cuando hay un filtro activo, también indica cuántas filas quedan tras el filtro, como **3 de 8 registros encontrados**.
