# Proteger una hoja o el libro

<!-- keywords: proteger, protección, contraseña, bloquear, celdas bloqueadas, desbloquear, desproteger, estructura, hojas, solo lectura, permitir, formato de celdas, insertar filas, ordenar, filtrar, hash, permitir rangos de edición, rango editable, contraseña del rango -->

## Proteger una hoja

**Revisar > Proteger hoja** evita que las personas cambien la hoja por accidente. En una hoja protegida, las celdas que están **bloqueadas** no se pueden modificar. Todas las celdas están bloqueadas hasta que indique lo contrario: seleccione las celdas que pueden cambiarse, abra **Formato de celdas > Protección** y desactive **Bloqueada**. (Esto solo importa cuando la hoja está protegida).

El cuadro de diálogo pregunta qué puede seguir haciendo todo el mundo en la hoja: seleccionar celdas bloqueadas o desbloqueadas, dar formato a las celdas, cambiar columnas o filas, insertar o eliminar filas y columnas, insertar vínculos, ordenar, usar filtros, y cambiar objetos de dibujo y notas. Lo que deje sin marcar se rechaza, con un mensaje que indica qué se ha rechazado.

Una **contraseña** es opcional. Si la indica, la hoja solo se podrá desproteger con ella. JeffCrunch guarda una forma codificada de la contraseña (un hash con sal, repetido 100 000 veces) y nunca la contraseña en sí, así que no puede mostrársela más tarde: guárdela en un lugar seguro. Las contraseñas creadas por otros programas, tanto en la forma antigua como en la nueva, se comprueban del mismo modo.

Proteger y desproteger son pasos que se pueden deshacer. Para quitar la protección, use **Desproteger hoja** y escriba la contraseña, si la hay.

Algunas cosas no se permiten en una hoja protegida, independientemente de lo que haya permitido: combinar celdas, y eliminar filas o columnas que contengan celdas bloqueadas.

## Permitir cambiar algunas celdas: Permitir rangos de edición

**Revisar > Permitir rangos de edición** muestra la lista de **rangos** que las personas pueden modificar aunque la hoja esté protegida. Un rango es un bloque de celdas, como A1:C5. Haga clic en **Nuevo** para agregar uno. Asígnele un **título**, escriba las celdas que abarca (use un espacio entre bloques) y, si lo desea, una **contraseña**. Un rango sin contraseña puede ser modificado por cualquiera. Un rango con contraseña solo se puede modificar después de que alguien escriba la contraseña la primera vez que intente cambiar una de sus celdas. A partir de entonces, el rango permanece abierto hasta que se cierre el libro.

**Modificar** cambia un rango y **Eliminar** lo quita. Puede configurar los rangos antes de proteger la hoja. Se guardan con el libro, y las contraseñas se conservan en la misma forma codificada que la contraseña de una hoja. Permitir rangos de edición es un paso que se puede deshacer. Cuando inserta o elimina filas, columnas o celdas, los rangos se desplazan con sus celdas, se reducen o amplían, o desaparecen si se eliminan todas sus celdas. Deshacer los restaura.

## Proteger el libro

**Revisar > Proteger libro** bloquea la **estructura**: no se puede agregar, eliminar, cambiar de nombre, mover, ocultar ni mostrar ninguna hoja. También puede pedir que las ventanas se mantengan como están. Puede tener una contraseña. **Desproteger libro** quita la protección.
