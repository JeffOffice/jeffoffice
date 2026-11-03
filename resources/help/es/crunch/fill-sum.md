# Rellenar celdas y sumarlas
<!-- keywords: rellenar, relleno, serie, series, arrastrar, controlador de relleno, autorrellenar, Ctrl+D, Ctrl+R, hacia abajo, hacia la derecha, hacia arriba, hacia la izquierda, número, fecha, día laborable, mes, año, incremento, límite, copiar, suma, sumar, autosuma, promedio, contar, máximo, mínimo, Alt+igual, total -->

## Rellenar

El relleno continúa lo que usted empezó. Seleccione las celdas que rellenó y arrastre el pequeño cuadrado de la esquina de la selección (el **controlador de relleno**) a lo largo de una columna o una fila, y suéltelo. Un borde discontinuo muestra hasta dónde llegará.

| Si empieza con | El relleno le da |
| --- | --- |
| 1, 2 | 3, 4, 5, etc.: continúa el incremento entre ellos |
| Un solo número, como 5 | 5, 5, 5: copias (mantenga presionada Ctrl mientras arrastra para obtener lo contrario) |
| Una fecha | Los días siguientes (use el botón **Opciones de relleno** o **Rellenar > Series** para días laborables, meses o años) |
| Lun o enero | Mar, Mié, ... o febrero, marzo, ... en su idioma |
| Elemento 1 | Elemento 2, Elemento 3, ... |
| Una fórmula | La fórmula copiada, con sus referencias desplazadas como al pegar |

**Rellenar > Hacia abajo** (**Ctrl+D**) y **Rellenar > Hacia la derecha** (**Ctrl+R**) copian la primera fila o columna de la selección sobre el resto. **Rellenar > Hacia arriba** y **Rellenar > Hacia la izquierda** parten de la última. **Rellenar > Series** pide el tipo (lineal, geométrica, cronológica), la unidad de tiempo de una fecha, el incremento y el límite. Cada celda rellenada toma el formato de la celda a la que sigue. Una celda que no se puede cambiar (parte de una matriz, una celda bloqueada o el encabezado de una tabla) detiene el relleno y no cambia nada.

Después de arrastrar el controlador de relleno, aparece el pequeño botón **Opciones de relleno** en la esquina de lo rellenado. Su menú repite el relleno de otra forma, y el relleno sigue siendo un solo paso de Deshacer: **Copiar celdas**, **Rellenar serie**, **Rellenar solo formato** (las celdas conservan su contenido y toman el formato), **Rellenar sin formato** (las celdas conservan su propio formato) y, para fechas, **Rellenar días**, **Rellenar días laborables**, **Rellenar meses** y **Rellenar años**. El botón desaparece en cuanto hace cualquier otra cosa.

Un relleno no entra ni sale de celdas combinadas, porque dejaría valores en celdas que el área combinada oculta. Separe primero las celdas.

**Rellenar > Justificar** distribuye el texto de las celdas seleccionadas de una columna en líneas tan anchas como la columna, en las celdas desde la primera hacia abajo; las celdas sobrantes de la selección se vacían. Si el texto necesita más celdas de las seleccionadas, JeffCrunch pregunta si debe usar las celdas de debajo. Solo funciona con texto.

**Rellenar > En varias hojas** copia las celdas seleccionadas en las mismas celdas de las hojas que active en una lista: todo, solo el contenido o solo los formatos. Es un solo paso de Deshacer.

## Suma

**Inicio > Suma** (**Alt y el signo igual**) coloca un total en la celda que seleccione. Busca los números situados encima de la celda; si no hay ninguno, los situados a su izquierda. La fórmula se coloca en la celda para que la revise, y **Entrar** la conserva. La flecha junto a Suma ofrece los demás totales: **Promedio**, **Contar números**, **Máx.** y **Mín.**

Si selecciona antes un bloque, cada columna del bloque obtiene su propia fórmula en las celdas vacías de debajo (o, si es una sola fila, a su derecha).
