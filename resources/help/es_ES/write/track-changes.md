# Control de cambios: aceptar o rechazar cambios
<!-- keywords: comparar, comparar documentos, dos versiones, qué ha cambiado, control de cambios, cambios registrados, revisiones, marcas, aceptar, rechazar, aceptar todo, rechazar todo, mostrar para revisión, marcas simples, todas las marcas, sin marcas, original, mostrar marcas, globos, panel de revisiones, barra de cambios, inserción, eliminación, ctrl+mayús+e, autor, revisor, nombre de usuario, ediciones de otra persona -->

El **control de cambios** guarda un registro de cada modificación que usted hace, para que usted (u otra persona) pueda ver qué ha cambiado y decidir si lo conserva. Úselo cuando edite un documento que pertenece a otra persona, o que alguien vaya a revisar después.

## Activar y desactivar el control de cambios

Haga clic en **Revisar > Control de cambios**, o pulse **Ctrl+Mayús+E**. La barra de estado, en la parte inferior de la ventana, indica si está activado. Si hace clic en esas palabras, el control cambia de estado. El ajuste se guarda en el documento, así que sigue activado cuando el documento vuelve de otra persona.

Mientras está activado:

- **Lo que escribe** se marca como inserción.
- **Lo que elimina** no se quita. Permanece en el texto, tachado, para que pueda recuperarse. La excepción es el texto que usted mismo escribió mientras el control estaba activado: al eliminarlo, simplemente desaparece, porque nadie más lo había visto.
- **Un salto de párrafo que inserta** (Entrar) se marca como insertado, y uno que elimina (Retroceso al principio de un párrafo) se marca como eliminado.
- **Un cambio de aspecto** (negrita, fuente, tamaño, color, alineación, sangrías o estilo) se registra con el aspecto que tenía el texto antes.

Cada cambio se guarda con el nombre de quien lo hizo y la fecha y hora. El nombre es el que aparece en **Archivo > Opciones**, en **Nombre de usuario**. Cámbielo allí si es incorrecto.

Cada modificación es un paso, así que **Ctrl+Z** la deshace como cualquier otra edición.

## Aspecto de un cambio

Los cambios de cada persona tienen un color propio. El texto insertado aparece subrayado con el color de la persona, y el texto eliminado aparece tachado. Una barra en el margen izquierdo marca cada línea que tiene un cambio.

## Elegir lo que ve: Mostrar para revisión

Haga clic en **Revisar > Mostrar para revisión** y elija una opción:

- **Todas las marcas** muestra todos los cambios en el texto, con las barras en el margen.
- **Marcas simples** muestra el documento tal como quedará con los cambios aceptados, y una barra en el margen junto a cada línea que tiene un cambio.
- **Sin marcas** muestra el documento tal como quedará con todos los cambios aceptados, sin nada más.
- **Original** muestra el documento tal como era antes de los cambios: el texto eliminado vuelve a aparecer, y el texto insertado desaparece.

Estas opciones solo cambian lo que ve. Nunca modifican el documento y no se guardan con él. Un documento que tiene cambios se abre con **Todas las marcas**.

**Revisar > Mostrar marcas** elige qué tipos de cambio se muestran: **Inserciones y eliminaciones**, **Formato** y **Comentarios**. Debajo, elija cómo se muestra el texto eliminado. **Mostrar todas las revisiones insertadas en el texto** lo conserva en el texto, tachado. **Mostrar revisiones en globos** saca el texto eliminado de la línea y lo pone en un globo en el margen. **Mostrar solo comentarios y formato en globos** mantiene las inserciones y eliminaciones en el texto.

## Aceptar o rechazar cambios

Aceptar un cambio es conservarlo de forma definitiva. Rechazarlo es deshacerlo.

- **Revisar > Aceptar** acepta el cambio en el que está el cursor (o todos los cambios del texto seleccionado) y luego pasa al siguiente. **Revisar > Rechazar** hace lo mismo, deshaciendo el cambio.
- Haga clic en la flecha situada debajo de **Aceptar** o **Rechazar** para ver más opciones: **Aceptar todos los cambios mostrados**, **Aceptar todos los cambios** y **Aceptar todos los cambios y detener el seguimiento**; y **Rechazar todos los cambios mostrados** y **Rechazar todos los cambios**. «Mostrados» significa los cambios de los tipos que **Mostrar marcas** tiene activados (y, en el panel **Revisiones**, del revisor que haya elegido).
- **Anterior** y **Siguiente**, en el grupo **Cambios**, pasan de un cambio al siguiente sin modificar nada.

Cada una de estas acciones es un paso, así que **Ctrl+Z** devuelve el documento exactamente a como estaba.

Aceptar una eliminación borra el texto de forma definitiva. Rechazar una inserción también elimina el texto. Aceptar una inserción conserva el texto y quita la marca de que era nuevo. Rechazar una eliminación conserva el texto y quita el tachado. Rechazar un cambio de aspecto devuelve el aspecto que tenía el texto.

## El panel Revisiones

Haga clic en **Revisar > Panel de revisiones** para abrir una lista de los cambios del documento: tipo, autor, fecha y las palabras. Si hace clic en uno, se muestra en el texto. Use **Aceptar**, **Rechazar**, **Anterior** y **Siguiente** debajo de la lista, o elija un revisor en el cuadro situado encima para ver solo los cambios de esa persona.

## Lo que todavía no se registra

Los cambios en tablas (filas, celdas, combinación de celdas), en numeración y listas, en secciones y configuración de página, y en el texto de encabezados, pies de página y notas se realizan como de costumbre, sin registro. Los cambios que otros programas hayan registrado en esos lugares se conservan y se guardan tal como estaban.

## Comparar dos versiones

Comparar busca las diferencias entre dos versiones de un documento y las muestra como cambios registrados.

1. Abra la versión más antigua. Es el **original**.
2. Haga clic en **Comparar** en el grupo **Comparar** de la ficha **Revisar**.
3. Haga clic en **Examinar** y elija la versión más reciente. Es la versión **revisada**.
4. Escriba el nombre con el que firmar los cambios. Elija las opciones que desee, como ignorar las mayúsculas.
5. Haga clic en **Aceptar**.

Se abre una ventana nueva con el texto original y los cambios que lo convierten en el texto revisado. Sus dos ficheros no se modifican. Use **Aceptar** y **Rechazar** como con cualquier cambio registrado. Si acepta todos los cambios, obtiene el texto revisado. Si los rechaza todos, obtiene el texto original.

Comparar examina el texto del cuerpo del documento. No examina tablas, encabezados, pies de página, notas ni el aspecto del texto. Un cambio solo de aspecto no se detecta.
