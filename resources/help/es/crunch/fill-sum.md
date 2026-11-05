# Rellenar celdas y sumarlas

<!-- keywords: rellenar, serie, arrastrar, controlador, doble clic, lista personalizada, Autorrellenar, Ctrl+D, Ctrl+R, abajo, derecha, arriba, izquierda, número, fecha, día laborable, mes, año, incremento, detener, copiar, suma, sumar, promedio, contar, máximo, mínimo, Alt+signo igual, total, Relleno relámpago, Ctrl+E, patrón, dividir nombres -->

## Rellenar

Rellenar continúa lo que ya ha escrito. Seleccione las celdas con los valores iniciales y, después, arrastre el cuadrado pequeño de la esquina de la selección (el **controlador de relleno**) a lo largo de una columna o una fila, y suelte el botón. Un borde discontinuo muestra hasta dónde llegará el relleno.

| Valores iniciales | El relleno da como resultado |
| --- | --- |
| 1, 2 | 3, 4, 5, y así sucesivamente: se mantiene el incremento entre ellos |
| Un único número, como 5 | 5, 5, 5: copias (mantenga pulsada Ctrl mientras arrastra para obtener la opción contraria) |
| Una fecha | Los días siguientes (use el botón **Opciones de relleno** o **Rellenar > Serie** para días laborables, meses o años) |
| Lun o enero | Mar, mié, ... o febrero, marzo, ... en su idioma |
| Elemento 1 | Elemento 2, Elemento 3, ... |
| Una palabra de una lista personalizada, como Pequeña | La palabra siguiente de la lista, y vuelve a la primera después de la última |
| Una fórmula | La fórmula copiada, con sus referencias desplazadas como en un pegado |

Haga doble clic en el controlador de relleno para rellenar hacia abajo hasta donde lleguen los datos contiguos a la selección. JeffCrunch mira primero la columna de la izquierda y, después, la de la derecha. Se detiene en la primera celda vacía de esa columna.

## Listas personalizadas

Una **lista personalizada** es una serie de palabras que Rellenar recorre, como los días de la semana. Para crear una, haga clic en **Archivo > Opciones > Listas personalizadas**. Escriba una lista por línea, con comas entre las palabras, por ejemplo `Pequeña, Mediana, Grande`. Haga clic en **Aceptar**. Ahora escriba **Pequeña** en una celda y arrastre el controlador de relleno: las celdas se rellenarán con Mediana, Grande, Pequeña, y así sucesivamente. Al buscar una palabra en una lista, JeffCrunch no distingue mayúsculas de minúsculas, y las celdas nuevas copian las mayúsculas de la celda de partida. Una lista necesita al menos dos palabras. Los días de la semana y los meses están integrados.

**Rellenar > Abajo** (**Ctrl+D**) y **Rellenar > Derecha** (**Ctrl+R**) copian la primera fila o columna de la selección sobre el resto. **Rellenar > Arriba** y **Rellenar > Izquierda** trabajan desde la última. **Rellenar > Serie** pide el tipo (lineal, geométrica, fecha), la unidad de una fecha, el incremento y el valor final. Cada celda rellenada toma el formato de la celda que la precede. Una celda que no se puede modificar (parte de una matriz, una celda bloqueada o el encabezado de una tabla) detiene el relleno, y no cambia nada.

Después de arrastrar el controlador de relleno, aparece el pequeño botón **Opciones de relleno** en la esquina de lo rellenado. Su menú vuelve a rellenar de otra forma, y el relleno sigue siendo un solo paso de Deshacer: **Copiar celdas**, **Serie de relleno**, **Rellenar solo formato** (las celdas conservan su contenido y adoptan el formato), **Rellenar sin formato** (las celdas conservan su propio formato) y, para fechas, **Rellenar días**, **Rellenar días laborables**, **Rellenar meses** y **Rellenar años**. El botón desaparece cuando hace cualquier otra cosa.

Un relleno no entra ni sale de las celdas combinadas, porque dejaría valores en celdas que el área combinada oculta. Sepárelas antes.

**Rellenar > Justificar** coloca el texto de las celdas seleccionadas de una columna en líneas tan anchas como la columna, en las celdas desde la primera hacia abajo; las celdas sobrantes de la selección se vacían. Si el texto necesita más celdas de las seleccionadas, JeffCrunch pregunta si debe usar las celdas inferiores. Solo funciona con texto.

**Rellenar > Relleno relámpago** (**Ctrl+E**) rellena una columna por usted a partir de un patrón que le muestra. Escriba a mano el resultado que desea en la primera o las dos primeras celdas de una columna, junto a las celdas de las que procede. Después, coloque el cursor en esa columna y elija Relleno relámpago. JeffCrunch deduce cómo se crearon sus ejemplos a partir de las celdas contiguas y rellena las celdas vacías del mismo modo. Por ejemplo, la columna A contiene nombres como **Smith, Ann**. En B1 escribe **Ann Smith**. Relleno relámpago completa B2 y las celdas siguientes con los demás nombres, invertidos de la misma forma. Puede tomar una palabra de una celda, la primera letra de una palabra o las primeras o últimas letras, en mayúsculas, en minúsculas o tal como está, y puede añadir un texto que sea igual en todos los ejemplos, como un espacio o un punto. Si no encuentra un patrón, se lo indica. Escriba uno o dos ejemplos más e inténtelo de nuevo. Es un solo paso de Deshacer.

**Rellenar > Entre hojas de cálculo** copia las celdas seleccionadas en las mismas celdas de las hojas que marque en una lista: todo, solo el contenido o solo los formatos. Es un solo paso de Deshacer.

## Suma

**Inicio > Suma** (**Alt y el signo igual**) coloca un total en la celda seleccionada. Busca los números situados encima de la celda; si no hay ninguno, busca los números a su izquierda. La fórmula se escribe en la celda para que la revise, y **Entrar** la conserva. La flecha junto a Suma ofrece los otros totales: **Promedio**, **Contar números**, **Máx** y **Mín**.

Si primero selecciona un bloque, cada columna del bloque recibe su propia fórmula en las celdas vacías situadas debajo (o, para una sola fila, a su derecha).
