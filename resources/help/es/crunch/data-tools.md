# Herramientas de datos
<!-- keywords: datos, pestaña datos, texto en columnas, dividir, separador, coma, tabulación, ancho fijo, quitar duplicados, filas repetidas, únicos, archivo de texto, csv, desde texto, columnas, delimitador, comillas -->

La pestaña **Datos** tiene las herramientas que trabajan con una lista de valores: obtener texto de un archivo, poner una lista en orden, dividir texto en columnas y quitar filas repetidas.

## Desde texto/CSV

**Desde texto/CSV** abre un archivo de texto (.csv, .tsv o .txt). Es lo mismo que **Abrir archivo de texto** en la pestaña Archivo. JeffCrunch pregunta cómo está dividido el archivo en columnas y qué tipo de texto contiene, y después lo muestra como un libro nuevo.

## Ordenar y filtrar

El grupo **Ordenar y filtrar** tiene los mismos comandos que el menú **Ordenar y filtrar** de la pestaña Inicio. Consulte [Ordenar y filtrar](sort-filter).

## Texto en columnas

Use esta herramienta cuando una columna contenga varios textos que desee separar en columnas distintas, como «García, Ana, 31».

1. Seleccione las celdas de **una** columna.
2. Elija **Texto en columnas** en la pestaña **Datos**.
3. Elija cómo dividir el texto:
   - **Dividir por un separador**: marque los caracteres que están entre los fragmentos: **Tabulación**, **Punto y coma**, **Coma**, **Espacio** u **Otro**, donde escribe su propio carácter. **Considerar separadores consecutivos como uno solo** evita que dos comas seguidas generen un fragmento vacío. **Texto entre comillas** indica el carácter que encierra un fragmento que no debe dividirse, como «López, Bo». Dentro de las comillas, dos caracteres seguidos equivalen a uno.
   - **Dividir en posiciones fijas**: escriba las posiciones en las que empieza una columna nueva, contando los caracteres desde 1. Por ejemplo, 5, 12 inicia una columna nueva en el quinto y en el duodécimo carácter.
4. Elija el tipo de cada fragmento: **General** lee los números y las fechas tal como los escribió, y **Texto** conserva cada fragmento tal cual, de modo que «007» sigue siendo «007».
5. En **Colocar las partes desde**, escriba la primera celda para las partes. Al principio es la primera celda que seleccionó.
6. El cuadro situado debajo de las opciones muestra las primeras filas divididas como eligió. Pulse **Aceptar**.

Las partes se colocan en las celdas a la derecha de la primera celda y sustituyen lo que había en ellas. Texto en columnas es un solo paso que puede deshacer.

## Quitar duplicados

Use esta herramienta para conservar solo la primera de cada grupo de filas repetidas.

1. Seleccione las celdas de la lista, o una celda de ella. JeffCrunch toma todo el bloque de datos que rodea a una sola celda.
2. Elija **Quitar duplicados**.
3. Marque **Mis datos tienen encabezados** si la primera fila contiene nombres. Esa fila se conserva y no se compara.
4. Marque las columnas que deben coincidir para que dos filas se consideren repetidas. Dos filas se repiten solo cuando todas las columnas marcadas son iguales. Las mayúsculas y las minúsculas se consideran iguales, y una fórmula cuenta como el valor que devuelve.
5. Pulse **Aceptar**. El cuadro de diálogo indica cuántas filas se han quitado y cuántas quedan.

Las filas situadas debajo de una fila eliminada suben dentro de las celdas seleccionadas, con sus formatos y fórmulas. Las celdas situadas debajo de la selección no se mueven. Las filas vacías que quedan al final de la selección se borran. Quitar duplicados es un solo paso que puede deshacer.
