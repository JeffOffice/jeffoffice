# Bienvenido a JeffCrunch
<!-- keywords: bienvenida, primeros pasos, introducción, principiante, empezar, inicio rápido, información general, qué es JeffCrunch, conceptos básicos, beta, hoja de cálculo, libro, hoja, celdas, qué funciona, todavía no, limitaciones -->

JeffCrunch es un programa de hojas de cálculo. Es gratuito y de código abierto. No hay que pagar nada, no requiere suscripción y no hay que crear ninguna cuenta. Forma parte de las aplicaciones de JeffOffice y funciona en Windows, macOS y Linux.

El fichero con el que trabaja se llama **libro**. Un libro contiene una o varias **hojas**, y cada hoja es una cuadrícula de **celdas**. JeffCrunch guarda los libros como ficheros .xlsx. También abre libros .xlsx y .xlsm creados con otros programas, así como ficheros .xls antiguos, hojas de cálculo OpenDocument y ficheros de texto de valores. Conserva lo que todavía no puede modificar.

## JeffCrunch es una versión beta

Esta versión de JeffCrunch es una **beta**. Está lista para trabajar de verdad, pero todavía no lo hace todo. Guarde una copia de cualquier libro que sea importante.

**Qué funciona**

- Abrir libros: .xlsx y .xlsm, .xls antiguos, hojas de cálculo OpenDocument (.ods y .fods) y ficheros de texto de valores (.csv, .tsv y .txt). Guardar en .xlsx, y copias en .xls o .ods. Consulte [Abrir y guardar libros](open-save).
- Abrir libros protegidos con contraseña (.xlsx, .xls y .ods) y proteger un libro .xlsx con contraseña. Consulte [Abrir libros protegidos con contraseña](open-password).
- Escribir números, texto y fórmulas en las celdas, con las opciones de deshacer y rehacer. Las fórmulas se calculan mientras escribe, con más de 200 funciones. Mientras escribe una fórmula, puede hacer clic en las celdas o desplazarse a ellas con las teclas de dirección, y el cuadro de diálogo **Argumentos de función** rellena los argumentos de una función y muestra el resultado al momento. Consulte [Escribir en las celdas](edit-cells), [Cómo se muestran y calculan las fórmulas](formulas) e [Introducir una función con el cuadro de diálogo Argumentos de función](function-arguments).
- Aplicar formato a las celdas: fuentes, colores, bordes, formatos de número, estilos de celda y celdas combinadas. Consulte [Aplicar formato a las celdas](format-cells).
- Cortar, copiar y pegar, rellenar celdas, sumarlas, buscar y reemplazar, ordenar, filtrar, tablas y tablas dinámicas. Consulte [Tablas dinámicas](pivot-tables).
- Filas, columnas y hojas: insertarlas, eliminarlas, cambiar su tamaño, ocultarlas y moverlas.
- Gráficos, imágenes y formas, así como notas y vínculos.
- Proteger una hoja o el libro.
- Configurar las páginas, verlas antes de imprimirlas, imprimir y crear un PDF. Consulte [Imprimir y crear un PDF](printing).

**Qué no funciona todavía**

- No se pueden crear, modificar ni eliminar reglas de formato condicional ni reglas de validación de datos. JeffCrunch muestra las que tiene un libro y las conserva.
- No se puede modificar un estilo de celda ni crear uno nuevo.
- Una fórmula nueva que use una función que JeffCrunch todavía no tiene se queda sin valor; una fórmula que ya venía en el libro conserva el resultado almacenado en el fichero.
- Las macros se conservan y se vuelven a guardar, pero nunca se ejecutan.
- Las tablas dinámicas tienen algunas limitaciones, que se enumeran en [Tablas dinámicas](pivot-tables).
- Un libro abierto desde un fichero .xls, .ods o de texto se guarda como un fichero .xlsx nuevo, a menos que elija guardar una copia en .xls o .ods. Una copia no puede contenerlo todo, y JeffCrunch indica qué omitirá antes de guardar.
- Una contraseña puede proteger un libro .xlsx, pero no una copia en .xls o .ods.
- Una tabla de datos (tabla de hipótesis) se puede calcular, pero no crear.

## Haga primero estas cinco cosas

1. **Abra un libro.** Pulse **{key:Open}**, o haga clic en **Archivo** y después en **Abrir**. Las primeras filas aparecen de inmediato, incluso en un libro muy grande, mientras JeffCrunch lee el resto.
2. **Explore.** Haga clic en una celda para seleccionarla. Su dirección aparece en el **Cuadro de nombres**, arriba a la izquierda, y su contenido aparece en la **barra de fórmulas**, a su lado. Use las teclas de dirección, la rueda del ratón y las barras de desplazamiento para moverse. Consulte [Seleccionar celdas y desplazarse](select-and-move).
3. **Cambie una celda.** Seleccione una celda y escriba. Pulse **Entrar** para conservar lo que ha escrito. Consulte [Escribir en las celdas](edit-cells).
4. **Aplique formato a las celdas.** Seleccione celdas y use la pestaña **Inicio** para cambiar la fuente, los colores, los bordes y el formato de número. Consulte [Aplicar formato a las celdas](format-cells).
5. **Cambie la vista.** Use la pestaña **Vista** para cambiar el zoom, activar o desactivar las líneas de cuadrícula y los encabezados, e inmovilizar las filas superiores o las primeras columnas. Consulte [Cambiar la vista](view).

## Lo que ve

Un libro se ve tal como se guardó: el ancho de las columnas y el alto de las filas, las fuentes, los colores, los rellenos y los bordes, las celdas combinadas, los paneles inmovilizados y los formatos de número que convierten un número almacenado en una fecha, un porcentaje o un precio. El texto demasiado largo para su celda se extiende a las celdas vacías contiguas. Un número demasiado ancho para su celda se muestra como una fila de signos #; amplíe la columna, o seleccione la celda para leer el número en la barra de fórmulas. Los números se escriben como los escribe el idioma de su equipo (consulte [Formatos de número y su idioma](number-formats)), y se dibujan los formatos condicionales del libro, como las escalas de color, las barras de datos y los iconos (consulte [Formatos condicionales](conditional-formats)).

Guarde su trabajo con **{key:Save}**, que escribe los cambios en el libro que abrió, o con **Guardar como**, que los escribe en un fichero nuevo. Consulte [Abrir y guardar libros](open-save).
