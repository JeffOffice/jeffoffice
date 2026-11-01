# Cambiar la vista
<!-- keywords: esquema, agrupar, desagrupar, contraer, expandir, detalle, nivel, vista, zoom, derecha a izquierda, de derecha a izquierda, árabe, hebreo, reflejado, inmovilizar, inmovilizar paneles, movilizar, fijar filas, fijar columnas, líneas de cuadrícula, encabezados, barra de fórmulas, mostrar fórmulas, pestañas de hoja, hojas ocultas, títulos -->

La pestaña **Vista** cambia el aspecto de la hoja en la pantalla. Nunca modifica el libro.

## Vistas

El grupo **Vistas** de la pestaña Vista (y los tres botones de la parte inferior derecha de la ventana) cambian lo que muestra la hoja. **Normal** muestra las celdas tal como son. **Diseño de página** muestra las páginas tal como se imprimen, con su encabezado y pie de página. **Saltos de página** sombrea lo que no se imprime, numera las páginas y permite arrastrar las líneas donde empiezan las páginas. Para las dos últimas, consulte [Configurar las páginas](page-layout). Cada hoja recuerda su vista, y el libro la guarda.

## Zoom

Use el control deslizante de la parte inferior derecha, el botón **Zoom** de la pestaña Vista, o mantenga presionada la tecla **Ctrl** y gire la rueda del mouse. Puede aplicar un zoom del 10 % al 400 %. **100 %** vuelve al tamaño normal.

## Mostrar y ocultar

En el grupo **Mostrar**, active o desactive las **Líneas de cuadrícula** (las líneas tenues entre celdas), los **Encabezados** (las letras de columna y los números de fila) y la **Barra de fórmulas**. Un libro puede pedir que sus líneas de cuadrícula o encabezados estén desactivados; usted puede cambiar esta opción.

## Inmovilizar filas y columnas

Las filas y columnas **inmovilizadas** permanecen fijas mientras se desplaza el resto de la hoja, de modo que una fila de encabezado siempre está a la vista. En la pestaña **Vista**, haga clic en **Inmovilizar paneles**:

- **Inmovilizar paneles** inmoviliza las filas situadas encima de la celda seleccionada y las columnas situadas a su izquierda, a partir de la primera fila y columna visibles. Las filas y columnas que haya desplazado fuera de la vista permanecen ocultas hasta que movilice los paneles.
- **Inmovilizar fila superior** inmoviliza la primera fila visible.
- **Inmovilizar primera columna** inmoviliza la primera columna visible.
- **Movilizar paneles** las libera, y la hoja se queda donde está.

Las filas y columnas inmovilizadas se guardan con el libro, de modo que vuelven a quedar fijas al abrirlo.

Un libro que se guardó con paneles inmovilizados se abre con ellos.

## Hojas

Las pestañas de la parte inferior son las hojas del libro. Haga clic en una pestaña para mostrar esa hoja. Cada hoja recuerda dónde estaba usted en ella. Las hojas ocultas no se muestran. Una línea de color bajo una pestaña indica el color de la pestaña de la hoja. Para agregar, cambiar el nombre, mover, copiar, ocultar, eliminar o colorear una hoja, consulte [Trabajar con hojas](sheets).

## Hojas que se leen de derecha a izquierda

Una hoja creada para un idioma como el árabe o el hebreo se lee de derecha a izquierda, y JeffCrunch la muestra así: la columna A está en el borde derecho, los números de fila están a la derecha y la barra de desplazamiento horizontal funciona en sentido contrario. Las teclas de dirección siguen la hoja, de modo que la flecha Izquierda va a la columna siguiente. El texto de una celda empieza desde el lado desde el que se lee su idioma. El libro indica en qué sentido se lee una hoja; JeffCrunch aún no tiene ningún comando para cambiarlo.

## Agrupar filas y columnas (esquema)

En la pestaña **Datos**, **Agrupar filas** sube un nivel las filas seleccionadas y **Desagrupar filas** lo baja; **Agrupar columnas** y **Desagrupar columnas** hacen lo mismo con las columnas. Las filas se pueden agrupar dentro de otros grupos, hasta siete niveles. Junto a las filas (y encima de las columnas), la hoja muestra un corchete para cada grupo y, en la línea que resume el grupo (la línea situada debajo de las filas o a la derecha de las columnas, salvo que el archivo las coloque encima o a la izquierda), un botón: **menos** oculta las líneas del grupo y **más** vuelve a mostrarlas. Los botones numerados de la esquina muestran el esquema hasta un nivel: **1** muestra solo las líneas que no están en ningún grupo, **2** también las líneas del primer nivel, y así sucesivamente. Un grupo dentro de un grupo oculto sigue oculto cuando se muestra el grupo exterior, hasta que se hace clic en su propio botón. Cada una de estas acciones es un paso de Deshacer, y las líneas ocultas y los grupos se guardan con el libro. Al desagrupar un grupo oculto se muestran sus líneas.

**Vista > Símbolos de esquema** desactiva y activa los corchetes y los botones.

## Mostrar fórmulas

En la pestaña **Fórmulas**, **Mostrar fórmulas** (Ctrl+`; en un Mac, la tecla Control, no la tecla Comando) muestra la fórmula de cada celda que la contiene, en lugar de su valor. Las columnas que contienen fórmulas se ensanchan, hasta un límite, para que las fórmulas se vean completas, y recuperan su ancho al desactivar la opción; los anchos del archivo no cambian.
