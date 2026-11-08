# Estilos de celda

<!-- keywords: estilo de celda, estilos, estilo con nombre, Bueno, Malo, Neutral, Encabezado, Título, Total, Énfasis, Entrada, Salida, Cálculo, Celda de comprobación, Texto explicativo, Nota, Texto de advertencia, Celda vinculada, Millares, Moneda, Porcentaje, Normal, nuevo estilo, nombre, eliminar estilo, quitar estilo -->

Un **estilo de celda** es un conjunto con nombre de aspectos: una fuente, un relleno, un borde, un formato de número y la forma en que se alinea el texto. Si asigna a una celda el estilo «Encabezado 1», adquiere todos ellos a la vez. Los estilos mantienen coherente un libro, porque todas las celdas con el mismo estilo tienen el mismo aspecto.

## Aplicar un estilo

1. Seleccione las celdas.
2. Haga clic en **Inicio > Estilos de celda** y haga clic en un estilo. El menú muestra una pequeña muestra de cada uno.

El menú tiene estos grupos:

- **Buenos, malos y neutrales**: Normal, Bueno, Malo y Neutral.
- **Datos y modelo**: Cálculo, Celda de comprobación, Texto explicativo, Entrada, Celda vinculada, Nota, Salida y Texto de advertencia.
- **Títulos y encabezados**: Título, Encabezado 1 a 4, y Total.
- **Estilos de celda con temas**: cuatro tintes de cada uno de los seis colores de énfasis del tema del libro.
- **Formato de número**: Millares, Millares [0], Moneda, Moneda [0] y Porcentaje.
- **Personalizados**: estilos que el libro ya tiene con otros nombres.

Un estilo que el libro ya tiene se usa tal cual, con el aspecto que tiene allí. Un estilo que el libro no tiene se crea la primera vez que lo usa, con la primera fuente del propio libro, y a partir de entonces forma parte del fichero junto con los demás.

## Crear un estilo propio

1. Dé formato a una celda como desee: una fuente, un relleno, un borde, un formato de número, etc.
2. Deje el cursor en esa celda. Seleccione las celdas que también deben recibir el estilo.
3. Haga clic en **Inicio > Estilos de celda > Nuevo estilo de celda**, escriba un nombre y haga clic en **Aceptar**.

El estilo conserva el aspecto de la celda donde está el cursor, y a las celdas seleccionadas se les aplica. Ahora aparece en el grupo **Personalizados** del menú y se guarda en el libro. Cada estilo necesita su propio nombre. No se admite un nombre que ya esté en uso (no importa si está en mayúsculas o minúsculas), ni un nombre vacío.

## Eliminar un estilo

Haga clic en **Inicio > Estilos de celda > Eliminar estilo de celda** y elija el estilo. Todas las celdas que lo usan vuelven a Normal, el aspecto sencillo, y el estilo desaparece del menú. Es un paso que puede deshacer, y al deshacerlo se recuperan el estilo y el aspecto de las celdas. No se puede eliminar Normal. El fichero del libro conserva el nombre del estilo, marcado como oculto, para que tampoco lo muestren otros programas.

## Notas

- Aplicar un estilo es un paso que puede deshacer. Sustituye la fuente, el relleno, el borde, el formato de número, la alineación y la protección de la celda por los del estilo.
- Una celda recuerda su estilo. Si después da formato a la celda, por ejemplo poniéndola en negrita, el cambio afecta solo a esa celda, y la celda sigue la parte del estilo que no ha modificado.
- JeffCrunch todavía no tiene un comando para cambiar la definición de un estilo ni para importar estilos desde otro libro. Un libro que tenga estilos propios los muestra en **Personalizados** y en su grupo.
