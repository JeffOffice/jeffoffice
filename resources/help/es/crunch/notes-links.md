# Notas y vínculos
<!-- keywords: nota, notas, comentario, comentarios, al pasar el puntero, marca roja, esquina, nueva nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas las notas, vínculo, vínculos, hipervínculo, dirección, página web, correo electrónico, lugar de este libro, Ctrl+K, Ctrl+clic, información en pantalla, abrir vínculo, quitar vínculo, conversación, estadísticas del libro, contar, celdas, fórmulas, gráficos, tablas dinámicas, accesibilidad, texto alternativo, celdas combinadas -->

## Notas

Una **nota** es un breve texto asociado a una celda. Una celda que tiene una nota muestra una pequeña marca roja en su esquina superior derecha. Si apunta a la celda con el puntero, la nota aparece junto a ella; si aleja el puntero, desaparece.

Para poner una nota en una celda, selecciónela y use **Revisar > Nueva nota** (o presione **Mayús+F2**). Escriba el texto y haga clic en **Aceptar**. En una celda que ya tiene una nota, el mismo comando aparece como **Editar nota**. Si borra el texto, la nota se elimina.

- **Eliminar nota** quita las notas de las celdas seleccionadas.
- **Mostrar u ocultar nota** mantiene la nota de las celdas seleccionadas visible en la hoja todo el tiempo, en un cuadro junto a cada celda, o vuelve a ocultarla.
- **Mostrar todas las notas** hace lo mismo con todas las notas de la hoja; el mismo botón pasa entonces a llamarse **Ocultar todas las notas**.
- **Nota anterior** y **Nota siguiente** lo llevan de una celda con nota a la siguiente.

Cada una de estas acciones se puede deshacer por separado. Una nota se desplaza con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan celdas. **Borrar todo** quita las notas de las celdas que borra.

Un libro puede contener una conversación de comentarios, con respuestas. JeffCrunch muestra la conversación completa como una nota y la conserva al guardar, pero no la modifica: la nota es de solo lectura.

Las notas se pueden guardar en un libro .xlsx. Un libro abierto desde un tipo de archivo anterior o desde un archivo de texto conserva sus notas solo después de guardarlo como libro .xlsx.

## Vínculos

Un **vínculo** le lleva a otro lugar al seguirlo: a una página web, a un archivo, a una dirección de correo electrónico o a un lugar de este libro (una celda, un rango de celdas o un nombre).

Para poner un vínculo en una celda, selecciónela y use **Insertar > Vínculo** (o presione **Ctrl+K**). Elija a dónde debe llevar, escriba la dirección o el lugar y, si lo desea, el texto que debe mostrar la celda y una **información en pantalla**. En una celda que ya tiene un vínculo, el mismo comando aparece como **Editar vínculo**. **Quitar vínculo** elimina los vínculos de las celdas seleccionadas.

Apunte con el puntero a una celda con vínculo para ver adónde lleva. Para seguirlo, mantenga pulsada **Ctrl** y haga clic en la celda, o selecciónela y use **Abrir vínculo** del menú contextual (clic derecho). Un clic normal solo selecciona la celda.

JeffCrunch nunca abre nada por su cuenta. Una página web o una dirección de correo electrónico se abre en su programa habitual al seguir su vínculo. Un vínculo a un archivo, o a otro tipo de dirección, le pregunta antes. Un vínculo que ejecutaría un script nunca se abre.

Un vínculo se desplaza con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan celdas, y se guarda con el libro.

## Estadísticas del libro

**Estadísticas del libro** (pestaña **Revisar**) cuenta lo que contiene el libro: cuántas **Hojas** tiene, cuántas **Celdas con datos** (celdas que contienen un número, un texto o una fórmula), cuántas **Fórmulas**, cuántos **Gráficos** y cuántas **Tablas dinámicas**. Las hojas que eliminó no se cuentan. Los mismos datos aparecen en la página **Información** de la pestaña Archivo.

## Comprobar accesibilidad

**Comprobar accesibilidad** (pestaña **Revisar**) abre una pequeña ventana que enumera lo que en el libro resulta difícil de usar para algunos lectores, o para un lector de pantalla (un programa que lee en voz alta lo que aparece en pantalla). Cada línea empieza por **Error**, **Advertencia** o **Consejo**. Haga clic en una línea para ir a la celda.

Busca lo siguiente:

- Un gráfico o una imagen sin **texto alternativo** (las palabras que describen lo que muestra). Es un error.
- Una tabla sin fila de encabezado, por lo que sus columnas no tienen nombre. Es un error.
- **Celdas combinadas**, que son difíciles de recorrer con el teclado. Es una advertencia.
- Una hoja que aún conserva el nombre que se le asignó, como Hoja1. Es un consejo.

Pulse **Volver a comprobar** después de hacer cambios.
