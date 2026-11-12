# Control de cambios: aceptar o rechazar cambios
<!-- keywords: comparar, comparar documentos, dos versiones, qué cambió, control de cambios, cambios registrados, revisiones, marcas, aceptar, rechazar, aceptar todo, rechazar todo, mostrar para revisión, marcado simple, todas las marcas, sin marcas, original, mostrar marcas, globos, panel de revisiones, barra de cambios, inserción, eliminación, ctrl+mayús+e, autor, revisor, nombre de usuario, ediciones de otra persona -->

**Control de cambios** guarda un registro de cada edición que realiza, para que usted (u otra persona) pueda ver qué cambió y decidir si lo conserva. Úselo cuando edite un documento que pertenece a otra persona, o que alguien vaya a revisar después.

## Activar y desactivar el control de cambios

Haga clic en **Revisar > Control de cambios** o presione **Ctrl+Mayús+E**. La barra de estado, en la parte inferior de la ventana, indica si está activado. Si hace clic en esas palabras, el estado cambia al contrario. La configuración se guarda en el documento, de modo que sigue activada cuando el documento vuelve de otra persona.

Mientras está activado:

- **Lo que escribe** se marca como inserción.
- **Lo que elimina** no se quita. Permanece en el texto, tachado, para que pueda recuperarse. El texto que usted mismo escribió mientras el control estaba activado es la única excepción: al eliminarlo, simplemente desaparece, porque nadie más lo había visto.
- **Un salto de párrafo que inserta** (Entrar) se marca como insertado, y uno que elimina (Retroceso al inicio de un párrafo) se marca como eliminado.
- **Un cambio de aspecto** (negrita, fuente, tamaño, color, alineación, sangrías o estilo) se registra junto con el aspecto que tenía el texto antes.

Cada cambio se guarda con quién lo hizo y cuándo. El nombre es el que aparece en **Archivo > Opciones**, en **Nombre de usuario**. Si es incorrecto, cámbielo allí.

Cada edición es un paso, de modo que **Ctrl+Z** la deshace como cualquier otra edición.

## Cómo se ve un cambio

De forma predeterminada, los cambios de cada persona tienen un color propio. El primer autor cuyo cambio aparece en el documento recibe el primer color, el siguiente recibe el segundo, y así sucesivamente; cada persona conserva su color mientras el documento esté abierto. El texto insertado aparece subrayado en el color de la persona, y el texto eliminado, tachado. El texto que se movió aparece en verde: con doble tachado donde estaba y con doble subrayado donde llegó. El texto cuyo formato cambió aparece en negrita. Una barra en el margen izquierdo marca cada línea que tiene un cambio.

Puede cambiar todo esto. Haga clic en **Revisar > Seguimiento > Opciones de control de cambios** para abrir el cuadro **Opciones de control de cambios**. En **Marcado**, cada tipo de cambio tiene un **Aspecto** y un **Color**:

- **Inserciones**: subrayado, doble subrayado, tachado, negrita, cursiva, solo color o (ninguno).
- **Eliminaciones**: tachado, doble tachado, oculto, cursor o solo color. Con **Oculto**, las palabras eliminadas no aparecen en la línea. Con **Cursor**, tampoco aparecen, y un pequeño ^ indica dónde estaban. El texto que se movió también se oculta cuando elige **Globos** para las eliminaciones, pero no en otro caso.
- **Movido desde** y **Movido a**: los aspectos y colores del texto movido. Están disponibles mientras **Registrar movimientos** esté marcada. Si la desmarca, el texto movido se muestra como una eliminación y una inserción normales.
- **Formato**: negrita, solo color o (ninguno). Si desmarca **Registrar formato**, los cambios de formato no se marcan en el texto.
- **Barras de cambios**: borde izquierdo, borde derecho, borde exterior (el lado derecho de una página impar y el izquierdo de una par) o (ninguna), y el color de la barra.

Cada cuadro **Color** ofrece **Por autor** (el color propio de cada persona) o un solo color para todas. Estas opciones son suyas. Se guardan en este equipo, no en el documento, se aplican a todos los documentos y a todas las ventanas abiertas, y cada ventana cambia en cuanto hace clic en **Aceptar**. Las barras se dibujan igual en pantalla, en una impresión y en un PDF siempre que se muestren las marcas.

## Elegir lo que ve: Mostrar para revisión

Haga clic en **Revisar > Mostrar para revisión** y elija una opción:

- **Todas las marcas** muestra cada cambio en el texto, con las barras en el margen.
- **Marcado simple** muestra el documento tal como quedará con los cambios aceptados, y una barra en el margen junto a cada línea que tiene un cambio.
- **Sin marcas** muestra el documento tal como quedará con todos los cambios aceptados, y nada más.
- **Original** muestra el documento tal como era antes de los cambios: el texto eliminado vuelve y el texto insertado desaparece.

Estas opciones solo cambian lo que ve. Nunca modifican el documento y no se guardan con él. Un documento que tiene cambios se abre con **Todas las marcas**.

**Revisar > Mostrar marcas** elige qué tipos de cambio se muestran: **Inserciones y eliminaciones**, **Formato** y **Comentarios**. Debajo, elija cómo se muestra el texto eliminado. **Mostrar todas las revisiones insertadas en el texto** lo mantiene en el texto, tachado. **Mostrar revisiones en globos** lo saca de la línea y lo pone en un globo en el margen. **Mostrar solo comentarios y formato en globos** mantiene las inserciones y eliminaciones en el texto.

## Aceptar o rechazar cambios

Aceptar un cambio es conservarlo de forma definitiva. Rechazarlo es deshacerlo.

- **Revisar > Aceptar** acepta el cambio en el que está el cursor (o cada cambio del texto que haya seleccionado) y luego pasa al siguiente. **Revisar > Rechazar** hace lo mismo, deshaciendo el cambio.
- Haga clic en la flecha situada debajo de **Aceptar** o **Rechazar** para ver más opciones: **Aceptar todos los cambios mostrados**, **Aceptar todos los cambios** y **Aceptar todos los cambios y detener el seguimiento**; y **Rechazar todos los cambios mostrados** y **Rechazar todos los cambios**. «Mostrados» significa los cambios de los tipos que **Mostrar marcas** tiene activados (y, en el **Panel de revisiones**, del revisor que haya elegido).
- **Anterior** y **Siguiente**, en el grupo **Cambios**, pasan de un cambio al siguiente sin modificar nada.

Cada una de estas acciones es un paso, de modo que **Ctrl+Z** devuelve el documento exactamente a como estaba.

Aceptar una eliminación quita el texto de forma definitiva. Rechazar una inserción también quita el texto. Aceptar una inserción conserva el texto y quita la marca de que era nuevo. Rechazar una eliminación conserva el texto y quita el tachado. Rechazar un cambio de aspecto restaura el aspecto que tenía el texto. El texto que se movió son dos cambios: uno donde salió y otro donde llegó. Cuando el par está marcado con el mismo nombre, como lo guardan otros programas, aceptar o rechazar una de las mitades hace lo mismo con la otra, de modo que el texto queda movido de forma definitiva o vuelve a su sitio.

## El Panel de revisiones

Haga clic en **Revisar > Panel de revisiones** para abrir una lista de los cambios del documento: tipo, autor, fecha y palabras. Al hacer clic en uno, se muestra en el texto. Use **Aceptar**, **Rechazar**, **Anterior** y **Siguiente** debajo de la lista, o elija un revisor en el cuadro situado encima para ver solo los cambios de esa persona.

## Lo que aún no se registra

Los cambios en tablas (filas, celdas, combinación), en numeración y listas, en secciones y configuración de página, y en el texto de encabezados, pies de página y notas se realizan como de costumbre, sin registro. Los cambios que otros programas hayan registrado en esas partes se conservan y se guardan tal como estaban.

## Comparar dos versiones

Comparar busca las diferencias entre dos versiones de un documento y las muestra como cambios registrados.

1. Abra la versión más antigua. Es el **original**.
2. Haga clic en **Comparar** en el grupo **Comparar** de la ficha **Revisar**.
3. Haga clic en **Examinar** y elija la versión más reciente. Es la versión **revisada**.
4. Escriba el nombre con el que firmar los cambios. Elija las opciones que desee, como ignorar las mayúsculas.
5. Haga clic en **Aceptar**.

Se abre una ventana nueva con el texto original y los cambios que lo convierten en el texto revisado. Sus dos archivos no se modifican. Use **Aceptar** y **Rechazar** como con cualquier cambio registrado. Si acepta todos los cambios, obtiene el texto revisado. Si rechaza todos, obtiene el texto original.

Comparar analiza el texto del cuerpo del documento. No analiza tablas, encabezados, pies de página, notas ni el aspecto del texto. Un cambio solo de aspecto no se detecta.
