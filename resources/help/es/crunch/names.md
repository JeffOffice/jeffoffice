# Nombres para celdas y valores
<!-- keywords: nombre, nombres, nombre definido, nuevo nombre, rango con nombre, ámbito, comentario, nombre desde celdas, insertar nombre, Ctrl+F3, se refiere a, ordenar nombres, nombre de error, advertencia -->

Un **nombre** es una palabra que representa una celda, un rango de celdas, un número o una fórmula, de modo que =Precio*Cantidad puede escribirse en lugar de =B2*C2. Un nombre pertenece a todo el libro o a una sola hoja. Un nombre de una hoja oculta a un nombre del libro que se escribe igual, en esa hoja.

## Las reglas de un nombre

- Empieza por una letra, un guion bajo o una barra invertida, y después solo admite letras, cifras, guiones bajos y puntos. No contiene espacios.
- Tiene como máximo 255 caracteres.
- No es la dirección de una celda, como **A1** o **F2C3**.
- Ningún otro nombre de la misma hoja (o del libro) se escribe igual, en mayúsculas o no.

Mientras escribe, el cuadro de diálogo indica qué está mal, y el botón **Aceptar** espera.

## Crear un nombre

1. Seleccione las celdas y haga clic en **Nuevo nombre** en la ficha **Fórmulas**.
2. Escriba el nombre, elija dónde funciona y diga a qué hace referencia. Las celdas que seleccionó se rellenan por usted.

Hay una forma más rápida para un nombre de todo el libro. Seleccione las celdas, haga clic en el **Cuadro de nombres** (a la izquierda de la barra de fórmulas), escriba el nombre y pulse **Entrar**. Si todavía no existe ningún nombre escrito así, se crea para las celdas seleccionadas. Si existe, **Entrar** lleva a sus celdas.

**Nombre desde celdas** crea muchos nombres a la vez a partir de las etiquetas de un bloque: con los encabezados en la fila superior y los números debajo, cada encabezado nombra las celdas que hay bajo él. Elija dónde están las etiquetas (fila superior, columna izquierda, fila inferior o columna derecha). Una etiqueta que tenga espacios o signos que un nombre no admite recibe guiones bajos en su lugar. Una etiqueta que está vacía, se repite o es un nombre que el libro ya tiene se omite, y la barra de estado indica cuántas.

## Ver todos los nombres

Haga clic en **Nombres** en la ficha **Fórmulas**, o pulse **Ctrl+F3**. La lista muestra cada nombre, el valor que tiene ahora, a qué hace referencia, dónde funciona y su comentario. Escriba en el filtro para mostrar solo los nombres que coinciden. **Nuevo**, **Cambiar** y **Eliminar** crean un nombre, lo modifican o lo quitan. Haga clic en un encabezado para ordenar la lista por él, y vuelva a hacer clic para invertir el orden. Un nombre que apunta a un error, como un lugar que se eliminó, lleva junto a él una pequeña señal de advertencia. Un libro con más de 20.000 nombres muestra los primeros 20.000.

Si cambia el nombre de un nombre, el cambio se aplica en todas las fórmulas que lo usan, en todas las hojas, en un solo paso. Si elimina un nombre, las fórmulas que lo usaban muestran **#¿NOMBRE?**. Cada cambio puede deshacerse.

## Poner un nombre en una fórmula

Mientras escribe una fórmula, los nombres que coinciden aparecen en la lista debajo de la celda. **Insertar nombre** en la ficha **Fórmulas** muestra la lista de nombres para elegir.
