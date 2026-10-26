# Proteger una hoja o el libro
<!-- keywords: proteger, protección, contraseña, bloquear, celdas bloqueadas, desbloquear, desproteger, estructura, hojas, solo lectura, permitir, formato de celdas, insertar filas, ordenar, filtrar, hash -->

## Proteger una hoja

**Revisar > Proteger hoja** impide que otras personas cambien la hoja por error. En una hoja protegida, las celdas **bloqueadas** no se pueden cambiar. Todas las celdas están bloqueadas mientras no indique lo contrario: seleccione las celdas que se podrán cambiar, abra **Formato de celdas > Protección** y desactive **Bloqueada**. (Esto solo tiene efecto una vez que la hoja está protegida).

El cuadro de diálogo pregunta qué podrán seguir haciendo todos los usuarios en la hoja: seleccionar celdas bloqueadas o desbloqueadas, aplicar formato a celdas, cambiar columnas o filas, insertar o eliminar filas y columnas, insertar vínculos, ordenar, usar filtros y cambiar objetos de dibujo y notas. Lo que deje sin activar se rechaza, con un mensaje que indica qué se ha rechazado.

La **contraseña** es opcional. Si escribe una, la hoja solo se podrá desproteger con ella. JeffCrunch guarda una forma cifrada de la contraseña (un hash con sal, repetido 100.000 veces) y nunca la contraseña en sí, por lo que no puede mostrársela más tarde: guárdela en un lugar seguro. Las contraseñas creadas por otros programas, tanto en el formato antiguo como en el nuevo, se comprueban de la misma manera.

Proteger y desproteger son acciones que se pueden deshacer. Para quitar la protección, use **Desproteger hoja** y escriba la contraseña, si la hay.

Algunas acciones siguen sin estar permitidas en una hoja protegida, independientemente de lo que permita: combinar celdas y eliminar filas o columnas que contengan celdas bloqueadas.

## Proteger el libro

**Revisar > Proteger libro** bloquea la **estructura**: no se puede agregar, eliminar, cambiar el nombre, mover, ocultar ni mostrar ninguna hoja. También puede pedir que las ventanas se mantengan como están. Puede tener también una contraseña. **Desproteger libro** quita la protección.
