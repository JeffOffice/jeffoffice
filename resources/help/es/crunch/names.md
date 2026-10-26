# Nombres de celdas y valores
<!-- keywords: nombre, nombres, nombre definido, nuevo nombre, rango con nombre, rango con nombre definido, ámbito, comentario, nombre desde celdas, insertar nombre, Ctrl+F3, hace referencia a, administrador de nombres -->

Un **nombre** es una palabra que representa una celda, un rango de celdas, un número o una fórmula, de modo que puede escribir =Precio*Cantidad en lugar de =B2*C2. Un nombre pertenece a todo el libro o a una sola hoja. En una hoja, un nombre de esa hoja oculta un nombre del libro que se escriba igual.

## Reglas para los nombres

- Empieza por una letra, un guion bajo o una barra diagonal inversa, y después solo contiene letras, dígitos, guiones bajos y puntos. No contiene espacios.
- Tiene como máximo 255 caracteres.
- No es la dirección de una celda, como **A1** o **R2C3**.
- Ningún otro nombre de la misma hoja (o del libro) se escribe igual, con mayúsculas o sin ellas.

Mientras escribe, el cuadro de diálogo indica qué está mal y el botón **Aceptar** no se activa hasta que se corrija.

## Crear un nombre

1. Seleccione las celdas y haga clic en **Nuevo nombre** en la pestaña **Fórmulas**.
2. Escriba el nombre, elija su ámbito e indique a qué hace referencia. Las celdas seleccionadas ya aparecen rellenadas.

**Nombre desde celdas** crea muchos nombres a la vez a partir de los rótulos que rodean un rango: si los encabezados están en la fila superior y los números debajo, cada encabezado da nombre a las celdas que tiene debajo. Elija dónde están los rótulos (fila superior, columna izquierda, fila inferior o columna derecha). Los espacios y los signos que un nombre no puede contener se sustituyen por guiones bajos. Los rótulos vacíos, repetidos o que coinciden con un nombre que ya existe en el libro se omiten, y la barra de estado indica cuántos.

## Ver todos los nombres

Haga clic en **Nombres** en la pestaña **Fórmulas** o presione **Ctrl+F3**. La lista muestra cada nombre, su valor actual, a qué hace referencia, su ámbito y su comentario. Escriba en el filtro para mostrar solo los nombres que coincidan. **Nuevo**, **Cambiar** y **Eliminar** crean, cambian y eliminan un nombre. Si un libro tiene más de 20.000 nombres, se muestran los primeros 20.000.

Al cambiar el nombre de un nombre, se cambia en todas las fórmulas que lo usan, en todas las hojas, en un solo paso. Al eliminar un nombre, las fórmulas que lo usaban muestran **#NAME?**. Todos los cambios se pueden deshacer.

## Usar un nombre en una fórmula

Mientras escribe una fórmula, los nombres que coinciden aparecen en la lista debajo de la celda. **Insertar nombre**, en la pestaña **Fórmulas**, muestra la lista de nombres entre los que elegir.
