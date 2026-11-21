# Notas, comentarios y vínculos
<!-- keywords: quitar hipervínculos, quitar aspecto de vínculo, mover nota, cambiar tamaño de nota, redimensionar nota, cuadro de nota, restablecer tamaño de nota, teclado, idioma, idioma de revisión, establecer idioma, no comprobar ortografía, traducir, traducción, libretranslate, panel de comentarios, nuevo comentario, responder, resolver, reabrir, hilo, comentario anterior, comentario siguiente, mostrar comentarios, convertir notas en comentarios, convertir comentarios en notas, ortografía, revisar ortografía, mal escrita, F7, omitir todas, cambiar todas, agregar al diccionario, sinónimos, tesauro, lista de palabras, nota, notas, comentario, comentarios, al pasar el puntero, marca roja, esquina, nueva nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas las notas, vínculo, vínculos, hipervínculo, dirección, página web, correo electrónico, lugar de este libro, Ctrl+K, Ctrl+clic, información en pantalla, abrir vínculo, quitar vínculo, conversación, estadísticas del libro, contar, celdas, fórmulas, gráficos, tablas dinámicas, accesibilidad, texto alternativo, celdas combinadas, archivos recientes -->

## Notas

Una **nota** es unas pocas palabras asociadas a una celda. Una celda que tiene una nota muestra una pequeña marca roja en su esquina superior derecha. Apunte con el puntero a la celda y la nota aparecerá junto a ella; aleje el puntero y desaparecerá.

Para poner una nota en una celda, selecciónela y use **Revisar > Nueva nota** (o presione **Mayús+F2**). Escriba las palabras y haga clic en **Aceptar**. En una celda que ya tiene una nota, el mismo comando dice **Editar nota**. Si borra las palabras, la nota desaparece.

- **Eliminar nota** quita las notas de las celdas seleccionadas.
- **Mostrar u ocultar nota** mantiene visible en la hoja la nota de las celdas seleccionadas todo el tiempo, en un cuadro junto a cada celda, o vuelve a ocultarla.
- **Mostrar todas las notas** hace lo mismo con todas las notas de la hoja; el mismo botón dice entonces **Ocultar todas las notas**.
- **Nota anterior** y **Nota siguiente** pasan de una celda con nota a la siguiente.
- **Mover o cambiar tamaño de nota** mueve o cambia el tamaño del cuadro de una nota visible con el teclado. Seleccione la celda y use el comando (**Revisar > Notas**, o haga clic con el botón derecho en la celda). Después, las **teclas de flecha** mueven el cuadro 1 mm, **Mayús con una flecha** lo hace más ancho, más estrecho, más alto o más bajo en pasos de 1 mm, y **Ctrl** (**Opción** en un Mac) hace los pasos de 0,1 mm. Las combinaciones con Alt o Meta (o Comando en un Mac) no hacen nada mientras tanto, y cualquier otro comando, un cambio de hoja, Deshacer o Rehacer hacen que se suelte el cuadro. **Intro** mantiene el cuadro donde está, como un paso que puede deshacer, y **Esc** lo suelta sin cambios. Un lector de pantalla dice la posición del cuadro después de cada tecla, en milímetros con el zoom al 100 % (cuánto avanza a la derecha y hacia abajo desde la esquina superior izquierda de la celda, y cuán ancho y alto es el cuadro), y lo dice al soltar una tecla que mantuvo pulsada, no en cada repetición; también avisa cuando el cuadro se mantiene o se suelta. Un cuadro tiene como máximo 17 cm de ancho y 13 cm de alto, y empieza a menos de 2,5 cm de su celda, como cuando lo arrastra. Arrastrar el cuadro y sus ocho controladores con el puntero hace lo mismo.
- **Restablecer tamaño de nota** devuelve el cuadro de la nota al tamaño predeterminado: junto a la celda, con el tamaño que necesitan sus palabras.

Cada una de estas acciones es un paso que puede deshacer. Una nota se mueve con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan las celdas. **Borrar todo** elimina las notas y los comentarios de las celdas que borra.

Una celda también puede tener una conversación de **comentarios**, con respuestas. Vea la siguiente parte.

Las notas se pueden guardar en un libro .xlsx, en un libro .ods y en un libro .xls. Un libro abierto desde un archivo .ods o .xls muestra sus notas, y puede agregarlas, cambiarlas, moverlas y eliminarlas igual que en un libro .xlsx. La fecha de una nota .ods no se conserva al guardar en JeffCrunch. Una conversación de comentarios guardada como archivo .ods o .xls se convierte en una nota sencilla con sus palabras, y sigue siendo una nota sencilla al abrir ese archivo. Un archivo .xls guarda la distancia de un cuadro hasta su celda con un tamaño fijo, así que un cuadro que queda muy lejos dentro de una celda ancha puede desplazarse un poco. Un libro abierto desde un archivo de texto no admite notas.

## Comentarios

Un **comentario** es como una nota, pero inicia un **hilo**: las personas pueden responderle, y una de ellas puede marcar el hilo como **resuelto** cuando la pregunta está contestada. Cada comentario muestra quién lo escribió y cuándo. Una celda con un hilo tiene la misma marca roja que una celda con nota. Apunte con el puntero a la celda para leer todo el hilo, con cada respuesta sangrada debajo del comentario al que responde. Un hilo resuelto empieza con la palabra **Resuelto**.

En la ficha **Revisar**, en el grupo **Comentarios**:

- **Nuevo comentario** (**Ctrl+Mayús+F2**) inicia un hilo en la celda seleccionada. Escriba las palabras y haga clic en **Aceptar**. Funciona en una celda que no tiene nota ni comentarios.
- **Responder** agrega sus palabras al hilo de la celda seleccionada.
- **Resolver hilo** marca el hilo como terminado. El mismo botón dice entonces **Reabrir hilo**, que lo vuelve a abrir.
- **Eliminar comentario** quita los hilos de las celdas seleccionadas.
- **Comentario anterior** y **Comentario siguiente** pasan de una celda con hilo a la siguiente.
- **Mostrar comentarios** abre el panel **Comentarios** junto a la hoja. Enumera cada hilo de la hoja como una tarjeta: la celda, cada comentario con su autor y hora, y los botones **Responder**, **Resolver** o **Reabrir**, y **Eliminar**. **Ir a** lleva a la celda. El panel sigue cada cambio, y también Deshacer.
- **Convertir notas en comentarios** convierte cada nota sencilla de la hoja en un hilo de un solo comentario. **Convertir comentarios en notas** convierte cada hilo de nuevo en una nota. Una nota no tiene respuestas, así que los comentarios de un hilo se convierten en una sola nota con todas sus palabras, y JeffCrunch pregunta antes de hacerlo con un hilo que tiene respuestas.

Cada una de estas acciones es un paso que puede deshacer.

Los programas que no pueden mostrar hilos muestran una nota sencilla en su lugar. JeffCrunch conserva esa nota, con las palabras del hilo, cada vez que guarda. Los hilos, las respuestas y las personas que los escribieron se mantienen tal como estaban al guardar y volver a abrir el libro. Un hilo con más de 500 comentarios se muestra y se conserva, pero JeffCrunch no le agrega nada ni lo cambia.

Los comentarios se guardan en un libro .xlsx. Un libro abierto desde un tipo de archivo más antiguo, o desde un archivo de texto, conserva sus comentarios solo después de guardarlo como libro .xlsx.

## Vínculos

Un **vínculo** lo lleva a otro lugar cuando lo sigue: a una página web, a un archivo, a una dirección de correo electrónico o a un lugar de este libro (una celda, varias celdas o un nombre).

Para poner un vínculo en una celda, selecciónela y use **Insertar > Vínculo** (o presione **Ctrl+K**). Elija adónde va, escriba la dirección o el lugar (o elija uno de los archivos que abrió recientemente en **O un archivo reciente**) y, si lo desea, las palabras que debe mostrar la celda y una **información en pantalla**. En una celda que ya tiene un vínculo, el mismo comando dice **Modificar vínculo**. **Quitar vínculo** quita los vínculos de las celdas seleccionadas. **Quitar hipervínculos** (en el menú **Borrar** de la ficha Inicio, y en el menú contextual de una celda con vínculo) quita los vínculos y también el aspecto que un vínculo da a su celda, su color azul y su subrayado, para que ya no se vea como un vínculo. Todo lo demás de la celda se mantiene: su relleno, formato de número, bordes, alineación y su propia fuente. Una celda que usted puso en azul y subrayada a mano, sin el estilo Hipervínculo, conserva ese aspecto. Ambas acciones son un paso que puede deshacer, y una hoja protegida que no permite vínculos los rechaza.

Los vínculos y su aspecto se guardan con el libro, en todos los formatos que admiten vínculos.

Apunte con el puntero a una celda con vínculo para ver adónde va. Para seguirlo, mantenga pulsado **Ctrl** y haga clic en la celda, o seleccione la celda y use **Abrir vínculo** del menú contextual. Un clic normal solo selecciona la celda.

JeffCrunch nunca abre nada por sí mismo. Una página web o una dirección de correo electrónico se abre en su programa habitual cuando sigue su vínculo. Un vínculo a un archivo, u a otro tipo de dirección, le pide confirmación antes. Nunca se abre un vínculo que ejecutaría un script.

Un vínculo se mueve con su celda cuando se insertan o eliminan filas o columnas, y cuando se ordenan las celdas, y se guarda con el libro.

## Estadísticas del libro

**Estadísticas del libro** (ficha **Revisar**) cuenta lo que contiene el libro: cuántas **Hojas** tiene, cuántas **Celdas con datos** (celdas que contienen un número, texto o una fórmula), cuántas **Fórmulas**, cuántos **Gráficos** y cuántas **Tablas dinámicas**. Las hojas que eliminó no se cuentan. Los mismos números aparecen en la página **Información** de la ficha Archivo.

## Ortografía

**Ortografía** (ficha **Revisar**, o **F7**) revisa las palabras de sus celdas de texto, una palabra incorrecta a la vez. Si ha seleccionado más de una celda, solo revisa esas celdas. Si no, revisa toda la hoja. No revisa fórmulas, números ni fechas.

La ventana muestra la palabra que no está en el diccionario, un cuadro llamado **Cambiar a** con la mejor suposición y una lista de otras sugerencias. Elija una sugerencia o escriba la suya, y luego elija:

- **Cambiar** pone la palabra nueva en lugar de la incorrecta en esa celda. Cada cambio es un paso que puede deshacer.
- **Cambiar todas** la pone en lugar de la misma palabra en todas las celdas revisadas, como un único paso que puede deshacer.
- **Omitir** deja la palabra como está esta vez. **Omitir todas** la deja como está en todas partes hasta que cierre JeffCrunch.
- **Agregar** pone la palabra en su propio diccionario, de modo que nunca vuelve a marcarse. Su diccionario se comparte con las demás aplicaciones de JeffOffice.

**Archivo > Opciones > Revisión** decide si se revisan las palabras en mayúsculas y las palabras con números. JeffCrunch revisa en el idioma en que se muestra, si tiene un diccionario para él, y en inglés de Estados Unidos si no lo tiene.

Mientras escribe en una celda, una palabra que no está en el diccionario recibe una línea roja ondulada debajo. Haga clic con el botón derecho en la palabra para elegir una corrección, o para elegir **Omitir todas** o **Agregar al diccionario**. Las celdas de la cuadrícula no se subrayan, y una entrada que empieza con signo igual nunca se revisa. Para ver, editar, importar o exportar su lista de palabras agregadas, y para listar palabras que siempre se marcan como mal escritas, haga clic en **Archivo > Opciones > Revisión > Diccionario personalizado**. Sus listas son las mismas en todas las aplicaciones de JeffOffice.

## Establecer el idioma de algunas celdas

La revisión ortográfica comprueba cada celda en su propio idioma. Para cambiarlo, seleccione las celdas y haga clic en **Establecer idioma de revisión** en el grupo **Idioma** de la ficha **Revisar**. Elija el idioma en la lista y haga clic en **Aceptar**. Una celda que no ha marcado se revisa en el idioma establecido en **Archivo > Opciones**. Un libro de hoja de cálculo OpenDocument (.ods) conserva el idioma de cada celda. Un libro .xlsx no tiene dónde guardarlo, así que el idioma se conserva solo mientras el libro está abierto.

## No revisar algunas celdas

Hay textos que no deben revisarse: un código, un nombre o una línea en un idioma inventado. Seleccione las celdas y haga clic en **No comprobar ortografía** en el grupo **Idioma**. **Ortografía** y las líneas rojas onduladas de la celda las omiten. Haga clic de nuevo para revisar las celdas una vez más. Un archivo .ods conserva esta marca, y un libro .xlsx la conserva solo mientras el libro está abierto.

## Traducir texto

1. Seleccione las celdas, o algunas de las palabras que está escribiendo en una celda. El texto de las celdas seleccionadas se envía una fila por línea.
2. Haga clic en **Traducir** en el grupo **Idioma**.
3. Elija el idioma al que desea traducir.
4. Haga clic en **Aceptar**.

Entonces su navegador web abre un servicio de traducción llamado LibreTranslate, con su texto. No se envía nada hasta que haga clic en **Aceptar**. Puede cambiar la dirección del servicio en el cuadro, por ejemplo por una que usted mismo administre. Solo se usan direcciones web seguras (https), y direcciones http de este equipo. Se envían como máximo 5000 letras.

## Sinónimos

**Sinónimos** (ficha **Revisar**, o **Mayús+F7**) busca palabras que significan casi lo mismo que una palabra de una celda. Busca la primera palabra del texto de la celda en la que está. Escriba otra palabra en **Buscar** y presione **Intro** para buscar esa en su lugar. Elija una palabra de la lista y haga clic en **Reemplazar**, o haga doble clic en ella. La palabra de la celda se reemplaza, y una primera letra en mayúscula se mantiene en mayúscula. **Buscar** busca la palabra que eligió, para seguir una cadena de significados. La lista de palabras funciona en inglés.

## Comprobar accesibilidad

**Comprobar accesibilidad** (ficha **Revisar**) abre una pequeña ventana que enumera lo que en el libro es difícil de usar para algunos lectores, o para que un lector de pantalla (un programa que lee la pantalla en voz alta) pueda decir. Cada línea empieza con **Error**, **Advertencia** o **Sugerencia**. Haga clic en una línea para ir a la celda.

Busca:

- Un gráfico o una imagen sin **texto alternativo** (las palabras que dicen lo que muestra). Esto es un error.
- Una tabla sin fila de encabezado, por lo que sus columnas no tienen nombres. Esto es un error.
- **Celdas combinadas**, que son difíciles de recorrer con el teclado. Esto es una advertencia.
- Texto difícil de leer sobre el color de fondo, como texto gris claro sobre blanco. JeffCrunch mide cuánto destaca el texto frente al relleno. El texto pequeño necesita una relación de al menos 4,5 a 1, y el texto grande (18 puntos, o 14 puntos en negrita) necesita 3 a 1. Nombra la primera celda de cada combinación de fuente y relleno. Esto es una advertencia.
- Una hoja que aún tiene el nombre que se le dio, como Hoja1. Esto es una sugerencia.

Presione **Comprobar de nuevo** después de hacer cambios.
