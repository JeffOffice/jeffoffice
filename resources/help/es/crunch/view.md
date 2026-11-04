# Cambiar la vista

<!-- keywords: esquema, agrupar, desagrupar, contraer, expandir, detalle, nivel, vista, zoom, de derecha a izquierda, árabe, hebreo, reflejado, inmovilizar, inmovilizar paneles, movilizar paneles, desbloquear, líneas de cuadrícula, encabezados, barra de fórmulas, contraer la cinta de opciones, pantalla completa, barra de estado, mínimo, máximo, mostrar fórmulas, pestañas de hojas, hojas ocultas, títulos -->

La pestaña **Vista** cambia el aspecto de la hoja en la pantalla. Nunca cambia el libro.

## Vistas

El grupo **Vistas** de la pestaña Vista (y los tres botones de la parte inferior derecha de la ventana) cambian lo que muestra la hoja. **Normal** muestra las celdas tal como están. **Diseño de página** muestra las páginas tal como se imprimirán, con su encabezado y pie de página. **Saltos de página** sombrea lo que no se imprime, numera las páginas y le permite arrastrar las líneas donde empiezan las páginas. Para las dos últimas, consulte [Configurar las páginas](page-layout). Cada hoja recuerda su vista, y el libro la guarda.

## Zoom

Use el control deslizante de la parte inferior derecha, el botón **Zoom** de la pestaña Vista, o mantenga presionada la tecla **Ctrl** y gire la rueda del ratón. Puede ampliar o reducir del 10 % al 400 %. **100 %** lo devuelve al tamaño normal.

## Mostrar y ocultar

En el grupo **Mostrar**, active o desactive las **Líneas de cuadrícula** (las líneas pálidas entre celdas), los **Encabezados** (las letras de columna y los números de fila) y la **Barra de fórmulas**. Un libro puede pedir que sus líneas de cuadrícula o encabezados estén desactivados; esta elección es suya para cambiarla.

Hay dos opciones más en el mismo grupo. **Contraer la cinta de opciones** (**Ctrl+F1**) pliega la cinta para que solo muestre los nombres de las pestañas, lo que deja más espacio para las celdas. Haga clic de nuevo, o haga clic en una pestaña, para volver a mostrarla. **Pantalla completa** (**Ctrl+Mayús+F1**) hace que la ventana ocupe toda la pantalla. Haga clic de nuevo para volver atrás.

## Los números de la barra de estado

Cuando selecciona dos o más celdas, la barra de estado muestra números sobre ellas. **Haga clic con el botón derecho** en los números para elegir cuáles se muestran: **Promedio**, **Recuento** (las celdas que contienen algo), **Recuento numérico** (solo las celdas que contienen números), **Mínimo** (el número más pequeño), **Máximo** (el número más grande) y **Suma**. Una marca de verificación indica cuáles están activados. Siempre queda activado al menos uno. JeffCrunch recuerda su elección.

## Inmovilizar filas y columnas

Las filas y columnas **inmovilizadas** permanecen fijas mientras el resto de la hoja se desplaza, de modo que una fila de encabezados siempre está visible. En la pestaña **Vista**, haga clic en **Inmovilizar paneles**:

- **Inmovilizar paneles** inmoviliza las filas situadas encima de la celda seleccionada y las columnas a su izquierda, desde la primera fila y columna que puede ver. Las filas y columnas que desplazó fuera de la vista seguirán fuera de la vista hasta que desbloquee los paneles.
- **Inmovilizar fila superior** inmoviliza la primera fila que puede ver.
- **Inmovilizar primera columna** inmoviliza la primera columna que puede ver.
- **Movilizar paneles** los devuelve a su estado normal, y la hoja se queda donde está.

Las filas y columnas inmovilizadas se guardan con el libro, de modo que se vuelven a aplicar al abrirlo.

Un libro que se guardó con paneles inmovilizados se abre con ellos.

## Hojas

Las pestañas de la parte inferior son las hojas del libro. Haga clic en una pestaña para mostrar esa hoja. Cada hoja recuerda dónde estaba en ella. Las hojas ocultas no se muestran. Una línea de color bajo una pestaña es el color de la pestaña de la hoja. Para agregar, cambiar de nombre, mover, copiar, ocultar, eliminar o dar color a una hoja, consulte [Trabajar con hojas](sheets).

## Hojas que se leen de derecha a izquierda

Una hoja preparada para un idioma como el árabe o el hebreo se lee de derecha a izquierda, y JeffCrunch la muestra así: la columna A está en el borde derecho, los números de fila están a la derecha y la barra de desplazamiento horizontal va en sentido contrario. Las teclas de flecha siguen la dirección de la hoja, de modo que la flecha Izquierda va a la columna siguiente. El texto de una celda empieza por el lado desde el que se lee su idioma. El libro indica en qué sentido se lee una hoja, y usted puede cambiarlo: en la pestaña **Diseño de página**, haga clic en **Hoja de derecha a izquierda**. Haga clic de nuevo para desactivarlo. La opción se guarda con la hoja.

## Agrupar filas y columnas (esquema)

En la pestaña **Datos**, **Agrupar filas** añade un nivel más a las filas seleccionadas, y **Desagrupar filas** quita un nivel; **Agrupar columnas** y **Desagrupar columnas** hacen lo mismo con las columnas. Las filas pueden agruparse dentro de otros grupos, hasta siete niveles. Junto a las filas (y encima de las columnas) la hoja muestra un corchete por cada grupo y, en la línea que resume el grupo (la línea debajo de las filas, o a la derecha de las columnas, salvo que su archivo las coloque arriba o a la izquierda), un botón: **menos** oculta las líneas del grupo, y **más** las muestra de nuevo. Los botones numerados de la esquina muestran el esquema hasta un nivel: **1** muestra solo las líneas que no están en ningún grupo, **2** también las líneas del primer nivel, y así sucesivamente. Un grupo dentro de un grupo oculto sigue oculto cuando se muestra el grupo exterior, hasta que se haga clic en su propio botón. Cada una de estas acciones es un paso de Deshacer, y las líneas ocultas y los grupos se guardan con el libro. Al desagrupar un grupo oculto, se muestran sus líneas.

**Vista > Símbolos de esquema** activa y desactiva los corchetes y los botones.

## Mostrar fórmulas

En la pestaña **Fórmulas**, **Mostrar fórmulas** (Ctrl+`; en un Mac, la tecla Control y no la tecla Comando) muestra la fórmula de cada celda que la tiene, en lugar de su valor. Las columnas que contienen fórmulas se ensanchan, hasta un límite, para que las fórmulas se vean completas, y vuelven a su anchura al desactivar la opción; las anchuras de su archivo no cambian.
