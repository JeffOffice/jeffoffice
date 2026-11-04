# Rellenar celdas y sumarlas
<!-- keywords: rellenar, serie, arrastrar, controlador, doble clic, lista personalizada, Autorrellenar, Ctrl+D, Ctrl+R, abajo, derecha, arriba, izquierda, número, fecha, día laborable, mes, año, paso, detener, copiar, Suma, suma, promedio, contar, máx, mín, Alt+=, total -->

## Rellenar

Rellenar continúa lo que ha empezado. Seleccione las celdas que ha rellenado y arrastre el cuadrado pequeño de la esquina de la selección (el **controlador de relleno**) a lo largo de una columna o una fila, y suelte el botón. Un borde discontinuo muestra hasta dónde llegará el relleno.

| Ha escrito | El relleno da como resultado |
| --- | --- |
| 1, 2 | 3, 4, 5, y así sucesivamente: continúa el paso entre ambos |
| Un solo número, como 5 | 5, 5, 5: copias (mantenga pulsada Ctrl mientras arrastra para obtener la opción contraria) |
| Una fecha | Los días siguientes (use el botón **Opciones de relleno**, o **Rellenar > Rellenar serie**, para días laborables, meses o años) |
| Lun, o enero | Mar, Mié, ... o febrero, marzo, ... en su idioma |
| Elemento 1 | Elemento 2, Elemento 3, ... |
| Una palabra de una lista personalizada, como Pequeño | La palabra siguiente de la lista, y vuelve a la primera después de la última |
| Una fórmula | La fórmula copiada, con sus referencias desplazadas como en un pegado |

Haga doble clic en el controlador de relleno para rellenar hacia abajo hasta donde lleguen los datos situados junto a la selección. JeffCrunch mira primero la columna de la izquierda y después la de la derecha. Se detiene en la primera celda vacía de esa columna.

## Listas personalizadas

Una **lista personalizada** es una fila de palabras que Rellenar recorre, como los días de la semana. Para crear una, haga clic en **Archivo > Opciones > Listas personalizadas**. Escriba una lista por línea, con comas entre las palabras, por ejemplo `Pequeño, Mediano, Grande`. Haga clic en **Aceptar**. Ahora escriba **Pequeño** en una celda y arrastre el controlador de relleno: las celdas se rellenarán con Mediano, Grande, Pequeño, y así sucesivamente. No importan las mayúsculas cuando JeffCrunch busca una palabra en una lista, y las celdas nuevas copian las mayúsculas de la celda de la que partió. Una lista necesita al menos dos palabras. Los días de la semana y los meses ya vienen incluidos.

**Rellenar > Abajo** (**Ctrl+D**) y **Rellenar > Derecha** (**Ctrl+R**) copian la primera fila o columna de la selección sobre el resto. **Rellenar > Arriba** e **Izquierda** trabajan desde la última. **Rellenar > Rellenar serie** pide el tipo (lineal, geométrica, fecha), la unidad de una fecha, el paso y el valor en el que debe detenerse. Cada celda rellenada adopta el formato de la celda que la precede. Una celda que no se puede cambiar (parte de una matriz, una celda bloqueada o el encabezado de una tabla) detiene el relleno, y no cambia nada.

Después de arrastrar el controlador de relleno, aparece el pequeño botón **Opciones de relleno** en la esquina de lo rellenado. Su menú vuelve a hacer el relleno de otra forma, y el relleno sigue siendo un solo paso de Deshacer: **Copiar celdas**, **Rellenar serie**, **Rellenar solo formato** (las celdas conservan su contenido y adoptan el formato), **Rellenar sin formato** (las celdas conservan su propio formato) y, para las fechas, **Rellenar días**, **Rellenar días laborables**, **Rellenar meses** y **Rellenar años**. El botón desaparece cuando hace cualquier otra cosa.

Un relleno no entra ni sale de las celdas combinadas, porque dejaría valores en celdas que el área combinada oculta. Sepárelas primero.

**Rellenar > Justificar** distribuye el texto de las celdas seleccionadas de una columna en líneas tan anchas como la columna, empezando por la primera celda y hacia abajo; las celdas sobrantes de la selección se vacían. Si el texto necesita más celdas de las que ha seleccionado, JeffCrunch pregunta si debe usar las celdas inferiores. Solo funciona con texto.

**Rellenar > Rellenar en hojas de cálculo** copia las celdas seleccionadas a las mismas celdas de las hojas que marque en una lista: todo, solo el contenido o solo los formatos. Es un solo paso de Deshacer.

## Suma

**Inicio > Suma** (**Alt+=**) pone un total en la celda que seleccione. Mira los números situados encima de la celda; si no hay ninguno, mira los números situados a su izquierda. La fórmula se escribe en la celda para que la compruebe, y **Intro** la mantiene. La flecha junto a Suma ofrece los otros totales: **Promedio**, **Contar números**, **Máx.** y **Mín.**

Si selecciona un bloque primero, cada columna del bloque recibe su propia fórmula en las celdas vacías situadas debajo (o, para una sola fila, a su derecha).
