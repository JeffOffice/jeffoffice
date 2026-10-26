# Reglas sobre lo que puede contener una celda
<!-- keywords: validación, regla, validación de datos, lista, desplegable, lista desplegable, número entero, decimal, fecha, hora, longitud, personalizada, detener, advertencia, información, mensaje de error, no permitido, opciones, flecha, Alt+Abajo -->

Un libro puede tener **reglas** que indican qué se puede escribir en algunas celdas: un número entero del 1 al 10, una fecha posterior a un día determinado, un texto de cinco letras como máximo o una opción de una lista. Las reglas forman parte del libro; JeffCrunch las aplica cuando **escribe** un valor en una celda (o elige uno de su lista). Los valores que pega, rellena o mueve a la celda no se comprueban, igual que en las hojas de cálculo habituales, de modo que una regla indica lo que se puede escribir, no lo que contiene la celda.

## Cuando un valor no cumple la regla

Cada regla tiene un **estilo**, y el mensaje es el de la propia regla cuando el libro lo incluye:

- **Detener**: aparece el mensaje y la entrada permanece en la celda para que la cambie. Presione **Esc** para descartarla.
- **Advertencia**: se le pregunta si desea conservar el valor de todos modos.
- **Información**: se le indica que el valor no cumple la regla, y puede continuar o descartarlo.

Un valor pegado no está sujeto a la regla: se introduce tal cual y no se pregunta nada. Siempre se puede borrar una celda.

## Elegir de una lista

Una celda que tiene una lista muestra un pequeño botón con una flecha en su borde derecho cuando el cursor está en ella. Haga clic en el botón, o presione **Alt+Abajo**, para ver las opciones, y haga clic en una para introducirla.

## Lo que aún no se hace

JeffCrunch conserva las reglas del libro, pero todavía no tiene ningún comando para crear reglas nuevas. Las reglas que leen sus opciones de otra hoja con la forma de escritura más reciente se conservan en el archivo, pero no se aplican. Los mensajes que aparecen al seleccionar una celda no se muestran.
