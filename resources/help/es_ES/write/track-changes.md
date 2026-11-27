# Control de cambios: aceptar y rechazar cambios
<!-- keywords: comparar, comparar documentos, dos versiones, qué ha cambiado, control de cambios, cambios registrados, revisiones, marcas, aceptar, rechazar, aceptar todo, rechazar todo, mostrar para revisión, marcado simple, todas las marcas, sin marcas, original, mostrar marcas, globos, panel de revisiones, barra de cambios, inserción, eliminación, ctrl+mayús+e, autor, revisor, nombre de usuario, ediciones de otra persona -->

**Control de cambios** guarda un registro de cada modificación que usted hace, para que usted o cualquier otra persona pueda ver qué ha cambiado y decidir si lo conserva. Utilícelo cuando edite un documento que pertenece a otra persona, o que alguien vaya a revisar después de usted.

## Activar y desactivar el control de cambios

Haga clic en **Revisar > Control de cambios**, o pulse **Ctrl+Mayús+E**. La barra de estado, en la parte inferior de la ventana, indica si está activado. Al hacer clic en esas palabras, el control cambia de estado. La configuración se guarda en el documento, de modo que sigue activada cuando el documento vuelve de otra persona.

Mientras está activado:

- **Lo que escribe** se marca como inserción.
- **Lo que elimina** no se borra. Permanece en el texto, tachado, para que pueda recuperarse. La única excepción es el texto que usted escribió mientras el control estaba activado: al eliminarlo, simplemente desaparece, porque nadie más lo había visto.
- **Un salto de párrafo que inserta** (Intro) se marca como insertado, y uno que elimina (Retroceso al principio de un párrafo) se marca como eliminado.
- **Un cambio de aspecto** (negrita, tipo de letra, tamaño, color, alineación, sangrías o estilo) se registra con el aspecto que tenía el texto antes.

Cada cambio se guarda con quién lo hizo y cuándo. El nombre es el que aparece en **Archivo > Opciones**, en **Nombre de usuario**. Si es incorrecto, cámbielo allí.

Cada modificación es un paso, así que **Ctrl+Z** la deshace como cualquier otra edición.

## Qué aspecto tiene un cambio

De forma predeterminada, los cambios de cada persona tienen un color propio. La primera persona cuyo cambio muestra el documento recibe el primer color, la siguiente el segundo, y así sucesivamente; cada persona conserva su color mientras el documento esté abierto. El texto insertado aparece subrayado en el color de la persona, y el texto eliminado, tachado. El texto que se ha movido aparece en verde: con doble tachado donde estaba y con doble subrayado donde llegó. El texto cuyo formato se ha cambiado aparece en negrita. Una barra en el margen izquierdo marca cada línea que tiene algún cambio.

Todo esto puede cambiarse. Haga clic en **Revisar > Control de cambios > Opciones de control de cambios** para abrir el cuadro **Opciones de control de cambios**. En **Marcado**, cada tipo de cambio tiene un **Aspecto** y un **Color**:

- **Inserciones**: subrayado, doble subrayado, tachado, negrita, cursiva, solo color o (ninguno).
- **Eliminaciones**: tachado, doble tachado, oculto, cursor o solo color. Con **Oculto**, las palabras eliminadas no aparecen en la línea. Con **Cursor**, tampoco aparecen, pero se muestra un pequeño ^ en el lugar donde estaban. El texto movido también se oculta cuando elige **Globos** para las eliminaciones, pero no en los demás casos.
- **Movido desde** y **Movido a**: los aspectos y colores del texto movido. Están disponibles mientras **Registrar movimientos** está marcada. Si la desmarca, el texto movido se muestra como una eliminación y una inserción normales.
- **Formato**: negrita, solo color o (ninguno). Si desmarca **Registrar formato**, los cambios de formato no se marcan en el texto.
- **Barras de cambios**: borde izquierdo, borde derecho, borde exterior (el lado derecho de las páginas impares y el izquierdo de las pares) o (ninguno), y el color de la barra.

Cada cuadro **Color** ofrece **Por autor** (un color propio para cada persona) o un único color para todas. Estas opciones son suyas. Se guardan en este equipo, no en el documento, se aplican a todos los documentos y a todas las ventanas abiertas, y cada ventana cambia en cuanto hace clic en **Aceptar**. Las barras se dibujan igual en pantalla, en una impresión y en un PDF siempre que se muestren las marcas.

## Elegir lo que ve: Mostrar para revisión

Haga clic en **Revisar > Mostrar para revisión** y elija una opción:

- **Todas las marcas** muestra todos los cambios en el texto, con las barras en el margen.
- **Marcado simple** muestra el documento tal como quedará con los cambios aceptados, y una barra en el margen junto a cada línea que tiene algún cambio.
- **Sin marcas** muestra el documento tal como quedará con todos los cambios aceptados, y nada más.
- **Original** muestra el documento tal como era antes de los cambios: el texto eliminado vuelve a aparecer, y el texto insertado desaparece.

Estas opciones solo cambian lo que ve. Nunca modifican el documento y no se guardan con él. Un documento que tiene cambios se abre con **Todas las marcas**.

**Revisar > Mostrar marcas** elige qué tipos de cambio se muestran: **Inserciones y eliminaciones**, **Formato** y **Comentarios**. Debajo, elija cómo se muestra el texto eliminado. **Mostrar todas las revisiones insertadas** lo mantiene en el texto, tachado. **Mostrar revisiones en globos** lo quita de la línea y lo coloca en un globo en el margen. **Mostrar solo comentarios y formato en globos** mantiene las inserciones y eliminaciones en el texto.

## Aceptar o rechazar cambios

Aceptar un cambio es conservarlo de forma definitiva. Rechazarlo es deshacerlo.

- **Revisar > Aceptar** acepta el cambio en el que está el cursor (o todos los cambios del texto seleccionado) y luego pasa al siguiente. **Revisar > Rechazar** hace lo mismo, deshaciendo el cambio.
- Haga clic en la flecha situada debajo de **Aceptar** o **Rechazar** para ver más opciones: **Aceptar todos los cambios mostrados**, **Aceptar todos los cambios** y **Aceptar todos los cambios y detener el seguimiento**; y **Rechazar todos los cambios mostrados** y **Rechazar todos los cambios**. «Mostrados» significa los cambios de los tipos que **Mostrar marcas** tiene activados (y, en el **Panel de revisiones**, el revisor que haya elegido).
- **Anterior** y **Siguiente**, en el grupo **Cambios**, pasan de un cambio al siguiente sin modificar nada. Con el cursor al principio absoluto del texto, de un encabezado o de una nota, **Siguiente** va a un cambio que empieza justo ahí.

Cada una de estas acciones es un paso, así que **Ctrl+Z** devuelve el documento exactamente a su estado anterior.

Al aceptar una eliminación, el texto se borra de forma definitiva. Al rechazar una inserción, el texto también se borra. Al aceptar una inserción, el texto se conserva y se quita la marca de que era nuevo. Al rechazar una eliminación, el texto se conserva y se quita el tachado. Al rechazar un cambio de aspecto, se recupera el aspecto que tenía el texto. El texto movido es un par de cambios: uno donde salió y otro donde llegó, unidos por el nombre del movimiento. Cuando el par lleva el mismo nombre, como lo guardan otros programas, aceptar o rechazar una de las dos mitades hace lo mismo con la otra, de modo que el texto queda movido de forma definitiva o vuelve a su sitio. Un nombre que se usa más de una vez, o que solo tiene una mitad, no une nada: cada mitad es un cambio independiente.

## El Panel de revisiones

Haga clic en **Revisar > Panel de revisiones** para abrir una lista de los cambios del documento: su tipo, quién lo hizo, cuándo y las palabras. Al hacer clic en uno, se muestra en el texto. Use **Aceptar**, **Rechazar**, **Anterior** y **Siguiente** debajo de la lista, o elija un revisor en el cuadro situado encima para ver solo los cambios de esa persona.

## Cambios en encabezados, pies de página, notas y tablas

Los cambios que otras personas hicieron en un encabezado, un pie de página, una nota al pie o una nota al final se muestran y se listan igual que los del texto, incluido un encabezado o pie de página que no usa ninguna página (el de las páginas pares cuando el documento no usa páginas pares e impares diferentes): **Siguiente** lo salta, y **Aceptar** y **Rechazar** actúan sobre el cambio al que llegó por último en ese lugar. **Siguiente**, **Anterior**, **Aceptar**, **Rechazar** y el **Panel de revisiones** lo llevan al encabezado, pie de página o nota que contiene el cambio, y el orden es primero el texto, después las notas al pie y por último los encabezados y pies de página. **Aceptar todos** y **Rechazar todos** los abarcan a todos.

También se listan los cambios en tablas, tanto en el texto como en encabezados y pies de página: una fila o celda insertada o eliminada, celdas combinadas y un cambio de aspecto de una tabla, una fila o una celda. Una barra en el margen marca las líneas de la fila o celda. Al aceptar una fila eliminada, o al rechazar una insertada, se quita la fila y sus palabras. Una fila en la que todas las celdas tienen el cambio se considera un cambio de esa fila. Al rechazar un cambio de aspecto, se recupera el aspecto que tenían la tabla, la fila o la celda; al rechazar una combinación, las celdas vuelven a separarse. Cuando quitar un cambio dejaría la tabla rota, el cambio se conserva en su lugar y el mensaje indica cuántos se han conservado. Esto ocurre con una celda unida a otras hacia abajo, que se conserva y pierde su marca, y con un cambio de la cuadrícula que las filas no pueden seguir, que se conserva como un cambio que puede aceptar. Los cambios dentro de una tabla de una nota al pie se conservan tal como se escribieron: no se listan, y **Aceptar** y **Rechazar** no actúan sobre ellos.

Un movimiento ocupa una línea en la lista y cuenta como un cambio en el recuento de **Aceptar todos** y **Rechazar todos**: su otra mitad y las marcas de párrafo que lo acompañaban quedan resueltas con él. Los cambios dentro de un cuadro de texto se listan uno a uno, después de los del texto y antes de los de las notas al pie. **Siguiente** y **Anterior** abren el cuadro para escribir, con el cambio seleccionado, y **Aceptar** y **Rechazar** resuelven ese cambio, cada uno como un paso. Si mueve el cursor dentro del cuadro abierto, **Siguiente** y **Anterior** continúan desde la posición del cursor. Un cuadro que se resuelve cambio a cambio se guarda sin la copia que leen los programas antiguos, igual que al escribir en un cuadro. Se listan así los primeros 200 cuadros que contienen cambios. Un cuadro de texto que forma parte de un grupo, o cuyo texto continúa en otros cuadros, un cuadro dentro de otro cuadro, un cuadro en un encabezado o una nota, y los cuadros posteriores a los primeros 200 no pueden abrirse de esa forma: el **Panel de revisiones** indica sus cambios en una línea debajo de la lista, y solo **Aceptar todos** y **Rechazar todos** los resuelven.

Un cambio en la numeración de un párrafo se lista como **Numerado** o **Cambio de numeración**. Cuando un cambio de aspecto de un párrafo registró la lista y el nivel que tenía, al rechazarlo el párrafo vuelve a esa lista y a ese nivel (o sale de la lista en la que no estaba). Un cambio que solo conserva la numeración anterior como etiqueta (como lo guardan algunos programas) no indica a qué lista volver: al aceptarlo se conserva la numeración, y al rechazarlo se quita la numeración solo si la etiqueta está vacía; en caso contrario, el cambio se mantiene y se cuenta como conservado. Al rechazar una numeración que se había añadido, el párrafo recupera lo que le da su estilo (un estilo de título que tiene numeración la conserva), y sin numeración si el estilo no la tiene. Si la lista que el cambio devolvería ya no está en el documento, ese párrafo no cambia y el cambio se cuenta como conservado.

Un cambio en una sección (tamaño de página, orientación, márgenes, columnas, numeración de páginas o tipo de salto) se lista como **Sección con formato**, en el párrafo que termina la sección, o al final del texto para la última sección. Al aceptarlo, la sección se mantiene tal como está y se elimina el registro; al rechazarlo, se recuperan las propiedades que guardaba el registro y las páginas se vuelven a maquetar de inmediato. Los encabezados y pies de página que la sección indica no cambian. Si las propiedades anteriores del registro no pueden leerse, al rechazar se deja la sección y el cambio tal como están, y se cuenta como conservado; nunca se sustituye por una página predeterminada. Cuando **Aceptar todos** o **Rechazar todos** también quita un salto de sección o lo vuelve a poner, primero se resuelve el cambio de cada sección, para que no quede ninguno sin resolver.

Un cambio que no puede resolverse sin dañar el documento (una tabla que se rompería, una lista o una sección cuyo estado anterior no puede recuperarse, un cuadro de texto demasiado grande para volver a escribirlo) se deja tal como está, y el mensaje que aparece tras **Aceptar todos** o **Rechazar todos** indica cuántos se han conservado.

## Qué cambios se registran y cuáles no

Mientras el control de cambios está activado, se registran: escribir, eliminar y pegar texto; Intro; reemplazar palabras; el aspecto del texto y de los párrafos (tipos de letra, tamaños, colores, alineación, espaciado, bordes y estilos); **Cambiar mayúsculas y minúsculas**; símbolos y caracteres especiales; saltos de página y de columna; la marca de párrafo de un salto de sección; y, en tablas, una fila que inserte (con **Insertar encima** o **Insertar debajo**, o con Tab en la última celda), una fila que elimine, una tabla que elimine y una tabla que inserte. Una fila insertada o eliminada aparece en el **Panel de revisiones** como un único cambio, con sus palabras, y **Aceptar** y **Rechazar** la conservan o la quitan. Al eliminar una fila que contiene una tabla, se marcan también las filas de esa tabla. Una fila que otra persona insertó y usted elimina lleva ambos cambios, y **Aceptar** o **Rechazar** la quita. Una fila que usted insertó y después elimina desaparece de forma definitiva.

Estos cambios se realizan como de costumbre, sin marca, y la barra de estado muestra cada vez el mensaje «Este cambio no se registró como cambio controlado»: viñetas, numeración y demás comandos de listas; **Aumentar sangría** y **Disminuir sangría**; configuración de página, columnas, márgenes y números de línea; encabezados y pies de página, y escribir, eliminar y reemplazar palabras en un encabezado, pie de página, nota al pie o nota al final; insertar una nota al pie, una nota al final, una tabla de contenido, un índice, un campo, un vínculo o un control de contenido; imágenes, gráficos, ecuaciones, cuadros de texto y formas dibujadas; columnas, celdas, combinar, dividir y el aspecto de una tabla; eliminar una fila que forma parte de una celda unida hacia abajo entre filas; ordenar párrafos o filas de una tabla; y mover una tabla o una sección. Mientras escribe en un encabezado o en una nota, el mensaje aparece una sola vez, no con cada tecla. **Deshacer** revierte cualquiera de estas acciones como un único paso.

Algunos cambios no se realizan en absoluto mientras el control de cambios está activado, porque sin marca dañarían un cambio que sí está registrado: en una tabla que tiene una fila insertada o eliminada, **Eliminar columnas**, **Eliminar celdas**, **Combinar celdas**, **Dividir celdas**, **Convertir en texto** y pegar sobre celdas. La barra de estado lo indica. Acepte o rechace antes los cambios de esa tabla, o desactive el control de cambios. En un documento que bloquea el control de cambios activado, sin posibilidad de aceptar ni rechazar, estos cambios sí se realizan, y la barra de estado indica que el cambio no se registró.

Los cambios registrados por otros programas en la numeración y en las secciones se listan y pueden aceptarse y rechazarse, tal como se describe arriba.

## Comparar dos versiones

Comparar busca las diferencias entre dos versiones de un documento y las muestra como cambios controlados.

1. Abra la versión más antigua. Es el **original**.
2. Haga clic en **Comparar** en el grupo **Comparar** de la ficha **Revisar**.
3. Haga clic en **Examinar** y elija la versión más reciente. Es la versión **revisada**.
4. Escriba el nombre con el que firmar los cambios. Elija las opciones que desee, como ignorar las mayúsculas. **Buscar texto que se haya movido** y **Buscar cambios de formato** están activadas salvo que las desactive.
5. Haga clic en **Aceptar**.

Se abre una ventana nueva con el texto original y los cambios que lo convierten en el texto revisado. Sus dos ficheros no se modifican. Use **Aceptar** y **Rechazar** como con cualquier cambio controlado. Si acepta todos los cambios, obtendrá el texto revisado; si rechaza todos, obtendrá el original.

Comparar examina el texto del cuerpo del documento. No examina tablas, encabezados, pies de página ni notas al pie.

Un párrafo de 12 palabras o más que se cortó de un lugar y se pegó en otro se muestra como un **movimiento**: un registro donde salió el texto y otro donde llegó. Al aceptar o rechazar uno de ellos se resuelven ambos. El párrafo tiene que ser el mismo, o bien tener al menos 9 de cada 10 palabras iguales y en su mayoría en el mismo orden; si no, se muestra como una eliminación y una inserción. Las palabras que difieren dentro de un párrafo movido se muestran como inserciones y eliminaciones dentro del movimiento.

El texto que es igual en ambas versiones pero tiene otro aspecto se muestra como un **cambio de formato**. Comparar revisa negrita, cursiva, subrayado, tamaño, tipo de letra, color, resaltado, y el estilo, la alineación, las sangrías y el espaciado de un párrafo. Al rechazar el cambio se recupera el aspecto anterior.

Si los dos documentos tienen demasiados cambios, Comparar lo indica. En ese caso muestra algunos párrafos como sustituidos completos, y puede que no busque movimientos ni cambios de formato en todos ellos.
