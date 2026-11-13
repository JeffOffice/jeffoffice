# Combinación de correspondencia, sobres y etiquetas
<!-- keywords: combinación de correspondencia, combinar, correspondencia, destinatarios, lista de direcciones, carta modelo, etiquetas, sobres, campo de combinación, bloque de direcciones, línea de saludo, reglas, si entonces si no, omitir registro, registro siguiente, vista previa de resultados, finalizar y combinar, directorio, mensajes de correo electrónico, csv, xlsx, ods, vcard, código de barras, combinar correo, lista de contactos -->

Una **combinación de correspondencia** crea muchas copias de una misma carta, cada una con los datos de una persona distinta. Usted escribe la carta una sola vez. Guarda los datos de las personas en una **lista**. JeffWrite pone el nombre y la dirección de la primera persona en la primera copia, los de la segunda persona en la segunda copia, y así sucesivamente.

Las herramientas están en la ficha **Correspondencia**. Para seguir un proceso guiado, haga clic en **Correspondencia > Iniciar combinación de correspondencia > Asistente paso a paso para combinación de correspondencia**.

## Elija qué va a crear

Haga clic en **Correspondencia > Iniciar combinación de correspondencia** y elija una opción:

- **Cartas**: una copia de la página para cada persona. Cada copia empieza en una página nueva.
- **Mensajes de correo electrónico**: un mensaje para cada persona. Vea «Enviar mensajes de correo electrónico» más abajo.
- **Sobres** y **Etiquetas**: una página preparada para ellos. Vea las secciones siguientes.
- **Directorio**: una sola lista con todas las personas, sin saltos de página entre ellas. Úselo para una lista de teléfonos o un catálogo.
- **Quitar tipo de combinación** convierte el documento de nuevo en un documento normal.

JeffWrite guarda dentro del fichero el tipo de combinación y el nombre de la lista al guardarlo. Al abrir el fichero otra vez, la combinación sigue ahí.

## Elija sus destinatarios

Un **destinatario** es una persona que recibe una copia. Haga clic en **Correspondencia > Seleccionar destinatarios**:

- **Escribir una lista nueva**: escriba las personas en la ventana que se abre. Haga clic en **Nueva entrada** para cada una. Después JeffWrite le ofrece guardar la lista como fichero .csv.
- **Usar una lista existente**: elija un fichero que ya sea una lista. Puede ser un fichero .csv o un fichero de texto con tabulaciones (una fila por persona, y la primera fila con los nombres de las columnas), una hoja de cálculo (.xlsx, .xls o .ods, de JeffCrunch u otro programa), o un fichero de contactos (.vcf).
- **Seleccionar de los contactos**: elija un fichero de contactos (.vcf). La mayoría de las agendas y teléfonos pueden guardar sus contactos en ese formato.

Después, haga clic en **Correspondencia > Editar lista de destinatarios** para revisar la lista:

- Desmarque una fila para dejar fuera a esa persona.
- Haga clic en un encabezado de columna para ordenar.
- Escriba en **Filtrar destinatarios** para mostrar solo las filas que contengan sus palabras.
- **Buscar duplicados** desmarca las repeticiones.
- **Validar direcciones** colorea las filas que no tienen dirección o que tienen un código postal que parece incorrecto.

## Ponga los datos en la carta

Un **campo de combinación** es un marcador de posición. Muestra el nombre de una columna entre comillas angulares dobles, como «Nombre». En cada copia, el campo se sustituye por el dato de esa persona.

Haga clic donde irá el dato y use **Correspondencia > Escribir e insertar campos**:

- **Insertar campo de combinación** muestra las columnas de su lista. Elija una.
  - **Código de barras desde campo**, en la misma lista, inserta un código de barras cuyo valor es una columna, como un número de artículo o el ISBN de 13 dígitos de un libro. Elija la columna y después el tipo, y revise la ventana **Insertar código de barras** (lo que escriba allí es solo un ejemplo). La ventana ofrece solo lo que un campo puede contener: el tipo, el tamaño, la altura de las barras, los dígitos y el nivel QR. No incluye el precio de un libro, un complemento, el carácter de control ni el fondo transparente, que un campo no puede indicar; el ISBN de un libro es un EAN-13. En el documento, el campo muestra el nombre de la columna entre comillas angulares. Al combinar, la copia de cada persona recibe un código de barras con su propio valor, dibujado como imagen en la línea, así que se imprime y se guarda en PDF como cualquier imagen. El valor es solo un dato: las comillas o una barra invertida que contenga no cambian nada. Si un valor está vacío, no hay código de barras. Si un código no puede contener el valor (una letra en un EAN-13, o más de 500 caracteres en Code 128), la copia muestra en su lugar un mensaje breve entre corchetes, y las demás copias no se ven afectadas. Una combinación crea como máximo 2000 códigos de barras, contando cada valor que no se pueda representar; si avisa de que hay demasiados, combine menos personas. En un `.docx` guardado, el campo es un campo DISPLAYBARCODE con la columna dentro, así que se conserva al guardar y volver a abrir el fichero. Un campo con un valor escrito a mano, o de un tipo que JeffWrite no dibuja (UPC-E, por ejemplo), se deja como está. Las combinaciones de correo electrónico insertan el valor como texto.
- **Bloque de direcciones** inserta una dirección postal completa: el nombre, la empresa, la calle y la ciudad. Elija las líneas que quiera. Una línea sin datos se omite. También puede indicar un país que se omita, como el suyo. La **Vista previa** muestra la dirección de la primera persona.
- **Línea de saludo** inserta una línea como «Estimado Dr. Fenwick:». Usted elige las palabras. La primera opción, «Estimado Dr. Fenwick:», usa el tratamiento de la persona. Si una persona no tiene tratamiento, se usan su nombre y apellido, como en «Estimado Nora Pell:». Si una persona no tiene nombre, el saludo dice otra cosa, como «Estimados señores:», que puede cambiar.
- **Asociar campos** indica al bloque de direcciones y a la línea de saludo qué columna contiene cada parte, cuando los nombres de sus columnas son poco habituales.

## Reglas

Una **regla** hace que una copia sea distinta para algunas personas. Haga clic en **Reglas** y elija una:

- **Si…Entonces…Si no**: muestra un texto cuando una columna tiene un valor, y otro texto cuando no lo tiene. Por ejemplo, si Provincia es «Madrid», escriba «Bienvenido a casa».
- **Omitir registro si**: deja fuera a una persona cuando una columna tiene un valor, como cuando el apellido está vacío.
- **Registro siguiente** y **Registro siguiente si**: usan los datos de la persona siguiente en el mismo lugar. Las etiquetas usan esta regla para poner una persona distinta en cada etiqueta.
- **Número de registro de combinación** inserta el número de la fila de la persona en la lista. **Número de secuencia de combinación** cuenta las copias hechas hasta ese momento.
- **Preguntar** y **Campo para rellenar**: le preguntan algo al terminar la combinación. Preguntar guarda la respuesta con un nombre. Campo para rellenar pone la respuesta en el texto.
- **Establecer marcador**: asigna un valor a un nombre. Una referencia cruzada a ese nombre muestra el valor.

## Vea los resultados antes de combinar

Haga clic en **Correspondencia > Vista previa de resultados**. Se abre un panel que muestra la página de una persona. Use **Primer registro**, **Registro anterior**, **Registro siguiente** y **Último registro** para recorrer las personas, o escriba un número. **Buscar destinatario** salta a la siguiente persona que tenga sus palabras en algún dato.

**Resaltar campos de combinación** sombrea en gris los campos de combinación de la página, para que vea dónde están. Los demás campos, como los números de página, no se sombrean. El sombreado se ve en pantalla y nunca se imprime.

Su documento no cambia. **Comprobar si hay errores** le avisa cuando la lista no tiene una columna para un campo, o cuando no se incluye ninguna persona.

## Finalice la combinación

Haga clic en **Correspondencia > Finalizar y combinar**:

- **Editar documentos individuales** crea un documento nuevo con la copia de cada persona. Puede cambiarlo antes de imprimirlo.
- **Imprimir documentos** crea ese documento y abre la página **Imprimir**.
- **Guardar como PDF** guarda todas las copias en un solo fichero PDF.
- **Enviar mensajes de correo electrónico** crea un fichero de mensaje .eml por persona, en una carpeta que usted elija. Elija la columna que contiene las direcciones de correo y escriba un asunto. JeffWrite no envía los mensajes. Ábralos en su programa de correo.

Las personas que desmarcó en la lista quedan fuera.

Solo se crea un fichero de mensaje para una persona cuya dirección sea una única dirección de correo. Si una dirección contiene dos (con una coma o un punto y coma entre ellas), tiene un nombre añadido o no tiene dominio, esa persona no recibe ningún mensaje, y JeffWrite le indica cuántas son. Un asunto largo se guarda en partes que los programas de correo vuelven a unir.

Una combinación muestra una ventana de progreso, y **Cancelar** la detiene. Si una regla hace una pregunta (Preguntar o Campo para rellenar), se le pregunta cada pregunta distinta una sola vez, hasta un máximo de 20. Después, el resto usa sus respuestas predeterminadas. Si cancela una pregunta, se detiene toda la combinación. La combinación se detiene, y se lo indica, si fuera a crear más de 10 000 páginas, así un documento muy largo con una lista muy larga no tarda horas. Combine una parte de la lista cada vez.

## Sobres

Haga clic en **Correspondencia > Sobres**. Escriba la dirección de entrega y su dirección de remitente, y elija el tamaño del sobre en la lista. Los tamaños se indican por su ancho y alto. Haga clic en **Agregar al documento** para colocar el sobre en una página propia al principio del documento, o en **Imprimir** para imprimirlo. La página tiene el ancho del lado largo del sobre, que es como la mayoría de las impresoras lo admiten.

Para crear sobres para una combinación, haga clic en **Iniciar combinación de correspondencia > Sobres**. Cada sobre tiene un bloque de direcciones.

## Etiquetas

Haga clic en **Correspondencia > Etiquetas**. Escriba la dirección y elija la hoja en la lista. Una hoja se nombra según cuántas etiquetas tiene y lo grande que es cada una. Si su hoja no aparece en la lista, elija **Hoja propia** y escriba las medidas: cuántas etiquetas caben a lo ancho y a lo alto, el tamaño de una etiqueta, el espacio por encima y al lado de la primera etiqueta, y la distancia entre etiquetas.

- **Página completa de la misma etiqueta** pone la misma dirección en todas las etiquetas de la hoja.
- **Una sola etiqueta** pone la dirección en una sola etiqueta, para aprovechar una hoja que ya está parcialmente usada. Indique la fila y la columna.

Haga clic en **Nuevo documento** para crear un documento que pueda revisar y modificar, o en **Imprimir** para imprimirlo.

Para crear etiquetas para una combinación, haga clic en **Iniciar combinación de correspondencia > Etiquetas**. La primera etiqueta tiene un bloque de direcciones, y cada una de las demás empieza con una regla **Registro siguiente**. Si cambia la primera etiqueta, haga clic en **Actualizar etiquetas** para copiar el cambio a todas las demás.

## Qué guarda el fichero

El tipo de combinación, el nombre y la ubicación de la lista, el asunto y la columna de correo electrónico se guardan en su fichero .docx. Las personas que desmarcó y sus opciones de Asociar campos no se guardan; se vuelven a pedir después de abrir el fichero. La lista se vuelve a leer de su fichero cada vez. Si el fichero ya no está donde estaba, JeffWrite busca junto a su documento y, si no lo encuentra, le pide que lo elija.

Una lista puede tener hasta 1000 columnas y 4 000 000 de celdas en total, y una combinación usa las primeras 100 000 personas. Si un fichero tiene más, JeffWrite usa lo que cabe y se lo indica. Las filas vacías se omiten de la lista.

## Temas relacionados

- [Usar campos](fields)
- [Imprimir](print)
- [Crear y modificar tablas](tables)
