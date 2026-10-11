# Combinación de correspondencia y combinación de catálogo
<!-- keywords: combinación de correspondencia, combinar correspondencia, combinar, destinatarios, lista de direcciones, etiquetas, cartas modelo, cartas personalizadas, csv, xlsx, vcard, campo de combinación, bloque de direcciones, catálogo, lista de productos, combinación de correo electrónico, vista previa -->

La combinación de correspondencia rellena una publicación con datos de una lista. Escribe una carta una sola vez y JeffPub crea una copia para cada persona, con su nombre y su dirección en el lugar correspondiente. La combinación de catálogo repite un mismo diseño para cada elemento de una lista.

Las herramientas están en la pestaña **Correspondencia**. Para seguir un proceso guiado, haga clic en **Correspondencia > Iniciar > Combinación de correspondencia > Asistente para combinación de correspondencia paso a paso** para abrir el panel **Combinación de correspondencia**.

## Elegir los destinatarios

Haga clic en **Correspondencia > Iniciar > Seleccionar destinatarios**:

- **Usar una lista existente…**: elija un archivo CSV, un archivo de texto separado por tabulaciones, una hoja de cálculo .xlsx (la primera hoja) o un archivo de contactos .vcf. En un archivo CSV, de texto o de hoja de cálculo, la primera fila debe contener los nombres de las columnas.
- **Seleccionar de los contactos…**: elija un archivo de contactos (.vcf o .vcard). La mayoría de las libretas de direcciones y los teléfonos pueden guardar sus contactos en uno de estos archivos; cada contacto se convierte en un destinatario, con columnas de nombre, dirección, teléfono y correo electrónico.
- **Escribir una nueva lista…**: rellene la ventana **Nueva lista de direcciones** y use **Nueva entrada** para cada persona. Después, JeffPub le ofrece guardar la lista como archivo CSV.

Para revisar la lista, haga clic en **Correspondencia > Iniciar > Editar lista de destinatarios**. En la ventana **Destinatarios de combinación de correspondencia**, desactive una fila para omitir a esa persona, haga clic en un encabezado para ordenar y escriba en **Filtrar destinatarios** para acotar la lista. **Buscar duplicados** desactiva las filas repetidas y **Validar direcciones** resalta las filas a las que les falta la dirección o que tienen un código postal extraño.

## Agregar campos de combinación

Un campo de combinación es un marcador de posición, como «Nombre», que se convierte en un valor real para cada persona. Haga clic en un cuadro de texto (si no lo hace, JeffPub agrega uno) y use **Correspondencia > Escribir e insertar campos**:

- **Bloque de direcciones**: una dirección postal formada a partir de columnas con nombres como Nombre, Apellidos y Dirección 1.
- **Línea de saludo**: "Estimado" y el nombre de la persona, o "Estimado amigo:" si no hay nombre.
- **Insertar campo de combinación**: elija cualquier columna.
- **Campo de imagen**: elija una columna con nombres de archivo de imagen. JeffPub agrega un marco vacío y busca los archivos junto a la lista. Después de guardar la publicación y volver a abrirla, una imagen cuyo archivo esté fuera de la carpeta de la publicación no se muestra hasta que vuelva a elegir la lista de destinatarios (**Correspondencia > Seleccionar destinatarios > Usar una lista existente…**). Entonces, las imágenes de la carpeta de la lista, y de las carpetas que contiene, se muestran hasta que cierre la publicación. JeffPub no lo recuerda para la próxima vez. También puede guardar los archivos de imagen en la carpeta de la publicación, o en una carpeta dentro de ella, y se mostrarán en cuanto la abra. Así se evita que una publicación que alguien le envíe lea por su cuenta imágenes de su equipo. La imagen de cada registro se muestra al obtener la vista previa de los resultados, imprimir, guardar un PDF o combinar en una nueva publicación o en un catálogo.

## Obtener una vista previa de los resultados

Haga clic en **Correspondencia > Vista previa de resultados > Vista previa de resultados** para ver los datos reales en lugar de los campos. Use **Primer registro**, **Registro anterior**, **Registro siguiente** y **Último registro** para recorrer las personas, o **Buscar destinatario** para ir a una coincidencia.

## Finalizar la combinación

Haga clic en **Correspondencia > Finalizar > Finalizar y combinar**:

- **Combinar en impresora…** abre la página **Imprimir**. Active **Imprimir todos los registros de la combinación de correspondencia** e imprima.
- **Combinar en nueva publicación** crea una publicación nueva con una copia de sus páginas para cada persona.
- **Combinar en PDF…** guarda un único PDF con las páginas de todas las personas.
- **Combinar en correo electrónico…** crea un archivo de correo electrónico .eml por persona, con la primera página como imagen. Elija la columna de correo electrónico, escriba un asunto y elija una carpeta. JeffPub no envía los mensajes. Abra los archivos en su programa de correo electrónico.
- **Exportar lista de destinatarios…** guarda la lista como archivo CSV.

## Combinación de catálogo

Haga clic en **Insertar > Páginas > Páginas de catálogo** para abrir el panel **Combinación de catálogo**.

1. Elija la lista de productos, como se ha indicado antes.
2. Haga clic en **Insertar área de catálogo**. Aparece un cuadro con la etiqueta "Área de catálogo". Establezca sus **Filas** y **Columnas**, así como su posición y tamaño. Coloque los campos de un elemento en la primera celda haciendo doble clic en ellos en la lista. Se repiten en las demás celdas. Para una imagen, elija la versión "(imagen)" del campo.
3. Obtenga una vista previa con las flechas. Cada página muestra tantos elementos como celdas tiene el área. Después, combine en una nueva publicación, en un PDF o en la impresora.

**Quitar área de catálogo** elimina el área.

## Temas relacionados

- [Imprimir una publicación](printing)
- [Agregar cuadros de texto y aplicarles formato](text-boxes)
- [La pestaña Correspondencia](tab-mailings)
