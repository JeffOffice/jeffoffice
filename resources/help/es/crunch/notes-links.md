# Notas, comentarios y vínculos
<!-- keywords: panel de comentarios, nuevo comentario, responder, resolver, reabrir, hilo, comentario anterior, comentario siguiente, mostrar comentarios, convertir notas en comentarios, convertir comentarios en notas, ortografía, revisar ortografía, error ortográfico, F7, omitir todo, cambiar todo, agregar al diccionario, sinónimos, lista de palabras, nota, notas, comentario, comentarios, al pasar el puntero, marca roja, esquina, nueva nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas las notas, vínculo, vínculos, hipervínculo, dirección, página web, correo electrónico, lugar de este libro, Ctrl+K, Ctrl+clic, información en pantalla, abrir vínculo, quitar vínculo, conversación, estadísticas del libro, contar, celdas, fórmulas, gráficos, tablas dinámicas, accesibilidad, texto alternativo, celdas combinadas, archivos recientes -->

## Notas

Una **nota** es unas pocas palabras guardadas junto a una celda. Una celda que tiene una nota muestra una pequeña marca roja en su esquina superior derecha. Coloque el puntero sobre la celda y la nota aparecerá junto a ella; aleje el puntero y desaparecerá.

Para poner una nota en una celda, selecciónela y use **Revisar > Nueva nota** (o presione **Mayús+F2**). Escriba las palabras y haga clic en **Aceptar**. En una celda que ya tiene una nota, el mismo comando dice **Editar nota**. Si borra las palabras, la nota se quita.

- **Eliminar nota** quita las notas de las celdas seleccionadas.
- **Mostrar u ocultar nota** mantiene visible en la hoja, todo el tiempo, la nota de las celdas seleccionadas, en un cuadro junto a cada celda, o la vuelve a ocultar.
- **Mostrar todas las notas** hace lo mismo con todas las notas de la hoja; el mismo botón entonces dice **Ocultar todas las notas**.
- **Nota anterior** y **Nota siguiente** pasan de una celda con nota a la siguiente.

Cada una de estas acciones es un paso que puede deshacer. Una nota se desplaza con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan celdas. **Borrar todo** quita las notas y los comentarios de las celdas que borra.

Una celda también puede contener una conversación de **comentarios**, con respuestas. Consulte la siguiente parte.

Las notas se pueden guardar en un libro .xlsx. Un libro abierto desde un tipo de archivo más antiguo, o desde un archivo de texto, conserva sus notas solo después de guardarlo como libro .xlsx.

## Comentarios

Un **comentario** es parecido a una nota, pero inicia un **hilo**: otras personas pueden responderle, y una de ellas puede marcar el hilo como **resuelto** cuando la pregunta está contestada. Cada comentario muestra quién lo escribió y cuándo. Una celda con un hilo tiene la misma marca roja que una celda con una nota. Coloque el puntero sobre la celda para leer todo el hilo, con cada respuesta sangrada debajo del comentario al que responde. Un hilo resuelto empieza con la palabra **Resuelto**.

En la ficha **Revisar**, en el grupo **Comentarios**:

- **Nuevo comentario** (**Ctrl+Mayús+F2**) inicia un hilo en la celda seleccionada. Escriba las palabras y haga clic en **Aceptar**. Funciona en una celda que no tiene nota ni comentarios.
- **Responder** agrega sus palabras al hilo de la celda seleccionada.
- **Resolver hilo** marca el hilo como terminado. El mismo botón entonces dice **Reabrir hilo**, que vuelve a abrirlo.
- **Eliminar comentario** quita los hilos de las celdas seleccionadas.
- **Comentario anterior** y **Comentario siguiente** pasan de una celda con hilo a la siguiente.
- **Mostrar comentarios** abre el panel **Comentarios** junto a la hoja. Enumera cada hilo de la hoja como una tarjeta: la celda, cada comentario con su autor y hora, y los botones **Responder**, **Resolver** o **Reabrir**, y **Eliminar**. **Ir a** lleva a la celda. El panel sigue cada cambio, y también Deshacer.
- **Convertir notas en comentarios** transforma cada nota sencilla de la hoja en un hilo de un solo comentario. **Convertir comentarios en notas** devuelve cada hilo a una nota. Una nota no tiene respuestas, así que los comentarios de un hilo se convierten en una sola nota con todas sus palabras, y JeffCrunch le pide confirmación antes de hacerlo con un hilo que tiene respuestas.

Cada una de estas acciones es un paso que puede deshacer.

Los programas que no pueden mostrar hilos muestran una nota sencilla en su lugar. JeffCrunch conserva esa nota, con las palabras del hilo, cada vez que guarda. Los hilos, las respuestas y las personas que los escribieron se mantienen como estaban al guardar y volver a abrir el libro. Un hilo con más de 500 comentarios se muestra y se conserva, pero JeffCrunch no le agrega nada ni lo modifica.

Los comentarios se guardan en un libro .xlsx. Un libro abierto desde un tipo de archivo más antiguo, o desde un archivo de texto, conserva sus comentarios solo después de guardarlo como libro .xlsx.

## Vínculos

Un **vínculo** le lleva a otro lugar cuando lo sigue: a una página web, a un archivo, a una dirección de correo electrónico o a un lugar de este libro (una celda, varias celdas o un nombre).

Para poner un vínculo en una celda, selecciónela y use **Insertar > Vínculo** (o presione **Ctrl+K**). Elija adónde lleva, escriba la dirección o el lugar (o elija uno de los archivos que abrió recientemente en **O un archivo reciente**) y, si lo desea, el texto que debe mostrar la celda y una **información en pantalla**. En una celda que ya tiene un vínculo, el mismo comando dice **Editar vínculo**. **Quitar vínculo** elimina los vínculos de las celdas seleccionadas.

Coloque el puntero sobre una celda con vínculo para ver adónde lleva. Para seguirlo, mantenga presionada **Ctrl** y haga clic en la celda, o selecciónela y use **Abrir vínculo** del menú contextual (clic con el botón derecho). Un clic simple solo selecciona la celda.

JeffCrunch nunca abre nada por sí mismo. Una página web o una dirección de correo electrónico se abre en su programa habitual cuando sigue el vínculo. Un vínculo a un archivo, o a otro tipo de dirección, le pide confirmación antes. Un vínculo que ejecutaría un script nunca se abre.

Un vínculo se desplaza con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan celdas, y se guarda con el libro.

## Estadísticas del libro

**Estadísticas del libro** (ficha **Revisar**) cuenta lo que contiene el libro: cuántas **Hojas** tiene, cuántas **Celdas con datos** (celdas que contienen un número, un texto o una fórmula), cuántas **Fórmulas**, cuántos **Gráficos** y cuántas **Tablas dinámicas**. Las hojas eliminadas no se cuentan. Los mismos números aparecen en la página **Información** de la ficha Archivo.

## Ortografía

**Ortografía** (ficha **Revisar**, o **F7**) revisa las palabras de sus celdas de texto, una palabra incorrecta a la vez. Si ha seleccionado más de una celda, solo revisa esas celdas. En caso contrario, revisa toda la hoja. No revisa fórmulas, números ni fechas.

La ventana muestra la palabra que no está en el diccionario, un cuadro llamado **Cambiar a** con la mejor sugerencia y una lista de otras sugerencias. Elija una sugerencia o escriba la suya, y luego elija:

- **Cambiar** pone la palabra nueva en lugar de la incorrecta en esa celda. Cada cambio es un paso que puede deshacer.
- **Cambiar todas** la pone en lugar de la misma palabra en todas las celdas revisadas, como un único paso que puede deshacer.
- **Omitir** deja la palabra como está esta vez. **Omitir todo** la deja como está en todas partes hasta que cierre JeffCrunch.
- **Agregar** pone la palabra en su propio diccionario, para que nunca vuelva a marcarse. Su diccionario se comparte con las otras aplicaciones de JeffOffice.

**Archivo > Opciones > Revisión** decide si se revisan las palabras en mayúsculas y las palabras con números. JeffCrunch revisa en el idioma en que se muestra, si tiene un diccionario para él, y en inglés de Estados Unidos si no lo tiene.

Mientras escribe en una celda, una palabra que no está en el diccionario recibe una línea ondulada roja debajo. Haga clic con el botón derecho en la palabra para elegir una corrección, u **Omitir todo** o **Agregar al diccionario**. Las celdas de la cuadrícula no se subrayan, y una entrada que empieza con un signo igual nunca se revisa.

## Sinónimos

**Sinónimos** (ficha **Revisar**, o **Mayús+F7**) busca palabras que significan casi lo mismo que una palabra de una celda. Busca la primera palabra del texto de la celda en la que está. Escriba otra palabra en **Buscar** y presione **Entrar** para buscar esa en su lugar. Elija una palabra de la lista y haga clic en **Reemplazar**, o haga doble clic en ella. La palabra de la celda se reemplaza, y una mayúscula inicial se mantiene. **Buscar** busca la palabra que eligió, para seguir una cadena de significados. La lista de palabras funciona en inglés.

## Comprobar accesibilidad

**Comprobar accesibilidad** (ficha **Revisar**) abre una pequeña ventana que enumera lo que en el libro resulta difícil de usar para algunos lectores, o para un lector de pantalla (un programa que lee en voz alta lo que aparece en la pantalla). Cada línea empieza con **Error**, **Advertencia** o **Sugerencia**. Haga clic en una línea para ir a la celda.

Busca:

- Un gráfico o una imagen sin **texto alternativo** (las palabras que dicen lo que muestra). Esto es un error.
- Una tabla sin fila de encabezado, de modo que sus columnas no tienen nombres. Esto es un error.
- **Celdas combinadas**, que son difíciles de recorrer con el teclado. Esto es una advertencia.
- Texto difícil de leer sobre el color de fondo, como texto gris claro sobre blanco. JeffCrunch mide cuánto destaca el texto frente al relleno. El texto pequeño necesita una relación de al menos 4,5 a 1, y el texto grande (18 puntos, o 14 puntos en negrita) necesita 3 a 1. Indica la primera celda de cada combinación de fuente y relleno. Esto es una advertencia.
- Una hoja que aún tiene el nombre que se le asignó, como Hoja1. Esto es una sugerencia.

Presione **Volver a comprobar** después de hacer cambios.
