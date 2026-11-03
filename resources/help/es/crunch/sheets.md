# Trabajar con hojas
<!-- keywords: hoja, hojas, hoja de cálculo, pestaña, pestañas, agregar, añadir, insertar, nueva hoja, hoja nueva, eliminar, borrar, quitar, cambiar nombre, renombrar, nombre, mover, copiar, duplicar, reordenar, ordenar hojas, arrastrar, ocultar, mostrar, color de pestaña, color, seleccionar varias hojas, Mayús+F11, #REF!, referencia 3D, Mover o copiar -->

Un libro se compone de **hojas**, y las **pestañas** de la parte inferior de la ventana son sus nombres. Haga clic en una pestaña para mostrar esa hoja. Cada uno de los cambios siguientes es un paso que puede deshacer y se guarda en el libro.

## Agregar una hoja

- Haga clic en el botón **+** situado tras la última pestaña para agregar una hoja al final.
- Presione **Mayús+F11**, o use **Inicio > Insertar > Insertar hoja**, para agregar una hoja delante de la que está viendo.

La hoja nueva está vacía, recibe el siguiente nombre libre (Hoja4, Hoja5, etc.) y se abre de inmediato. Un libro puede tener hasta 1.000 hojas; si se supera esa cifra, JeffCrunch lo indica y no agrega ninguna, porque no puede abrir un archivo con más hojas.

## Cambiar el nombre de una hoja

**Haga doble clic** en la pestaña, escriba el nombre nuevo y presione **Entrar** (**Esc** lo cancela). También puede usar **Inicio > Formato > Cambiar nombre de hoja**.

Un nombre puede tener hasta 31 caracteres. No puede estar vacío, no puede contener ninguno de los caracteres **: \ / ? * [ ]**, no puede empezar ni terminar con un apóstrofo y no puede coincidir con el nombre de otra hoja (las mayúsculas y minúsculas no se consideran diferentes). El nombre "History" está reservado para el programa. Si el nombre no se acepta, JeffCrunch indica el motivo y la hoja conserva su nombre.

Todas las fórmulas que hacen referencia a la hoja siguen el nombre nuevo. Ejemplo: **=Presupuesto!B2** pasa a ser **='Presupuesto T1'!B2** al cambiar el nombre de Presupuesto a Presupuesto T1. También lo siguen los nombres que apuntan a la hoja, las reglas de formato condicional y de validación, y los gráficos.

## Eliminar una hoja

Haga clic con el botón derecho en la pestaña y elija **Eliminar hoja**, o use **Inicio > Eliminar > Eliminar hoja**. Si la hoja tiene contenido, JeffCrunch le pide confirmación. Las fórmulas de otras hojas que leen la hoja eliminada muestran **#REF!**, lo que significa que lo que leían ya no existe. **Deshacer** recupera la hoja y esas fórmulas.

Un libro siempre conserva al menos una hoja visible.

## Mover o copiar una hoja

- **Arrastre** una pestaña a otra posición para mover la hoja.
- O bien use **Inicio > Formato > Mover o copiar hoja**. Elija la hoja delante de la cual debe colocarse la hoja elegida (o **(después de la última hoja)**) y active **Copiarlas y dejar las originales donde están** para copiarla en lugar de moverla. La copia recibe un nombre como **Presupuesto (2)** y se abre.

Las fórmulas siguen leyendo las mismas hojas, estén donde estén. Una copia tiene las mismas celdas, fórmulas, formatos y tamaños que la hoja de origen. Una referencia sin nombre de hoja (como **A1**) lee la celda de la propia copia; una referencia que nombra una hoja (como **Hoja1!A1**) sigue haciendo referencia a esa hoja, aunque sea la hoja de la que procede la copia.

Una copia también conserva la protección, el filtro, los vínculos, la configuración de página y los saltos de página, los grupos de filas y columnas, y el aspecto de las barras de datos del original; los vínculos siguen las filas insertadas o eliminadas, en el original antes de la copia y en la copia después de ella. No incluye las imágenes, gráficos, notas, tablas dinámicas, minigráficos ni los nombres que pertenecen solo al original, porque JeffCrunch aún no los copia; cuando esto ocurre, le indica cuáles. Una hoja con tablas se copia con copias de ellas (cada una con su propio nombre, identificador y parte, y las fórmulas de la copia hacen referencia a sus propias tablas). Al eliminar una hoja, se eliminan también sus tablas; una fórmula de otra hoja que hacía referencia a una de ellas muestra entonces #REF!, y Deshacer la recupera.

## Ocultar y mostrar una hoja

Haga clic con el botón derecho en la pestaña y elija **Ocultar hoja** (o use **Inicio > Formato > Ocultar hoja**). La hoja no se muestra, pero sigue en el libro y las fórmulas la siguen leyendo. Para volver a mostrarla, haga clic con el botón derecho en cualquier pestaña, elija **Mostrar hoja** y haga clic en su nombre en la lista. La lista está vacía cuando no hay ninguna hoja oculta.

## Colorear una pestaña

Haga clic con el botón derecho en la pestaña, elija **Color de pestaña** y seleccione un color, o elija **Sin color**. El color aparece como una línea debajo del nombre. Un color elegido de la paleta del tema se guarda como el color fijo que es en ese momento.

## Varias hojas a la vez

Haga **Ctrl+clic** en las pestañas para seleccionar más de una, o haga clic en una pestaña y **Mayús+clic** en otra para seleccionar todas las pestañas intermedias. Las pestañas seleccionadas aparecen sombreadas. **Eliminar**, **Ocultar**, **Color de pestaña** y **Mover o copiar** actúan entonces sobre todas ellas en un solo paso. Haga clic en una pestaña no seleccionada para seleccionar solo esa.

Lo que escriba o el formato que aplique mientras hay varias hojas seleccionadas solo cambia la hoja que está viendo.

## Lo que no se puede hacer

- En un libro que contiene una fórmula que abarca un bloque de hojas, como **=SUMA(Hoja1:Hoja3!A1)**, aún no se pueden agregar, eliminar, mover ni cambiar el nombre de las hojas, porque eso cambiaría las hojas que abarca la fórmula.
- Después de guardar, **Deshacer** no puede retroceder más allá de una hoja que agregó, copió, eliminó, movió o cuyo nombre cambió antes de guardar. El archivo guardado las contiene tal como están. Ocultar, mostrar y el color de pestaña sí se pueden seguir deshaciendo.
