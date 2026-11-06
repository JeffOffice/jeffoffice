# Notas y vínculos
<!-- keywords: diccionario de sinónimos, sinónimos, lista de palabras, nota, notas, comentario, comentarios, al pasar el puntero, marca roja, esquina, nueva nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas las notas, vínculo, vínculos, hipervínculo, dirección, página web, correo electrónico, lugar de este libro, Ctrl+K, Ctrl+clic, información en pantalla, abrir vínculo, quitar vínculo, conversación, estadísticas del libro, contar, celdas, fórmulas, gráficos, tablas dinámicas, accesibilidad, texto alternativo, celdas combinadas, archivos recientes -->

## Notas

Una **nota** es unas pocas palabras asociadas a una celda. Una celda que tiene una muestra una pequeña marca roja en su esquina superior derecha. Coloque el puntero sobre la celda y la nota aparecerá junto a ella; al alejar el puntero, desaparece.

Para poner una nota en una celda, selecciónela y use **Revisar > Nueva nota** (o presione **Mayús+F2**). Escriba las palabras y haga clic en **Aceptar**. En una celda que ya tiene una nota, el mismo comando aparece como **Editar nota**. Si borra las palabras, la nota desaparece.

- **Eliminar nota** quita las notas de las celdas seleccionadas.
- **Mostrar u ocultar nota** mantiene visible en la hoja la nota de las celdas seleccionadas, en un cuadro junto a cada celda, o la vuelve a ocultar.
- **Mostrar todas las notas** hace lo mismo con todas las notas de la hoja; el mismo botón pasa entonces a llamarse **Ocultar todas las notas**.
- **Nota anterior** y **Nota siguiente** pasan de una celda con nota a la siguiente.

Cada una de estas acciones se puede deshacer en un solo paso. Una nota se desplaza con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan las celdas. **Borrar todo** quita las notas de las celdas que borra.

Un libro puede contener una conversación de comentarios, con respuestas. JeffCrunch muestra toda la conversación como una nota y la conserva al guardar, pero no la modifica: la nota es de solo lectura.

Las notas se pueden guardar en un libro .xlsx. Un libro abierto desde un tipo de archivo más antiguo, o desde un archivo de texto, conserva sus notas solo después de guardarlo como libro .xlsx.

## Vínculos

Un **vínculo** le lleva a otro lugar al seguirlo: a una página web, a un archivo, a una dirección de correo electrónico o a un lugar de este libro (una celda, varias celdas o un nombre).

Para poner un vínculo en una celda, selecciónela y use **Insertar > Vínculo** (o presione **Ctrl+K**). Elija a dónde debe llevar, escriba la dirección o el lugar (o elija uno de los archivos abiertos recientemente en **O un archivo reciente**) y, si lo desea, el texto que debe mostrar la celda y una **información en pantalla**. En una celda que ya tiene un vínculo, el mismo comando aparece como **Editar vínculo**. **Quitar vínculo** elimina los vínculos de las celdas seleccionadas.

Coloque el puntero sobre una celda con vínculo para ver adónde lleva. Para seguirlo, mantenga presionada **Ctrl** y haga clic en la celda, o seleccione la celda y use **Abrir vínculo** del menú contextual (clic con el botón derecho). Un clic normal solo selecciona la celda.

JeffCrunch nunca abre nada por sí mismo. Una página web o una dirección de correo electrónico se abre en su programa habitual cuando sigue su vínculo. Un vínculo a un archivo, o a otro tipo de dirección, le pide confirmación antes. Un vínculo que ejecutaría un script nunca se abre.

Un vínculo se desplaza con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan las celdas, y se guarda con el libro.

## Estadísticas del libro

**Estadísticas del libro** (pestaña **Revisar**) cuenta lo que contiene el libro: cuántas **Hojas** tiene, cuántas **Celdas con datos** (celdas que contienen un número, un texto o una fórmula), cuántas **Fórmulas**, cuántos **Gráficos** y cuántas **Tablas dinámicas**. Las hojas que eliminó no se cuentan. Los mismos datos aparecen en la página **Información** de la ficha Archivo.

## Diccionario de sinónimos

**Sinónimos** (pestaña **Revisar**, o **Mayús+F7**) busca palabras de significado casi igual a una palabra de una celda. Consulta la primera palabra del texto de la celda en la que se encuentra. Escriba otra palabra en **Buscar** y presione **Entrar** para consultar esa en su lugar. Elija una palabra de la lista y haga clic en **Reemplazar**, o haga doble clic en ella. La palabra de la celda se reemplaza, y una inicial en mayúscula se mantiene en mayúscula. **Buscar** consulta la palabra que eligió, para seguir una cadena de significados. La lista de palabras funciona en inglés.

## Comprobar accesibilidad

**Comprobar accesibilidad** (pestaña **Revisar**) abre una ventana pequeña que enumera lo que en el libro resulta difícil de usar para algunos lectores, o para un lector de pantalla (un programa que lee en voz alta lo que aparece en pantalla). Cada línea empieza por **Error**, **Advertencia** o **Sugerencia**. Haga clic en una línea para ir a la celda.

Busca:

- Un gráfico o una imagen sin **texto alternativo** (las palabras que describen lo que muestra). Esto es un error.
- Una tabla sin fila de encabezado, de modo que sus columnas no tienen nombre. Esto es un error.
- **Celdas combinadas**, que son difíciles de recorrer con el teclado. Esto es una advertencia.
- Texto que se lee mal sobre el color de fondo, como texto gris claro sobre blanco. JeffCrunch mide cuánto destaca el texto sobre el relleno. El texto pequeño necesita una relación de al menos 4,5 a 1, y el texto grande (18 puntos, o 14 puntos en negrita) necesita 3 a 1. Indica la primera celda de cada combinación de fuente y relleno. Esto es una advertencia.
- Una hoja que aún tiene el nombre que se le asignó, como Hoja1. Esto es una sugerencia.

Pulse **Volver a comprobar** después de hacer cambios.
