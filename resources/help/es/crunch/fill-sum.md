# Rellenar celdas y sumarlas
<!-- keywords: rellenar, relleno, serie, series, arrastrar, controlador de relleno, Autorrellenar, Ctrl+D, Ctrl+R, abajo, derecha, arriba, izquierda, número, fecha, día laborable, mes, año, incremento, límite, copiar, Autosuma, suma, sumar, promedio, media, contar, recuento, máximo, mínimo, Alt+igual, total -->

## Rellenar

Rellenar continúa lo que ha empezado. Seleccione las celdas que ha rellenado, arrastre el pequeño cuadrado de la esquina de la selección (el **controlador de relleno**) a lo largo de una columna o una fila y suéltelo. Un borde discontinuo muestra hasta dónde llegará.

| Si empieza con | El relleno le da |
| --- | --- |
| 1, 2 | 3, 4, 5, etc.: continúa el incremento entre ellos |
| Un solo número, como 5 | 5, 5, 5: copias (mantenga presionada Ctrl mientras arrastra para obtener lo contrario) |
| Una fecha | Los días siguientes (use **Rellenar > Series** para días laborables, meses o años) |
| Lun, o enero | Mar, Mié, ... o febrero, marzo, ... en su idioma |
| Elemento 1 | Elemento 2, Elemento 3, ... |
| Una fórmula | La fórmula copiada, con sus referencias desplazadas como al pegar |

**Rellenar > Abajo** (**Ctrl+D**) y **Rellenar > Derecha** (**Ctrl+R**) copian la primera fila o columna de la selección en el resto. **Rellenar > Arriba** y **Rellenar > Izquierda** lo hacen a partir de la última. **Rellenar > Series** le pide el tipo (lineal, geométrica, cronológica), la unidad de tiempo de una fecha, el incremento y el límite. Cada celda rellenada toma el formato de la celda a la que sigue. Una celda que no se puede cambiar (parte de una matriz, una celda bloqueada o el encabezado de una tabla) detiene el relleno y no cambia nada.

## Autosuma

**Inicio > Autosuma** (**Alt y el signo igual**) coloca un total en la celda que seleccione. Busca los números situados encima de la celda; si no los hay, los situados a su izquierda. La fórmula se coloca en la celda para que la compruebe, y **Entrar** la conserva. La flecha situada junto a Autosuma ofrece los demás totales: **Promedio**, **Contar números**, **Máx.** y **Mín.**

Si selecciona antes un bloque, cada columna del bloque obtiene su propia fórmula en las celdas vacías situadas debajo (o, si es una sola fila, a su derecha).
