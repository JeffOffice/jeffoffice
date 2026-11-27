# Trabajar con hojas
<!-- keywords: hoja, hojas, pestaña, pestañas, agregar, insertar, nueva hoja, eliminar, quitar, cambiar nombre, renombrar, nombre, mover, copiar, duplicar, reordenar, arrastrar, ocultar, mostrar, color de pestaña, color, seleccionar varias hojas, Mayús+F11, #REF!, referencia 3D, Mover o copiar -->

Un libro de trabajo se compone de **hojas**, y las **pestañas** de la parte inferior de la ventana son sus nombres. Haga clic en una pestaña para mostrar esa hoja. Cada cambio que se describe a continuación es un paso que puede deshacer y se guarda en el libro de trabajo.

## Agregar una hoja

- Haga clic en el botón **+** después de la última pestaña para agregar una hoja al final.
- Pulse **Mayús+F11**, o use **Inicio > Insertar > Insertar hoja**, para agregar una hoja delante de la que está viendo.

La hoja nueva está vacía, tiene el siguiente nombre libre (Hoja4, Hoja5, y así sucesivamente) y se abre de inmediato. Un libro de trabajo puede tener hasta 1000 hojas; a partir de ese número, JeffCrunch lo indica y no agrega ninguna, porque un fichero con más no es uno que pueda abrir.

## Cambiar el nombre de una hoja

**Haga doble clic** en la pestaña, escriba el nuevo nombre y pulse **Entrar** (**Esc** cancela el cambio). También puede usar **Inicio > Formato > Cambiar nombre de hoja**.

Un nombre puede tener hasta 31 caracteres. No puede estar vacío, no puede contener ninguno de los caracteres **: \ / ? * [ ]**, no puede empezar ni terminar con un apóstrofo, y no puede ser igual al nombre de otra hoja (las mayúsculas no cuentan como diferencia). El nombre «History» está reservado para el programa. Si no se acepta el nombre, JeffCrunch indica el motivo y la hoja conserva su nombre.

Toda fórmula que nombre la hoja se actualiza con el nuevo nombre. Por ejemplo, **=Presupuesto!B2** pasa a ser **='Presupuesto T1'!B2** si cambia el nombre de Presupuesto a Presupuesto T1. Los nombres que apuntan a la hoja, las reglas de formato condicional y de validación de datos, y los gráficos también la siguen.

## Eliminar una hoja

Haga clic con el botón derecho en la pestaña y elija **Eliminar hoja**, o use **Inicio > Eliminar > Eliminar hoja**. Si la hoja contiene algo, JeffCrunch le pedirá confirmación antes. Las fórmulas de otras hojas que leen la hoja eliminada muestran **#REF!**, lo que significa que los datos que leían ya no existen. **Deshacer** devuelve la hoja y esas fórmulas.

Un libro de trabajo siempre conserva al menos una hoja visible.

## Mover o copiar una hoja

- **Arrastre** una pestaña a otra posición para mover la hoja.
- O use **Inicio > Formato > Mover o copiar hoja**. Elija delante de qué hoja debe colocarse la hoja seleccionada (o **(después de la última hoja)**), y marque **Copiar, y dejar los originales donde están** para copiarla en lugar de moverla. La copia se llama como **Presupuesto (2)** y se abre.

Las fórmulas siguen leyendo las hojas que leían, estén donde estén. Una copia tiene las mismas celdas, fórmulas, formatos y tamaños que la hoja de origen. Una referencia sin nombre de hoja (como **A1**) lee la celda de la propia copia; una referencia que nombra una hoja (como **Hoja1!A1**) sigue apuntando a esa hoja, aunque sea la hoja de la que procede la copia.

Una copia también conserva la protección, el filtro, los vínculos, la configuración de página y los saltos de página de la original, los grupos de filas y columnas, y el aspecto de sus barras de datos; los vínculos siguen las filas que haya insertado o eliminado, en la original antes de la copia y en la copia después de ella. No tiene las imágenes, gráficos, notas, tablas dinámicas, minigráficos ni los nombres que pertenecen solo a la original, porque JeffCrunch todavía no los copia; en ese caso se lo indica. Una hoja que contiene tablas se copia junto con copias de ellas (cada una con un nombre, un identificador y una parte propia, y las fórmulas de la copia nombran sus propias tablas). Al eliminar una hoja se eliminan también sus tablas; una fórmula de otra hoja que nombraba alguna de ellas mostrará entonces #REF!, y **Deshacer** la devuelve.

Para copiar una hoja a **otro libro de trabajo** que esté abierto, elíjalo en **Al libro de trabajo** dentro del mismo cuadro. El cuadro enumera lo que la copia no tendrá (por ahora, las imágenes, gráficos, formatos condicionales, tablas, notas y la forma en que la hoja se imprime) y lo indica antes de copiar nada. Las fórmulas que leen otras hojas del primer libro se convierten en vínculos a su fichero, así que el primer libro debe haberse guardado al menos una vez como fichero .xlsx. La copia muestra los valores que el primer libro tiene en ese momento, y cambian al actualizar los vínculos (**Actualizar valores** en **Datos > Editar vínculos**). Si el primer libro tiene cambios sin guardar, los vínculos leen su fichero guardado al actualizarlos, así que guárdelo para conservar estos valores. Las fórmulas que leen la propia hoja copiada permanecen en la copia. Las celdas que usan nombres, tablas, una matriz o un rango derramado, o que leen un tercer libro de trabajo, se copian como los valores que muestran, y el cuadro indica cuántas son. La forma en que se muestra la hoja (filas y columnas inmovilizadas, zoom, líneas de cuadrícula y sentido de lectura), así como su altura de fila y ancho de columna predeterminados, también se copian. La copia conserva el mismo aspecto: formatos, fuentes y colores se añaden al otro libro (un color del tema del primer libro se conserva como el color que muestra cuando los dos temas son diferentes). Ambos libros deben contar las fechas desde el mismo año (1900 o 1904). **Deshacer** en el otro libro quita la copia; el primer libro no se modifica. Mover una hoja a otro libro de trabajo todavía no está disponible.

## Ocultar y mostrar una hoja

Haga clic con el botón derecho en la pestaña y elija **Ocultar hoja** (o use **Inicio > Formato > Ocultar hoja**). La hoja deja de mostrarse, pero sigue en el libro de trabajo y las fórmulas siguen leyéndola. Para volver a mostrarla, haga clic con el botón derecho en cualquier pestaña, elija **Mostrar hoja** y, a continuación, haga clic en su nombre en la lista. La lista está vacía cuando no hay ninguna hoja oculta.

## Dar color a una pestaña

Haga clic con el botón derecho en la pestaña y elija **Color de pestaña**; después, seleccione un color o elija **Sin color**. El color se muestra como una línea debajo del nombre. Un color que elija de la paleta del tema se guarda como el color sin más que tiene en ese momento.

## Varias hojas a la vez

**Ctrl+clic** en las pestañas para seleccionar más de una, o haga clic en una pestaña y **Mayús+clic** en otra para seleccionar todas las pestañas situadas entre ambas. Las pestañas seleccionadas aparecen sombreadas. **Eliminar**, **Ocultar**, **Color de pestaña** y **Mover o copiar** se aplican entonces a todas ellas en un solo paso. Haga clic en una pestaña que no esté seleccionada para seleccionar solo esa.

Lo que escriba o formatee mientras hay varias hojas seleccionadas solo cambia la hoja que está viendo.

## Lo que no se puede hacer

- Un libro de trabajo que tenga una fórmula sobre un rango de hojas, como **=SUMA(Hoja1:Hoja3!A1)**, no puede tener hojas agregadas, eliminadas, renombradas o movidas todavía, porque eso cambiaría las hojas que abarca la fórmula.
- Después de guardar, **Deshacer** no puede retroceder más allá de una hoja que haya agregado, copiado, eliminado, renombrado o movido antes del guardado. El fichero guardado las conserva tal como están. Ocultar, mostrar y el color de pestaña todavía se pueden deshacer.
