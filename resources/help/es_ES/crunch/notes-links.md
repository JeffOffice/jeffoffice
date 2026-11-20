# Notas, comentarios y vínculos

<!-- keywords: mover nota, desplazar nota, cambiar tamaño de nota, cuadro de nota, restablecer tamaño de nota, teclado, idioma, idioma de revisión, establecer idioma, no comprobar ortografía, traducir, traducción, libretranslate, panel de comentarios, nuevo comentario, responder, resolver, reabrir, hilo, comentario anterior, comentario siguiente, mostrar comentarios, convertir notas en comentarios, convertir comentarios en notas, ortografía, revisar ortografía, mal escrita, F7, omitir todo, cambiar todo, agregar al diccionario, sinónimos, lista de palabras, nota, notas, comentario, comentarios, pasar el puntero, marca roja, esquina, nueva nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas las notas, vínculo, vínculos, hipervínculo, dirección, página web, correo electrónico, lugar de este libro, Ctrl+K, Ctrl+clic, información en pantalla, abrir vínculo, quitar vínculo, conversación, estadísticas del libro, contar, celdas, fórmulas, gráficos, tablas dinámicas, accesibilidad, texto alternativo, celdas combinadas, archivos recientes, ficheros recientes -->

## Notas

Una **nota** es unas pocas palabras guardadas junto a una celda. Una celda que tiene una muestra una pequeña marca roja en su esquina superior derecha. Sitúe el puntero sobre la celda, y la nota aparecerá junto a ella; aleje el puntero, y desaparecerá.

Para poner una nota en una celda, selecciónela y use **Revisar > Nueva nota** (o pulse **Mayús+F2**). Escriba las palabras y haga clic en **Aceptar**. En una celda que ya tiene una nota, el mismo comando aparece como **Editar nota**. Si borra las palabras, la nota desaparece.

- **Eliminar nota** quita las notas de las celdas seleccionadas.
- **Mostrar u ocultar nota** mantiene la nota de las celdas seleccionadas visible en la hoja todo el tiempo, en un cuadro junto a cada celda, o vuelve a ocultarla.
- **Mostrar todas las notas** hace lo mismo con todas las notas de la hoja; el mismo botón pasa entonces a decir **Ocultar todas las notas**.
- **Nota anterior** y **Nota siguiente** pasan de una celda con nota a la siguiente.
- **Mover o cambiar tamaño de nota** mueve o cambia el tamaño del cuadro de una nota visible con el teclado. Seleccione la celda y use el comando (**Revisar > Notas**, o haga clic con el botón derecho en la celda). Después, las **teclas de flecha** mueven el cuadro 10 píxeles; **Mayús con una flecha** lo hace más ancho, más estrecho, más alto o más bajo en 10 píxeles, y **Ctrl** (**Opción** en un Mac) hace que los pasos sean de 1 píxel. Las combinaciones con Alt o Meta (o Comando en un Mac) no hacen nada mientras tanto, y cualquier otro comando, un cambio de hoja, un Deshacer o un Rehacer hace que el cuadro se pierda. **Intro** mantiene el cuadro donde está, como un único paso que puede deshacer, y **Esc** cancela el cambio. Un lector de pantalla indica la posición del cuadro después de cada tecla, en píxeles con el zoom al 100 % (cuánto se desplaza a la derecha y hacia abajo desde la esquina superior izquierda de la celda, y cuán ancho y alto es el cuadro), y lo indica al soltar una tecla que mantuvo pulsada, no en cada repetición; también indica cuándo el cuadro se mantiene o se pierde. Un cuadro tiene como máximo 640 píxeles de ancho y 480 de alto, y empieza a menos de 1000 píxeles de su celda, como al arrastrarlo. Arrastrar el cuadro y sus ocho controladores con el puntero produce el mismo resultado.
- **Restablecer tamaño de nota** devuelve el cuadro de la nota a su valor predeterminado: junto a la celda, con el tamaño que necesitan sus palabras.

Cada una de estas acciones es un paso que puede deshacer. Una nota se mueve con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan las celdas. **Borrar todo** quita las notas y los comentarios de las celdas que borra.

Una celda también puede contener una conversación de **comentarios**, con respuestas. Consulte la siguiente parte.

Las notas se pueden guardar en un libro .xlsx, en un libro .ods y en un libro .xls. Un libro abierto desde un fichero .ods o .xls muestra sus notas, y puede agregarlas, cambiarlas, moverlas y eliminarlas como en un libro .xlsx. La fecha de una nota .ods no se conserva al guardar en JeffCrunch. Una conversación de comentarios guardada como fichero .ods o .xls se convierte en una nota sencilla con sus palabras, y sigue siendo una nota sencilla al abrir ese fichero. Un fichero .xls guarda la distancia de un cuadro a su celda con un tamaño fijo, de modo que un cuadro situado muy hacia dentro de una celda ancha puede desplazarse un poco. Un libro abierto desde un fichero de texto no admite notas.

## Comentarios

Un **comentario** es como una nota, pero inicia un **hilo**: otras personas pueden responder, y una de ellas puede marcar el hilo como **resuelto** cuando la pregunta se haya contestado. Cada comentario muestra quién lo escribió y cuándo. Una celda con un hilo tiene la misma marca roja que una celda con una nota. Sitúe el puntero sobre la celda para leer el hilo completo, con cada respuesta sangrada debajo del comentario al que responde. Un hilo resuelto empieza con la palabra **Resuelto**.

En la ficha **Revisar**, en el grupo **Comentarios**:

- **Nuevo comentario** (**Ctrl+Mayús+F2**) inicia un hilo en la celda seleccionada. Escriba las palabras y haga clic en **Aceptar**. Funciona en una celda que no tiene nota ni comentarios.
- **Responder** agrega sus palabras al hilo de la celda seleccionada.
- **Resolver hilo** marca el hilo como terminado. El mismo botón pasa entonces a decir **Reabrir hilo**, que lo vuelve a abrir.
- **Eliminar comentario** quita los hilos de las celdas seleccionadas.
- **Comentario anterior** y **Comentario siguiente** pasan de una celda con hilo a la siguiente.
- **Mostrar comentarios** abre el panel **Comentarios** junto a la hoja. Enumera todos los hilos de la hoja como una tarjeta: la celda, cada comentario con su autor y hora, y los botones **Responder**, **Resolver** o **Reabrir**, y **Eliminar**. **Ir a** lleva a la celda. El panel sigue cada cambio, y también Deshacer.
- **Convertir notas en comentarios** convierte cada nota sencilla de la hoja en un hilo de un comentario. **Convertir comentarios en notas** convierte cada hilo de nuevo en una nota. Una nota no tiene respuestas, así que los comentarios de un hilo se convierten en una sola nota con todas sus palabras, y JeffCrunch pide confirmación antes de hacerlo con un hilo que tiene respuestas.

Cada una de estas acciones es un paso que puede deshacer.

Los programas que no pueden mostrar hilos muestran una nota sencilla en su lugar. JeffCrunch conserva esa nota, con las palabras del hilo, cada vez que guarda. Los hilos, las respuestas y las personas que los escribieron se mantienen tal como estaban al guardar y volver a abrir el libro. Un hilo con más de 500 comentarios se muestra y se conserva, pero JeffCrunch no le agrega nada ni lo modifica.

Los comentarios se guardan en un libro .xlsx. Un libro abierto desde un tipo de fichero más antiguo, o desde un fichero de texto, conserva sus comentarios solo después de guardarlo como libro .xlsx.

## Vínculos

Un **vínculo** le lleva a otro lugar cuando lo sigue: a una página web, a un fichero, a una dirección de correo electrónico o a un lugar de este libro (una celda, varias celdas o un nombre).

Para poner un vínculo en una celda, selecciónela y use **Insertar > Vínculo** (o pulse **Ctrl+K**). Elija adónde debe llevar, escriba la dirección o el lugar (o elija uno de los ficheros que abrió recientemente en **O un fichero reciente**) y, si lo desea, el texto que debe mostrar la celda y una **información en pantalla**. En una celda que ya tiene un vínculo, el mismo comando aparece como **Editar vínculo**. **Quitar vínculo** elimina los vínculos de las celdas seleccionadas.

Sitúe el puntero sobre una celda con vínculo para ver adónde lleva. Para seguirlo, mantenga pulsada **Ctrl** y haga clic en la celda, o selecciónela y use **Abrir vínculo** del menú contextual. Un clic simple solo selecciona la celda.

JeffCrunch nunca abre nada por su cuenta. Una página web o una dirección de correo electrónico se abre en su programa habitual cuando sigue el vínculo. Un vínculo a un fichero, o a otro tipo de dirección, le pide confirmación antes. Nunca se abre un vínculo que ejecutaría un script.

Un vínculo se mueve con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan las celdas, y se guarda con el libro.

## Estadísticas del libro

**Estadísticas del libro** (ficha **Revisar**) cuenta lo que contiene el libro: cuántas **Hojas** tiene, cuántas **Celdas con datos** (celdas que contienen un número, un texto o una fórmula), cuántas **Fórmulas**, cuántos **Gráficos** y cuántas **Tablas dinámicas**. Las hojas que eliminó no se cuentan. Los mismos números aparecen en la página **Información** de la ficha Archivo.

## Ortografía

**Ortografía** (ficha **Revisar**, o **F7**) revisa las palabras de sus celdas de texto, una palabra incorrecta cada vez. Si ha seleccionado más de una celda, revisa solo esas celdas. En caso contrario, revisa toda la hoja. No revisa fórmulas, números ni fechas.

La ventana muestra la palabra que no está en el diccionario, un cuadro llamado **Cambiar a** con la mejor sugerencia y una lista de otras sugerencias. Elija una sugerencia o escriba la suya, y después elija:

- **Cambiar** pone la nueva palabra en lugar de la incorrecta en esa celda. Cada cambio es un paso que puede deshacer.
- **Cambiar todas** la pone en lugar de la misma palabra en todas las celdas revisadas, como un solo paso que puede deshacer.
- **Omitir** deja la palabra como está en esta ocasión. **Omitir todas** la deja como está en todas partes hasta que cierre JeffCrunch.
- **Agregar** pone la palabra en su propio diccionario, para que no vuelva a marcarse. Su diccionario se comparte con las demás aplicaciones de JeffOffice.

**Archivo > Opciones > Revisión** decide si se revisan las palabras en mayúsculas y las palabras que contienen números. JeffCrunch revisa en el idioma en el que se muestra, si tiene diccionario para él, y en inglés de Estados Unidos si no lo tiene.

Mientras escribe en una celda, una palabra que no está en el diccionario recibe una línea roja ondulada debajo. Haga clic con el botón derecho en la palabra para elegir una corrección, o para elegir **Omitir todas** o **Agregar al diccionario**. Las celdas de la cuadrícula no llevan subrayado, y una entrada que empieza con un signo igual nunca se revisa. Para ver, editar, importar o exportar su lista de palabras agregadas, y para indicar las palabras que siempre se marcan como incorrectas, haga clic en **Archivo > Opciones > Revisión > Diccionario personalizado**. Sus listas son las mismas en todas las aplicaciones de JeffOffice.

## Establecer el idioma de algunas celdas

La revisión ortográfica comprueba cada celda en su propio idioma. Para cambiarlo, seleccione las celdas y haga clic en **Establecer idioma de revisión** en el grupo **Idioma** de la ficha **Revisar**. Elija el idioma en la lista y, después, haga clic en **Aceptar**. Una celda que no ha marcado se revisa en el idioma establecido en **Archivo > Opciones**. Una hoja de cálculo de OpenDocument (.ods) conserva el idioma de cada celda. Un libro .xlsx no tiene lugar para ello, así que el idioma se conserva solo mientras el libro está abierto.

## No revisar algunas celdas

Hay textos que no deben revisarse: un código, un nombre o una línea en un idioma inventado. Seleccione las celdas y haga clic en **No comprobar ortografía** en el grupo **Idioma**. **Ortografía** y las líneas rojas onduladas de la celda los omiten. Haga clic de nuevo para volver a revisar las celdas. Un fichero .ods conserva esta marca, y un libro .xlsx la conserva solo mientras el libro está abierto.

## Traducir texto

1. Seleccione las celdas, o seleccione algunas de las palabras que está escribiendo en una celda. El texto de las celdas seleccionadas se envía una fila por línea.
2. Haga clic en **Traducir** en el grupo **Idioma**.
3. Elija el idioma al que traducir.
4. Haga clic en **Aceptar**.

Entonces su navegador web abre un servicio de traducción llamado LibreTranslate, con su texto dentro. No se envía nada hasta que haga clic en **Aceptar**. Puede cambiar la dirección del servicio en el cuadro, por ejemplo por una que usted mismo ejecute. Solo se usan direcciones web seguras (https), y direcciones http de este equipo. Se envían como máximo 5000 letras.

## Sinónimos

**Sinónimos** (ficha **Revisar**, o **Mayús+F7**) busca palabras que significan casi lo mismo que una palabra de una celda. Busca la primera palabra del texto de la celda en la que se encuentra. Escriba otra palabra en **Buscar** y pulse **Intro** para buscar esa en su lugar. Elija una palabra de la lista y haga clic en **Reemplazar**, o haga doble clic en ella. La palabra de la celda se reemplaza, y una primera letra mayúscula se mantiene como mayúscula. **Buscar** busca la palabra que eligió, para seguir una cadena de significados. La lista de palabras funciona en inglés.

## Comprobar accesibilidad

**Comprobar accesibilidad** (ficha **Revisar**) abre una pequeña ventana que enumera lo que en el libro es difícil de usar para algunos lectores, o para un lector de pantalla (un programa que lee en voz alta lo que aparece en la pantalla). Cada línea empieza con **Error**, **Advertencia** o **Sugerencia**. Haga clic en una línea para ir a la celda.

Busca:

- Un gráfico o una imagen sin **texto alternativo** (las palabras que dicen lo que muestra). Esto es un error.
- Una tabla sin fila de encabezado, así que sus columnas no tienen nombres. Esto es un error.
- **Celdas combinadas**, que son difíciles de recorrer con el teclado. Esto es una advertencia.
- Texto difícil de leer sobre el color de fondo, como texto gris claro sobre blanco. JeffCrunch mide cuánto destaca el texto frente al relleno. El texto pequeño necesita una relación de al menos 4,5 a 1, y el texto grande (18 puntos, o 14 puntos en negrita) necesita 3 a 1. Indica la primera celda de cada combinación de fuente y relleno. Esto es una advertencia.
- Una hoja que todavía tiene el nombre que se le asignó, como Hoja1. Esto es una sugerencia.

Pulse **Volver a comprobar** después de hacer cambios.
