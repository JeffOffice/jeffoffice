# Nombres para celdas y valores
<!-- keywords: nombre, nombres, nombre definido, nuevo nombre, rango con nombre, ámbito, comentario, nombre desde celdas, insertar nombre, aplicar nombres, Ctrl+F3, se refiere a, ordenar nombres, nombre de error, advertencia, definir nombre -->

Un **nombre** es una palabra que representa una celda, un rango de celdas, un número o una fórmula, de modo que =Precio*Cantidad puede escribirse en lugar de =B2*C2. Un nombre pertenece a todo el libro o a una sola hoja. Un nombre de una hoja oculta un nombre del libro que se escribe igual, en esa hoja.

## Las reglas de un nombre

- Empieza por una letra, un guion bajo o una barra invertida, y después solo contiene letras, dígitos, guiones bajos y puntos. No tiene espacios.
- Tiene como máximo 255 caracteres.
- No es la dirección de una celda, como **A1** o **R2C3**.
- Ningún otro nombre de la misma hoja (o del libro) se escribe igual, con mayúsculas o sin ellas.

Mientras escribe, el cuadro de diálogo le indica qué es incorrecto, y el botón **Aceptar** no se activa.

## Crear un nombre

1. Seleccione las celdas y haga clic en **Nuevo nombre** en la pestaña **Fórmulas**.
2. Escriba el nombre, elija dónde funciona y diga qué representa. Las celdas que ha seleccionado se rellenan automáticamente.

Hay un modo más rápido para un nombre de todo el libro. Seleccione las celdas, haga clic en el **Cuadro de nombres** (a la izquierda de la barra de fórmulas), escriba el nombre y pulse **Entrar**. Si todavía no existe ningún nombre así, se crea para las celdas seleccionadas. Si ya existe, **Entrar** lo lleva a sus celdas.

**Nombre desde celdas** crea muchos nombres a la vez a partir de las etiquetas de un rango: si los encabezados están en la fila superior y los números debajo, cada encabezado da nombre a las celdas que hay bajo él. Elija dónde están las etiquetas (fila superior, columna izquierda, fila inferior o columna derecha). Una etiqueta que tenga espacios o signos que un nombre no puede contener recibe guiones bajos en su lugar. Se omite una etiqueta que está vacía, se repite o es un nombre que el libro ya tiene, y la barra de estado indica cuántas se han omitido.

## Ver todos los nombres

Haga clic en **Nombres** en la pestaña **Fórmulas**, o pulse **Ctrl+F3**. La lista muestra cada nombre, el valor que tiene ahora, a qué se refiere, dónde funciona y su comentario. Escriba en el filtro para mostrar solo los nombres que coincidan. **Nuevo**, **Cambiar** y **Eliminar** crean un nombre, lo modifican o lo quitan. Haga clic en un encabezado para ordenar la lista por él, y vuelva a hacer clic para invertir el orden. Un nombre que apunta a un error, como un lugar que se eliminó, tiene a su lado un pequeño signo de advertencia. Un libro con más de 20 000 nombres muestra solo los primeros 20 000.

Si cambia el nombre de un nombre, se cambia en todas las fórmulas que lo usan, en todas las hojas, en un solo paso. Si quita un nombre, las fórmulas que lo usaban muestran **#¿NOMBRE?**. Cada cambio se puede deshacer.

## Poner un nombre en una fórmula

Mientras escribe una fórmula, los nombres que coinciden aparecen en la lista bajo la celda. **Insertar nombre** en la pestaña **Fórmulas** muestra la lista de nombres para elegir.

## Poner nombres en fórmulas ya escritas

Supongamos que creó el nombre Tasa para $A$1 cuando muchas fórmulas ya usaban $A$1. Haga clic en **Aplicar nombres** en la pestaña **Fórmulas** para escribir Tasa en su lugar. Seleccione antes las celdas que desea revisar, o seleccione una sola celda para revisar toda la hoja. Elija los nombres que desea usar. Solo se cambia una referencia cuando coincide exactamente con las celdas del nombre, con o sin signos de dólar: A1:B2 no se cambia por un nombre de A1. Solo se ofrecen nombres de un rango de celdas de una hoja, y solo nombres de todo el libro. El cambio es un solo paso, así que **Deshacer** lo revierte.
