# Buscar y reemplazar palabras
<!-- keywords: buscar avanzado, suena como, soundex, formas de palabra, todas las formas de una palabra, ignorar signos de puntuación, ignorar espacios, especial, marca de párrafo, tabulación, buscar por formato, formato, negrita, fuente, estilo, comodines, comodín, patrón, expresión regular, grupos, recuento de coincidencias, resaltar todas las coincidencias, reemplazar, reemplazar todo, buscar, buscar palabras, cambiar palabras, coincidir mayúsculas y minúsculas, palabras completas, buscar siguiente, ctrl+f, ctrl+h, edición, panel, texto oculto, deshacer -->

Use Buscar para ir a palabras de su documento. Use Reemplazar para cambiarlas. Ambas funciones trabajan en un mismo panel a la derecha de la ventana.

## Buscar palabras

1. Presione **Ctrl+F**, o haga clic en **Inicio > Edición > Buscar**. Se abre el panel **Buscar y reemplazar**, con el cursor en **Buscar:**.
2. Escriba las palabras que desea encontrar. Después presione **Entrar**, o haga clic en **Buscar siguiente**.

JeffWrite empieza donde esté el cursor. Selecciona el siguiente lugar donde encuentra sus palabras y desplaza las páginas para que lo vea. Ese lugar se llama **coincidencia**. Haga clic en **Buscar siguiente** de nuevo para ir a la siguiente coincidencia. Después de la última, vuelve al inicio del documento.

Mientras escribe, el panel cuenta las coincidencias, por ejemplo "9 coincidencias", y cada coincidencia recibe una franja amarilla en las páginas. Las franjas también aparecen en celdas de tablas, encabezados, pies de página y notas al pie. Después de **Buscar siguiente**, el panel indica cuál es la actual, por ejemplo "Coincidencia 2 de 4". El recuento y las franjas cambian al activar **Coincidir mayúsculas y minúsculas** o **Solo palabras completas**, o al cambiar el texto. Desaparecen al cerrar el panel.

## Buscar con comodines

Active **Usar comodines** para buscar un patrón, no solo palabras. Un **comodín** es un signo que representa otras letras. Estos son los signos:

- **?** representa cualquier letra. **c?t** encuentra "cat" y "cot".
- **\*** representa cualquier secuencia de letras. **c\*t** encuentra "cat" y "cart".
- **<** es el inicio de una palabra, y **>** es el final. **<the>** encuentra "the" pero no "other".
- **[abc]** es cualquiera de esas letras, **[a-c]** es cualquier letra de la a a la c, y **[!abc]** es cualquier letra excepto esas.
- **{2}** repite dos veces la letra anterior. **{1,3}** la repite de una a tres veces.
- **@** significa una o más veces la letra anterior.
- **\\** delante de un signo busca el signo en sí. **\\?** encuentra un signo de interrogación.
- **^p** es una marca de párrafo, **^t** es un tabulador, **^#** es un dígito, **^$** es una letra y **^w** es un espacio en blanco.

Coloque una parte del patrón entre **( )** para crear un **grupo**. En **Reemplazar con:**, **\1** es el primer grupo, **\2** el segundo, y así hasta **\9**. **^&** es todo lo encontrado. Por ejemplo, busque **(<[A-Z]@>), (<[A-Z]@>)** y reemplace con **\2 \1** para convertir "Smith, John" en "John Smith". **Solo palabras completas** aparece atenuada mientras los comodines están activados, porque **<** y **>** hacen ese trabajo.

Si el patrón tiene un error, el panel indica cuál es.

Si sus palabras no están en el documento, el panel lo indica y nada se mueve.

Puede seguir trabajando en el documento mientras el panel está abierto. Haga clic en la página para escribir. Haga clic en la **X** de la parte superior del panel para cerrarlo.

## Búsqueda avanzada

Estas opciones están en el panel debajo de **Usar comodines**. Cada una cambia la manera en que JeffWrite interpreta las palabras de **Buscar:**.

- **Suena como** encuentra palabras que se pronuncian igual que la suya. Asigna a cada palabra un código según sus sonidos y encuentra las palabras con el mismo código. Escriba **Smith** y también encontrará "Smyth".
- **Buscar todas las formas de la palabra** encuentra todas las formas de la palabra. Escriba **correr** y encontrará "corre," "corrió" y "corriendo". Funciona con palabras completas. Si escribe dos palabras, como **perro grande**, las encuentra juntas, en cualquiera de sus formas.
- **Ignorar caracteres de puntuación** omite signos como guiones y puntos entre las letras. Escriba **correo** y también encontrará "correo-e" o "correo.e".
- **Ignorar caracteres de espacio en blanco** omite espacios y tabulaciones entre las letras. Escriba **correo e** y encontrará "correo-e" sin espacios, es decir, "correoe".

Solo una de las opciones **Suena como**, **Buscar todas las formas de la palabra** y **Usar comodines** puede estar activada a la vez. **Suena como** y **Buscar todas las formas de la palabra** siempre encuentran palabras completas, por eso **Solo palabras completas** y **Coincidir mayúsculas y minúsculas** aparecen atenuadas.

### Especial

Haga clic en **Especial** para poner un código en **Buscar:**: **Marca de párrafo** (^p), **Tabulación** (^t), **Cualquier dígito** (^#), **Cualquier letra** (^$), **Espacio en blanco** (^w), **Salto de sección** (^b), **Campo** (^d), **Marca de nota al pie** (^f), **Marca de nota al final** (^e) o **Carácter de intercalación** (^^). Los códigos funcionan sin **Usar comodines**. Un **salto de sección** es la marca que termina una sección. Un **campo** es un fragmento de texto que el programa calcula, como un número de página o una fecha, y **^d** encuentra el inicio de cada uno. Las marcas de nota son los números pequeños del texto que señalan una nota al pie o al final. Estas cuatro se encuentran, pero **Reemplazar** y **Reemplazar todo** no las modifican, porque cambiar una supondría perder lo que representa. Los demás signos del cuadro, como **?** y **\***, son entonces solo esos caracteres.

### Formato

Haga clic en **Formato** para buscar solo texto con un aspecto determinado: **Negrita**, **Cursiva**, **Subrayado**, **Tachado**, **Resaltado**, una **Fuente** o un **Estilo** de párrafo. El panel muestra debajo de los botones los aspectos que eligió. Seleccione **Sin formato** para quitarlos.

Puede dejar **Buscar:** vacío. Entonces, cada fragmento de texto con ese aspecto es una coincidencia. Si escribe palabras en **Buscar:**, solo se encuentran las palabras con ese aspecto. **Reemplazar todo** cambia solo esas palabras.

## Acotar la búsqueda

- **Coincidir mayúsculas y minúsculas** encuentra solo palabras con las mismas mayúsculas. Con esta opción activada, "Plan" no encuentra "plan".
- **Solo palabras completas** omite las palabras más largas que contienen la suya. Con esta opción activada, "plan" no encuentra "planeta".

## Reemplazar palabras

1. Presione **Ctrl+H**, o haga clic en **Inicio > Edición > Reemplazar**. El cursor pasa a **Reemplazar con:**.
2. Escriba las palabras nuevas en **Reemplazar con:**. Para eliminar las palabras encontradas, déjelo vacío.
3. Escriba las palabras antiguas en **Buscar:**.
4. Haga clic en **Reemplazar** para ver la primera coincidencia. Haga clic de nuevo en **Reemplazar** para cambiarla. JeffWrite pasa entonces a la siguiente coincidencia. Cada cambio es un paso, así que **Deshacer** (Ctrl+Z) retira uno cada vez.
5. Haga clic en **Reemplazar todo** para cambiar todas las coincidencias a la vez. El panel indica cuántas cambió. **Reemplazar todo** es un solo paso, así que un **Deshacer** devuelve todas las palabras a su estado original.

Las palabras nuevas adoptan el formato de la primera letra de las palabras que reemplazan. Si reemplaza una palabra en negrita, la palabra nueva queda en negrita.

Por ejemplo, para cambiar "color" por "colour" en todo el documento: escriba **color** en **Buscar:**, escriba **colour** en **Reemplazar con:** y haga clic en **Reemplazar todo**.

## Lo que busca la búsqueda

- Busca en el texto del documento, incluido el texto de las tablas. Un marcador en medio de algunas palabras, que tienen algunos archivos, no impide encontrarlas. **Reemplazar** y **Reemplazar todo** conservan el marcador.
- **Buscar siguiente** mira primero en el texto. Después continúa con los encabezados, luego con los pies de página, luego con las notas al pie y, por último, vuelve al inicio del texto. Cuando una coincidencia está en uno de ellos, JeffWrite lo abre para editarlo y selecciona la coincidencia. El panel indica dónde está, por ejemplo "Encontrado en el encabezado de la página 3." **Reemplazar** cambia la coincidencia actual, esté en el texto, en un encabezado, en un pie de página o en una nota al pie. **Reemplazar todo** cambia todas las coincidencias de todos ellos, y un **Deshacer** las devuelve todas. Las notas al final forman parte del texto, así que se buscan con él. Consulte [Encabezados y pies de página](headers-footers).
- Omite el texto marcado como **Oculto**, porque ese texto no se muestra. Consulte [Cambiar el aspecto del texto](font).

## Temas relacionados

- [Cambiar el aspecto del texto](font)
- [Usar JeffWrite con el teclado](keyboard)
