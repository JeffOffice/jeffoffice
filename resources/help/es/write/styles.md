# Usar estilos
<!-- keywords: estilo de lista, guardar un estilo de lista, estilo de lista multinivel, estilos de lista, administrar estilos, estilo bloqueado, bloquear un estilo, limitar formato, restringir formato, exportar estilos, archivo de plantilla, dotx, ocultar un estilo, estilos recomendados, estilos, estilo, panel de estilos, título, normal, crear estilo, nuevo estilo, modificar estilo, cambiar estilo, basado en, estilo siguiente, actualizar automáticamente, galería, estilo de carácter, estilo de párrafo, ctrl+alt+shift+s -->

Un **estilo** es un conjunto guardado de aspectos: una fuente, un tamaño, un espaciado y más. Cuando da un estilo a un párrafo, el párrafo adopta todos sus aspectos a la vez. Si cambia el estilo, cambian también todos los párrafos que lo tienen. Por eso los estilos mantienen ordenados los documentos largos.

JeffWrite tiene dos tipos:

- Un **estilo de párrafo** define el aspecto de párrafos completos. **Título 1** y **Normal** son estilos de párrafo.
- Un **estilo de carácter** define el aspecto de las palabras que selecciona. **Énfasis fuerte** es un estilo de carácter.

## Aplicar un estilo al texto

1. Haga clic en un párrafo o seleccione las palabras que desea cambiar.
2. Haga clic en un estilo de la galería **Estilos** de la ficha **Inicio**. También puede pulsar **Ctrl+Mayús+N** para Normal, o **Ctrl+Alt+1**, **Ctrl+Alt+2** o **Ctrl+Alt+3** para Título 1, 2 o 3.

Un estilo de párrafo se aplica a todos los párrafos que toque. Un estilo de carácter se aplica a las palabras seleccionadas.

## Abrir el panel Estilos

Haga clic en la pequeña flecha de la esquina del grupo **Estilos**, o pulse **Ctrl+Alt+Mayús+S**. El panel **Estilos** se abre a la derecha y muestra los estilos del documento, cada uno con su propio aspecto.

- El estilo de la posición del cursor aparece en negrita, con una flecha delante del nombre.
- Haga clic en un estilo para aplicarlo a la selección.
- Use la lista que hay debajo de los estilos para elegir cuáles mostrar:
  - **En uso** muestra solo los estilos que usa el texto.
  - **Todos los estilos** muestra todos, incluidos los que el documento todavía no tiene. Un estilo se añade al documento la primera vez que lo usa.
  - **Recomendados** muestra los estilos pensados para verse. Un estilo que permanece oculto hasta que se usa aparece aquí en cuanto lo usa.
- Haga clic en **Administrar…** debajo de la lista para elegir, en cada estilo, si es **Recomendado** (se muestra en la galería), si está **Oculto** (no aparece en ninguna lista) y su **Orden en las listas**. Un número menor aparece antes. Haga clic en **Aceptar** y todos los cambios se realizan en un solo paso, así que un **Deshacer** los revierte todos.
- En **Administrar…**, haga clic en **Eliminar** para quitar un estilo que creó. Los párrafos que lo tenían vuelven a Normal, y los estilos basados en él pasan a basarse en lo que él estaba basado. Los estilos que vienen con JeffWrite no se pueden eliminar. Eliminar es un solo paso, así que **Deshacer** restaura el estilo.
- En **Administrar…**, haga clic en **Importar desde archivo…** para copiar estilos de otro documento. Elija el archivo, marque los estilos que quiere y haga clic en **Aceptar**. Un estilo que necesita otro estilo trae también ese. Los estilos que ya tiene se dejan como están, salvo que marque **Reemplazar estilos que tienen el mismo nombre**. Importar es un solo paso para deshacer.
- En **Administrar…**, haga clic en **Exportar a archivo…** para guardar algunos estilos en un archivo de plantilla (un archivo **.dotx**). Marque los estilos que quiere, haga clic en **Aceptar** y elija dónde guardarlo. Otro documento puede tomarlos de ese archivo con **Importar desde archivo…**. Un estilo que necesita otro estilo trae también ese.
- Marque **Bloqueado** en un estilo para bloquearlo. Después, marque **Limitar formato a los estilos que no están bloqueados** para activar la limitación. Mientras esté activa, nadie puede dar formato manualmente al texto, por ejemplo con **Negrita** o **Centrar**, ni usar un estilo bloqueado. Sí pueden usar los demás estilos. La barra de estado lo indica cuando se rechaza un comando. Desmarque la casilla para desactivar la limitación. Si el documento está protegido con contraseña, primero quite esa protección en **Revisar > Restringir edición**. La limitación y los bloqueos se guardan en el archivo.
- Debajo de la lista, el cuadro **Ordenar** pone los estilos en orden. **Alfabético** ordena por nombre. **Según recomendación** pone primero los estilos más útiles. **Por tipo** muestra los estilos de párrafo, luego los vinculados y después los de carácter. **Basado en** agrupa cada estilo con el estilo en el que se basa.

## Crear un estilo a partir del texto

1. Dé formato a un párrafo como desee. Por ejemplo, póngalo en negrita, centrelo y ajuste el tamaño.
2. Haga clic en **Nuevo estilo** en la parte inferior del panel, o en **Crear un estilo** en la ficha **Inicio**.
3. Escriba un **Nombre**. Puede tener hasta 255 letras y no puede ser el nombre de otro estilo.
4. Revise las demás opciones y haga clic en **Aceptar**.

El **Tipo de estilo** puede ser **Párrafo**, **Carácter** o **Vinculado**. Un estilo vinculado es un estilo de párrafo con un estilo de carácter de igual aspecto junto a él (con el mismo nombre que el estilo de párrafo y «Car» añadido), de modo que el aspecto se puede aplicar a un párrafo completo o solo a algunas palabras. Si cambia después el estilo de párrafo, su estilo de carácter también adopta el nuevo aspecto.

**Disponible en** permite elegir **Solo en este documento** o **Nuevos documentos basados en esta plantilla**. Con la segunda opción, el estilo también se guarda en la plantilla de la que parten los documentos nuevos.

El nuevo estilo se guarda con su documento, y su párrafo lo adopta. Un **Deshacer** (Ctrl+Z) elimina el estilo.

## Cambiar un estilo

1. Haga clic en un párrafo que tenga el estilo, o haga clic en el estilo en el panel.
2. Haga clic en **Modificar** en la parte inferior del panel, o en **Modificar estilo** en la ficha **Inicio**.
3. Cambie lo que desee y haga clic en **Aceptar**.

El cuadro de diálogo tiene estas opciones:

- **Nombre** es el nombre del estilo. Un estilo que viene con JeffWrite conserva su nombre.
- **Basado en** es el estilo del que parte este. El nuevo estilo se parece a ese, salvo en lo que cambie. Si cambia después el estilo base, este estilo lo seguirá.
- **Estilo para el párrafo siguiente** es el estilo que recibe el párrafo siguiente al pulsar **Intro** al final de este. Normalmente, a un título le sigue Normal.
- **Fuente**, **tamaño**, **Negrita**, **Cursiva**, **Subrayado** y **color** definen el aspecto de las palabras.
- **Alineación**, **sangría**, **espaciado** e **interlineado** definen el aspecto de los párrafos.
- **Añadir a la galería de estilos** coloca el estilo en la galería de la ficha **Inicio**.
- **Actualizar automáticamente** hace que el estilo siga su formato. Al cambiar un párrafo que tiene este estilo, el estilo cambia para coincidir, y también cambian todos los demás párrafos que lo tienen.

Cuando cambia un estilo, cambian los párrafos que lo tienen. Si dio a un párrafo un aspecto propio, como una palabra en rojo, lo conserva.

Si hace clic en **Aceptar** sin cambiar nada, el documento no se modifica.

## Actualizar un estilo según el texto

Dé formato a un párrafo como desee. Después, haga clic en **Actualizar estilo para que coincida con la selección** en la ficha **Inicio**. El estilo de ese párrafo adopta los cambios que hizo, y todos los párrafos con ese estilo se actualizan.

## Guardar una lista como estilo de lista

Un **estilo de lista** es una lista numerada o con viñetas con nombre. Conserva los nueve niveles: el número o la viñeta de cada nivel, su texto, dónde empieza y dónde se sitúa el texto.

1. Haga clic en **Lista multinivel** en la ficha **Inicio** y, después, en **Definir nueva lista multinivel…**.
2. Configure los niveles como desee.
3. Marque **Guardar como estilo de lista** y escriba un nombre. Si el nombre está vacío o ya se usa, el cuadro lo indica y permanece abierto.
4. Haga clic en **Aceptar**. Los párrafos seleccionados adoptan el estilo de lista.

El estilo de lista aparece ahora en dos lugares: bajo **Estilos de lista** en la galería **Lista multinivel** (los primeros 100), y en el panel **Estilos** (todos). Seleccione párrafos y haga clic en su nombre para aplicarles el estilo de lista.

- **Modificarlo.** Haga clic en **Lista multinivel**, **Modificar estilo de lista** y el nombre, o elija el estilo de lista en el panel **Estilos** y haga clic en **Modificar…**. Todos los párrafos que tienen el estilo de lista cambian con él, tanto en el texto como en encabezados, pies de página y notas. En el texto es un solo paso, así que un **Deshacer** lo revierte.
- **Eliminarlo.** Haga clic en **Lista multinivel**, **Eliminar estilo de lista** y el nombre. Los párrafos conservan sus números y viñetas tal como se ven, solo que ya no siguen a ningún estilo.
- **Guardarlo en un archivo.** Un estilo de lista se guarda en un archivo `.docx` como estilo de numeración, y en un archivo `.odt` como estilo de lista. Vuelve a ser un estilo de lista al abrir de nuevo el archivo. Un archivo puede tener hasta 1.000 estilos de lista, y los restantes se usan como listas normales.

Un nivel puede seguir vinculado a un estilo de párrafo, como en cualquier lista multinivel. Entonces, el estilo de párrafo se numera con el estilo de lista, y **Modificar estilo de lista** muestra el vínculo para que pueda cambiarlo.

Si el cuadro no tiene una lista a la que aplicar el estilo (por ejemplo, porque el texto seleccionado está bloqueado), no se guarda ningún estilo de lista y se muestra un mensaje que lo indica.

## Temas relacionados

- [Cambiar el aspecto del texto](font)
- [Copiar un aspecto con Copiar formato](format-painter)
- [Usar JeffWrite con el teclado](keyboard)
