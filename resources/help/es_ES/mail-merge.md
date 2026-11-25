# Combinación de correspondencia y combinación de catálogo
<!-- keywords: combinación de correspondencia, combinar, destinatarios, lista de direcciones, etiquetas, cartas modelo, csv, xlsx, vcard, campo de combinación, bloque de direcciones, catálogo, lista de productos, combinación de correo electrónico, vista previa -->

La combinación de correspondencia rellena una publicación con datos de una lista. Usted escribe una carta una sola vez, y JeffPub crea una copia para cada persona, con el nombre y la dirección de esa persona en su lugar. Una combinación de catálogo repite un mismo diseño para cada artículo de una lista.

Las herramientas están en la ficha **Correspondencia**. Para seguir un proceso guiado, haga clic en **Correspondencia > Iniciar > Asistente paso a paso para combinación de correspondencia** para abrir el panel **Combinación de correspondencia**.

## Elegir los destinatarios

Haga clic en **Correspondencia > Iniciar > Seleccionar destinatarios**:

- **Usar una lista existente…**: elija un fichero CSV, un fichero de texto delimitado por tabulaciones, una hoja de cálculo .xlsx (la primera hoja) o un fichero de contactos .vcf. En un fichero CSV, de texto o de hoja de cálculo, la primera fila debe contener los nombres de las columnas.
- **Seleccionar de contactos…**: elija un fichero de contactos (.vcf o .vcard). La mayoría de las agendas y teléfonos pueden guardar sus contactos en uno; cada contacto se convierte en un destinatario, con columnas de nombre, dirección, teléfono y correo electrónico.
- **Escribir una lista nueva…**: complete la ventana **Nueva lista de direcciones**, usando **Nueva entrada** para cada persona. Después, JeffPub le ofrece guardar la lista como fichero CSV.

Para revisar la lista, haga clic en **Correspondencia > Iniciar > Editar lista de destinatarios**. En la ventana **Destinatarios de combinación de correspondencia**, desmarque una fila para omitir a esa persona, haga clic en un encabezado para ordenar y escriba en **Filtrar destinatarios** para reducir la lista. **Distinguir destinatarios por** elige una columna que diferencie a las personas; la lista se guarda con sus filas desmarcadas en su publicación, y al elegir de nuevo el mismo fichero de lista, esas mismas personas vuelven a quedar excluidas, aunque haya ordenado las filas antes. **Buscar duplicados** desmarca los repetidos, y **Validar direcciones** resalta las filas con una dirección incompleta o un código postal extraño.

## Agregar campos de combinación

Un campo de combinación es un marcador de posición, como «Nombre», que se convierte en un valor real para cada persona. Haga clic en un cuadro de texto (si no existe, JeffPub agrega uno) y use **Correspondencia > Escribir e insertar campos**:

- **Bloque de direcciones**: una dirección postal formada con columnas llamadas, por ejemplo, Nombre, Apellidos y Dirección 1.
- **Línea de saludo**: «Estimado» y el nombre de la persona, o «Estimado amigo:» si no hay nombre.
- **Insertar campo de combinación**: elija cualquier columna.
- **Campo de imagen**: elija una columna con nombres de fichero de imagen. JeffPub agrega un marco vacío y busca los ficheros junto a su lista. Después de guardar la publicación y volver a abrirla, una imagen cuyo fichero está fuera de la carpeta de la publicación no se muestra hasta que vuelva a elegir la lista de destinatarios (**Correspondencia > Seleccionar destinatarios > Usar una lista existente…**). Entonces se muestran las imágenes de la carpeta de la lista y de las carpetas que contiene, hasta que cierre la publicación. JeffPub no recuerda esto para la próxima vez. También puede guardar los ficheros de imagen en la carpeta de la publicación, o en una carpeta dentro de ella; en ese caso se muestran en cuanto se abre. Así, una publicación que alguien le envíe no leerá por sí sola imágenes de su equipo. La imagen de cada registro se muestra al previsualizar los resultados, al imprimir, al guardar un PDF o al combinar en una publicación nueva o en un catálogo.

## Mostrar un número, un precio, una fecha o una hora a su manera

Un campo de combinación muestra el valor tal como aparece en su lista. Para cambiarlo, haga clic junto al campo (o selecciónelo, o selecciónelos varios) y haga clic en **Correspondencia > Escribir e insertar campos > Dar formato al campo de combinación…**. En **Mostrar el valor como**, elija:

- **Número**, con de 0 a 4 **Posiciones decimales**. Por ejemplo, 1234,5 con 1 posición decimal se muestra como 1.234,5.
- **Moneda**, con el símbolo monetario de su región: 1234,5 se muestra como 1.234,50 € donde ese sea el símbolo.
- **Porcentaje**: 0,256 se muestra como 25,6 % con 1 posición decimal.
- **Fecha**, con un **Estilo** como Corto, Largo, Año, mes, día, Mes y día o Mes y año.
- **Hora**, con un **Estilo** de 24 horas, 12 horas o corto.

La línea **Ejemplo** muestra el resultado de su elección. Los valores se leen y se muestran en el idioma del texto del campo (inglés de EE. UU. si no se ha establecido ninguno), no en el idioma de su equipo. En inglés de EE. UU., 03/04/2026 es el 4 de marzo, y en un idioma que escribe primero el día, como el alemán o el inglés británico, es el 3 de abril; en alemán, 1.234,5 es un número con coma decimal. Un año de dos cifras se lee como 2000 a 2029 para 00 a 29, y como 1930 a 1999 para 30 a 99. Un número entre paréntesis, como (500), es negativo. Un valor que no es un número, una fecha o una hora del tipo elegido se muestra exactamente como está en su lista: una palabra, «nan» o «inf» (una persona puede llamarse Nan), un valor vacío, una hora con formato de fecha, o una fecha con formato de hora. La vista previa y la combinación final muestran el valor con formato. La elección es un paso de **Deshacer** y se guarda con la publicación.

## Vista previa de los resultados

Haga clic en **Correspondencia > Vista previa de resultados > Vista previa de resultados** para ver datos reales en lugar de los campos. Use **Primer registro**, **Registro anterior**, **Registro siguiente** y **Último registro** para recorrer las personas, o **Buscar destinatario** para ir a una coincidencia.

## Finalizar la combinación

Haga clic en **Correspondencia > Finalizar > Finalizar y combinar**:

- **Combinar en impresora…** abre la página **Imprimir**. Marque **Imprimir todos los registros de la combinación de correspondencia** y, después, imprima.
- **Combinar en nueva publicación** crea una publicación nueva con una copia de sus páginas para cada persona.
- **Combinar en PDF…** guarda un solo PDF con las páginas de todas las personas.
- **Combinar en correo electrónico…** crea un fichero de correo electrónico .eml por persona, con la primera página como imagen. Elija la columna del correo electrónico, escriba un asunto y seleccione una carpeta. JeffPub no envía los mensajes; ábralos en su programa de correo.
- **Guardar cada uno como fichero…** crea una publicación .jpub por persona, en la carpeta que elija. Nombre los ficheros con columnas entre llaves, como {Apellidos}, {Nombre}. Se le pedirá confirmación si son más de 200 ficheros, y una vez si ya hay ficheros con los mismos nombres. Haga clic en **Cancelar** para detenerse entre ficheros.
- **Exportar lista de destinatarios…** guarda la lista como fichero CSV.

## Combinación de catálogo

Haga clic en **Insertar > Páginas > Páginas de catálogo** para abrir el panel **Combinación de catálogo**.

1. Elija la lista de productos, como se explicó antes.
2. Haga clic en **Insertar área de catálogo**. Aparece un cuadro con la etiqueta «Área de catálogo». Establezca sus **Filas** y **Columnas**, así como su posición y tamaño. Coloque los campos de un artículo en la primera celda haciendo doble clic en ellos en la lista. Se repiten en las demás celdas. Para una imagen, elija la versión «(picture)» del campo.
3. Previsualice con las flechas. Cada página muestra tantos artículos como celdas tenga el área. Después, combine en una publicación nueva, en un PDF o en la impresora.

**Quitar área de catálogo** elimina el área.

## Temas relacionados

- [Imprimir una publicación](printing)
- [Agregar y dar formato a cuadros de texto](text-boxes)
- [La ficha Correspondencia](tab-mailings)
