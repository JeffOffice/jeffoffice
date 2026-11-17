# Funciones de texto
<!-- keywords: functions, ASC, BAHTTEXT, CHAR, CLEAN, CODE, CONCAT, CONCATENATE, DBCS, DOLLAR, ENCODEURL, EXACT, FILTERXML, FIND, FINDB, FIXED, LEFT, LEFTB, LEN, LENB, LOWER, MID, MIDB, NUMBERVALUE, PHONETIC, PROPER, REPLACE, REPLACEB, REPT, RIGHT, RIGHTB, SEARCH, SEARCHB, SUBSTITUTE, T, TEXT, TEXTJOIN, TRIM, UNICHAR, UNICODE, UPPER, VALUE -->

Estas son las funciones de texto que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y qué es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y "..." significa que puede dar tantos más del último como necesite. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece una pequeña sugerencia debajo de la celda con sus argumentos, y si hace clic en el nombre de la función en la sugerencia, se abre su entrada en esta página.

## ASC

`ASC(texto)`

Cambia las letras y los dígitos de ancho completo del texto por los de medio ancho.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## BAHTTEXT

`BAHTTEXT(número)`

Escribe un número en palabras tailandesas con la palabra «baht».

- `número`: El número con el que se trabaja.

## CHAR

`CHAR(número)`

Devuelve el carácter que corresponde a un número de código.

- `número`: El número con el que se trabaja.

## CLEAN

`CLEAN(texto)`

Quita del texto los caracteres que no se imprimen.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## CODE

`CODE(texto)`

Devuelve el número de código del primer carácter de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## CONCAT

`CONCAT(texto1, ...)`

Une textos y los textos de rangos de celdas en uno solo.

- `texto1`: El primer texto, o rango de celdas con texto.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## CONCATENATE

`CONCATENATE(texto1, ...)`

Une textos en uno solo.

- `texto1`: El primer texto, o rango de celdas con texto.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## DBCS

`DBCS(texto)`

Cambia las letras y los dígitos de medio ancho del texto por los de ancho completo.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## DOLLAR

`DOLLAR(número, [decimales])`

Convierte un número en texto como importe de dinero.

- `número`: El número con el que se trabaja.
- `decimales` (opcional): Cuántos dígitos conservar después de la coma decimal.

## ENCODEURL

`ENCODEURL(texto)`

Cambia el texto para que pueda usarse en una dirección web.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## EXACT

`EXACT(texto1, texto2)`

Devuelve verdadero si dos textos son exactamente iguales, incluidas las mayúsculas.

- `texto1`: El primer texto, o rango de celdas con texto.
- `texto2`: El segundo texto.

## FILTERXML

`FILTERXML(xml, xpath)`

Extrae valores de un texto que contiene XML, usando una expresión XPath.

- `xml`: El texto que contiene el XML.
- `xpath`: El XPath que indica qué partes devolver, por ejemplo //item/price.

## FIND

`FIND(texto_buscado, texto_original, [núm_inicial])`

Devuelve la posición en que un texto empieza dentro de otro, distinguiendo mayúsculas de minúsculas.

- `texto_buscado`: El texto que se busca.
- `texto_original`: El texto en el que se busca.
- `núm_inicial` (opcional): La posición del texto donde se empieza a buscar, contando el primer carácter como 1.

## FINDB

`FINDB(texto_buscado, texto_original, [núm_inicial])`

Devuelve la posición en que un texto aparece por primera vez en otro texto, distinguiendo mayúsculas de minúsculas.

- `texto_buscado`: El texto que se busca.
- `texto_original`: El texto en el que se busca.
- `núm_inicial` (opcional): La posición del texto donde se empieza a buscar, contando el primer carácter como 1.

## FIXED

`FIXED(número, [decimales], [sin_comas])`

Convierte un número en texto con un número fijo de decimales.

- `número`: El número con el que se trabaja.
- `decimales` (opcional): Cuántos dígitos conservar después de la coma decimal.
- `sin_comas` (opcional): Verdadero para omitir las comas que agrupan los miles.

## LEFT

`LEFT(texto, [núm_caracteres])`

Devuelve los primeros caracteres de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `núm_caracteres` (opcional): Cuántos caracteres tomar (1 si se omite).

## LEFTB

`LEFTB(texto, [núm_caracteres])`

Devuelve los primeros caracteres de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `núm_caracteres` (opcional): Cuántos caracteres tomar (1 si se omite).

## LEN

`LEN(texto)`

Cuenta los caracteres de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## LENB

`LENB(texto)`

Devuelve el número de caracteres de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## LOWER

`LOWER(texto)`

Convierte un texto en minúsculas.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## MID

`MID(texto, núm_inicial, núm_caracteres)`

Devuelve caracteres de la parte central de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `núm_inicial`: La posición del texto donde se empieza, contando el primer carácter como 1.
- `núm_caracteres`: Cuántos caracteres tomar (1 si se omite).

## MIDB

`MIDB(texto, núm_inicial, núm_caracteres)`

Devuelve caracteres del centro de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `núm_inicial`: La posición del texto donde se empieza, contando el primer carácter como 1.
- `núm_caracteres`: Cuántos caracteres tomar (1 si se omite).

## NUMBERVALUE

`NUMBERVALUE(texto, [separador_decimal], [separador_grupo])`

Lee un texto como número, con los separadores que indique.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `separador_decimal` (opcional): El carácter que marca los decimales en el texto (el punto si se omite).
- `separador_grupo` (opcional): El carácter que agrupa los dígitos en el texto (la coma si se omite).

## PHONETIC

`PHONETIC(referencia)`

Devuelve el texto de una celda, para las lecturas del texto en japonés.

- `referencia`: Una celda o un rango de celdas.

## PROPER

`PROPER(texto)`

Pone en mayúscula la primera letra de cada palabra de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## REPLACE

`REPLACE(texto_original, núm_inicial, núm_caracteres, texto_nuevo)`

Pone texto nuevo en lugar de algunos caracteres de un texto.

- `texto_original`: El texto que se va a cambiar.
- `núm_inicial`: La posición del texto donde se empieza, contando el primer carácter como 1.
- `núm_caracteres`: Cuántos caracteres tomar (1 si se omite).
- `texto_nuevo`: El texto que se va a insertar.

## REPLACEB

`REPLACEB(texto_original, núm_inicial, núm_caracteres, texto_nuevo)`

Sustituye parte de un texto por otro texto.

- `texto_original`: El texto que se va a cambiar.
- `núm_inicial`: La posición del texto donde se empieza, contando el primer carácter como 1.
- `núm_caracteres`: Cuántos caracteres tomar (1 si se omite).
- `texto_nuevo`: El texto que se va a insertar.

## REPT

`REPT(texto, núm_veces)`

Repite un texto un número determinado de veces.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `núm_veces`: Cuántas veces repetir el texto.

## RIGHT

`RIGHT(texto, [núm_caracteres])`

Devuelve los últimos caracteres de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `núm_caracteres` (opcional): Cuántos caracteres tomar (1 si se omite).

## RIGHTB

`RIGHTB(texto, [núm_caracteres])`

Devuelve los últimos caracteres de un texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `núm_caracteres` (opcional): Cuántos caracteres tomar (1 si se omite).

## SEARCH

`SEARCH(texto_buscado, texto_original, [núm_inicial])`

Devuelve la posición en que un texto empieza dentro de otro, sin distinguir mayúsculas de minúsculas, y permite usar ? y * en lo que se busca.

- `texto_buscado`: El texto que se busca.
- `texto_original`: El texto en el que se busca.
- `núm_inicial` (opcional): La posición del texto donde se empieza a buscar, contando el primer carácter como 1.

## SEARCHB

`SEARCHB(texto_buscado, texto_original, [núm_inicial])`

Devuelve la posición en que un texto aparece por primera vez en otro texto, sin distinguir mayúsculas de minúsculas.

- `texto_buscado`: El texto que se busca.
- `texto_original`: El texto en el que se busca.
- `núm_inicial` (opcional): La posición del texto donde se empieza a buscar, contando el primer carácter como 1.

## SUBSTITUTE

`SUBSTITUTE(texto, texto_original, texto_nuevo, [núm_aparición])`

Cambia un fragmento de texto por otro allí donde aparezca.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
- `texto_original`: El texto que se busca.
- `texto_nuevo`: El texto que se va a insertar.
- `núm_aparición` (opcional): Qué coincidencia reemplazar (todas si se omite).

## T

`T(valor)`

Devuelve el propio texto, y nada para lo que no es texto.

- `valor`: El valor con el que se trabaja, o una celda que lo contiene.

## TEXT

`TEXT(valor, formato)`

Convierte un número en texto con el formato que elija.

- `valor`: El valor con el que se trabaja, o una celda que lo contiene.
- `formato`: El formato de número en el que mostrar el valor, por ejemplo "0,00" o "dd/mm/aaaa".

## TEXTJOIN

`TEXTJOIN(delimitador, ignorar_vacíos, texto1, ...)`

Une textos con un separador entre ellos.

- `delimitador`: El texto que se coloca entre los fragmentos.
- `ignorar_vacíos`: Verdadero para omitir los fragmentos vacíos; falso para conservarlos.
- `texto1`: El primer texto, o rango de celdas con texto.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## TRIM

`TRIM(texto)`

Quita los espacios sobrantes de un texto, dejando un solo espacio entre palabras.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## UNICHAR

`UNICHAR(número)`

Devuelve el carácter que corresponde a un número Unicode.

- `número`: El número con el que se trabaja.

## UNICODE

`UNICODE(texto)`

Devuelve el número Unicode del primer carácter del texto.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## UPPER

`UPPER(texto)`

Convierte un texto en mayúsculas.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.

## VALUE

`VALUE(texto)`

Convierte un texto que parece un número en un número.

- `texto`: El texto con el que se trabaja, o una celda que lo contiene.
