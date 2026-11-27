# Buscar y reemplazar palabras
<!-- keywords: búsqueda avanzada, suena como, soundex, formas de palabra, todas las formas de una palabra, ignorar puntuación, ignorar espacios, especial, marca de párrafo, tabulación, buscar por formato, formato, negrita, fuente, estilo, comodines, comodín, patrón, expresión regular, grupos, recuento de coincidencias, resaltar todas las coincidencias, reemplazar, reemplazar todo, buscar, buscar palabras, cambiar palabras, coincidir mayúsculas y minúsculas, palabras completas, buscar siguiente, ctrl+f, ctrl+h, edición, panel, texto oculto, deshacer -->

Use Buscar para ir a palabras de su documento. Use Reemplazar para cambiarlas. Ambas funciones funcionan en un único panel a la derecha de la ventana.

## Buscar palabras

1. Presione **Ctrl+F** o haga clic en **Inicio > Edición > Buscar**. Se abre el panel **Buscar y reemplazar**, con el cursor en **Buscar**.
2. Escriba las palabras que desea encontrar. Después presione **Entrar** o haga clic en **Buscar siguiente**.

JeffWrite empieza en la posición del cursor. Selecciona el siguiente lugar donde encuentra las palabras y desplaza las páginas para que lo vea. Ese lugar se llama **coincidencia**. Haga clic de nuevo en **Buscar siguiente** para ir a la siguiente coincidencia. Después de la última, vuelve al principio del documento.

Mientras escribe, el panel cuenta las coincidencias, por ejemplo «9 coincidencias», y cada una aparece con una banda amarilla en las páginas. Las bandas también aparecen en las celdas de tabla, los encabezados, los pies de página y las notas al pie. Después de **Buscar siguiente**, el panel indica en cuál está, por ejemplo «Coincidencia 2 de 4». El recuento y las bandas cambian al hacer clic en **Coincidir mayúsculas y minúsculas** o en **Solo palabras completas**, o al cambiar el texto. Las bandas se mantienen en las páginas al cerrar el panel y al hacer clic en **Modo de lectura**, para que pueda leer con todas las coincidencias a la vista. Nada del documento cambia, y las bandas nunca se imprimen ni se guardan. Desaparecen cuando cambia el documento, cuando vuelve a abrir el panel o cuando hace clic en **Inicio > Edición > Borrar resaltados de búsqueda**.

## Buscar con comodines

Marque **Usar comodines** para buscar un patrón, y no solo palabras. Un **comodín** es un signo que representa otras letras. Estos son los signos:

- **?** representa una letra cualquiera. **c?t** encuentra «cat» y «cot».
- **\*** representa cualquier secuencia de letras. **c\*t** encuentra «cat» y «cart».
- **<** es el inicio de una palabra y **>** es el final. **<the>** encuentra «the», pero no «other».
- **[abc]** es una de esas letras, **[a-c]** es cualquier letra de la a a la c, y **[!abc]** es cualquier letra menos esas.
- **{2}** repite dos veces la letra anterior. **{1,3}** la repite de una a tres veces.
- **@** significa una o más veces la letra anterior.
- **\\** delante de un signo encuentra el signo en sí. **\\?** encuentra un signo de interrogación.
- **^p** es una marca de párrafo, **^t** es un tabulador, **^#** es un dígito, **^$** es una letra y **^w** es un espacio en blanco.

Ponga parte del patrón entre **( )** para crear un **grupo**. En **Reemplazar con**, **\1** es el primer grupo, **\2** el segundo, y así hasta **\9**. **^&** es todo lo encontrado. Por ejemplo, busque **(<[A-Z]@>), (<[A-Z]@>)** y reemplace con **\2 \1** para convertir «Smith, John» en «John Smith». **Solo palabras completas** aparece atenuado mientras los comodines están activados, porque **<** y **>** cumplen esa función.

Si el patrón tiene un error, el panel indica cuál es.

Si sus palabras no están en el documento, el panel lo indica y nada se mueve.

Puede seguir trabajando en el documento mientras el panel está abierto. Haga clic en la página para escribir. Haga clic en la **X** de la parte superior del panel para cerrarlo.

## Buscar con expresiones regulares

Marque **Usar expresiones regulares** para patrones más avanzados que los comodines. Una **expresión regular** es un patrón breve formado por letras y signos. Se activa en lugar de **Usar comodines**: solo una de las dos opciones puede estar activa. Estos son los signos más comunes:

- **.** es cualquier carácter, **\d** es un dígito, **\w** es una letra, un dígito o un guion bajo, y **\s** es un espacio o un tabulador. **[a-c]** es cualquier letra de la a a la c, y **[^abc]** es cualquier carácter menos esos.
- **^** es el inicio de un párrafo, **$** es el final de uno, y **\b** es el límite de una palabra. **\bgato\b** encuentra «gato», pero no «gatos».
- **\*** significa ninguno o más de lo que va antes, **+** significa uno o más, y **?** significa ninguno o uno. **{2}** significa exactamente dos veces, y **{2,5}** significa de dos a cinco veces; el número puede llegar a 200. Añada **?** después de una repetición, como en **.+?**, para encontrar la secuencia más corta posible.
- **|** significa «o»: **gato|perro** encuentra cualquiera de las dos palabras.
- **( )** crea un **grupo**. Use **(?: )** para un grupo que no necesite en **Reemplazar con**.
- **\\** delante de un signo encuentra el signo en sí: **\.** encuentra un punto. **\t** es un tabulador.

**Coincidir mayúsculas y minúsculas** funciona igual que con las palabras. Las letras con tilde y las letras de otros alfabetos cuentan como letras.

El patrón se busca párrafo a párrafo, así que **^** y **$** significan el inicio y el final de cada párrafo. Para buscar a través del final de un párrafo, ponga **\n** (una marca de párrafo) en el patrón: **casa\.\nLee** encuentra «casa.» al final de un párrafo y «Lee» al inicio del siguiente.

En **Reemplazar con**, **$1** o **\1** es el primer grupo, **$2** o **\2** el segundo, y así hasta nueve grupos. **$0** o **\0** es todo lo encontrado. **$$** es un signo de dólar, **\\** es una barra invertida y **\t** es un tabulador. Por ejemplo, busque **(\w+), (\w+)** y reemplace con **$2 $1** para convertir «Smith, John» en «John Smith».

**Reemplazar todo** cambia cada coincidencia del texto, los encabezados, los pies de página, las notas y los cuadros de texto en un solo paso, así que un único **Deshacer** lo devuelve todo. Con **Control de cambios** activado, cada cambio se registra, igual que con las palabras. El texto oculto no se modifica.

Si el patrón tiene un error, el panel indica cerca de qué carácter está. Un patrón puede no coincidir con ningún carácter, como **^** (el inicio de un párrafo) o **$** (el final): en ese caso, **Reemplazar con** inserta sus palabras en ese lugar, así que **^** con **> ** añade esas marcas al principio de cada párrafo. Un patrón que tardaría demasiado en su texto, como **(a+)+$** en una línea larga de aes, lo detiene JeffWrite y lo indica; no se cambia nada. Una búsqueda larga muestra un cuadro **Buscando** con un botón **Cancelar**; al cancelar, el documento queda como estaba, y no puede escribir en el documento hasta que termine la búsqueda. No se admiten los patrones que miran hacia delante o hacia atrás sin consumir lo que comprueban, como **x(?=\d)**, ni los patrones en modo extendido, como **(?x)**; JeffWrite lo indica. Un párrafo muy largo se busca por partes, y cada coincidencia es la que encontraría una búsqueda de todo el párrafo. Un patrón que coincide con un tramo muy largo de un párrafo muy largo (decenas de miles de caracteres) puede detenerse con el mensaje «Se necesita demasiado trabajo», y una coincidencia de más de 200 000 caracteres no se reemplaza: JeffWrite se detiene, lo indica y no cambia nada.

## Búsqueda avanzada

Estas opciones están en el panel, debajo de **Usar expresiones regulares**. Cada una cambia la forma en que JeffWrite lee las palabras de **Buscar**.

- **Suena como** encuentra palabras que se pronuncian como su palabra. Asigna a cada palabra un código según sus sonidos y encuentra las palabras con el mismo código. Escriba **Smith** y encontrará también «Smyth».
- **Buscar todas las formas de la palabra** encuentra todas las formas de la palabra. Escriba **correr** y encontrará «corro», «corre», «corrió» y «corriendo». Funciona con palabras completas. Si escribe dos palabras, como **perro grande**, las encuentra juntas, en cualquiera de sus formas.
- **Ignorar caracteres de puntuación** omite signos como guiones y puntos entre las letras. Escriba **correo** y encontrará también «correo-e».
- **Ignorar caracteres de espacio en blanco** omite espacios y tabuladores entre las letras. Escriba **correo e** y encontrará «correoe».

Solo una de las opciones **Suena como**, **Buscar todas las formas de la palabra**, **Usar comodines** y **Usar expresiones regulares** puede estar activa a la vez. **Suena como** y **Buscar todas las formas de la palabra** siempre encuentran palabras completas, así que **Solo palabras completas** y **Coincidir mayúsculas y minúsculas** aparecen atenuadas.

### Especial

Haga clic en **Especial** para poner un código en **Buscar**: **Marca de párrafo** (^p), **Carácter de tabulación** (^t), **Cualquier dígito** (^#), **Cualquier letra** (^$), **Espacio en blanco** (^w), **Salto de sección** (^b), **Campo** (^d), **Marca de nota al pie** (^f), **Marca de nota al final** (^e) o **Carácter de intercalación** (^^). Los códigos funcionan sin **Usar comodines**. Un **salto de sección** es la marca que termina una sección. Un **campo** es un fragmento de texto que el programa calcula, como un número de página o una fecha, y **^d** encuentra dónde empieza cada uno. Las marcas de nota son los números pequeños del texto que remiten a una nota al pie o a una nota al final. Estas cuatro se encuentran, pero **Reemplazar** y **Reemplazar todo** no las tocan, porque cambiar una supondría perder lo que representa. Los demás signos del cuadro, como **?** y **\***, son entonces solo esos caracteres.

### Formato

Haga clic en **Formato** para buscar solo el texto con un aspecto determinado: **Negrita**, **Cursiva**, **Subrayado**, **Tachado**, **Resaltado**, una **Fuente** o un **Estilo** de párrafo. El panel muestra debajo de los botones los aspectos que eligió. Elija **Sin formato** para quitarlos.

Puede dejar **Buscar** vacío. Entonces, cada tramo de texto con ese aspecto es una coincidencia. Si escribe palabras en **Buscar**, solo se encuentran las palabras con ese aspecto. **Reemplazar todo** cambia solo esas palabras.

## Acotar la búsqueda

- **Coincidir mayúsculas y minúsculas** encuentra solo las palabras con las mismas mayúsculas. Con esta opción activada, «Plan» no encuentra «plan».
- **Solo palabras completas** omite las palabras más largas que contienen la suya. Con esta opción activada, «plan» no encuentra «planeta».
- **Buscar en** define hacia dónde avanza **Buscar siguiente** desde el cursor. **Abajo** va hasta el final del documento, y **Arriba** hasta el principio. En cada caso, se detiene allí y lo indica. **Todo** sigue hasta el otro extremo y es la opción habitual. **Reemplazar todo** siempre cambia todo el documento.

## Reemplazar palabras

1. Presione **Ctrl+H** o haga clic en **Inicio > Edición > Reemplazar**. El cursor pasa a **Reemplazar con**.
2. Escriba las palabras nuevas en **Reemplazar con**. Para quitar las palabras encontradas, déjelo vacío.
3. Escriba las palabras antiguas en **Buscar**.
4. Haga clic en **Reemplazar** para ver la primera coincidencia. Haga clic de nuevo en **Reemplazar** para cambiarla. JeffWrite pasa entonces a la siguiente coincidencia. Cada cambio es un paso, así que **Deshacer** (Ctrl+Z) los devuelve uno a uno.
5. Haga clic en **Reemplazar todo** para cambiar todas las coincidencias a la vez. El panel le indica cuántas ha cambiado. **Reemplazar todo** también cambia las palabras de los cuadros de texto y las formas, y es un solo paso, así que un único **Deshacer** devuelve todas las palabras. Con **Control de cambios** activado, cada reemplazo se registra como un cambio controlado, tanto en el texto como en los cuadros. Un cuadro demasiado grande para cambiarlo se cuenta en el mensaje. **Reemplazar todo** no llega a los cuadros de texto de los encabezados y pies de página, a las formas agrupadas ni a los cuadros vinculados entre sí. **Buscar siguiente** todavía no busca en los cuadros de texto.

Las palabras nuevas toman la forma de la primera letra de las palabras que sustituyen. Si reemplaza una palabra en negrita, la palabra nueva también queda en negrita.

Por ejemplo, para cambiar «color» por «colour» en todo el documento: escriba **color** en **Buscar**, escriba **colour** en **Reemplazar con** y haga clic en **Reemplazar todo**.

## Qué busca la búsqueda

- Busca en el texto del documento, incluido el texto de las tablas. Un marcador en medio de algunas palabras, que tienen algunos archivos, no impide que se encuentren. **Reemplazar** y **Reemplazar todo** conservan el marcador.
- **Buscar siguiente** busca primero en el texto. Luego pasa a los encabezados, después a los pies de página, a continuación a las notas al pie y, por último, vuelve al principio del texto. Cuando una coincidencia está en uno de ellos, JeffWrite lo abre para editarlo y selecciona la coincidencia. El panel indica dónde está, por ejemplo «Encontrado en el encabezado de la página 3». **Reemplazar** cambia la coincidencia en la que se encuentra, ya esté en el texto o en un encabezado, pie de página o nota al pie. **Reemplazar todo** cambia cada coincidencia de todos ellos, y un único **Deshacer** los devuelve todos. Las notas al final forman parte del texto, así que se buscan junto con él. Consulte [Encabezados y pies de página](headers-footers).
- Omite el texto marcado como **Oculto**, porque ese texto no se muestra. Consulte [Cambiar el aspecto del texto](font).

## Temas relacionados

- [Cambiar el aspecto del texto](font)
- [Usar JeffWrite con el teclado](keyboard)
