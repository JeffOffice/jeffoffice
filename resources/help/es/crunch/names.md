# Nombres para celdas y valores
<!-- keywords: nombre, nombres, nombre definido, nuevo nombre, rango con nombre, ámbito, comentario, nombre desde celdas, insertar nombre, Ctrl+F3, se refiere a -->

Un **nombre** es una palabra que representa una celda, un rango de celdas, un número o una fórmula, de modo que =Precio*Cantidad puede escribirse en lugar de =B2*C2. Un nombre pertenece a todo el libro o a una sola hoja. Un nombre de una hoja oculta un nombre del libro que se escribe igual, en esa hoja.

## Las reglas para un nombre

- Empieza por una letra, un guion bajo o una barra invertida, y después solo tiene letras, dígitos, guiones bajos y puntos. No tiene espacios.
- Tiene como máximo 255 caracteres.
- No es la dirección de una celda, como **A1** o **R2C3**.
- Ningún otro nombre de la misma hoja (o del libro) se escribe igual, con mayúsculas o sin ellas.

Mientras escribe, el cuadro de diálogo indica qué está mal, y el botón **Aceptar** espera.

## Crear un nombre

1. Seleccione las celdas y haga clic en **Nuevo nombre** en la pestaña **Fórmulas**.
2. Escriba el nombre, elija dónde funciona y diga a qué hace referencia. Las celdas seleccionadas se rellenan por usted.

Hay una forma más rápida para un nombre de todo el libro. Seleccione las celdas, haga clic en el **Cuadro de nombres** (a la izquierda de la barra de fórmulas), escriba el nombre y presione **Entrar**. Si todavía no existe ningún nombre así, se crea para las celdas seleccionadas. Si existe, **Entrar** lleva a sus celdas.

**Crear desde la selección** crea muchos nombres a la vez a partir de las etiquetas que rodean un rango: con los encabezados en la fila superior y los números debajo, cada encabezado da nombre a las celdas que están bajo él. Elija dónde están las etiquetas (fila superior, columna izquierda, fila inferior o columna derecha). Una etiqueta que tiene espacios o marcas que un nombre no puede tener se escribe con guiones bajos en su lugar. Una etiqueta que está vacía, se repite o es un nombre que el libro ya tiene se omite, y la barra de estado indica cuántas.

## Ver todos los nombres

Haga clic en **Administrador de nombres** en la pestaña **Fórmulas**, o presione **Ctrl+F3**. La lista muestra cada nombre, el valor que tiene ahora, a qué hace referencia, dónde funciona y su comentario. Escriba en el filtro para mostrar solo los nombres que coinciden. **Nuevo**, **Modificar** y **Eliminar** crean un nombre, lo cambian o lo quitan. Un libro con más de 20.000 nombres muestra los primeros 20.000.

Al cambiar el nombre de un nombre, se cambia en todas las fórmulas que lo usan, en todas las hojas, en un solo paso. Si quita un nombre, las fórmulas que lo usaban muestran **#NOMBRE?**. Cada cambio puede deshacerse.

## Poner un nombre en una fórmula

Mientras escribe una fórmula, los nombres que coinciden aparecen en la lista debajo de la celda. **Pegar nombre** en la pestaña **Fórmulas** muestra los nombres para elegir.
