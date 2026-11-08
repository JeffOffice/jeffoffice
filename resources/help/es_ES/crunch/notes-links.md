# Notas, comentarios y vínculos

<!-- keywords: comentarios, panel de comentarios, comentario nuevo, responder, resolver, reabrir, hilo, comentario anterior, comentario siguiente, mostrar comentarios, convertir notas en comentarios, convertir comentarios en notas, sinónimos, diccionario de sinónimos, lista de palabras, nota, notas, comentario, comentarios, al pasar el puntero, marca roja, esquina, nueva nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas las notas, vínculo, vínculos, hipervínculo, dirección, página web, correo electrónico, lugar de este libro, Ctrl+K, Ctrl+clic, información en pantalla, abrir vínculo, quitar vínculo, conversación, estadísticas del libro, contar, celdas, fórmulas, gráficos, tablas dinámicas, accesibilidad, texto alternativo, celdas combinadas, archivos recientes, ficheros recientes -->

## Notas

Una **nota** es unas pocas palabras asociadas a una celda. Una celda que tiene una nota muestra una pequeña marca roja en su esquina superior derecha. Coloque el puntero sobre la celda y la nota aparecerá junto a ella; al alejar el puntero, desaparece.

Para añadir una nota a una celda, selecciónela y use **Revisar > Nueva nota** (o pulse **Mayús+F2**). Escriba las palabras y haga clic en **Aceptar**. En una celda que ya tiene una nota, el mismo comando aparece como **Editar nota**. Si borra las palabras, la nota desaparece.

- **Eliminar nota** quita las notas de las celdas seleccionadas.
- **Mostrar u ocultar nota** mantiene la nota de las celdas seleccionadas visible en la hoja todo el tiempo, en un cuadro junto a cada celda, o la oculta de nuevo.
- **Mostrar todas las notas** hace lo mismo con todas las notas de la hoja; el mismo botón pasa entonces a llamarse **Ocultar todas las notas**.
- **Nota anterior** y **Nota siguiente** pasan de una celda con nota a la siguiente.

Cada una de estas acciones es un paso que puede deshacer. Una nota se desplaza con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan las celdas. **Borrar todo** quita las notas y los comentarios de las celdas que borra.

Una celda también puede contener una conversación de **comentarios**, con respuestas. Consulte la siguiente parte.

Las notas se pueden guardar en un libro .xlsx. Un libro abierto desde un tipo de fichero anterior o desde un fichero de texto conserva sus notas solo después de guardarlo como libro .xlsx.

## Comentarios

Un **comentario** es parecido a una nota, pero inicia un **hilo**: otras personas pueden responderle, y una de ellas puede marcar el hilo como **resuelto** cuando la pregunta está contestada. Cada comentario muestra quién lo escribió y cuándo. Una celda con un hilo tiene la misma marca roja que una celda con una nota. Coloque el puntero sobre la celda para leer el hilo completo, con cada respuesta sangrada debajo del comentario al que responde. Un hilo resuelto empieza con la palabra **Resuelto**.

En la ficha **Revisar**, en el grupo **Comentarios**:

- **Nuevo comentario** (**Ctrl+Mayús+F2**) inicia un hilo en la celda seleccionada. Escriba las palabras y haga clic en **Aceptar**. Funciona en una celda que no tiene nota ni comentarios.
- **Responder** añade sus palabras al hilo de la celda seleccionada.
- **Resolver hilo** marca el hilo como terminado. El mismo botón pasa entonces a llamarse **Reabrir hilo**, que vuelve a abrirlo.
- **Eliminar comentario** quita los hilos de las celdas seleccionadas.
- **Comentario anterior** y **Comentario siguiente** pasan de una celda con hilo a la siguiente.
- **Mostrar comentarios** abre el panel **Comentarios** junto a la hoja. Muestra todos los hilos de la hoja como una tarjeta: la celda, cada comentario con su autor y hora, y los botones **Responder**, **Resolver** o **Reabrir**, y **Eliminar**. **Ir a** lleva a la celda. El panel se actualiza con cada cambio, y también al deshacer.
- **Convertir notas en comentarios** transforma cada nota sencilla de la hoja en un hilo de un solo comentario. **Convertir comentarios en notas** devuelve cada hilo a una nota. Una nota no tiene respuestas, así que los comentarios de un hilo se convierten en una sola nota con todas sus palabras, y JeffCrunch le pide confirmación antes de hacerlo con un hilo que tenga respuestas.

Cada una de estas acciones es un paso que puede deshacer.

Los programas que no pueden mostrar hilos muestran en su lugar una nota sencilla. JeffCrunch conserva esa nota, con las palabras del hilo, cada vez que guarda. Los hilos, las respuestas y las personas que los escribieron se mantienen como estaban al guardar y volver a abrir el libro. Un hilo con más de 500 comentarios se muestra y se conserva, pero JeffCrunch no le añade nada ni lo modifica.

Los comentarios se guardan en un libro .xlsx. Un libro abierto desde un tipo de fichero anterior o desde un fichero de texto conserva sus comentarios solo después de guardarlo como libro .xlsx.

## Vínculos

Un **vínculo** le lleva a otro lugar al seguirlo: a una página web, a un fichero, a una dirección de correo electrónico o a un lugar de este libro (una celda, un rango de celdas o un nombre).

Para añadir un vínculo a una celda, selecciónela y use **Insertar > Vínculo** (o pulse **Ctrl+K**). Elija a dónde debe llevar, escriba la dirección o el lugar (o elija uno de los ficheros abiertos recientemente en **O un fichero reciente**) y, si lo desea, el texto que debe mostrar la celda y una **información en pantalla**. En una celda que ya tiene un vínculo, el mismo comando aparece como **Modificar vínculo**. **Quitar vínculo** elimina los vínculos de las celdas seleccionadas.

Coloque el puntero sobre una celda con un vínculo para ver adónde lleva. Para seguirlo, mantenga pulsada **Ctrl** y haga clic en la celda, o seleccione la celda y use **Abrir vínculo** en el menú contextual del botón derecho. Un clic normal solo selecciona la celda.

JeffCrunch nunca abre nada por sí solo. Una página web o una dirección de correo electrónico se abre en su programa habitual cuando sigue el vínculo. Un vínculo a un fichero, o a otro tipo de dirección, le pregunta antes. Un vínculo que ejecutaría un script nunca se abre.

Un vínculo se desplaza con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan las celdas, y se guarda con el libro.

## Estadísticas del libro

**Estadísticas del libro** (ficha **Revisar**) cuenta lo que contiene el libro: cuántas **Hojas** tiene, cuántas **Celdas con datos** (celdas que contienen un número, un texto o una fórmula), cuántas **Fórmulas**, cuántos **Gráficos** y cuántas **Tablas dinámicas**. Las hojas que eliminó no se cuentan. Los mismos datos aparecen en la página **Información** de la ficha Archivo.

## Sinónimos

**Sinónimos** (ficha **Revisar**, o **Mayús+F7**) busca palabras que significan casi lo mismo que una palabra de una celda. Busca la primera palabra del texto de la celda en la que se encuentra. Escriba otra palabra en **Buscar** y pulse **Entrar** para buscar esa en su lugar. Elija una palabra de la lista y haga clic en **Reemplazar**, o haga doble clic en ella. La palabra de la celda se reemplaza, y una primera letra en mayúscula se mantiene en mayúscula. **Buscar** busca la palabra que eligió, para seguir una cadena de significados. La lista de palabras funciona en inglés.

## Comprobar accesibilidad

**Comprobar accesibilidad** (ficha **Revisar**) abre una pequeña ventana que enumera lo que en el libro es difícil de usar para algunos lectores, o para un lector de pantalla (un programa que lee en voz alta lo que aparece en la pantalla). Cada línea empieza por **Error**, **Advertencia** o **Sugerencia**. Haga clic en una línea para ir a la celda.

Busca:

- Un gráfico o una imagen sin **texto alternativo** (las palabras que describen lo que muestra). Esto es un error.
- Una tabla sin fila de encabezado, de modo que sus columnas no tienen nombre. Esto es un error.
- **Celdas combinadas**, que son difíciles de recorrer con el teclado. Esto es una advertencia.
- Texto que se lee mal sobre el color de fondo, como texto gris claro sobre blanco. JeffCrunch mide cuánto destaca el texto sobre el relleno. El texto pequeño necesita una relación de al menos 4,5 a 1, y el texto grande (18 puntos, o 14 puntos en negrita) necesita 3 a 1. Indica la primera celda de cada combinación de fuente y relleno. Esto es una advertencia.
- Una hoja que aún tiene el nombre que se le asignó, como Hoja1. Esto es una sugerencia.

Pulse **Comprobar de nuevo** después de hacer cambios.
