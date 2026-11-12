# Contar palabras, comprobar y buscar palabras
<!-- keywords: celdas combinadas, texto del vínculo, recuento de palabras, contar palabras, accesibilidad, comprobar accesibilidad, texto alternativo, fila de encabezado, lector de pantalla, contraste, sinónimos, sinónimo, traducir, traducción, libretranslate, recuento de palabras, contar palabras, caracteres, párrafos, páginas, selección, notas al pie, notas al final, idioma, idioma de revisión, establecer idioma, no comprobar ortografía, idioma de ortografía, barra de estado -->

Estas herramientas están en la pestaña **Revisar**.

## Contar las palabras

1. Haga clic en **Recuento de palabras** en el grupo **Revisión**.
2. El cuadro muestra las páginas, las palabras, los caracteres sin espacios, los caracteres con espacios y los párrafos.

Si antes selecciona un texto, el cuadro cuenta solo ese texto. La barra de estado, en la parte inferior de la ventana, también lo muestra, como «12 de 340 palabras».

Marque **Incluir notas al pie y notas al final** para contarlas también. El texto oculto nunca se cuenta, tampoco el texto que un cambio registrado eliminó.

## Establecer el idioma del texto

JeffWrite comprueba la ortografía con el diccionario del idioma en que está el texto. Si marca una cita en francés como francés, sus palabras se comprueban en francés.

1. Seleccione el texto.
2. En el grupo **Idioma**, haga clic en **Establecer idioma de revisión**. También puede hacer clic en el nombre del idioma en la barra de estado.
3. Elija un idioma y haga clic en **Aceptar**.

El idioma se guarda en el archivo, de modo que otros programas también lo conocen. La barra de estado muestra el idioma del texto donde está el cursor.

## No comprobar algunos textos

Hay textos que no conviene comprobar: un código, un nombre o una línea en un idioma inventado. Selecciónelos y haga clic en **No comprobar ortografía** en el grupo **Idioma**. Las líneas rojas onduladas desaparecen. Haga clic de nuevo para volver a comprobar el texto.

## Buscar una palabra en el diccionario de sinónimos

Un diccionario de sinónimos enumera palabras que significan casi lo mismo.

1. Haga clic en una palabra o selecciónela.
2. Haga clic en **Sinónimos** en el grupo **Revisión**, o presione **Mayús+F7**.
3. Elija una palabra de la lista y haga clic en **Reemplazar**. Para buscar una de las palabras de la lista, haga clic en **Buscar**. También puede escribir otra palabra en el cuadro y presionar **Entrar**.

La lista procede de un archivo de su equipo. No se usa ninguna conexión a internet.

## Traducir texto

1. Seleccione el texto. Si no selecciona nada, se usa todo el documento.
2. Haga clic en **Traducir** en el grupo **Idioma**.
3. Elija el idioma al que desea traducir.
4. Haga clic en **Aceptar**.

Después se abre su navegador web con un servicio de traducción llamado LibreTranslate, con su texto. No se envía nada hasta que haga clic en **Aceptar**. Puede cambiar la dirección del servicio en el cuadro, por ejemplo por una que usted mismo administre. Solo se usan direcciones web (http o https). Se envían como máximo 5000 letras.

## Comprobar la accesibilidad

Algunas personas leen con un lector de pantalla, que lee el documento en voz alta. Otras tienen dificultades para ver el texto pálido. La comprobación de accesibilidad encuentra lo que dificulta la lectura del documento a esas personas.

1. Haga clic en **Comprobar accesibilidad** en el grupo **Revisión**.
2. A la derecha se abre una lista. Cada línea indica qué está mal. Haga clic en una línea para ir al lugar.
3. Corrija el problema. Haga clic en **Volver a comprobar**.

Busca lo siguiente:

- **Una imagen sin texto alternativo.** El texto alternativo es una descripción breve de una imagen. Para agregarlo, haga clic con el botón derecho en la imagen y elija su texto alternativo.
- **Una tabla sin fila de encabezado.** La fila de encabezado contiene los nombres de las columnas.
- **Una celda de tabla en blanco.**
- **Un título que salta un nivel**, como Título 3 justo después de Título 1.
- **Texto difícil de leer**, porque su color es demasiado parecido al color del fondo.
- **Una imagen flotante en la página** en lugar de estar en línea con el texto.
- **Una tabla con celdas combinadas.** Una celda combinada es una sola celda que abarca dos o más columnas o filas. Un lector de pantalla puede perderse en una tabla así. Esto se indica una vez por cada tabla.
- **Un vínculo cuyas palabras no dicen nada sobre su destino**, como «haga clic aquí» o «leer más», o una dirección web mostrada tal cual. Quien escucha la lista de vínculos de una página necesita palabras que nombren la página.

También puede iniciar la comprobación desde **Archivo > Información**: haga clic en **Comprobar accesibilidad**.
