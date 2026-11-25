# Análisis rápido
<!-- keywords: análisis rápido, analizar, análisis, gráfico instantáneo, gráfico rápido, totales rápidos, totales, total acumulado, porcentaje del total, barras de datos, escala de colores, conjunto de iconos, minigráficos, tabla, tabla dinámica, Ctrl+Q, vista previa, sumar, suma, resumen -->

**Análisis rápido** reúne en un cuadro pequeño las formas más habituales de ver un bloque de números: dar color a las celdas, crear un gráfico con ellas, calcular totales, convertirlas en una tabla o añadir a cada fila un gráfico diminuto. Usted elige lo que quiere y lo ve en la hoja antes de decidir.

## Abrirlo

1. Seleccione dos o más celdas que tengan algún contenido. Aparece un botón pequeño justo debajo y a la derecha de la última celda seleccionada.
2. Haga clic en el botón, pulse **Ctrl+Q** (en un Mac, **Control+Q**) o haga clic en **Inicio > Edición > Análisis rápido**.
3. Elija una pestaña y, después, una opción.

El botón nunca quita el teclado a la hoja. Desaparece mientras edita una celda, cuando selecciona una sola celda o celdas sin contenido, o cuando la hoja está protegida de forma que no permite hacer nada de lo que ofrece el cuadro.

## Vea antes de elegir

Sitúe el puntero sobre una opción, o pulse las teclas de **flecha izquierda** y **flecha derecha** para moverse por las opciones de una pestaña (las teclas de **flecha arriba** y **flecha abajo** cambian de pestaña). La hoja muestra lo que haría la opción, pero no se cambia nada. (En los totales de un número muy grande de celdas, se indica que hay demasiadas celdas para mostrar una vista previa, y la opción sigue funcionando al hacer clic en ella.) No hay ningún paso que deshacer, y un archivo guardado en ese momento no contiene nada de ello. Pulse **Esc** o haga clic en otra parte, y la vista previa desaparece. Haga clic en una opción, o pulse **Intro**, para aplicarla. Cada opción que aplique es un paso de **Deshacer**.

## Las pestañas

- **Formato**: **Barras de datos**, **Escala de colores** y **Conjunto de iconos** muestran lo grande que es cada número. **Mayor que** da color a las celdas cuyo número es mayor que el promedio de todos los números. **Superior al 10 %** da color a la décima parte más alta. **Borrar formato** quita el formato condicional de las celdas seleccionadas. Son las mismas reglas que en **Inicio > Formato condicional**, y allí puede cambiarlas.
- **Gráficos**: hasta cuatro tipos de gráfico que se ajustan a la forma de sus celdas, los mismos que elige **Insertar > Gráficos recomendados**. Una vista previa muestra el gráfico en la esquina de la hoja. Al elegir uno, el gráfico se coloca en la hoja.
- **Totales**: **Suma**, **Promedio**, **Contar**, **% del total** y **Total acumulado**, escritos como fórmulas normales. La primera fila de opciones pone un total debajo de cada columna de números. La segunda fila (**Suma de filas**, etc.) pone un total a la derecha de cada fila. Si la primera fila o la primera columna solo contienen palabras, se consideran nombres y se dejan fuera; una primera fila de números, como años, se suma igual que el resto. Las filas ocultas y filtradas se incluyen. **% del total** muestra la parte de cada columna (o fila) sobre el conjunto de los números, en forma de porcentaje. **Total acumulado** va sumando a medida que avanza, en celdas nuevas junto a (o debajo de) sus celdas. Si alguna de las celdas donde van los totales ya contiene algo, se le pregunta antes de reemplazarla. Los totales no se escriben sobre celdas que ocupe un resultado desbordado ni sobre celdas combinadas: Análisis rápido se lo indica y no cambia nada.
- **Tablas**: **Tabla** convierte las celdas en una tabla, con botones para ordenar y filtrar. **Tabla dinámica** crea una tabla dinámica que totaliza los datos, en una hoja nueva; la primera fila debe contener los nombres de las columnas.
- **Minigráficos**: **Línea**, **Columna** y **Ganancia/pérdida** dibujan un pequeño gráfico de cada fila en la celda situada a su derecha.

## Límites

Análisis rápido examina hasta 250 000 celdas para los totales y los minigráficos. Si selecciona más, se lo indica y no cambia nada. Una opción que no quepa (por ejemplo, si no hay espacio debajo de las celdas) también se lo indica.
