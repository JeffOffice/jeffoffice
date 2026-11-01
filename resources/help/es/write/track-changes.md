# Controlar cambios y aceptarlos o rechazarlos
<!-- keywords: control de cambios, controlar cambios, cambios controlados, revisiones, marcas, aceptar, rechazar, aceptar todo, rechazar todo, mostrar para revisión, marcas simples, todas las marcas, sin marcas, original, mostrar marcas, globos, panel de revisiones, barra de cambios, inserción, eliminación, ctrl+mayús+e, autor, revisor, nombre de usuario, modificaciones de otra persona, correcciones -->

El **Control de cambios** guarda un registro de cada modificación que haga, para que usted (u otra persona) pueda ver qué ha cambiado y decidir si conservarlo. Úselo cuando modifique un documento que pertenece a otra persona o que otra persona revisará después.

## Activar y desactivar el control de cambios

Haga clic en **Revisar > Control de cambios** o presione **Ctrl+Mayús+E**. La barra de estado, en la parte inferior de la ventana, indica si está activado. Al hacer clic en ese texto, cambia al estado contrario. La configuración se guarda en el documento, de modo que sigue activada cuando el documento vuelve de otra persona.

Mientras está activado:

- **Lo que escribe** se marca como inserción.
- **Lo que elimina** no desaparece. Permanece en el texto, tachado, para poder recuperarlo. La única excepción es el texto que haya escrito usted mismo con el control activado: al eliminarlo, simplemente desaparece, porque nadie más lo ha visto.
- **Un salto de párrafo que crea** (Entrar) se marca como insertado, y uno que elimina (Retroceso al principio de un párrafo) se marca como eliminado.
- **Un cambio de aspecto** (negrita, fuente, tamaño, color, alineación, sangrías o estilo) se registra junto con el aspecto que tenía antes el texto.

Cada cambio se guarda con el nombre de quien lo hizo y la fecha. El nombre es el que figura en **Archivo > Opciones**, en **Nombre de usuario**. Cámbielo ahí si no es correcto.

Cada modificación es un solo paso, así que **Ctrl+Z** la deshace como cualquier otra.

## Aspecto de un cambio

Los cambios de cada persona tienen un color propio. El texto insertado aparece subrayado en el color de esa persona y el texto eliminado, tachado. Una barra en el margen izquierdo marca cada línea que contiene un cambio.

## Elegir qué se ve: Mostrar para revisión

Haga clic en **Revisar > Mostrar para revisión** y elija una opción:

- **Todas las marcas** muestra todos los cambios en el texto, con las barras en el margen.
- **Marcas simples** muestra el documento tal como quedará con los cambios aceptados, con una barra en el margen junto a cada línea que contiene un cambio.
- **Sin marcas** muestra el documento tal como quedará con todos los cambios aceptados, sin nada más.
- **Original** muestra el documento tal como era antes de los cambios: el texto eliminado vuelve a aparecer y el insertado desaparece.

Estas opciones solo cambian lo que ve. Nunca modifican el documento ni se guardan con él. Un documento que contiene cambios se abre con **Todas las marcas**.

**Revisar > Mostrar marcas** permite elegir qué tipos de cambio se muestran: **Inserciones y eliminaciones**, **Formato** y **Comentarios**. Debajo, elija cómo se muestra el texto eliminado. **Mostrar todas las revisiones insertadas en el texto** lo mantiene en el texto, tachado. **Mostrar revisiones en globos** quita el texto eliminado de la línea y lo coloca en un globo en el margen. **Mostrar solo comentarios y formato en globos** mantiene las inserciones y eliminaciones en el texto.

## Aceptar o rechazar cambios

Aceptar un cambio significa conservarlo definitivamente. Rechazarlo significa deshacerlo.

- **Revisar > Aceptar** acepta el cambio en el que está el cursor (o todos los cambios del texto seleccionado) y pasa al siguiente. **Revisar > Rechazar** hace lo mismo, pero deshace el cambio.
- Haga clic en la flecha situada bajo **Aceptar** o **Rechazar** para ver más opciones: **Aceptar todos los cambios mostrados**, **Aceptar todos los cambios** y **Aceptar todos los cambios y detener el seguimiento**; y **Rechazar todos los cambios mostrados** y **Rechazar todos los cambios**. "Mostrados" se refiere a los cambios de los tipos activados en **Mostrar marcas** (y, en el **Panel de revisiones**, a los del revisor elegido).
- **Anterior** y **Siguiente**, en el grupo **Cambios**, permiten ir de un cambio a otro sin modificar nada.

Cada una de estas acciones es un solo paso, así que **Ctrl+Z** devuelve el documento exactamente a como estaba.

Aceptar una eliminación quita el texto definitivamente. Rechazar una inserción también quita el texto. Aceptar una inserción conserva el texto y quita la marca que lo señalaba como nuevo. Rechazar una eliminación conserva el texto y quita el tachado. Rechazar un cambio de aspecto devuelve al texto el aspecto que tenía.

## El panel de revisiones

Haga clic en **Revisar > Panel de revisiones** para abrir una lista de los cambios del documento: tipo, autor, fecha y texto. Al hacer clic en uno, se muestra en el texto. Use **Aceptar**, **Rechazar**, **Anterior** y **Siguiente**, debajo de la lista, o elija un revisor en el cuadro situado encima para ver solo los cambios de esa persona.

## Qué no se controla todavía

Los cambios en tablas (filas, celdas, combinación), en la numeración y las listas, en las secciones y la configuración de página, y en el texto de encabezados, pies de página y notas se aplican de la forma habitual, sin registro. Los cambios que otros programas hayan registrado en esos lugares se conservan y se guardan tal como estaban.
