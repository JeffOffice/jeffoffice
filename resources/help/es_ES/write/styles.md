# Usar estilos
<!-- keywords: estilo de lista, guardar un estilo de lista, estilo de lista multinivel, estilos de lista, administrar estilos, estilo bloqueado, bloquear un estilo, limitar formato, restringir formato, exportar estilos, archivo de plantilla, dotx, ocultar un estilo, estilos recomendados, estilos, estilo, panel Estilos, título, normal, crear estilo, nuevo estilo, modificar estilo, cambiar estilo, basado en, estilo siguiente, actualizar automáticamente, galería, estilo de carácter, estilo de párrafo, ctrl+alt+mayús+s, fichero de plantilla -->

Un **estilo** es un conjunto guardado de aspectos: una fuente, un tamaño, un espaciado y más. Cuando aplica un estilo a un párrafo, el párrafo adopta todos sus aspectos a la vez. Si cambia el estilo, cambian también todos los párrafos que lo tienen. Por eso los estilos mantienen ordenados los documentos largos.

JeffWrite tiene dos tipos:

- Un **estilo de párrafo** define el aspecto de párrafos completos. **Título 1** y **Normal** son estilos de párrafo.
- Un **estilo de carácter** define el aspecto de las palabras que selecciona. **Strong** es un estilo de carácter.

## Aplicar un estilo a su texto

1. Haga clic en un párrafo o seleccione las palabras que desea cambiar.
2. Haga clic en un estilo de la galería **Estilos** de la ficha **Inicio**. O pulse **Ctrl+Mayús+N** para Normal, o **Ctrl+Alt+1**, **Ctrl+Alt+2** o **Ctrl+Alt+3** para Título 1, 2 o 3.

Un estilo de párrafo se aplica a cada párrafo que toque. Un estilo de carácter se aplica a las palabras seleccionadas.

## Abrir el panel Estilos

Haga clic en la pequeña flecha de la esquina del grupo **Estilos**, o pulse **Ctrl+Alt+Mayús+S**. El panel **Estilos** se abre a la derecha. Muestra los estilos de su documento, cada uno con su propio aspecto.

- El estilo del cursor aparece en negrita, con una flecha delante del nombre.
- Haga clic en un estilo para aplicarlo a su selección.
- Use la lista situada debajo de los estilos para elegir cuáles mostrar:
  - **En uso** muestra solo los estilos que usa su texto.
  - **Todos los estilos** muestra todos, incluidos los que el documento aún no tiene. Un estilo se añade al documento la primera vez que lo usa.
  - **Recomendados** muestra los estilos pensados para verse. Un estilo que está oculto hasta que se usa aparece aquí en cuanto lo usa.
- Haga clic en **Administrar…** debajo de la lista para decidir, en cada estilo, si es **Recomendado** (se muestra en la galería), si está **Oculto** (no aparece en ninguna lista) y su **Orden en las listas**. Un número menor aparece primero. Haga clic en **Aceptar**; todos los cambios se realizan en un solo paso, así que un **Deshacer** los revierte.
- En **Administrar…**, haga clic en **Eliminar** para quitar un estilo que haya creado. Los párrafos que lo tenían vuelven a Normal, y los estilos basados en él pasan a basarse en lo que él estaba basado. Los estilos que vienen con JeffWrite no se pueden eliminar. Eliminar es un solo paso, así que **Deshacer** restaura el estilo.
- En **Administrar…**, haga clic en **Importar desde fichero…** para copiar estilos de otro documento. Elija el fichero, marque los estilos que quiera y haga clic en **Aceptar**. Un estilo que necesite otro estilo trae también ese. Los estilos que ya tiene se dejan como están, salvo que marque **Reemplazar estilos que tienen el mismo nombre**. Importar es un solo paso para deshacer.
- En **Administrar…**, haga clic en **Exportar a fichero…** para guardar algunos estilos en un fichero de plantilla (un fichero **.dotx**). Marque los estilos que quiera, haga clic en **Aceptar** y elija dónde guardarlo. Otro documento puede tomarlos de ese fichero con **Importar desde fichero…**. Un estilo que necesite otro estilo trae también ese.
- Marque **Bloqueado** en un estilo para bloquearlo. Después marque **Limitar formato a los estilos que no están bloqueados** para activar la limitación. Mientras esté activa, nadie puede dar formato manual al texto, por ejemplo con **Negrita** o **Centrar**, y nadie puede usar un estilo bloqueado. Sí puede usar los demás estilos. La barra de estado lo indica cuando se rechaza un comando. Desmarque la casilla para desactivar la limitación. Si el documento está protegido con contraseña, primero detenga esa protección en **Revisar > Restringir edición**. La limitación y los bloqueos se guardan en el fichero.
- Debajo de la lista, el cuadro **Ordenar** pone los estilos en orden. **Alfabéticamente** los ordena por nombre. **Según recomendación** pone primero los estilos más útiles. **Por tipo** muestra los estilos de párrafo, luego los estilos vinculados y después los de carácter. **Basado en** agrupa cada estilo con el estilo en el que se basa.

## Crear un estilo a partir de su texto

1. Dé formato a un párrafo como desee. Por ejemplo, póngalo en negrita, céntrelo y ajuste el tamaño.
2. Haga clic en **Nuevo estilo** en la parte inferior del panel, o en **Crear un estilo** de la ficha **Inicio**.
3. Escriba un **Nombre**. Un nombre puede tener hasta 255 letras, y no puede ser el nombre de otro estilo.
4. Revise las demás opciones y haga clic en **Aceptar**.

El nuevo estilo se guarda con su documento, y su párrafo lo adopta. Un **Deshacer** (Ctrl+Z) elimina el estilo.

## Cambiar un estilo

1. Haga clic en un párrafo que tenga el estilo. O haga clic en el estilo en el panel.
2. Haga clic en **Modificar** en la parte inferior del panel, o en **Modificar estilo** de la ficha **Inicio**.
3. Cambie lo que desee y haga clic en **Aceptar**.

El cuadro de diálogo tiene estas opciones:

- **Nombre** es el nombre del estilo. Un estilo que viene con JeffWrite conserva su nombre.
- **Basado en** es el estilo del que parte este. El nuevo estilo se parece a ese, salvo por lo que cambie. Si cambia después el estilo base, este lo seguirá.
- **Estilo para el párrafo siguiente** es el estilo que recibe el párrafo siguiente al pulsar **Intro** al final de este. Un encabezado suele ir seguido de Normal.
- **Fuente**, **tamaño**, **Negrita**, **Cursiva**, **Subrayado** y **color** definen el aspecto de las palabras.
- **Alineación**, **sangría**, **espaciado** e **interlineado** definen el aspecto de los párrafos.
- **Agregar a la galería de estilos** coloca el estilo en la galería de la ficha **Inicio**.
- **Actualizar automáticamente** hace que el estilo siga su formato. Cuando cambia un párrafo que tiene este estilo, el estilo cambia para coincidir, y también lo hacen todos los demás párrafos que lo tienen.

Cuando cambia un estilo, cambian los párrafos que lo tienen. Si dio a un párrafo un aspecto propio, como una palabra en rojo, ese aspecto se conserva.

Si hace clic en **Aceptar** sin cambiar nada, el documento no se modifica.

## Actualizar un estilo para que coincida con su texto

Dé formato a un párrafo como desee. Después haga clic en **Actualizar estilo para que coincida con la selección** en la ficha **Inicio**. El estilo de ese párrafo adopta los cambios que hizo, y todos los párrafos con ese estilo lo siguen.

## Guardar una lista como estilo de lista

Un **estilo de lista** es una lista numerada o con viñetas con nombre. Conserva los nueve niveles: el número o la viñeta de cada nivel, su texto, dónde empieza y dónde se sitúa el texto.

1. Haga clic en **Lista multinivel** en la ficha **Inicio** y después en **Definir nueva lista multinivel…**.
2. Configure los niveles como desee.
3. Marque **Guardar como estilo de lista** y escriba un nombre. Si el nombre está vacío o ya se usa, el cuadro lo indica y permanece abierto.
4. Haga clic en **Aceptar**. Los párrafos seleccionados adoptan el estilo de lista.

El estilo de lista aparece en dos lugares: debajo de **Estilos de lista** en la galería **Lista multinivel** (los primeros 100), y en el panel **Estilos** (todos). Seleccione párrafos y haga clic en su nombre para aplicarles el estilo de lista.

- **Cambiarlo.** Haga clic en **Lista multinivel**, **Modificar estilo de lista** y el nombre; o elija el estilo de lista en el panel **Estilos** y haga clic en **Modificar…**. Todos los párrafos que tienen el estilo de lista cambian con él, en el texto y en encabezados, pies de página y notas. En el texto es un solo paso, así que un **Deshacer** lo revierte.
- **Eliminarlo.** Haga clic en **Lista multinivel**, **Eliminar estilo de lista** y el nombre. Los párrafos conservan sus números y viñetas tal como se ven. Simplemente ya no siguen un estilo.
- **Guardarlo en un fichero.** Un estilo de lista se guarda en un fichero `.docx` como estilo de numeración, y en un fichero `.odt` como estilo de lista. Vuelve a ser un estilo de lista al abrir el fichero de nuevo. Un fichero puede tener hasta 1000 estilos de lista; los demás se usan como listas normales.

Un nivel puede seguir vinculado a un estilo de párrafo, como en cualquier lista multinivel. Entonces el estilo de párrafo se numera con el estilo de lista, y **Modificar estilo de lista** muestra el vínculo para que pueda cambiarlo.

Si no hay ninguna lista a la que aplicar el estilo (por ejemplo, porque el texto seleccionado está bloqueado), no se guarda ningún estilo de lista y un mensaje lo indica.

## Temas relacionados

- [Cambiar el aspecto del texto](font)
- [Copiar un aspecto con Copiar formato](format-painter)
- [Usar JeffWrite con el teclado](keyboard)
