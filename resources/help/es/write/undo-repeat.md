# Deshacer, rehacer y repetir
<!-- keywords: deshacer, rehacer, repetir, f4, ctrl+y, historial, retroceder, volver a hacer, última acción, paso -->

**{key:Undo}** anula tu último cambio, y **{key:Redo}** lo vuelve a aplicar. Ambos están en la barra de herramientas rápida, en la parte superior. Cada comando que das es un paso, y una serie de palabras escritas es un solo paso.

## Retroceder varios pasos a la vez

Cada uno de **Deshacer** y **Rehacer** tiene una flecha a su lado. Haga clic en la flecha para ver los pasos, del más reciente al más antiguo, con un máximo de 100 en la lista. Un paso lleva el nombre del comando que lo hizo, como «Negrita» o «Escritura», y «Edición» cuando el cambio procede de otro lugar. Mueva el puntero hacia abajo en la lista para marcar cuántos pasos quiere y haga clic para aplicarlos. Si elige el tercer paso, se deshacen los tres primeros. Los pasos deshechos permanecen en la lista de Rehacer hasta que haga un cambio nuevo.

## Repetir el último comando

**F4** vuelve a aplicar su último comando sobre lo que está seleccionado ahora. Por ejemplo, ponga en negrita una palabra, seleccione otra palabra y pulse **F4**. La segunda palabra quedará en negrita, y un solo **Deshacer** la devuelve a su estado anterior. Repetir siempre es un paso propio, incluso justo después de escribir.

Los comandos que activan o desactivan algo, como negrita, cursiva, subrayado, viñetas y numeración, se repiten como el resultado que obtuvo, no como un interruptor. Si puso en negrita una palabra, **F4** pone en negrita la siguiente selección, aunque parte de ella ya esté en negrita, y una selección que ya está en negrita se deja tal cual. Si quitó la negrita a una palabra, **F4** quita la negrita a la siguiente selección.

Repetir funciona con:

- Formato: negrita, cursiva, subrayado, tachado, superíndice, subíndice, fuente, tamaño de fuente, color de fuente, resaltado, sombreado de texto, cambiar mayúsculas y minúsculas, y borrar formato.
- Opciones de párrafo: alineación, sangrías, espaciado, viñetas, numeración, bordes y sombreado.
- Estilos de la galería, además de Normal y los estilos de título.
- Comandos de tabla que insertan, eliminan, combinan, alinean o establecen bordes.
- Saltos de página y páginas en blanco, y los caracteres especiales que inserte con un comando.
- Escritura: después de escribir algunas palabras, **F4** vuelve a escribir las mismas palabras donde está el cursor. Solo se conservan las palabras de una serie, hasta 1000 letras, y una serie termina cuando mueve el cursor. La serie es el texto tal como quedó después de las correcciones, como la Autocorrección, de modo que al repetir se escriben esas palabras tal como están. Retroceso reduce la serie en las letras que borra.

Repetir no funciona con abrir y guardar, cambiar la vista, Buscar y reemplazar, copiar y pegar, ni con comandos que abren una ventana para preguntarle algo. Esos comandos nunca son el «último comando» para Repetir, de modo que **F4** sigue repitiendo el formato que aplicó antes de ellos.

La información emergente del botón de la barra de herramientas rápida indica qué haría, por ejemplo «Repetir negrita (F4)». Repetir sigue las mismas reglas que el comando que repite: con el Control de cambios activado, la repetición se registra como un cambio; en un documento abierto solo para lectura, o protegido, no hace nada y lo indica en la barra de estado. Cada documento tiene su propio último comando, así que Repetir nunca traslada un comando de un documento a otro.

Repetir funciona donde esté editando. En un cuadro de texto o en una forma, escribe dentro del cuadro y aplica los comandos que el cuadro tenga; un comando que el cuadro no tenga no se repite allí, y la barra de estado lo indica. En un encabezado, un pie de página, una nota o una celda de tabla, funciona en ese lugar.

**Ctrl+Y** es **Rehacer** mientras tenga pasos que rehacer. Cuando no tenga ninguno, **Ctrl+Y** repite como **F4**.
