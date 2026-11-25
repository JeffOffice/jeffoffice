# Buscar y reemplazar palabras
<!-- keywords: búsqueda avanzada, suena como, soundex, formas de palabra, todas las formas de una palabra, ignorar signos de puntuación, ignorar espacios, especial, marca de párrafo, tabulación, buscar por formato, formato, negrita, fuente, estilo, comodines, comodín, patrón, expresión regular, grupos, recuento de coincidencias, resaltar todas las coincidencias, reemplazar, reemplazar todo, buscar, buscar palabras, cambiar palabras, coincidir mayúsculas y minúsculas, palabras completas, buscar siguiente, ctrl+f, ctrl+h, edición, panel, texto oculto, deshacer, buscar y reemplazar -->

Use **Buscar** para ir a palabras de su documento. Use **Reemplazar** para cambiarlas. Ambas funciones funcionan en un único panel a la derecha de la ventana.

## Buscar palabras

1. Pulse **Ctrl+F**, o haga clic en **Inicio > Edición > Buscar**. Se abre el panel **Buscar y reemplazar**, con el cursor en **Buscar**.
2. Escriba las palabras que desea encontrar. Después pulse **Entrar**, o haga clic en **Buscar siguiente**.

JeffWrite empieza donde está el cursor. Selecciona el siguiente lugar donde encuentra sus palabras y desplaza las páginas para que pueda verlo. Ese lugar se llama **coincidencia**. Haga clic de nuevo en **Buscar siguiente** para ir a la siguiente coincidencia. Después de la última, vuelve al principio del documento.

Mientras escribe, el panel cuenta las coincidencias, por ejemplo "9 coincidencias", y cada una aparece con una franja amarilla en las páginas. Las franjas también se muestran en las celdas de tablas, en los encabezados, en los pies de página y en las notas al pie. Después de **Buscar siguiente**, el panel indica en cuál está, por ejemplo "Coincidencia 2 de 4". El recuento y las franjas cambian al activar **Coincidir mayúsculas y minúsculas** o **Buscar solo palabras completas**, o al cambiar el texto. Desaparecen al cerrar el panel.

## Buscar con comodines

Active **Usar comodines** para buscar un patrón, no solo palabras. Un **comodín** es un signo que representa otras letras. Estos son los signos:

- **?** representa cualquier letra. **c?t** encuentra "cat" y "cot".
- **\*** representa cualquier secuencia de letras. **c\*t** encuentra "cat" y "cart".
- **<** es el inicio de una palabra, y **>** es el final. **<the>** encuentra "the" pero no "other".
- **[abc]** es cualquiera de esas letras, **[a-c]** es cualquier letra de la a a la c, y **[!abc]** es cualquier letra excepto esas.
- **{2}** repite dos veces la letra anterior. **{1,3}** la repite de una a tres veces.
- **@** significa una o más veces la letra anterior.
- **\\** delante de un signo encuentra el propio signo. **\\?** encuentra un signo de interrogación.
- **^p** es una marca de párrafo, **^t** es una tabulación, **^#** es un dígito, **^$** es una letra, y **^w** es un espacio en blanco.

Ponga parte del patrón entre **( )** para crear un **grupo**. En **Reemplazar con**, **\1** es el primer grupo, **\2** es el segundo, y así hasta **\9**. **^&** es todo lo encontrado. Por ejemplo, busque **(<[A-Z]@>), (<[A-Z]@>)** y reemplace con **\2 \1** para convertir "Smith, John" en "John Smith". **Buscar solo palabras completas** aparece atenuado mientras los comodines están activados, porque **<** y **>** hacen ese trabajo.

Si el patrón tiene un error, el panel indica cuál es.

Si sus palabras no están en el documento, el panel lo indica y nada se mueve.

Puede seguir trabajando en el documento mientras el panel está abierto. Haga clic en la página para escribir. Haga clic en la **X** de la parte superior del panel para cerrarlo.

## Buscar con expresiones regulares

Active **Usar expresiones regulares** para patrones más avanzados que los comodines. Una **expresión regular** es un patrón breve formado por letras y signos. Se activa en lugar de **Usar comodines**: solo puede estar activada una de las dos opciones. Estos son los signos más habituales:

- **.** es cualquier carácter, **\d** es un dígito, **\w** es una letra, un dígito o un guion bajo, y **\s** es un espacio o una tabulación. **[a-c]** es cualquier letra de la a a la c, y **[^abc]** es cualquier carácter excepto esos.
- **^** es el inicio de un párrafo, **$** es el final, y **\b** es el límite de una palabra. **\bcat\b** encuentra "cat" pero no "category".
- **\*** significa ninguna o más veces lo anterior, **+** significa una o más veces, y **?** significa ninguna o una vez. **{2}** significa exactamente dos veces, y **{2,5}** significa de dos a cinco veces; el número puede llegar a 200. Añada **?** después de una repetición, como en **.+?**, para encontrar la secuencia más corta que encaje.
- **|** significa "o": **cat|dog** encuentra cualquiera de las dos palabras.
- **( )** crea un **grupo**. Use **(?: )** para un grupo que no necesite en **Reemplazar con**.
- **\\** delante de un signo encuentra el propio signo: **\.** encuentra un punto. **\t** es una tabulación.

**Coincidir mayúsculas y minúsculas** funciona igual que con las palabras. Las letras con tilde y las letras de otros alfabetos cuentan como letras.

El patrón se busca párrafo a párrafo, así que **^** y **$** significan el inicio y el final de cada párrafo. Para buscar a través del final de un párrafo, ponga **\n** (una marca de párrafo) en el patrón: **casa\.\nLuis** encuentra "casa." al final de un párrafo y "Luis" al principio del siguiente.

En **Reemplazar con**, **$1** o **\1** es el primer grupo, **$2** o **\2** es el segundo, y así hasta nueve grupos. **$0** o **\0** es todo lo encontrado. **$$** es un signo de dólar, **\\** es una barra invertida, y **\t** es una tabulación. Por ejemplo, busque **(\w+), (\w+)** y reemplace con **$2 $1** para convertir "Smith, John" en "John Smith".

**Reemplazar todo** cambia todas las coincidencias del texto, de los encabezados, de los pies de página, de las notas y de los cuadros de texto en un solo paso, así que un **Deshacer** las devuelve todas. Con **Control de cambios** activado, cada cambio se registra, igual que con las palabras. El texto oculto no se modifica.

Si el patrón tiene un error, el panel indica cerca de qué carácter. Un patrón puede no coincidir con ningún carácter, como **^** (el inicio de un párrafo) o **$** (el final): **Reemplazar con** inserta entonces su texto en ese lugar, así que **^** con **> ** añade esos signos al principio de cada párrafo. Un patrón que tardaría demasiado en su texto, como **(a+)+$** en una línea larga de letras a, lo detiene JeffWrite y lo indica; no se cambia nada. Una búsqueda larga muestra un cuadro **Buscando** con un botón **Cancelar**; al cancelar, el documento queda como estaba, y no puede escribir en el documento hasta que termine la búsqueda. Los patrones que miran hacia delante o hacia atrás sin consumir lo que comprueban, como **x(?=\d)**, y los patrones en modo extendido, como **(?x)**, no son compatibles; JeffWrite lo indica. Un párrafo muy largo se busca por partes, y cada coincidencia es la que encontraría una búsqueda de todo el párrafo. Un patrón que coincida con un fragmento muy largo de un párrafo muy largo (decenas de miles de caracteres) puede detenerse con el mensaje "necesitó demasiado trabajo", y una coincidencia de más de 200 000 caracteres no se reemplaza: JeffWrite se detiene, lo indica y no cambia nada.

## Búsqueda avanzada

Estas opciones están en el panel debajo de **Usar expresiones regulares**. Cada una cambia la manera en que JeffWrite lee las palabras de **Buscar**.

- **Suena como** encuentra palabras que se pronuncian igual que la suya. Asigna a cada palabra un código según sus sonidos y busca las palabras con el mismo código. Si escribe **Smith**, encuentra también "Smyth".
- **Encontrar todas las formas de la palabra** encuentra todas las formas de la palabra. Si escribe **correr**, encuentra "corre," "corrió" y "corriendo". Funciona con palabras completas. Si escribe dos palabras, como **perro grande**, las encuentra juntas, en cualquiera de sus formas.
- **Ignorar signos de puntuación** omite signos como guiones y puntos entre las letras. Si escribe **correo**, encuentra también "correo-e".
- **Ignorar espacios en blanco** omite espacios y tabulaciones entre las letras. Si escribe **correo e**, encuentra "correo-e" y "correoe".

Solo puede estar activada una de estas opciones a la vez: **Suena como**, **Encontrar todas las formas de la palabra**, **Usar comodines** y **Usar expresiones regulares**. **Suena como** y **Encontrar todas las formas de la palabra** siempre buscan palabras completas, así que **Buscar solo palabras completas** y **Coincidir mayúsculas y minúsculas** aparecen atenuadas.

### Especial

Haga clic en **Especial** para poner un código en **Buscar**: **Marca de párrafo** (^p), **Carácter de tabulación** (^t), **Cualquier dígito** (^#), **Cualquier letra** (^$), **Espacio en blanco** (^w), **Salto de sección** (^b), **Campo** (^d), **Marca de nota al pie** (^f), **Marca de nota al final** (^e), o **Carácter de intercalación** (^^). Los códigos funcionan sin **Usar comodines**. Un **salto de sección** es la marca que termina una sección. Un **campo** es un fragmento de texto que el programa calcula, como un número de página o una fecha, y **^d** encuentra dónde empieza cada uno. Las marcas de nota son los números pequeños del texto que apuntan a una nota al pie o al final. Estas cuatro se encuentran, pero **Reemplazar** y **Reemplazar todo** no las modifican, porque cambiar una supondría perder lo que representa. Los demás signos del cuadro, como **?** y **\***, son entonces solo esos caracteres.

### Formato

Haga clic en **Formato** para buscar solo el texto con cierto aspecto: **Negrita**, **Cursiva**, **Subrayado**, **Tachado**, **Resaltado**, una **Fuente**, o un **Estilo** de párrafo. El panel muestra los aspectos que eligió debajo de los botones. Seleccione **Sin formato** para quitarlos.

Puede dejar **Buscar** vacío. Entonces, cada fragmento de texto con ese aspecto es una coincidencia. Si escribe palabras en **Buscar**, solo se encuentran las palabras con ese aspecto. **Reemplazar todo** cambia solo esas palabras.

## Reducir la búsqueda

- **Coincidir mayúsculas y minúsculas** encuentra solo las palabras con las mismas mayúsculas. Con esta opción activada, "Plan" no encuentra "plan".
- **Buscar solo palabras completas** omite las palabras más largas que contienen la suya. Con esta opción activada, "plan" no encuentra "planeta".
- **Buscar en** determina en qué sentido va **Buscar siguiente** desde el cursor. **Abajo** llega hasta el final del documento, y **Arriba** hasta el principio. Cada una se detiene allí y lo indica. **Todo** continúa hasta el otro extremo y es la opción habitual. **Reemplazar todo** siempre cambia todo el documento.

## Reemplazar palabras

1. Pulse **Ctrl+H**, o haga clic en **Inicio > Edición > Reemplazar**. El cursor pasa a **Reemplazar con**.
2. Escriba las palabras nuevas en **Reemplazar con**. Para eliminar las palabras encontradas, déjelo vacío.
3. Escriba las palabras originales en **Buscar**.
4. Haga clic en **Reemplazar** para ver la primera coincidencia. Haga clic de nuevo en **Reemplazar** para cambiarla. JeffWrite pasa entonces a la siguiente coincidencia. Cada cambio es un paso, así que **Deshacer** (Ctrl+Z) los devuelve uno a uno.
5. Haga clic en **Reemplazar todo** para cambiar todas las coincidencias de una vez. El panel indica cuántas ha cambiado. **Reemplazar todo** también cambia las palabras de los cuadros de texto y de las formas, y es un solo paso, así que un **Deshacer** devuelve todas las palabras. Con **Control de cambios** activado, cada reemplazo se registra como cambio controlado, en el texto y en los cuadros. Un cuadro demasiado grande para cambiarlo se cuenta en el mensaje. **Reemplazar todo** no llega a los cuadros de texto de los encabezados y pies de página, a las formas agrupadas ni a los cuadros vinculados entre sí. **Buscar siguiente** todavía no busca en los cuadros de texto.

Las palabras nuevas toman la mayúscula inicial de las palabras que sustituyen. Si reemplaza una palabra en negrita, la nueva palabra también va en negrita.

Por ejemplo, para cambiar "color" por "colour" en todo el documento: escriba **color** en **Buscar**, escriba **colour** en **Reemplazar con** y haga clic en **Reemplazar todo**.

## Lo que busca la búsqueda

- Busca en el texto del documento, incluido el texto de las tablas. Un marcador en medio de algunas palabras, que tienen algunos archivos, no impide encontrarlas. **Reemplazar** y **Reemplazar todo** conservan el marcador.
- **Buscar siguiente** busca primero en el texto. Después pasa a los encabezados, luego a los pies de página, luego a las notas al pie, y finalmente vuelve al principio del texto. Cuando una coincidencia está en alguno de ellos, JeffWrite lo abre para editarlo y selecciona la coincidencia. El panel indica dónde está, por ejemplo "Encontrado en el encabezado de la página 3." **Reemplazar** cambia la coincidencia en la que está, ya sea en el texto, en un encabezado, en un pie de página o en una nota al pie. **Reemplazar todo** cambia todas las coincidencias de todos ellos, y un solo **Deshacer** las devuelve. Las notas al final forman parte del texto, así que se buscan junto con él. Consulte [Encabezados y pies de página](headers-footers).
- Omite el texto marcado como **Oculto**, porque ese texto no se muestra. Consulte [Cambiar el aspecto del texto](font).

## Temas relacionados

- [Cambiar el aspecto del texto](font)
- [Usar JeffWrite con el teclado](keyboard)
