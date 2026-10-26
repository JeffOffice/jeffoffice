# Formatos condicionales
<!-- keywords: formato condicional, formatos condicionales, regla, reglas, resaltar, escala de colores, barra de datos, barras de datos, conjunto de iconos, iconos, superiores, inferiores, por encima del promedio, por debajo del promedio, duplicados, únicos, fórmula, detener si es verdad, prioridad, barras, flechas, semáforos, degradado, formato diferencial -->

Un **formato condicional** es una regla que cambia el aspecto de una celda cuando su contenido cumple una condición. Por ejemplo, una regla dice "colorear la celda de rojo si es menor que 100". Una celda que contiene 80 aparece en rojo y una que contiene 120 no; si cambia 80 por 150, el rojo desaparece automáticamente. El formato propio de la celda no se modifica: la regla solo decide lo que se muestra.

## Qué muestra JeffCrunch

JeffCrunch muestra las reglas que tiene un libro:

- **Reglas de resaltado**: reglas que cambian la fuente, el relleno, el borde o el formato de número de las celdas que las cumplen. La condición puede ser una comparación con un valor (mayor que, entre, igual a, etc.), una fórmula, un texto que contiene, comienza por o termina en unas palabras, una fecha como "la semana pasada", celdas en blanco o con un error, valores que están entre los mayores o los menores (una cantidad o un porcentaje), valores por encima o por debajo del promedio y valores duplicados o únicos.
- **Escalas de colores**: cada celda se rellena con un color que varía entre dos o tres colores a medida que su número pasa de bajo a alto. Una escala de blanco a verde hace que los números más altos sean los más verdes.
- **Barras de datos**: una barra dentro de la celda, más larga cuanto mayor es el número. Las barras pueden ser de un color sólido o con degradado, pueden tener borde y eje, y para los números negativos van en sentido contrario y en otro color.
- **Conjuntos de iconos**: una pequeña imagen en la celda que indica en qué parte del rango está el número, como una flecha hacia arriba, hacia un lado o hacia abajo, o una luz roja, amarilla o verde. Hay veinte conjuntos.

Cuando dos reglas afectan a la misma celda, la que va primero en el orden del libro prevalece en lo que ambas establecen, y una regla marcada con "detener si es verdad" pone fin a la búsqueda. Los números de una escala de colores, una barra de datos o un conjunto de iconos se calculan sobre las celdas de la propia regla, y se vuelven a calcular cuando cambia alguna de esas celdas.

## Qué se mantiene

- Las reglas siguen a las celdas a las que pertenecen. Si inserta o elimina filas o columnas, o corta y pega, las reglas siguen abarcando las celdas correctas; **Deshacer** las restaura.
- Las reglas que JeffCrunch no muestra se conservan en el archivo tal como estaban.
- Al guardar se escriben las reglas que tenía el libro, sin cambios.

## Qué falta por hacer

**Todavía no se pueden crear, modificar ni eliminar reglas.** JeffCrunch no tiene ningún comando **Formato condicional**: muestra lo que el libro ya contiene y lo mantiene correcto mientras edita. La edición de reglas llegará en una versión posterior. Mientras tanto, cree o modifique las reglas en el programa con el que se creó el libro.
