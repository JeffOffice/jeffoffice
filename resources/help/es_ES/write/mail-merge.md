# Combinación de correspondencia, sobres y etiquetas
<!-- keywords: combinación de correspondencia, combinar, correspondencia, destinatarios, lista de direcciones, carta modelo, etiquetas, sobres, campo de combinación, bloque de direcciones, línea de saludo, reglas, si entonces si no, omitir registro, registro siguiente, vista previa de resultados, finalizar y combinar, directorio, mensajes de correo electrónico, csv, xlsx, ods, vcard, combinar correo, mezcla de correspondencia, lista de contactos -->

Una **combinación de correspondencia** crea muchas copias de una misma carta, cada una con los datos de una persona distinta. Usted escribe la carta una sola vez. Guarda los datos de las personas en una **lista**. JeffWrite pone el nombre y la dirección de la primera persona en la primera copia, los de la segunda persona en la segunda copia, y así sucesivamente.

Las herramientas están en la ficha **Correspondencia**. Para seguir un proceso guiado, haga clic en **Correspondencia > Iniciar combinación de correspondencia > Asistente paso a paso para combinación de correspondencia**.

## Elija qué va a crear

Haga clic en **Correspondencia > Iniciar combinación de correspondencia** y elija una opción:

- **Cartas**: una copia de la página para cada persona. Cada copia empieza en una página nueva.
- **Mensajes de correo electrónico**: un mensaje para cada persona. Consulte «Enviar mensajes de correo electrónico» más abajo.
- **Sobres** y **Etiquetas**: una página preparada para ellos. Consulte las secciones siguientes.
- **Directorio**: una sola lista de todas las personas, sin salto de página entre ellas. Úselo para una lista de teléfonos o un catálogo.
- **Borrar tipo de combinación** convierte el documento de nuevo en un documento normal.

JeffWrite guarda el tipo de combinación y el nombre de la lista dentro del fichero al guardarlo. Al abrir el fichero otra vez, la combinación sigue ahí.

## Elija los destinatarios

Un **destinatario** es una persona que recibe una copia. Haga clic en **Correspondencia > Seleccionar destinatarios**:

- **Escribir una nueva lista**: escriba las personas en la ventana que se abre. Haga clic en **Nueva entrada** para cada una. JeffWrite le ofrece entonces guardar la lista como fichero .csv.
- **Usar una lista existente**: elija un fichero que ya sea una lista. Puede ser un fichero .csv o un fichero de texto con tabulaciones (una fila por persona, y la primera fila contiene los nombres de las columnas), una hoja de cálculo (.xlsx, .xls u .ods, de JeffCrunch u otro programa) o un fichero de contactos (.vcf).
- **Seleccionar de los contactos**: elija un fichero de contactos (.vcf). La mayoría de las agendas y teléfonos pueden guardar sus contactos en un fichero de este tipo.

Después, haga clic en **Correspondencia > Editar lista de destinatarios** para revisar la lista:

- Desmarque una fila para dejar fuera a esa persona.
- Haga clic en un encabezado de columna para ordenar.
- Escriba en **Filtrar destinatarios** para mostrar solo las filas que contengan sus palabras.
- **Distinguir destinatarios por** elige la columna que diferencia a una persona de otra, como una dirección de correo electrónico. Cuando vuelve a abrir el documento y se vuelve a leer la lista, las personas que desmarcó vuelven a quedar fuera, incluso si la lista tiene otro orden o personas nuevas. Con **Colocar en la lista**, se recuerdan por el número de fila del fichero de la lista, aunque haya ordenado o filtrado las filas en esta ventana. Si la lista ha cambiado de forma que esto ya no encaja, JeffWrite se lo indica e incluye a todas las personas que no pudo identificar. El documento guarda una forma codificada de cada dirección, no la dirección en sí, pero alguien que tenga el documento y adivine una dirección todavía puede comprobar si acierta. **Inspeccionar documento** (Archivo > Información) elimina el registro de quién ha quedado fuera.
- **Buscar duplicados** desmarca las repeticiones.
- **Validar direcciones** colorea las filas que no tienen dirección o que tienen un código postal que parece incorrecto.

## Coloque los datos en la carta

Un **campo de combinación** es un marcador de posición. Muestra el nombre de una columna entre dobles comillas angulares, como «Nombre», y en cada copia se sustituye por el dato de esa persona.

Haga clic donde debe ir el dato y use **Correspondencia > Escribir e insertar campos**:

- **Insertar campo de combinación** muestra las columnas de su lista. Elija una.
  - **Código de barras desde campo**, en la misma lista, inserta un código de barras cuyo valor es una columna, como un número de pieza o el ISBN de 13 dígitos de un libro. Elija la columna, después el tipo, y consulte la ventana **Insertar código de barras** (lo que escriba allí es solo un ejemplo). La ventana ofrece solo lo que puede contener un campo: el tipo, el tamaño, la altura de las barras, los dígitos y el nivel QR. No incluye el precio de un libro, un complemento, un carácter de control ni el fondo blanco, algo que un campo no puede indicar; el ISBN de un libro es un EAN-13. En el documento, el campo aparece como un código de barras, con un ejemplo del tipo hasta que combine; para cambiar el tipo o el aspecto, haga doble clic en él, o haga clic con el botón derecho y elija **Editar código de barras**. Al combinar, la copia de cada persona recibe un código de barras con el valor propio de esa persona, dibujado como imagen en la línea, de modo que se imprime y se guarda en PDF como cualquier imagen. El valor es solo un dato: las comillas o una barra invertida que contenga no cambian nada. Si un valor está vacío, no hay código de barras. Si un código no puede contener un valor (una letra en un EAN-13, o más de 500 caracteres en Code 128), la copia muestra un breve mensaje entre corchetes en su lugar, y las demás copias no se ven afectadas. Una combinación genera como máximo 2.000 códigos de barras, contando cada valor que no pueda representarse; si indica que hay demasiados, combine con menos personas. En un `.docx` guardado, el campo es un campo DISPLAYBARCODE con la columna dentro, de modo que se conserva al guardar y abrir de nuevo el fichero. Un campo con un valor escrito a mano, o de un tipo que JeffWrite no dibuja (UPC-E, por ejemplo), se deja tal cual. Las combinaciones de correo electrónico insertan el valor como texto.
- **Bloque de direcciones** inserta una dirección postal completa: el nombre, la empresa, la calle y la ciudad. Elija las líneas que desee. Una línea sin datos se omite. También puede indicar un país que se omita, como el suyo. La **Vista previa** muestra la dirección de la primera persona.
- **Línea de saludo** inserta una línea como «Estimado/a Dr. Fenwick:». Usted elige las palabras. La primera opción, «Estimado/a Dr. Fenwick:», usa el tratamiento de la persona. Si una persona no tiene tratamiento, se usan su nombre y apellido, como en «Estimada Nora Pell:». Si una persona no tiene nombre, el saludo dice otra cosa, como «Estimado/a señor/a:», que puede cambiar.
- **Asignar campos** indica al bloque de direcciones y a la línea de saludo qué columna contiene cada dato, cuando sus nombres de columna son poco habituales.

## Reglas

Una **regla** hace que una copia sea distinta para algunas personas. Haga clic en **Reglas** y elija una:

- **Si…Entonces…Si no**: muestra un texto cuando una columna tiene un valor, y otro texto cuando no lo tiene. Por ejemplo, si Provincia es «Madrid», escriba «Bienvenido a casa».
- **Omitir registro si**: deja fuera a una persona cuando una columna cumple una condición que usted fije, como cuando el apellido está vacío.
- **Registro siguiente** y **Registro siguiente si**: usan los datos de la persona siguiente en el mismo lugar. Las etiquetas usan esta regla para poner a una persona distinta en cada etiqueta.
- **Número de registro de combinación** inserta el número de fila de la persona en la lista. **Número de secuencia de combinación** cuenta las copias hechas hasta ese momento.
- **Preguntar** y **Campo para rellenar**: le preguntan algo al terminar la combinación. Preguntar guarda la respuesta con un nombre. Campo para rellenar coloca la respuesta en el texto.
- **Establecer marcador**: asigna un valor a un nombre. Una referencia cruzada a ese nombre muestra el valor.

## Vista previa de los resultados

Haga clic en **Correspondencia > Vista previa de resultados**. Se abre un panel que muestra la página de una persona. Use **Primer registro**, **Registro anterior**, **Registro siguiente** y **Último registro** para recorrer las personas, o escriba un número. **Buscar destinatario** salta a la siguiente persona que tenga sus palabras en algún dato.

**Resaltar campos de combinación** sombrea en gris los campos de combinación de la página para que vea dónde están. Otros campos, como los números de página, no se sombrean. El sombreado se ve en pantalla y nunca se imprime.

Su documento no cambia. **Comprobar si hay errores** le indica cuándo la lista no tiene una columna para un campo, o cuando no se incluye a nadie.

## Finalizar la combinación

Haga clic en **Correspondencia > Finalizar y combinar**:

- **Editar documentos individuales** crea un documento nuevo con la copia de cada persona. Puede cambiarlo antes de imprimirlo.
- **Imprimir documentos** crea ese documento y abre la página **Imprimir**.
- **Guardar como PDF** guarda todas las copias en un solo fichero PDF.
- **Enviar mensajes de correo electrónico** crea un fichero de mensaje .eml por cada persona, en una carpeta que usted elige. Elija la columna que contiene las direcciones de correo electrónico y escriba un asunto. JeffWrite no envía los mensajes. Abra los ficheros en su programa de correo.
- **Guardar cada una como fichero** crea un fichero por cada persona, en una carpeta que usted elige, como documento .docx o como PDF. Nombre los ficheros con columnas entre llaves, como {Apellidos}, {Nombre}; un nombre usado dos veces recibe un número. Un cuadro muestra los nombres de los primeros ficheros y cuántos se crearán. Se le pregunta antes de crear más de 200 ficheros, y una vez si ya hay ficheros con los mismos nombres en la carpeta. Haga clic en **Cancelar** para detenerse entre ficheros: los ficheros ya creados se conservan; cerrar la ventana tiene el mismo efecto. Un fichero que es un vínculo a otro fichero se sustituye por un fichero nuevo, y el fichero al que apuntaba no se modifica. Los ficheros contienen la carta de una sola persona: no contienen la lista, su ubicación ni quién ha quedado fuera. Una combinación genera como máximo 100.000 ficheros.

Las personas que desmarcó en la lista quedan fuera.

Solo se crea un fichero de mensaje para una persona cuya dirección sea una única dirección de correo electrónico. Si una dirección contiene dos (separadas por una coma o un punto y coma), tiene un nombre delante o no tiene dominio, esa persona no recibe ningún mensaje, y JeffWrite le indica cuántas personas son. Un asunto largo se guarda en partes que los programas de correo vuelven a unir.

Durante la combinación aparece una ventana de progreso, y **Cancelar** la detiene. Si una regla hace una pregunta (Preguntar o Campo para rellenar), se le pregunta cada pregunta distinta una vez, hasta 20. Después, el resto usa sus respuestas predeterminadas. Si cancela una pregunta, se detiene toda la combinación. La combinación se detiene, e informa de ello, si generaría más de 10.000 páginas, para que un documento muy largo con una lista muy grande no tarde horas. En ese caso, combine una parte de la lista cada vez.

## Sobres

Haga clic en **Correspondencia > Sobres**. Escriba la dirección de envío y su dirección de remitente, y elija el tamaño del sobre en la lista. Los tamaños aparecen con su anchura y altura. Haga clic en **Agregar al documento** para colocar el sobre en una página propia al principio del documento, o en **Imprimir** para imprimirlo. La página tiene la anchura del lado largo del sobre, que es como la mayoría de las impresoras admiten un sobre.

Para crear sobres en una combinación, haga clic en **Iniciar combinación de correspondencia > Sobres**. Cada sobre tiene un bloque de direcciones.

## Etiquetas

Haga clic en **Correspondencia > Etiquetas**. Escriba la dirección y elija la hoja en la lista. Una hoja se nombra según cuántas etiquetas tiene y el tamaño de cada una. Si su hoja no aparece en la lista, elija **Hoja propia** y escriba las medidas: cuántas etiquetas caben a lo ancho y a lo alto, el tamaño de una etiqueta, el espacio por encima y al lado de la primera etiqueta, y la distancia entre las etiquetas.

- **Página completa de la misma etiqueta** pone la misma dirección en cada etiqueta de la hoja.
- **Una sola etiqueta** pone la dirección en una sola etiqueta, para aprovechar una hoja que ya se usó en parte. Indique la fila y la columna.

Haga clic en **Nuevo documento** para crear un documento que pueda revisar y modificar, o en **Imprimir** para imprimirlo.

Para crear etiquetas en una combinación, haga clic en **Iniciar combinación de correspondencia > Etiquetas**. La primera etiqueta tiene un bloque de direcciones, y cada una de las demás empieza con una regla de **Registro siguiente**. Si cambia la primera etiqueta, haga clic en **Actualizar etiquetas** para copiar el cambio a todas las demás.

## Qué guarda el fichero

El tipo de combinación, el nombre y la ubicación de la lista, el asunto y la columna de correo electrónico se guardan en su fichero .docx. Las personas que desmarcó también se guardan, como códigos breves de la columna que eligió, no como sus nombres o direcciones (solo un fichero .docx conserva esto; Comprobar compatibilidad lo indica para los demás tipos). Sus elecciones en Asignar campos también se guardan. La lista se vuelve a leer de su fichero cada vez. Si el fichero no está donde estaba, JeffWrite busca junto a su documento y, si no lo encuentra, le pide que lo elija.

Una lista puede tener hasta 1.000 columnas y 4.000.000 de celdas en total, y una combinación usa las primeras 100.000 personas. Si un fichero tiene más, JeffWrite usa lo que cabe y se lo indica. Las filas vacías se omiten en la lista.

## Temas relacionados

- [Usar campos](fields)
- [Imprimir](print)
- [Crear y modificar tablas](tables)
