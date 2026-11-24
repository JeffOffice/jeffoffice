# Usar estilos
<!-- keywords: estilo de lista, guardar estilo de lista, estilo de lista multinivel, estilos de lista, administrar estilos, estilo bloqueado, bloquear estilo, limitar formato, restringir formato, exportar estilos, archivo de plantilla, dotx, ocultar estilo, estilos recomendados, estilos, estilo, panel de estilos, encabezado, normal, crear estilo, nuevo estilo, modificar estilo, cambiar estilo, basado en, estilo siguiente, actualizar automáticamente, galería, estilo de carácter, estilo de párrafo, ctrl+alt+mayús+s, formato, plantilla -->

Un **estilo** es un conjunto guardado de aspectos: una fuente, un tamaño, un espaciado y más. Cuando aplica un estilo a un párrafo, el párrafo adopta todos sus aspectos a la vez. Si cambia el estilo, cambian también todos los párrafos que lo tienen. Por eso los estilos mantienen ordenados los documentos largos.

JeffWrite tiene dos tipos:

- Un **estilo de párrafo** define el aspecto de párrafos completos. **Título 1** y **Normal** son estilos de párrafo.
- Un **estilo de carácter** define el aspecto de las palabras que selecciona. **Strong** es un estilo de carácter.

## Aplicar un estilo a su texto

1. Haga clic en un párrafo o seleccione las palabras que desea cambiar.
2. Haga clic en un estilo de la galería **Estilos** de la ficha **Inicio**. También puede pulsar **Ctrl+Mayús+N** para Normal, o **Ctrl+Alt+1**, **Ctrl+Alt+2** o **Ctrl+Alt+3** para Título 1, 2 o 3.

Un estilo de párrafo se aplica a todos los párrafos que toque. Un estilo de carácter se aplica a las palabras que haya seleccionado.

## Abrir el panel Estilos

Haga clic en la pequeña flecha de la esquina del grupo **Estilos**, o pulse **Ctrl+Alt+Mayús+S**. El panel **Estilos** se abre a la derecha. Muestra los estilos de su documento, cada uno con su propio aspecto.

- El estilo en el que está el cursor aparece en negrita, con una flecha delante de su nombre.
- Haga clic en un estilo para aplicarlo a su selección.
- Use la lista bajo los estilos para elegir cuáles mostrar:
  - **En uso** muestra solo los estilos que usa su texto.
  - **Todos los estilos** muestra todos los estilos, incluidos los que el documento todavía no tiene. Un estilo se añade al documento la primera vez que lo usa.
  - **Recomendados** muestra los estilos pensados para verse. Un estilo que permanece oculto hasta que se usa aparece aquí una vez que lo usa.
- Haga clic en **Administrar…** debajo de la lista para elegir, en cada estilo, si es **Recomendado** (se muestra en la galería), si está **Oculto** (no aparece en ninguna lista) y su **Orden en las listas**. Un número menor aparece primero. Haga clic en **Aceptar** y todos los cambios se realizan en un solo paso, así que un **Deshacer** los revierte.
- En **Administrar…**, haga clic en **Eliminar** para quitar un estilo que haya creado. Los párrafos que lo tenían vuelven a Normal, y los estilos basados en él pasan a basarse en lo que él estaba basado. Los estilos que vienen con JeffWrite no se pueden eliminar. Eliminar es un solo paso, así que **Deshacer** restaura el estilo.
- En **Administrar…**, haga clic en **Importar desde archivo…** para copiar estilos de otro documento. Elija el archivo, marque los estilos que quiere y haga clic en **Aceptar**. Un estilo que necesita otro estilo trae también a ese. Los estilos que ya tiene se dejan como están, salvo que marque **Reemplazar estilos que tengan el mismo nombre**. Importar es un solo paso que se puede deshacer.
- En **Administrar…**, haga clic en **Exportar a archivo…** para guardar algunos de sus estilos en un archivo de plantilla (un archivo **.dotx**). Marque los estilos que quiere, haga clic en **Aceptar** y elija dónde guardarlo. Otro documento puede tomarlos de ese archivo con **Importar desde archivo…**. Un estilo que necesita otro estilo trae también a ese.
- Marque **Bloqueado** en un estilo para bloquearlo. Después marque **Limitar formato a los estilos que no están bloqueados** para activar la limitación. Mientras la limitación está activa, nadie puede dar formato a mano al texto, por ejemplo con **Negrita** o **Centrar**, ni usar un estilo bloqueado. Sí pueden usar todos los demás estilos. La barra de estado lo indica cuando se rechaza un comando. Desmarque la casilla para desactivar la limitación. Si el documento está protegido con una contraseña, primero quite esa protección en **Revisar > Restringir edición**. La limitación y los bloqueos se guardan en el archivo.
- Debajo de la lista, el cuadro **Ordenar** pone los estilos en orden. **Alfabéticamente** ordena por nombre. **Según recomendación** pone primero los estilos más útiles. **Por tipo** muestra los estilos de párrafo, luego los vinculados y después los de carácter. **Basado en** agrupa cada estilo con el estilo en el que se basa.

## Crear un estilo a partir de su texto

1. Dé formato a un párrafo como desee. Por ejemplo, póngalo en negrita, céntrelo y ajuste el tamaño.
2. Haga clic en **Nuevo estilo** en la parte inferior del panel, o haga clic en **Crear un estilo** en la ficha **Inicio**.
3. Escriba un **Nombre**. Un nombre puede tener hasta 255 letras, y no puede ser el nombre de otro estilo.
4. Revise las demás opciones y haga clic en **Aceptar**.

El **Tipo de estilo** puede ser **Párrafo**, **Carácter** o **Vinculado**. Un estilo vinculado es un estilo de párrafo con un estilo de carácter de igual aspecto a su lado (con el mismo nombre que el de párrafo y «Car» añadido), de modo que el aspecto se puede aplicar a un párrafo completo o solo a algunas palabras. Cuando cambie después el estilo de párrafo, su estilo de carácter también adopta el nuevo aspecto.

Un estilo de **Tabla** controla los bordes, la fila de encabezado y las filas con bandas de una tabla, y un estilo de **Lista** controla los números o viñetas de una lista. Para estos dos, escriba el **Nombre** y haga clic en **Aceptar**; se abre el cuadro de ese tipo con el nombre ya escrito. Una lista creada así se guarda siempre como estilo de lista. Un estilo de tabla o de lista no se conserva al guardar el documento como archivo .odt (estilos de tabla) o .rtf (ambos); **Comprobar compatibilidad** (Archivo > Información) se lo indica.

**Disponible en** permite elegir **Solo en este documento** o **Nuevos documentos basados en esta plantilla**. Con la segunda opción, el estilo también se guarda en la plantilla con la que empiezan los nuevos documentos.

El nuevo estilo se guarda con su documento, y su párrafo lo adopta. Un **Deshacer** (Ctrl+Z) elimina el estilo.

## Cambiar un estilo

1. Haga clic en un párrafo que tenga el estilo. También puede hacer clic en el estilo en el panel.
2. Haga clic en **Modificar** en la parte inferior del panel, o haga clic en **Modificar estilo** en la ficha **Inicio**.
3. Cambie lo que desee y haga clic en **Aceptar**.

El cuadro de diálogo tiene estas opciones:

- **Nombre** es el nombre del estilo. Un estilo que viene con JeffWrite conserva su nombre.
- **Basado en** es el estilo del que parte este. El nuevo estilo se parece a ese estilo, salvo por lo que cambie. Si cambia después el estilo base, este sigue el cambio.
- **Estilo para el párrafo siguiente** es el estilo que recibe el párrafo siguiente al pulsar **Intro** al final de este. Un encabezado suele ir seguido de Normal.
- **Fuente**, **tamaño**, **Negrita**, **Cursiva**, **Subrayado** y **color** definen el aspecto de las palabras.
- **Alineación**, **sangría**, **espaciado** e **interlineado** definen el aspecto de los párrafos.
- **Añadir a la galería de estilos** coloca el estilo en la galería de la ficha **Inicio**.
- **Actualizar automáticamente** hace que el estilo siga su formato. Cuando cambia un párrafo que tiene este estilo, el estilo cambia para coincidir, y también cambian todos los demás párrafos que lo tienen.

Cuando cambia un estilo, cambian los párrafos que lo tienen. Si dio a un párrafo un aspecto propio, como una palabra en rojo, lo conserva.

Si hace clic en **Aceptar** sin cambiar nada, el documento no se modifica.

## Ver de dónde viene el aspecto de su texto

Haga clic en **Inspector de estilos** en la parte inferior del panel Estilos. El panel sigue al cursor, ya esté en el texto, en un encabezado, en un pie de página o en una nota, y muestra tres cosas:

- El **Estilo de párrafo** del párrafo. Si el párrafo está en una tabla, el estilo de la tabla aparece bajo el nombre. Debajo del nombre, «Adicional» indica lo que el párrafo añade por sí mismo sobre su estilo, como su alineación, sangrías, espaciado, líneas, tabulaciones, bordes, sombreado, salto de página antes, nivel de esquema, numeración o viñetas, y dirección del texto. **Borrar formato de párrafo** elimina esos aspectos, para que el párrafo se vea como indica su estilo. Lo que el estilo de una tabla da a sus párrafos no se muestra, porque no es propio del párrafo.
- El **Estilo de carácter** del texto, si tiene uno. **Borrar estilo de carácter** lo quita.
- El **Formato de texto** que tienen las letras por sí mismas, como la fuente, el tamaño, la negrita, un color, el texto oculto o un borde. **Borrar formato de carácter** lo elimina, para que las letras se vean como indican sus estilos. El idioma del texto y su estilo de carácter se mantienen.

Seleccione primero el texto para borrar solo ese texto. Si no hay texto seleccionado, el panel describe la letra que está antes del cursor, y **Borrar estilo de carácter** y **Borrar formato de carácter** actúan sobre todo el tramo de letras alrededor del cursor que tienen el mismo aspecto que esa letra. Si selecciona varios párrafos o letras distintas, el panel indica «Varios estilos en la selección» o «Varios aspectos en la selección», enumera lo que contiene la selección y los botones actúan sobre todo ello.

Cada botón es un solo paso, así que **Deshacer** (Ctrl+Z) restaura el aspecto, y el cursor permanece donde estaba. Cambiar un estilo nunca hace que sus párrafos tengan aspecto propio, así que un párrafo que sigue a su estilo no muestra nada en «Adicional». Mayús+F1 sigue abriendo Mostrar formato, que muestra el aspecto en sí.

## Actualizar un estilo para que coincida con su texto

Dé formato a un párrafo como desee. Después haga clic en **Actualizar estilo para que coincida con la selección** en la ficha **Inicio**. El estilo de ese párrafo adopta los cambios que hizo, y todos los párrafos con ese estilo lo siguen.

## Guardar una lista como estilo de lista

Un **estilo de lista** es una lista numerada o con viñetas con nombre. Conserva los nueve niveles: el número o viñeta de cada nivel, su texto, dónde empieza y dónde se sitúa el texto.

1. Haga clic en **Lista multinivel** en la ficha **Inicio** y, después, en **Definir nueva lista multinivel…**.
2. Configure los niveles como desee.
3. Marque **Guardar como estilo de lista** y escriba un nombre. Si el nombre está vacío o ya se usa, el cuadro lo indica y permanece abierto.
4. Haga clic en **Aceptar**. Los párrafos seleccionados adoptan el estilo de lista.

El estilo de lista aparece ahora en dos sitios: bajo **Estilos de lista** en la galería **Lista multinivel** (los primeros 100), y en el panel **Estilos** (todos). Seleccione párrafos y haga clic en su nombre para aplicarles el estilo de lista.

- **Cambiarlo.** Haga clic en **Lista multinivel**, **Modificar estilo de lista** y el nombre, o elija el estilo de lista en el panel **Estilos** y haga clic en **Modificar…**. Todos los párrafos que tienen el estilo de lista cambian con él, tanto en el texto como en encabezados, pies de página y notas. En el texto es un solo paso, así que un **Deshacer** lo revierte.
- **Eliminarlo.** Haga clic en **Lista multinivel**, **Eliminar estilo de lista** y el nombre. Los párrafos conservan sus números y viñetas tal como se ven. Simplemente dejan de seguir un estilo.
- **Guardarlo en un archivo.** Un estilo de lista se guarda en un archivo `.docx` como estilo de numeración, y en un archivo `.odt` como estilo de lista. Vuelve a ser un estilo de lista al abrir de nuevo el archivo. Un archivo puede tener hasta 1000 estilos de lista, y el resto se usan como listas normales.

Un nivel puede seguir vinculado a un estilo de párrafo, como en cualquier lista multinivel. Entonces el estilo de párrafo se numera con el estilo de lista, y **Modificar estilo de lista** muestra el vínculo para que pueda cambiarlo.

Si el cuadro no tiene una lista a la que aplicar el estilo (por ejemplo, porque el texto seleccionado está bloqueado), no se guarda ningún estilo de lista, y un mensaje lo indica.

## Temas relacionados

- [Cambiar el aspecto del texto](font)
- [Copiar un aspecto con Copiar formato](format-painter)
- [Usar JeffWrite con el teclado](keyboard)
