# Bienvenido a JeffCrunch
<!-- keywords: bienvenida, primeros pasos, introducción, principiante, inicio rápido, descripción general, qué es JeffCrunch, conceptos básicos, beta, hoja de cálculo, libro, hoja, celdas, qué funciona, todavía no, limitaciones -->

JeffCrunch es un programa para hojas de cálculo. Es gratuito y de código abierto. No tiene ninguna cuota, ni suscripción, ni necesita crear una cuenta. Es una de las aplicaciones de JeffOffice y funciona en Windows, macOS y Linux.

El fichero con el que trabaja se llama **libro**. Un libro contiene una o varias **hojas**, y cada hoja es una cuadrícula de **celdas**. JeffCrunch guarda los libros como ficheros .xlsx. También abre libros .xlsx y .xlsm creados por otros programas, ficheros .xls antiguos, hojas de cálculo OpenDocument y ficheros de texto con valores. Lo que todavía no puede editar, lo conserva.

## JeffCrunch es una beta

Esta versión de JeffCrunch es una **beta**. Está lista para trabajar en serio, pero todavía no hace todo. Guarde una copia de cualquier libro importante.

**Lo que funciona**

- Abrir libros: .xlsx y .xlsm, .xls antiguos, hojas de cálculo OpenDocument (.ods y .fods) y ficheros de texto con valores (.csv, .tsv y .txt). Guardar en .xlsx, y copias en .xls o .ods. Consulte [Abrir y guardar libros](open-save).
- Abrir libros protegidos con contraseña (.xlsx, .xls y .ods), y proteger un libro .xlsx con una contraseña. Consulte [Abrir libros protegidos con contraseña](open-password).
- Escribir números, texto y fórmulas en las celdas, con deshacer y rehacer. Las fórmulas se calculan mientras escribe, con más de 200 funciones. Puede hacer clic o usar las flechas para seleccionar celdas mientras escribe una fórmula, y el cuadro de diálogo **Argumentos de función** completa los argumentos de una función con un resultado en vivo. Consulte [Escribir en las celdas](edit-cells), [Cómo se muestran y calculan las fórmulas](formulas) y [Introducir una función con el cuadro de diálogo Argumentos de función](function-arguments).
- Dar formato a las celdas: fuentes, colores, bordes, formatos de número, estilos de celda y celdas combinadas. Consulte [Dar formato a las celdas](format-cells).
- Cortar, copiar y pegar, rellenar celdas, sumar, buscar y reemplazar, ordenar, filtrar, tablas y tablas dinámicas. Consulte [Tablas dinámicas](pivot-tables).
- Filas, columnas y hojas: insertar, eliminar, cambiar el tamaño, ocultar y mover.
- Gráficos, imágenes y formas, y notas y vínculos.
- Proteger una hoja o el libro.
- Configurar las páginas, verlas antes de imprimir, imprimir y crear un PDF. Consulte [Imprimir y crear un PDF](printing).

**Lo que todavía no funciona**

- No puede crear, cambiar ni eliminar reglas de formato condicional ni reglas de validación de datos. JeffCrunch muestra las que tiene un libro y las conserva.
- No puede cambiar qué es un estilo de celda, ni crear uno nuevo.
- Una función que JeffCrunch todavía no tiene deja una fórmula nueva sin valor; una fórmula que venía con el libro conserva el resultado que guardó el fichero.
- Las macros se conservan y se vuelven a guardar, pero nunca se ejecutan.
- Las tablas dinámicas tienen algunas limitaciones, que [Tablas dinámicas](pivot-tables) enumera.
- Un libro abierto desde .xls, .ods o un fichero de texto se guarda como un nuevo fichero .xlsx, a menos que elija una copia en .xls o .ods. Una copia no puede contener todo, y JeffCrunch le indica qué dejará fuera antes de guardar.
- Una contraseña puede proteger un libro .xlsx y una copia en .xls, .ods u .ots, pero no una copia en texto.
- Se puede calcular una tabla de datos (tabla de hipótesis), pero no crearla.

## Haga estas cinco cosas primero

1. **Abra un libro.** Pulse **{key:Open}** o haga clic en **Archivo** y después en **Abrir**. Las primeras filas aparecen de inmediato, incluso en un libro muy grande, mientras JeffCrunch lee el resto.
2. **Explore el libro.** Haga clic en una celda para seleccionarla. Su dirección aparece en el **Cuadro de nombres**, en la parte superior izquierda, y su contenido, en la **barra de fórmulas** junto a él. Use las teclas de flecha, la rueda del ratón y las barras de desplazamiento para moverse. Consulte [Seleccionar celdas y moverse](select-and-move).
3. **Cambie una celda.** Seleccione una celda y escriba. Pulse **Intro** para conservar lo que escribió. Consulte [Escribir en las celdas](edit-cells).
4. **Dé formato a las celdas.** Seleccione celdas y use la pestaña **Inicio** para cambiar la fuente, los colores, los bordes y el formato de número. Consulte [Dar formato a las celdas](format-cells).
5. **Cambie la vista.** Use la pestaña **Vista** para ampliar o reducir, activar o desactivar las líneas de cuadrícula y los encabezados, y inmovilizar las filas superiores o las primeras columnas. Consulte [Cambiar la vista](view).

## Lo que ve

Un libro se muestra tal como se guardó: el ancho de las columnas y el alto de las filas, las fuentes, colores, rellenos y bordes, las celdas combinadas, los paneles inmovilizados y los formatos de número que convierten un número guardado en una fecha, un porcentaje o un precio. El texto demasiado largo para su celda se extiende a las celdas vacías contiguas. Un número demasiado ancho para su celda se muestra como una serie de signos #; amplíe la columna o seleccione la celda para leer el número en la barra de fórmulas. Los números se escriben según el idioma de su equipo (consulte [Formatos de número e idioma](number-formats)), y los formatos condicionales de un libro, como las escalas de color, las barras de datos y los iconos, se dibujan (consulte [Formatos condicionales](conditional-formats)).

Guarde su trabajo con **{key:Save}**, que escribe los cambios en el libro que abrió, o con **Guardar como**, que los escribe en un fichero nuevo. Consulte [Abrir y guardar libros](open-save).
