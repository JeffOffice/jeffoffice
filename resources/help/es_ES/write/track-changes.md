# Control de cambios: aceptar o rechazar cambios
<!-- keywords: comparar, comparar documentos, dos versiones, qué ha cambiado, control de cambios, cambios registrados, revisiones, marcas de revisión, aceptar, rechazar, aceptar todo, rechazar todo, mostrar para revisión, marcas simples, todas las marcas, sin marcas, original, mostrar marcas, globos, panel de revisión, barra de cambios, inserción, eliminación, ctrl+mayús+e, autor, revisor, nombre de usuario, ediciones de otra persona -->

**Control de cambios** guarda un registro de cada modificación que usted hace, de modo que usted u otra persona puedan ver qué ha cambiado y decidir si lo conservan. Utilícelo cuando edite un documento que pertenece a otra persona o que alguien va a revisar después.

## Activar y desactivar el control de cambios

Haga clic en **Revisar > Control de cambios**, o pulse **Ctrl+Mayús+E**. La barra de estado, en la parte inferior de la ventana, indica si está activado. Al hacer clic en esas palabras, cambia de estado. El ajuste se guarda en el documento, así que sigue activado cuando el documento vuelve de otra persona.

Mientras está activado:

- **Lo que escribe** se marca como inserción.
- **Lo que elimina** no desaparece. Permanece en el texto, tachado, para que se pueda recuperar. La única excepción es el texto que usted mismo escribió mientras el control estaba activado: al eliminarlo, simplemente desaparece, porque nadie más lo había visto.
- **Un salto de párrafo que inserta** (Entrar) se marca como insertado, y uno que elimina (Retroceso al principio de un párrafo) se marca como eliminado.
- **Un cambio de aspecto** (negrita, tipo de letra, tamaño, color, alineación, sangrías o estilo) se registra junto con el aspecto que tenía el texto antes.

Cada cambio se guarda con el nombre de quien lo hizo y la fecha y hora. El nombre es el que aparece en **Archivo > Opciones**, en **Nombre de usuario**. Si es incorrecto, cámbielo allí.

Cada modificación es un solo paso, así que **Ctrl+Z** la deshace como cualquier otra edición.

## Aspecto de los cambios

De forma predeterminada, los cambios de cada persona tienen un color propio. La primera persona cuyo cambio muestre el documento recibe el primer color, la siguiente el segundo, y así sucesivamente; cada persona conserva el mismo color mientras el documento esté abierto. El texto insertado aparece subrayado con el color de la persona, y el texto eliminado, tachado. El texto que se ha movido aparece en verde: con doble tachado donde estaba y con doble subrayado donde ha llegado. El texto cuyo formato ha cambiado aparece en negrita. Una barra en el margen izquierdo marca cada línea que tiene algún cambio.

Puede cambiar todo esto. Haga clic en **Revisar > Control de cambios > Opciones de control de cambios** para abrir el cuadro **Opciones de control de cambios**. En **Marcas**, cada tipo de cambio tiene un **Aspecto** y un **Color**:

- **Inserciones**: subrayado, doble subrayado, tachado, negrita, cursiva, solo color o (ninguno).
- **Eliminaciones**: tachado, doble tachado, oculto, cursor o solo color. Con **Oculto**, las palabras eliminadas no aparecen en la línea. Con **Cursor**, tampoco aparecen, y un pequeño ^ indica dónde estaban. El texto trasladado también se oculta cuando elige **Globos** para las eliminaciones, pero no en los demás casos.
- **Movido desde** y **Movido a**: los aspectos y colores del texto movido. Están disponibles mientras **Registrar movimientos** esté marcada. Si la desmarca, el texto movido se muestra como una eliminación y una inserción normales.
- **Formato**: negrita, solo color o (ninguno). Si desmarca **Registrar formato**, los cambios de formato no se marcan en el texto.
- **Barras de cambios**: borde izquierdo, borde derecho, borde exterior (el lado derecho de las páginas impares y el izquierdo de las pares) o (ninguno), y el color de la barra.

Cada cuadro **Color** ofrece **Por autor** (el color propio de cada persona) o un solo color para todas. Estas opciones son suyas. Se guardan en este equipo, no en el documento, se aplican a todos los documentos y a todas las ventanas abiertas, y cada ventana cambia en cuanto hace clic en **Aceptar**. Las barras se dibujan igual en pantalla, en una impresión y en un PDF siempre que se muestren las marcas.

## Elegir qué ve: Mostrar para revisión

Haga clic en **Revisar > Mostrar para revisión** y elija una opción:

- **Todas las marcas** muestra todos los cambios en el texto, con las barras en el margen.
- **Marcas simples** muestra el documento tal como quedará con los cambios aceptados, y una barra en el margen junto a cada línea que tenga algún cambio.
- **Sin marcas** muestra el documento tal como quedará con todos los cambios aceptados, y nada más.
- **Original** muestra el documento tal como era antes de los cambios: el texto eliminado vuelve a aparecer, y el texto insertado desaparece.

Estas opciones solo cambian lo que ve. Nunca modifican el documento y no se guardan con él. Un documento que contiene cambios se abre con **Todas las marcas**.

**Revisar > Mostrar marcas** elige qué tipos de cambio se muestran: **Inserciones y eliminaciones**, **Formato** y **Comentarios**. Debajo, elija cómo se muestra el texto eliminado. **Mostrar todas las revisiones insertadas en el texto** lo conserva en el texto, tachado. **Mostrar revisiones en globos** lo quita de la línea y lo pone en un globo en el margen. **Mostrar solo comentarios y formato en globos** mantiene las inserciones y eliminaciones en el texto.

## Aceptar o rechazar cambios

Aceptar un cambio es conservarlo de forma definitiva. Rechazarlo es deshacerlo.

- **Revisar > Aceptar** acepta el cambio en el que está el cursor (o todos los cambios del texto seleccionado) y pasa al siguiente. **Revisar > Rechazar** hace lo mismo, deshaciendo el cambio.
- Haga clic en la flecha situada debajo de **Aceptar** o **Rechazar** para ver más opciones: **Aceptar todos los cambios mostrados**, **Aceptar todos los cambios** y **Aceptar todos los cambios y detener el seguimiento**; y **Rechazar todos los cambios mostrados** y **Rechazar todos los cambios**. «Mostrados» significa los cambios de los tipos que **Mostrar marcas** tiene activados (y, en el panel **Revisiones**, del revisor que haya elegido).
- **Anterior** y **Siguiente**, en el grupo **Cambios**, pasan de un cambio al siguiente sin modificar nada. Con el cursor al inicio absoluto del texto, de un encabezado o de una nota, **Siguiente** va a un cambio que empieza justo ahí.

Cada una de estas acciones es un solo paso, así que **Ctrl+Z** devuelve el documento exactamente a su estado anterior.

Al aceptar una eliminación, el texto desaparece de forma definitiva. Al rechazar una inserción, el texto también desaparece. Al aceptar una inserción, el texto se conserva y se quita la marca de que era nuevo. Al rechazar una eliminación, el texto se conserva y se quita el tachado. Al rechazar un cambio de aspecto, se restaura el aspecto que tenía el texto. El texto movido es un par de cambios, uno en el lugar de origen y otro en el de destino, unidos por el nombre del movimiento. Cuando el par lleva el mismo nombre, como lo guardan otros programas, aceptar o rechazar una de las dos mitades afecta también a la otra, de modo que el texto se mueve de forma definitiva o vuelve a su sitio. Un nombre usado más de una vez, o con una sola mitad, no une nada: cada mitad es un cambio independiente.

## El panel Revisiones

Haga clic en **Revisar > Panel de revisión** para abrir una lista de los cambios del documento: tipo, autor, fecha y palabras. Un clic en uno lo muestra en el texto. Use **Aceptar**, **Rechazar**, **Anterior** y **Siguiente** debajo de la lista, o elija un revisor en el cuadro situado encima para ver solo los cambios de esa persona.

## Cambios en encabezados, pies de página, notas y tablas

Los cambios que otras personas hacen en un encabezado, un pie de página, una nota al pie o una nota al final se muestran y aparecen en la lista igual que los del texto, incluido un encabezado o pie de página que no usa ninguna página (el de las páginas pares cuando el documento no usa páginas pares e impares diferentes): **Siguiente** pasa por encima, y **Aceptar** y **Rechazar** actúan sobre el cambio al que llegó por último. **Siguiente**, **Anterior**, **Aceptar**, **Rechazar** y el panel **Revisiones** llevan al encabezado, pie de página o nota que contiene el cambio, y el orden es primero el texto, después las notas al pie y, por último, los encabezados y pies de página. **Aceptar todos los cambios** y **Rechazar todos los cambios** afectan a todos ellos.

Sus propios cambios en un encabezado, un pie de página, una nota al pie o una nota al final se registran igual que en el texto: escribir, eliminar, pegar, el aspecto del texto y sustituir palabras. **Insertar nota al pie** e **Insertar nota al final** registran la marca en el texto y la nota como un único cambio insertado: **Aceptar** conserva ambas y **Rechazar** quita las dos; si elimina una nota que usted mismo insertó, desaparece sin más. Si elimina una nota que hizo otra persona, la marca del texto y las palabras de la nota se marcan como eliminadas. Si corta o copia texto que contiene una marca de nota y lo pega con el control de cambios activado, la marca pegada recibe una nota nueva propia, una copia limpia de las palabras, registrada como un cambio insertado. Si las palabras de una nota copiada contienen cambios registrados, la nota pegada los conserva tal como se leen: se mantienen las inserciones y se omiten las eliminaciones. Se copian todas las notas del texto que corte o copie; si entre todas superan los 200 000 caracteres, no se corta ni se copia nada y aparece un mensaje que lo indica. En un documento con primera página diferente o con páginas pares e impares diferentes, un cambio va al encabezado o pie de página de la página en la que se encuentra. Un documento que solo puede leer no le permite cambiar un encabezado, un pie de página ni una nota. Al guardar como fichero de texto OpenDocument (.odt), los cambios en encabezados y pies de página se convierten en texto normal, y **Archivo > Información > Comprobar compatibilidad** lo indica. Insertar un encabezado o pie de página que no existía no se marca; las palabras que escriba en él son cambios insertados, y al rechazarlos queda un encabezado vacío, que se ve igual que si no hubiera ninguno.

También se listan los cambios en las tablas: una fila o celda insertada o eliminada, celdas combinadas y un cambio de aspecto de una tabla, una fila o una celda. Una barra en el margen marca las líneas de la fila o celda. Aceptar una fila eliminada, o rechazar una insertada, quita la fila y sus palabras. Una fila en la que todas las celdas tienen el cambio es el cambio de esa fila. Rechazar un cambio de aspecto restaura el aspecto que tenían la tabla, la fila o la celda, y rechazar una combinación vuelve a dividir las celdas. Cuando quitar un cambio rompería la tabla, el cambio se conserva y el mensaje indica cuántos se han conservado. Esto ocurre con una celda combinada con otras hacia abajo, que se conserva y pierde su marca, y con un cambio de la cuadrícula que las filas no pueden seguir, que se conserva como un cambio que puede aceptar. Los cambios dentro de una tabla de una nota al pie se conservan tal como se escribieron: no se listan, y **Aceptar** y **Rechazar** no actúan sobre ellos.

Un movimiento ocupa una línea en la lista y cuenta como un cambio en el recuento de **Aceptar todos los cambios** y **Rechazar todos los cambios**: su otra mitad y las marcas de párrafo que lo acompañaban se resuelven con él. Los cambios dentro de un cuadro de texto se listan uno a uno, después de los del texto y antes de los de las notas al pie. **Siguiente** y **Anterior** abren el cuadro para escribir, con el cambio seleccionado, y **Aceptar** y **Rechazar** resuelven ese cambio, cada uno como un paso. Si mueve el cursor dentro del cuadro abierto, **Siguiente** y **Anterior** continúan desde la posición del cursor. Un cuadro que se resuelve cambio a cambio se guarda sin la copia que leen los programas antiguos, igual que cuando escribe en un cuadro. Se listan así los primeros 200 cuadros que contienen cambios. Un cuadro de texto que forma parte de un grupo, o cuyas palabras continúan en otros cuadros, un cuadro dentro de otro cuadro, un cuadro en un encabezado o en una nota, y los cuadros más allá de los primeros 200 no se pueden abrir de esta forma: el panel **Revisiones** cuenta sus cambios en una línea bajo la lista, y solo **Aceptar todos los cambios** y **Rechazar todos los cambios** los resuelven.

Un cambio en la numeración de un párrafo se lista como **Numerado** o **Numeración cambiada**. Cuando un cambio de aspecto de un párrafo registró la lista y el nivel que tenía, rechazarlo devuelve el párrafo a esa lista y a ese nivel (o lo saca de la lista en la que no estaba). Un cambio que solo conserva la numeración antigua como etiqueta (como lo guardan algunos programas) no indica a qué lista volver: al aceptarlo se mantiene la numeración, y al rechazarlo se quita la numeración solo si la etiqueta está vacía; en caso contrario, el cambio permanece y se cuenta como conservado. Rechazar una numeración añadida devuelve al párrafo lo que le asigna su estilo (un estilo de título que tiene numeración la conserva) y ninguna numeración si el estilo no la tiene. Si la lista a la que el cambio devolvería el párrafo ya no está en el documento, no cambia nada de ese párrafo y el cambio se cuenta como conservado.

Un cambio en una sección (tamaño de página, orientación, márgenes, columnas, numeración de páginas o tipo de salto) se lista como **Sección con formato**, en el párrafo que termina la sección, o al final del texto para la última sección. Aceptarlo mantiene la sección tal como está y elimina el registro, y rechazarlo devuelve las propiedades que guardaba el registro, y las páginas se vuelven a componer de inmediato. Los encabezados y pies de página que nombra una sección no cambian. Si no se pueden leer las propiedades antiguas del registro, rechazar deja la sección y el cambio tal como están, y los cuenta como conservados; no se sustituye nada por una página predeterminada. Cuando **Aceptar todos los cambios** o **Rechazar todos los cambios** también quita un salto de sección o vuelve a poner uno, primero se resuelve el cambio de cada sección, para que no quede ninguno pendiente.

Un cambio que no puede resolverse sin dañar el documento (una tabla que se rompería, una lista o una sección cuyo estado anterior no se puede restaurar, un cuadro de texto demasiado grande para volver a escribirlo) se deja como está, y el mensaje que aparece tras **Aceptar todos los cambios** o **Rechazar todos los cambios** indica cuántos se han conservado.

## Qué cambios se registran y cuáles no

Mientras el control de cambios está activado, se registran estos cambios: escribir, eliminar y pegar texto, también en encabezados, pies de página, notas al pie y notas al final; Entrar; sustituir palabras; insertar una nota al pie o una nota al final; el aspecto del texto y de los párrafos (tipos de letra, tamaños, colores, alineación, espaciado, bordes y estilos); **Cambiar mayúsculas y minúsculas**; símbolos y caracteres especiales; saltos de página y de columna; la marca de párrafo de un salto de sección; y, en las tablas, una fila que inserte (con **Insertar arriba** o **Insertar debajo**, o con Tabulador en la última celda), una fila que elimine, una tabla que elimine y una tabla que inserte. Una fila que inserte o elimine aparece en el panel **Revisiones** como un único cambio, con sus palabras, y **Aceptar** y **Rechazar** la conservan o la quitan. Eliminar una fila que contiene una tabla marca también las filas de esa tabla. Una fila que otra persona insertó y que usted elimina lleva ambos cambios, y **Aceptar** o **Rechazar** la quitan. Una fila que usted insertó y elimina después desaparece de forma definitiva.

Estos cambios se realizan como de costumbre, sin marca, y la barra de estado indica cada vez «Este cambio no se registró como cambio controlado»: viñetas, numeración y demás comandos de listas; **Aumentar sangría** y **Disminuir sangría**; configurar página, columnas, márgenes y números de línea; un número de página u otro campo que se inserta en un encabezado o pie de página; insertar una tabla de contenido, un índice, un campo, un vínculo o un control de contenido; imágenes, gráficos, ecuaciones, cuadros de texto y formas dibujadas; columnas, celdas, combinar, dividir y el aspecto de una tabla; eliminar una fila que forma parte de una celda combinada hacia abajo entre filas; ordenar párrafos o las filas de una tabla; y mover una tabla o una sección. Mientras escribe, el mensaje aparece una sola vez, no con cada tecla. **Deshacer** revierte cualquiera de estos cambios en un solo paso.

Algunos cambios no se realizan en absoluto mientras el control de cambios está activado, porque sin marca dañarían un cambio que sí se registra: en una tabla que tiene una fila insertada o eliminada, **Eliminar columnas**, **Eliminar celdas**, **Combinar celdas**, **Dividir celdas**, **Convertir en texto** y pegar sobre celdas. La barra de estado lo indica. Acepte o rechace antes los cambios de esa tabla, o desactive el control de cambios. En un documento que bloquea el control de cambios activado, de modo que no se pueden aceptar ni rechazar cambios, estos también se realizan, y la barra de estado indica que el cambio no se registró.

Los cambios que otros programas registraron en la numeración y en las secciones aparecen en la lista y se pueden aceptar y rechazar, como se ha descrito.

## Comparar dos versiones

**Comparar** busca las diferencias entre dos versiones de un documento y las muestra como cambios controlados.

1. Abra la versión más antigua. Es el **original**.
2. Haga clic en **Comparar** en el grupo **Comparar** de la ficha **Revisar**.
3. Haga clic en **Examinar** y elija la versión más reciente. Es la versión **revisada**.
4. Escriba el nombre con el que firmar los cambios. Elija las opciones que desee, como ignorar las mayúsculas. **Buscar texto que se ha movido** y **Buscar cambios de formato** están activadas salvo que las desactive.
5. Haga clic en **Aceptar**.

Se abre una ventana nueva con el texto original y los cambios que lo convierten en el texto revisado. Sus dos ficheros no se modifican. Use **Aceptar** y **Rechazar** como con cualquier cambio controlado. Si acepta todos los cambios, obtendrá el texto revisado; si los rechaza todos, obtendrá el texto original.

Comparar examina el texto del cuerpo del documento, sus notas al pie, sus encabezados y pies de página, y las tablas del cuerpo. Todavía no examina las notas al final ni los cuadros de texto. Cuando una diferencia está en un lugar que Comparar no puede mostrar, lo indica con el número de diferencias que no pudo mostrar y de qué tipo, y deja esa parte como estaba en el original.

Las tablas del cuerpo se comparan fila por fila y celda por celda. Una fila que solo existe en la versión revisada se muestra como fila insertada, y una que solo existe en el original, como fila eliminada. Una tabla que solo está en una versión se muestra como tabla insertada o eliminada. Las filas presentes en ambas se comparan celda por celda. Dos tablas son la misma tabla con filas cambiadas cuando al menos la mitad de las celdas que tienen palabras en la más grande también tienen las mismas palabras en la otra, contando cada celda una vez. Si coinciden menos celdas, la tabla antigua se muestra como eliminada y la nueva como insertada. Comparar no muestra una columna añadida o quitada, celdas combinadas o divididas, ni una tabla dentro de otra tabla. Cuenta cada uno de estos casos, lo indica y deja esa tabla como estaba. Una tabla en un encabezado, un pie de página o una nota al pie solo se compara celda por celda.

Un párrafo de 12 palabras o más que se cortó de un lugar y se pegó en otro se muestra como **movimiento**: un registro donde el texto salió y otro donde llegó. Aceptar o rechazar uno de ellos afecta a los dos. El párrafo debe ser el mismo, o tener al menos 9 de cada 10 palabras iguales y en su mayoría en el mismo orden; de lo contrario, se muestra como una eliminación y una inserción. Las palabras que difieren dentro de un párrafo movido se muestran como inserciones y eliminaciones dentro del movimiento.

El texto que es igual en ambas versiones pero tiene distinto aspecto se muestra como **cambio de formato**. Comparar revisa negrita, cursiva, subrayado, tamaño, tipo de letra, color, resaltado, y el estilo, la alineación, las sangrías y el espaciado de un párrafo. Al rechazar el cambio, se recupera el aspecto anterior.

Si los dos documentos tienen demasiados cambios, Comparar lo indica. Entonces muestra algunos párrafos como reemplazados completos, y es posible que no busque movimientos ni cambios de formato en todos ellos. Cuando dos fragmentos largos de texto no tienen nada en común, Comparar muestra todo el fragmento antiguo como eliminado y todo el nuevo como insertado, y lo indica. El texto nuevo conserva sus estilos, listas, negrita y cursiva, pero no sus colores de texto, sombreados ni el espaciado entre párrafos. Las imágenes, los campos, los vínculos y las marcas de notas al pie o al final que estén en un texto que Comparar inserta como nuevo no se trasladan a él; Comparar los cuenta e indica cuántos son. También deja de marcar filas y tablas cuando un documento tiene más de 5000 filas que marcar, y cuenta el resto.
