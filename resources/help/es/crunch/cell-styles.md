# Estilos de celda

<!-- keywords: modificar estilo de celda, cambiar un estilo, combinar estilos, importar estilos, estilo de celda, estilos, estilo con nombre, Bueno, Malo, Neutral, Encabezado, Título, Total, Énfasis, Entrada, Salida, Cálculo, Celda de comprobación, Texto explicativo, Nota, Texto de advertencia, Celda vinculada, Millares, Moneda, Porcentaje, Normal, nuevo estilo, nombre, eliminar estilo, quitar estilo, estilos de celda, formato de celda, plantilla de estilo -->

Un **estilo de celda** es un conjunto con nombre de aspectos: una fuente, un relleno, un borde, un formato de número y la forma en que se coloca el texto. Si asigna a una celda el estilo «Encabezado 1», la celda toma todos ellos a la vez. Los estilos mantienen coherente un libro, porque todas las celdas con el mismo estilo tienen el mismo aspecto.

## Aplicar un estilo

1. Seleccione las celdas.
2. Haga clic en **Inicio > Estilos de celda** y haga clic en un estilo. El menú muestra una pequeña muestra de cada uno.

El menú tiene estos grupos:

- **Bueno, Malo y Neutral**: Normal, Bueno, Malo y Neutral.
- **Datos y modelo**: Cálculo, Celda de comprobación, Texto explicativo, Entrada, Celda vinculada, Nota, Salida y Texto de advertencia.
- **Títulos y encabezados**: Título, Encabezado 1 a 4, y Total.
- **Estilos de celda con tema**: cuatro tonos de cada uno de los seis colores de énfasis del tema del libro.
- **Formato de número**: Millares, Millares [0], Moneda, Moneda [0] y Porcentaje.
- **Personalizado**: estilos que el libro ya tiene con otros nombres.

Un estilo que el libro ya tiene se usa tal como es, con el aspecto que tiene allí. Un estilo que el libro no tiene se crea la primera vez que lo utiliza, a partir de la primera fuente del propio libro, y a partir de entonces forma parte del archivo junto con los demás.

## Crear un estilo propio

1. Dé formato a una celda como desee: una fuente, un relleno, un borde, un formato de número, etc.
2. Deje el cursor en esa celda. Seleccione las celdas que también deben recibir el estilo.
3. Haga clic en **Inicio > Estilos de celda > Nuevo estilo de celda**, escriba un nombre y haga clic en **Aceptar**.

El estilo conserva el aspecto de la celda donde está el cursor, y las celdas seleccionadas reciben ese estilo. Ahora aparece en el grupo **Personalizado** del menú y se guarda en el libro. Cada estilo necesita su propio nombre. No se admite un nombre que ya esté en uso (no importan las mayúsculas ni las minúsculas), ni un nombre vacío.

## Modificar un estilo

Haga clic en **Inicio > Estilos de celda > Modificar estilo de celda** y elija un estilo que tenga el libro (uno que aparezca en la galería está en el libro una vez que alguna celda lo ha usado). Se abre el cuadro de diálogo Formato de celdas, partiendo del aspecto propio del estilo. Cambie lo que desee y haga clic en **Aceptar**: el estilo adopta el cambio, y también lo hace cada celda que lo usa, en un único paso que puede deshacer. Solo cambia lo que una celda toma del estilo: una celda que tiene formato propio lo conserva, así que una celda con su propio relleno mantiene ese relleno cuando cambia el relleno del estilo. Las columnas y filas completas que tienen formato con el estilo también reciben el cambio. Una hoja protegida contra el formato de celdas que tenga una celda con el estilo rechaza el cambio, y no se modifica nada. No puede modificar Normal de este modo. El estilo se guarda en el libro con su nuevo aspecto, y una celda a la que se asigne el estilo más adelante lo tendrá.

## Combinar estilos de otro libro

Abra el libro que tiene los estilos y, en el libro que debe tenerlos, haga clic en **Inicio > Estilos de celda > Combinar estilos** y elija el libro. Se agrega cada estilo del otro libro que este no tenga. Cuando un estilo tiene el mismo nombre aquí (no importan las mayúsculas ni las minúsculas) y un aspecto distinto, se le pregunta una sola vez si desea reemplazarlos: **Reemplazar** les da el aspecto del otro libro, como lo hace Modificar (las celdas que los usan cambian solo en lo que toman del estilo), y **Conservar** deja los suyos. Un color que es un color del tema se importa como el color que muestra en el otro libro, salvo que ambos libros tengan el mismo tema. Los estilos que tienen el mismo aspecto se omiten. Es un único paso que puede deshacer, y Deshacer quita todos los estilos que añadió y devuelve todos los aspectos que cambió. Los estilos del otro libro solo se leen, y ese libro no se modifica. Se importan como máximo 1000 estilos a la vez (si hay más, se rechaza la operación y no se combina nada); si el otro libro está ocupado, se le indicará que lo intente de nuevo; Normal no cambia, y un estilo que eliminó aquí sigue eliminado.

## Eliminar un estilo

Haga clic en **Inicio > Estilos de celda > Eliminar estilo de celda** y elija el estilo. Cada celda que lo usa vuelve a Normal, el aspecto sencillo, y el estilo desaparece del menú. Es un único paso que puede deshacer, y al deshacer se recuperan el estilo y el aspecto de las celdas. No puede eliminar Normal. El archivo del libro conserva el nombre del estilo, marcado como oculto, para que tampoco lo muestren otros programas.

## Notas

- Aplicar un estilo es un paso que puede deshacer. Sustituye la fuente, el relleno, el borde, el formato de número, la alineación y la protección de la celda por los del estilo.
- Una celda recuerda su estilo. Si después da formato a la celda, por ejemplo poniéndola en negrita, solo cambia esa celda, y la celda sigue adoptando el resto del estilo.
- JeffCrunch todavía no tiene un comando para cambiar la definición de un estilo ni para incorporar estilos desde otro libro. Un libro que tiene estilos propios los muestra en **Personalizado** y en su grupo.
