# Control de cambios: aceptar o rechazar cambios
<!-- keywords: comparar, comparar documentos, dos versiones, qué cambió, control de cambios, cambios registrados, revisiones, marcas, aceptar, rechazar, aceptar todos, rechazar todos, mostrar para revisión, marcas simples, todas las marcas, sin marcas, original, mostrar marcas, globos, panel de revisión, barra de cambios, inserción, eliminación, ctrl+mayús+e, autor, revisor, nombre de usuario, ediciones de otra persona, cambios, revisar, comparación -->

**Control de cambios** guarda un registro de cada edición que usted hace, para que usted u otra persona puedan ver qué cambió y decidir si lo conservan. Úselo cuando edite un documento que pertenece a otra persona, o que alguien vaya a revisar después de usted.

## Activar y desactivar el control de cambios

Haga clic en **Revisar > Control de cambios**, o pulse **Ctrl+Mayús+E**. La barra de estado, en la parte inferior de la ventana, indica si está activado. Al hacer clic en esas palabras, se cambia el estado. La configuración se guarda en el documento, por lo que sigue activada cuando el documento vuelve de otra persona.

Mientras está activado:

- **Lo que escribe** se marca como inserción.
- **Lo que elimina** no se borra. Permanece en el texto, tachado, para que pueda recuperarse. La excepción es el texto que usted mismo escribió mientras el control de cambios estaba activado: al eliminarlo, simplemente desaparece, porque nadie más lo había visto.
- **Un salto de párrafo que inserta** (Intro) se marca como insertado, y uno que elimina (Retroceso al principio de un párrafo) se marca como eliminado.
- **Un cambio de aspecto** (negrita, tipo de letra, tamaño, color, alineación, sangrías o estilo) se registra junto con el aspecto que tenía el texto antes.

Cada cambio se guarda con quién lo hizo y cuándo. El nombre es el que figura en **Archivo > Opciones**, en **Nombre de usuario**. Cámbielo allí si es incorrecto.

Cada edición es un paso, así que **Ctrl+Z** la deshace como cualquier otra edición.

## Aspecto de un cambio

De forma predeterminada, los cambios de cada persona tienen un color propio. La primera persona cuyo cambio muestra el documento recibe el primer color, la siguiente persona el segundo, y así sucesivamente; cada persona conserva el mismo color mientras el documento esté abierto. El texto insertado aparece subrayado en el color de la persona, y el texto eliminado, tachado. El texto que se movió aparece en verde: con doble tachado donde estaba y con doble subrayado donde llegó. El texto cuyo formato cambió aparece en negrita. Una barra en el margen izquierdo marca cada línea que tiene algún cambio.

Puede cambiar todo esto. Haga clic en **Revisar > Control de cambios > Opciones de control de cambios** para abrir el cuadro **Opciones de control de cambios**. En **Marcas**, cada tipo de cambio tiene un **Aspecto** y un **Color**:

- **Inserciones**: subrayado, doble subrayado, tachado, negrita, cursiva, solo color o (ninguno).
- **Eliminaciones**: tachado, doble tachado, oculto, cursor o solo color. Con **Oculto**, las palabras eliminadas no aparecen en la línea. Con **Cursor**, tampoco aparecen, y un pequeño ^ indica dónde estaban. El texto trasladado también se oculta cuando elige **Globos** para las eliminaciones, pero no en otro caso.
- **Movido desde** y **Movido a**: el aspecto y los colores del texto movido. Están disponibles mientras **Registrar movimientos** está marcado. Si lo desmarca, el texto movido se muestra como una eliminación y una inserción normales.
- **Formato**: negrita, solo color o (ninguno). Si desmarca **Registrar formato**, los cambios de formato no se marcan en el texto.
- **Barras de cambios**: borde izquierdo, borde derecho, borde exterior (el lado derecho de una página impar y el izquierdo de una par) o (ninguno), y el color de la barra.

Cada cuadro **Color** ofrece **Por autor** (un color propio para cada persona) o un solo color para todas. Estas opciones son suyas. Se guardan en este equipo, no en el documento, se aplican a todos los documentos y a todas las ventanas abiertas, y cada ventana cambia en cuanto hace clic en **Aceptar**. Las barras se dibujan del mismo modo en pantalla, en una impresión y en un PDF siempre que se muestren las marcas.

## Elegir lo que ve: Mostrar para revisión

Haga clic en **Revisar > Mostrar para revisión** y elija una opción:

- **Todas las marcas** muestra todos los cambios en el texto, con las barras en el margen.
- **Marcas simples** muestra el documento tal como quedará con los cambios aceptados, y una barra en el margen junto a cada línea que tiene algún cambio.
- **Sin marcas** muestra el documento tal como quedará con todos los cambios aceptados, y nada más.
- **Original** muestra el documento tal como era antes de los cambios: el texto eliminado vuelve a aparecer y el texto insertado desaparece.

Estas opciones solo cambian lo que ve. Nunca modifican el documento y no se guardan con él. Un documento que tiene cambios se abre con **Todas las marcas**.

**Revisar > Mostrar marcas** elige qué tipos de cambio se muestran: **Inserciones y eliminaciones**, **Formato** y **Comentarios**. Debajo, elija cómo se muestra el texto eliminado. **Mostrar todas las revisiones en línea** lo mantiene en el texto, tachado. **Mostrar revisiones en globos** saca el texto eliminado de la línea y lo coloca en un globo en el margen. **Mostrar solo comentarios y formato en globos** mantiene las inserciones y eliminaciones en el texto.

## Aceptar o rechazar cambios

Aceptar un cambio es conservarlo para siempre. Rechazarlo es deshacerlo.

- **Revisar > Aceptar** acepta el cambio en el que está el cursor (o cada cambio del texto que haya seleccionado) y luego pasa al siguiente. **Revisar > Rechazar** hace lo mismo, deshaciendo el cambio.
- Haga clic en la flecha situada debajo de **Aceptar** o **Rechazar** para ver más opciones: **Aceptar todos los cambios mostrados**, **Aceptar todos los cambios** y **Aceptar todos los cambios y detener el seguimiento**; y **Rechazar todos los cambios mostrados** y **Rechazar todos los cambios**. «Mostrados» significa los cambios de los tipos que **Mostrar marcas** tiene activados (y, en el panel **Revisión**, del revisor que haya elegido).
- **Anterior** y **Siguiente**, en el grupo **Cambios**, pasan de un cambio al siguiente sin modificar nada.

Cada una de estas acciones es un paso, así que **Ctrl+Z** devuelve el documento exactamente a como estaba.

Aceptar una eliminación borra el texto para siempre. Rechazar una inserción también borra el texto. Aceptar una inserción conserva el texto y quita la marca de que era nuevo. Rechazar una eliminación conserva el texto y quita el tachado. Rechazar un cambio de aspecto restablece el aspecto que tenía el texto. El texto que se movió es un par de cambios: uno donde salió y otro donde llegó, unidos por el nombre del movimiento. Cuando el par está marcado con el mismo nombre, como lo guardan otros programas, aceptar o rechazar una de las mitades hace lo mismo con la otra, de modo que el texto queda movido para siempre o se devuelve a su sitio. Un nombre usado más de una vez, o que tiene una sola mitad, no une nada: cada mitad es un cambio propio.

## El panel Revisión

Haga clic en **Revisar > Panel de revisión** para abrir una lista de los cambios del documento: tipo, autor, fecha y palabras. Al hacer clic en uno, se muestra en el texto. Use **Aceptar**, **Rechazar**, **Anterior** y **Siguiente** debajo de la lista, o elija un revisor en el cuadro situado encima para ver solo los cambios de esa persona.

## Cambios en encabezados, pies de página, notas y tablas

Los cambios que otras personas hicieron en un encabezado, un pie de página, una nota al pie o una nota al final se muestran y aparecen en la lista igual que los del texto, incluido un encabezado o pie de página que no usa ninguna página (el de las páginas pares, cuando el documento no usa páginas pares e impares diferentes): **Siguiente** lo salta, y **Aceptar** y **Rechazar** realizan el cambio al que llegó por última vez. **Siguiente**, **Anterior**, **Aceptar**, **Rechazar** y el panel **Revisión** lo llevan al encabezado, pie de página o nota que contiene el cambio, y el orden es primero el texto, después las notas al pie y por último los encabezados y pies de página. **Aceptar todos** y **Rechazar todos** los abarcan a todos.

Los cambios en tablas también aparecen en la lista, tanto en el texto como en encabezados y pies de página: una fila o celda insertada o eliminada, celdas combinadas, y un cambio de aspecto de una tabla, una fila o una celda. Una barra en el margen marca las líneas de la fila o celda. Aceptar una fila eliminada, o rechazar una insertada, quita la fila y sus palabras. Una fila en la que todas las celdas tienen el cambio pasa a ser el cambio de esa fila. Rechazar un cambio de aspecto restablece el aspecto que tenían la tabla, la fila o la celda, y rechazar una combinación vuelve a dividir las celdas. Cuando quitar un cambio rompería la tabla, el cambio se conserva y el mensaje indica cuántos se conservaron. Eso ocurre con una celda unida a otras por abajo, que se conserva y pierde su marca, y con un cambio de la cuadrícula que las filas no pueden seguir, que se conserva como un cambio que puede aceptar. Los cambios dentro de una tabla de una nota al pie se conservan tal como se escribieron: no aparecen en la lista, y Aceptar y Rechazar no actúan sobre ellos.

Un movimiento es una línea en la lista y un cambio en el recuento que dan Aceptar todos y Rechazar todos: su otra mitad y las marcas de párrafo que lo acompañaban quedan resueltas con él. Los cambios dentro de cuadros de texto no se listan uno a uno. El panel **Revisión** indica cuántos hay (por ejemplo, «3 cambios más están dentro de cuadros de texto»), y **Aceptar todos** y **Rechazar todos** los resuelven junto con el resto.

## Lo que todavía no se registra

Los cambios que usted hace directamente en tablas (filas, celdas, combinaciones), en la numeración y en las listas, en las secciones y en la configuración de página, y en el texto de encabezados, pies de página y notas, se hacen como de costumbre, sin registro. Los cambios que otros programas registraron en la numeración y en las secciones se conservan y se guardan tal como estaban, pero no aparecen en la lista, y Aceptar y Rechazar no actúan sobre ellos.

## Comparar dos versiones

**Comparar** busca las diferencias entre dos versiones de un documento y las muestra como cambios registrados.

1. Abra la versión más antigua. Es el **original**.
2. Haga clic en **Comparar** en el grupo **Comparar** de la pestaña **Revisar**.
3. Haga clic en **Examinar** y elija la versión más reciente. Es la versión **revisada**.
4. Escriba el nombre con el que firmar los cambios. Elija las opciones que desee, como ignorar las mayúsculas. **Buscar texto que se movió** y **Buscar cambios de formato** están activadas salvo que las desactive.
5. Haga clic en **Aceptar**.

Se abre una ventana nueva con el texto original y los cambios que lo convierten en el texto revisado. Sus dos ficheros no se modifican. Use **Aceptar** y **Rechazar** como con cualquier cambio registrado. Si acepta todos los cambios, obtiene el texto revisado. Si los rechaza todos, obtiene el texto original.

Comparar analiza el texto del cuerpo del documento. No examina tablas, encabezados, pies de página ni notas al pie.

Un párrafo de 12 palabras o más que se cortó de un lugar y se pegó en otro se muestra como un **movimiento**: un registro donde salió el texto y otro donde llegó. Aceptar o rechazar uno de ellos afecta a ambos. El párrafo tiene que ser idéntico, o bien tener al menos 9 de cada 10 palabras iguales y en su mayoría en el mismo orden; de lo contrario, se muestra como una eliminación y una inserción. Las palabras que difieren dentro de un párrafo movido se muestran como inserciones y eliminaciones dentro del movimiento.

El texto que es igual en ambas versiones, pero tiene distinto aspecto, se muestra como un **cambio de formato**. Comparar revisa negrita, cursiva, subrayado, tamaño, tipo de letra, color, resaltado y el estilo, la alineación, las sangrías y el espaciado de un párrafo. Al rechazar el cambio se recupera el aspecto anterior.

Si los dos documentos tienen demasiados cambios, Comparar lo indica. Entonces muestra algunos párrafos como sustituidos completos, y es posible que no busque movimientos ni cambios de formato en todos ellos.
