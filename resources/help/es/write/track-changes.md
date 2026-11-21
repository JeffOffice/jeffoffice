# Control de cambios: aceptar o rechazar cambios

<!-- keywords: comparar, comparar documentos, dos versiones, qué cambió, control de cambios, cambios registrados, revisiones, marcas de revisión, aceptar, rechazar, aceptar todos, rechazar todos, mostrar para revisión, marcas simples, todas las marcas, sin marcas, original, mostrar marcas, globos, panel de revisión, barra de cambios, inserción, eliminación, ctrl+mayús+e, autor, revisor, nombre de usuario, ediciones de otra persona, revisar cambios -->

**Control de cambios** guarda un registro de cada modificación que usted hace, para que usted u otra persona puedan ver qué cambió y decidir si lo conservan. Úselo cuando edite un documento que pertenece a otra persona, o cuando alguien vaya a revisarlo después de usted.

## Activar y desactivar el control de cambios

Haga clic en **Revisar > Control de cambios**, o presione **Ctrl+Mayús+E**. La barra de estado, en la parte inferior de la ventana, indica si está activado. Al hacer clic en esas palabras, se cambia el estado. La configuración se guarda en el documento, así que sigue activada cuando el documento vuelve de otra persona.

Mientras está activado:

- **Lo que escribe** se marca como inserción.
- **Lo que elimina** no se quita. Permanece en el texto, tachado, para que pueda recuperarse. La única excepción es el texto que usted mismo escribió mientras el control de cambios estaba activado: al eliminarlo, simplemente desaparece, porque nadie más lo había visto.
- **Un salto de párrafo que usted inserta** (Intro) se marca como insertado, y uno que elimina (Retroceso al principio de un párrafo) se marca como eliminado.
- **Un cambio de aspecto** (negrita, tipo de letra, tamaño, color, alineación, sangrías o estilo) se registra junto con el aspecto que tenía el texto antes.

Cada cambio se guarda con el nombre de quien lo hizo y la fecha y hora. El nombre es el que figura en **Archivo > Opciones**, en **Nombre de usuario**. Si es incorrecto, cámbielo allí.

Cada edición es un paso, así que **Ctrl+Z** la deshace como cualquier otra edición.

## Cómo se ve un cambio

De forma predeterminada, los cambios de cada persona tienen un color propio. La primera persona cuyo cambio aparece en el documento recibe el primer color, la siguiente el segundo, y así sucesivamente; cada persona conserva su color mientras el documento esté abierto. El texto insertado aparece subrayado con el color de la persona, y el texto eliminado, tachado. El texto que se movió aparece en verde: con doble tachado donde salió y con doble subrayado donde llegó. El texto cuyo formato cambió aparece en negrita. Una barra en el margen izquierdo marca cada línea que tiene un cambio.

Puede cambiar todo esto. Haga clic en **Revisar > Seguimiento > Opciones de seguimiento** para abrir el cuadro **Opciones de control de cambios**. En **Marcas**, cada tipo de cambio tiene un **Aspecto** y un **Color**:

- **Inserciones**: subrayado, doble subrayado, tachado, negrita, cursiva, solo color o (ninguno).
- **Eliminaciones**: tachado, doble tachado, oculto, cursor o solo color. Con **Oculto**, las palabras eliminadas no aparecen en la línea. Con **Cursor**, tampoco aparecen, pero un pequeño ^ indica dónde estaban. El texto movido también se oculta cuando elige **Globos** para las eliminaciones, pero no en los demás casos.
- **Movido desde** y **Movido a**: los aspectos y colores del texto movido. Están disponibles mientras **Marcar movimientos** esté activada. Si la desmarca, el texto movido se muestra como una eliminación y una inserción normales.
- **Formato**: negrita, solo color o (ninguno). Si desmarca **Marcar formato**, los cambios de formato no se marcan en el texto.
- **Barras de cambios**: borde izquierdo, borde derecho, borde exterior (el lado derecho de una página impar y el izquierdo de una par) o (ninguna), y el color de la barra.

Cada cuadro **Color** ofrece **Por autor** (el color propio de cada persona) o un único color para todas. Estas opciones son suyas. Se guardan en este equipo, no en el documento, se aplican a todos los documentos y a todas las ventanas abiertas, y cada ventana cambia en cuanto hace clic en **Aceptar**. Las barras se dibujan igual en pantalla, en una impresión y en un PDF, siempre que se muestren las marcas.

## Elegir lo que ve: Mostrar para revisión

Haga clic en **Revisar > Mostrar para revisión** y elija una opción:

- **Todas las marcas** muestra todos los cambios en el texto, con las barras en el margen.
- **Marcas simples** muestra el documento tal como quedará con los cambios aceptados, y una barra en el margen junto a cada línea que tiene un cambio.
- **Sin marcas** muestra el documento tal como quedará con todos los cambios aceptados, y nada más.
- **Original** muestra el documento tal como era antes de los cambios: el texto eliminado vuelve, y el texto insertado desaparece.

Estas opciones solo cambian lo que usted ve. Nunca modifican el documento y no se guardan con él. Un documento que tiene cambios se abre con **Todas las marcas**.

**Revisar > Mostrar marcas** elige qué tipos de cambio se muestran: **Inserciones y eliminaciones**, **Formato** y **Comentarios**. Debajo, elija cómo se muestra el texto eliminado. **Mostrar todas las revisiones insertadas en el texto** lo mantiene en el texto, tachado. **Mostrar revisiones en globos** lo saca de la línea y lo pone en un globo en el margen. **Mostrar solo comentarios y formato en globos** mantiene las inserciones y eliminaciones en el texto.

## Aceptar o rechazar cambios

Aceptar un cambio es conservarlo de forma definitiva. Rechazarlo es deshacerlo.

- **Revisar > Aceptar** acepta el cambio en el que está el cursor (o todos los cambios del texto seleccionado) y luego pasa al siguiente. **Revisar > Rechazar** hace lo mismo, deshaciendo el cambio.
- Haga clic en la flecha situada debajo de **Aceptar** o **Rechazar** para ver más opciones: **Aceptar todos los cambios mostrados**, **Aceptar todos los cambios** y **Aceptar todos los cambios y detener el seguimiento**; y **Rechazar todos los cambios mostrados** y **Rechazar todos los cambios**. «Mostrados» significa los cambios de los tipos que **Mostrar marcas** tiene activados (y, en el panel **Revisión**, el revisor que haya elegido).
- **Anterior** y **Siguiente**, en el grupo **Cambios**, pasan de un cambio al siguiente sin modificar nada. Con el cursor al principio absoluto del texto, de un encabezado o de una nota, **Siguiente** va a un cambio que empieza justo ahí.

Cada una de estas acciones es un paso, así que **Ctrl+Z** devuelve el documento exactamente a como estaba.

Aceptar una eliminación quita el texto de forma definitiva. Rechazar una inserción también quita el texto. Aceptar una inserción conserva el texto y quita la marca de que era nuevo. Rechazar una eliminación conserva el texto y quita el tachado. Rechazar un cambio de aspecto devuelve el aspecto que tenía el texto. El texto movido es un par de cambios: uno donde salió y otro donde llegó, unidos por el nombre del movimiento. Cuando el par está marcado con el mismo nombre, como lo guardan otros programas, aceptar o rechazar una de las mitades hace lo mismo con la otra, de modo que el texto queda movido de forma definitiva o vuelve a su sitio. Un nombre que se usa más de una vez, o que tiene una sola mitad, no une nada: cada mitad es un cambio independiente.

## El panel Revisión

Haga clic en **Revisar > Panel de revisión** para abrir una lista de los cambios del documento: tipo, autor, fecha y palabras. Al hacer clic en uno, se muestra en el texto. Use **Aceptar**, **Rechazar**, **Anterior** y **Siguiente** debajo de la lista, o elija un revisor en el cuadro situado encima para ver solo los cambios de esa persona.

## Cambios en encabezados, pies de página, notas y tablas

Los cambios que otras personas hicieron en un encabezado, un pie de página, una nota al pie o una nota al final se muestran y aparecen en la lista igual que los del texto, incluido un encabezado o pie de página que no usa ninguna página (el de las páginas pares cuando el documento no usa páginas pares e impares diferentes): **Siguiente** lo salta, y **Aceptar** y **Rechazar** actúan sobre el cambio al que llegó por última vez allí. **Siguiente**, **Anterior**, **Aceptar**, **Rechazar** y el panel **Revisión** le llevan al encabezado, pie de página o nota que tiene el cambio, y el orden es primero el texto, luego las notas al pie y después los encabezados y pies de página. **Aceptar todos** y **Rechazar todos** los abarcan a todos.

También se listan los cambios en tablas, en el texto y en encabezados y pies de página: una fila o celda insertada o eliminada, celdas combinadas y un cambio de aspecto de una tabla, una fila o una celda. Una barra en el margen marca las líneas de la fila o celda. Aceptar una fila eliminada, o rechazar una insertada, quita la fila y su contenido. Una fila en la que todas las celdas tienen el cambio es el cambio de esa fila. Rechazar un cambio de aspecto devuelve el aspecto que tenían la tabla, la fila o la celda, y rechazar una combinación vuelve a separar las celdas. Cuando quitar un cambio dejaría la tabla rota, el cambio se conserva y el mensaje indica cuántos se conservaron. Ocurre con una celda unida a otras hacia abajo, que se conserva y pierde su marca, y con un cambio en la cuadrícula que las filas no pueden seguir, que se conserva como un cambio que puede aceptar. Los cambios dentro de una tabla de una nota al pie se conservan tal como se escribieron: no aparecen en la lista, y **Aceptar** y **Rechazar** no actúan sobre ellos.

Un movimiento es una línea en la lista y un cambio en el recuento que dan **Aceptar todos** y **Rechazar todos**: su otra mitad y las marcas de párrafo que lo acompañaban se resuelven con él. Los cambios dentro de un cuadro de texto se listan uno a uno, después de los cambios del texto y antes de los de las notas al pie. **Siguiente** y **Anterior** abren el cuadro para escribir, con el cambio seleccionado, y **Aceptar** y **Rechazar** resuelven ese cambio, cada uno como un paso. Si mueve el cursor dentro del cuadro abierto, **Siguiente** y **Anterior** continúan desde el cursor. Un cuadro que se resuelve cambio a cambio se guarda sin la copia que leen los programas antiguos, como ocurre al escribir en un cuadro. Se listan así los primeros 200 cuadros que tienen cambios. Un cuadro de texto que forma parte de un grupo, o cuyo texto continúa en otros cuadros, un cuadro dentro de otro cuadro, un cuadro en un encabezado o en una nota, y los cuadros más allá de los primeros 200, no se pueden abrir de esa manera: el panel **Revisión** cuenta sus cambios en una línea debajo de la lista, y solo **Aceptar todos** y **Rechazar todos** los resuelven.

Un cambio en la numeración de un párrafo aparece como **Numerada** o **Numeración cambiada**. Cuando un cambio de aspecto de un párrafo registró la lista y el nivel que tenía, rechazarlo vuelve a colocar el párrafo en esa lista y ese nivel (o lo saca de la lista en la que no estaba). Un cambio que solo conserva la numeración antigua como una etiqueta (como lo guardan algunos programas) no indica ninguna lista a la que volver: aceptarlo mantiene la numeración, y rechazarlo quita la numeración solo si la etiqueta está vacía; en otro caso deja el cambio y lo cuenta como conservado. Rechazar una numeración que se añadió devuelve al párrafo lo que le da su estilo (un estilo de título que tiene numeración la conserva), y ninguna numeración si el estilo no la tiene. Si la lista que un cambio devolvería ya no está en el documento, no cambia nada de ese párrafo y el cambio se cuenta como conservado.

Un cambio en una sección (tamaño de página, orientación, márgenes, columnas, numeración de páginas o tipo de salto) aparece como **Sección con formato**, en el párrafo que termina la sección, o al final del texto en la última sección. Aceptarlo mantiene la sección tal como está y elimina el registro; rechazarlo devuelve las propiedades que guardaba el registro, y las páginas se vuelven a componer de inmediato. Los encabezados y pies de página que la sección indica no cambian. Si las propiedades antiguas del registro no se pueden leer, rechazar deja la sección y el cambio como están y lo cuenta como conservado; no se sustituye nada por una página predeterminada. Cuando **Aceptar todos** o **Rechazar todos** también quita un salto de sección o lo vuelve a poner, primero se resuelve el cambio de cada sección, para que no quede ninguno sin resolver.

Un cambio que no puede resolverse sin dañar el documento (una tabla que se rompería, una lista o una sección cuyo estado anterior no se puede restaurar, un cuadro de texto demasiado grande para volver a escribirlo) se deja tal como está, y el mensaje que aparece después de **Aceptar todos** o **Rechazar todos** indica cuántos se conservaron.

## Lo que aún no se registra

Los cambios que usted hace directamente en tablas (filas, celdas, combinaciones), en la numeración y las listas, en las secciones y la configuración de página, y en el texto de encabezados, pies de página y notas se hacen como siempre, sin registro. Los cambios que otros programas registraron en la numeración y en las secciones se listan y se pueden aceptar y rechazar, como se describe arriba.

## Comparar dos versiones

Comparar busca las diferencias entre dos versiones de un documento y las muestra como cambios controlados.

1. Abra la versión más antigua. Es el **original**.
2. Haga clic en **Comparar** en el grupo **Comparar** de la ficha **Revisar**.
3. Haga clic en **Examinar** y elija la versión más reciente. Es la versión **revisada**.
4. Escriba el nombre con el que firmar los cambios. Elija las opciones que desee, como ignorar las mayúsculas. **Buscar texto que se movió** y **Buscar cambios de formato** están activadas salvo que las desactive.
5. Haga clic en **Aceptar**.

Se abre una ventana nueva con el texto original y los cambios que lo convierten en el texto revisado. Sus dos archivos no se modifican. Use **Aceptar** y **Rechazar** como con cualquier cambio controlado. Si acepta todos los cambios, obtiene el texto revisado. Si rechaza todos, obtiene el texto original.

Comparar analiza el texto del cuerpo del documento. No analiza tablas, encabezados, pies de página ni notas al pie.

Un párrafo de 12 palabras o más que se cortó de un lugar y se pegó en otro se muestra como un **movimiento**: un registro donde salió el texto y otro donde llegó. Aceptar o rechazar uno de ellos afecta a los dos. El párrafo tiene que ser igual, o tener al menos 9 de cada 10 palabras iguales y en su mayoría en el mismo orden; si no, se muestra como una eliminación y una inserción. Las palabras que difieren dentro de un párrafo movido se muestran como inserciones y eliminaciones dentro del movimiento.

El texto que es igual en ambas versiones pero tiene otro aspecto se muestra como un **cambio de formato**. Comparar revisa la negrita, la cursiva, el subrayado, el tamaño, el tipo de letra, el color, el resaltado, y el estilo, la alineación, las sangrías y el espaciado de un párrafo. Rechazar el cambio devuelve el aspecto anterior.

Si los dos documentos tienen demasiados cambios, Comparar lo indica. En ese caso muestra algunos párrafos como sustituidos completos, y puede que no busque movimientos ni cambios de formato en todos ellos.
