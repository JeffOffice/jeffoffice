# Insertar una función con el cuadro de diálogo Argumentos de función
<!-- keywords: argumentos de función, argumento, fx, resultado de la fórmula, Mayús+F3, seleccionar celdas, rango, intervalo, opcional, repetir, modificar una función, editar función, función anidada, error -->

El cuadro de diálogo **Argumentos de función** le ayuda a completar una función argumento por argumento y muestra el resultado de la fórmula antes de insertarla en la celda.

## Abrirlo

- Elija **Elegir una función** en la pestaña Fórmulas (**Mayús+F3**), o haga clic en **fx** junto a la barra de fórmulas, y elija una función. El cuadro de diálogo se abre para esa función.
- Para cambiar una función que ya está en una fórmula, coloque el cursor dentro de sus paréntesis (en la celda o en la barra de fórmulas) y presione **Mayús+F3** o haga clic en **fx**. El cuadro de diálogo se abre para esa función, con sus argumentos en los cuadros. En una fórmula como =SUMA(A1; SI(B1>2; 5; 6)), con el cursor dentro de SI se cambia SI, y con el cursor dentro de SUMA pero fuera de SI se cambia SUMA.

## Completarlo

Cada argumento tiene una fila: su nombre, un cuadro para escribirlo y, a la derecha, su valor actual, como 12,5, "texto", VERDADERO o {1;2;3} para un rango de celdas. Un nombre en negrita es obligatorio; un nombre que no está en negrita es opcional y se puede dejar vacío.

- **Tab** pasa al cuadro siguiente y **Mayús+Tab**, al anterior.
- La línea bajo los cuadros indica qué hace la función, y la línea siguiente describe el argumento en el que se encuentra.
- Funciones como SUMA admiten tantos argumentos como necesite: al escribir en el último cuadro, aparece uno nuevo.
- El botón a la derecha de un cuadro permite seleccionar celdas. El cuadro de diálogo se reduce a una pequeña barra y usted selecciona las celdas en la hoja con el mouse (el cuadro muestra su referencia mientras arrastra). Haga clic en el botón de la barra o presione **Entrar** para volver.

## El resultado

**Resultado de la fórmula =** muestra el resultado de toda la función, un momento después de que deje de escribir. No se escribe nada en la hoja hasta que presione **Aceptar**. Si algo está mal, una línea bajo el resultado lo explica con palabras: por ejemplo, que no se encontró un valor, que algo se divide entre cero o que un número no está permitido. Una fórmula que tardaría demasiado en calcularse aquí no se muestra; presione **Aceptar** y la hoja la calculará.

**Ayuda sobre esta función** abre esta página.

## Finalizar

Presione **Entrar** o haga clic en **Aceptar** para insertar la función. En una celda que no se está editando, la fórmula se escribe en la celda. En una fórmula que está escribiendo, la función se inserta en la posición del cursor, o reemplaza la que cambió, y puede seguir escribiendo. Presione **Esc** o haga clic en **Cancelar** y no cambiará nada.
