# Buscar y reemplazar palabras
<!-- keywords: búsqueda avanzada, suena como, soundex, formas de palabra, todas las formas de una palabra, ignorar signos de puntuación, ignorar espacios, especial, marca de párrafo, tabulación, buscar por formato, formato, negrita, fuente, estilo, comodines, comodín, patrón, expresión regular, grupos, recuento de coincidencias, resaltar todas las coincidencias, reemplazar, reemplazar todo, buscar, buscar palabras, cambiar palabras, coincidir mayúsculas y minúsculas, palabras completas, buscar siguiente, ctrl+f, ctrl+h, edición, panel, texto oculto, deshacer -->

Use **Buscar** para ir a palabras de su documento. Use **Reemplazar** para cambiarlas. Ambas funciones funcionan en un único panel a la derecha de la ventana.

## Buscar palabras

1. Presione **Ctrl+F** o haga clic en **Inicio > Edición > Buscar**. Se abre el panel **Buscar y reemplazar**, con el cursor en **Buscar**.
2. Escriba las palabras que desea buscar. Después presione **Entrar** o haga clic en **Buscar siguiente**.

JeffWrite empieza donde esté el cursor. Selecciona el siguiente lugar donde encuentra sus palabras y desplaza las páginas para que lo vea. A ese lugar se le llama **coincidencia**. Haga clic de nuevo en **Buscar siguiente** para ir a la siguiente coincidencia. Después de la última, vuelve al inicio del documento.

Mientras escribe, el panel cuenta las coincidencias, por ejemplo «9 coincidencias», y cada una aparece con una franja amarilla en las páginas. Las franjas se ven también en las celdas de tablas, en los encabezados, en los pies de página y en las notas al pie. Después de **Buscar siguiente**, el panel indica cuál es la coincidencia actual, por ejemplo «Coincidencia 2 de 4». El recuento y las franjas cambian al hacer clic en **Coincidir mayúsculas y minúsculas** o en **Buscar solo palabras completas**, o al cambiar el texto. Desaparecen al cerrar el panel.

## Buscar con comodines

Marque **Usar comodines** para buscar un patrón, no solo palabras. Un **comodín** es un signo que representa otras letras. Estos son los signos:

- **?** representa una letra cualquiera. **c?t** encuentra «cat» y «cot».
- **\*** representa cualquier secuencia de letras. **c\*t** encuentra «cat» y «cart».
- **<** es el inicio de una palabra, y **>** es el final. **<the>** encuentra «the», pero no «other».
- **[abc]** es una de esas letras, **[a-c]** es cualquier letra de la a a la c, y **[!abc]** es cualquier letra excepto esas.
- **{2}** repite dos veces la letra anterior. **{1,3}** la repite de una a tres veces.
- **@** significa una o más veces la letra anterior.
- **\\** delante de un signo encuentra el propio signo. **\\?** encuentra un signo de interrogación.
- **^p** es una marca de párrafo, **^t** es una tabulación, **^#** es un dígito, **^$** es una letra y **^w** es un espacio en blanco.

Ponga una parte del patrón entre **( )** para crear un **grupo**. En **Reemplazar con**, **\1** es el primer grupo, **\2** el segundo, y así hasta **\9**. **^&** es todo lo encontrado. Por ejemplo, busque **(<[A-Z]@>), (<[A-Z]@>)** y reemplace con **\2 \1** para convertir «Smith, John» en «John Smith». **Buscar solo palabras completas** aparece atenuada mientras los comodines están activados, porque **<** y **>** hacen ese trabajo.

Si el patrón tiene un error, el panel indica cuál es.

Si sus palabras no están en el documento, el panel lo indica y nada se mueve.

Puede seguir trabajando en el documento mientras el panel está abierto. Haga clic en la página para escribir. Haga clic en la **X** de la parte superior del panel para cerrarlo.

## Búsqueda avanzada

Estas opciones están en el panel, debajo de **Usar comodines**. Cada una cambia la forma en que JeffWrite lee las palabras de **Buscar**.

- **Suena como** encuentra palabras que se pronuncian igual que la suya. Asigna a cada palabra un código según sus sonidos y encuentra las palabras con el mismo código. Escriba **Smith** y también encontrará «Smyth».
- **Buscar todas las formas de la palabra** encuentra todas las formas de la palabra. Escriba **correr** y encontrará «corre», «corría» y «corrieron». Funciona con palabras completas. Si escribe dos palabras, como **perro grande**, las encuentra una junto a otra, en cualquiera de sus formas.
- **Ignorar signos de puntuación** omite signos como guiones y puntos entre las letras. Escriba **correo** y también encontrará «correo-e».
- **Ignorar caracteres de espacio en blanco** omite los espacios y tabulaciones entre las letras. Escriba **correo electrónico** y encontrará «correoelectrónico».

Solo una de las opciones **Suena como**, **Buscar todas las formas de la palabra** y **Usar comodines** puede estar activada a la vez. **Suena como** y **Buscar todas las formas de la palabra** siempre encuentran palabras completas, por eso **Buscar solo palabras completas** y **Coincidir mayúsculas y minúsculas** aparecen atenuadas.

### Especial

Haga clic en **Especial** para poner un código en **Buscar**: **Marca de párrafo** (^p), **Carácter de tabulación** (^t), **Cualquier dígito** (^#), **Cualquier letra** (^$), **Espacio en blanco** (^w), **Salto de sección** (^b), **Campo** (^d), **Marca de nota al pie** (^f), **Marca de nota al final** (^e) o **Carácter de intercalación** (^^). Los códigos funcionan sin **Usar comodines**. Un **salto de sección** es la marca que termina una sección. Un **campo** es un fragmento de texto que el programa calcula, como un número de página o una fecha, y **^d** encuentra dónde empieza cada uno. Las marcas de nota son los números pequeños del texto que señalan una nota al pie o una nota al final. Estos cuatro se encuentran, pero **Reemplazar** y **Reemplazar todo** no los tocan, porque cambiar uno haría perder lo que representa. Los demás signos de la lista, como **?** y **\***, son entonces solo esos caracteres.

### Formato

Haga clic en **Formato** para buscar solo el texto que tiene un aspecto concreto: **Negrita**, **Cursiva**, **Subrayado**, **Tachado**, **Resaltado**, una **Fuente** o un **Estilo** de párrafo. El panel muestra debajo de los botones los aspectos que eligió. Elija **Sin formato** para borrarlos.

Puede dejar **Buscar** vacío. Entonces, cada fragmento de texto con ese aspecto es una coincidencia. Si hay palabras en **Buscar**, solo se encuentran las palabras con ese aspecto. **Reemplazar todo** cambia solo esas palabras.

## Limitar la búsqueda

- **Coincidir mayúsculas y minúsculas** encuentra solo las palabras con las mismas mayúsculas. Con esta opción activada, «Plan» no encuentra «plan».
- **Buscar solo palabras completas** omite las palabras más largas que contienen la suya. Con esta opción activada, «plan» no encuentra «planeta».
- **Buscar en** indica en qué sentido va **Buscar siguiente** desde el cursor. **Hacia abajo** va hasta el final del documento, y **Hacia arriba** hasta el inicio. En cada caso se detiene allí y lo indica. **Todo** continúa hasta el otro extremo y es la opción habitual. **Reemplazar todo** siempre cambia todo el documento.

## Reemplazar palabras

1. Presione **Ctrl+H** o haga clic en **Inicio > Edición > Reemplazar**. El cursor pasa a **Reemplazar con**.
2. Escriba las palabras nuevas en **Reemplazar con**. Para eliminar las palabras encontradas, déjelo vacío.
3. Escriba las palabras antiguas en **Buscar**.
4. Haga clic en **Reemplazar** para ver la primera coincidencia. Haga clic de nuevo en **Reemplazar** para cambiarla. Después JeffWrite pasa a la siguiente coincidencia. Cada cambio es un paso, así que **Deshacer** (Ctrl+Z) los revierte uno a uno.
5. Haga clic en **Reemplazar todo** para cambiar todas las coincidencias a la vez. El panel indica cuántas cambió. **Reemplazar todo** también cambia las palabras de los cuadros de texto y de las formas, y es un solo paso, así que un único **Deshacer** devuelve todas las palabras a su estado anterior. Cuando **Control de cambios** está activado, cada reemplazo se registra como un cambio controlado, tanto en el texto como en los cuadros. Un cuadro demasiado grande para cambiarlo se cuenta en el mensaje. **Reemplazar todo** no llega a los cuadros de texto de los encabezados y pies de página, a las formas agrupadas ni a los cuadros vinculados entre sí. **Buscar siguiente** todavía no busca en los cuadros de texto.

Las palabras nuevas toman la misma mayúscula inicial que las palabras que reemplazan. Si reemplaza una palabra en negrita, la nueva palabra también va en negrita.

Por ejemplo, para cambiar «color» por «colour» en todo el documento: escriba **color** en **Buscar**, escriba **colour** en **Reemplazar con** y haga clic en **Reemplazar todo**.

## Dónde busca

- Busca en el texto del documento, incluido el texto de las tablas. Un marcador en medio de algunas palabras, que tienen algunos archivos, no impide encontrarlas. **Reemplazar** y **Reemplazar todo** conservan el marcador.
- **Buscar siguiente** busca primero en el texto. Después pasa a los encabezados, luego a los pies de página, luego a las notas al pie y, por último, vuelve al inicio del texto. Cuando una coincidencia está en uno de ellos, JeffWrite lo abre para editarlo y selecciona la coincidencia. El panel indica dónde está, por ejemplo «Encontrado en el encabezado de la página 3». **Reemplazar** cambia la coincidencia actual, ya esté en el texto, en un encabezado, en un pie de página o en una nota al pie. **Reemplazar todo** cambia todas las coincidencias de todos ellos, y un solo **Deshacer** las devuelve. Las notas al final forman parte del texto, así que se buscan junto con él. Consulte [Encabezados y pies de página](headers-footers).
- Omite el texto configurado como **Oculto**, porque ese texto no se muestra. Consulte [Cambiar el aspecto del texto](font).

## Temas relacionados

- [Cambiar el aspecto del texto](font)
- [Usar JeffWrite con el teclado](keyboard)
