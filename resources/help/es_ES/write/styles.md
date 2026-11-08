# Usar estilos
<!-- keywords: administrar estilos, estilo bloqueado, bloquear un estilo, limitar el formato, restringir el formato, exportar estilos, archivo de plantilla, dotx, ocultar un estilo, estilos recomendados, estilos, estilo, panel de estilos, título, normal, crear estilo, nuevo estilo, modificar estilo, cambiar estilo, basado en, siguiente estilo, actualizar automáticamente, galería, estilo de carácter, estilo de párrafo, ctrl+alt+mayús+s, fichero de plantilla -->

Un **estilo** es un conjunto guardado de aspectos: una fuente, un tamaño, un espaciado y más. Cuando aplica un estilo a un párrafo, el párrafo adopta todos sus aspectos a la vez. Si cambia el estilo, cambian también todos los párrafos que lo tienen. Por eso los estilos mantienen ordenados los documentos largos.

JeffWrite tiene dos tipos:

- Un **estilo de párrafo** define el aspecto de párrafos completos. **Título 1** y **Normal** son estilos de párrafo.
- Un **estilo de carácter** define el aspecto de las palabras que selecciona. **Strong** es un estilo de carácter.

## Aplicar un estilo a su texto

1. Haga clic en un párrafo o seleccione las palabras que desea cambiar.
2. Haga clic en un estilo de la galería **Estilos** de la pestaña **Inicio**. También puede pulsar **Ctrl+Mayús+N** para Normal, o **Ctrl+Alt+1**, **Ctrl+Alt+2** o **Ctrl+Alt+3** para Título 1, 2 o 3.

Un estilo de párrafo se aplica a cada párrafo que toque. Un estilo de carácter se aplica a las palabras que seleccionó.

## Abrir el panel Estilos

Haga clic en la pequeña flecha de la esquina del grupo **Estilos**, o pulse **Ctrl+Alt+Mayús+S**. El panel **Estilos** se abre a la derecha. Muestra los estilos de su documento, cada uno con su propio aspecto.

- El estilo donde está el cursor aparece en negrita, con una flecha delante de su nombre.
- Haga clic en un estilo para aplicarlo a su selección.
- Use la lista debajo de los estilos para elegir cuáles mostrar:
  - **En uso** muestra solo los estilos que usa su texto.
  - **Todos los estilos** muestra todos los estilos, incluidos los que su documento todavía no tiene. Un estilo se agrega al documento la primera vez que lo usa.
  - **Recomendados** muestra los estilos pensados para verse. Un estilo que permanece oculto hasta que se usa aparece aquí en cuanto lo usa.
- Haga clic en **Administrar…** debajo de la lista para elegir, en cada estilo, si es **Recomendado** (se muestra en la galería), si está **Oculto** (no aparece en ninguna lista) y su **Orden en las listas**. Un número menor aparece primero. Haga clic en **Aceptar**; todos los cambios se realizan en un solo paso, así que un **Deshacer** los revierte.
- En **Administrar…**, haga clic en **Eliminar** para quitar un estilo que usted creó. Los párrafos que lo tenían vuelven a Normal, y los estilos basados en él pasan a basarse en lo que él estaba basado. Los estilos que vienen con JeffWrite no se pueden eliminar. Eliminar es un solo paso, así que **Deshacer** devuelve el estilo.
- En **Administrar…**, haga clic en **Importar desde fichero…** para copiar estilos de otro documento. Elija el fichero, marque los estilos que desea y haga clic en **Aceptar**. Un estilo que necesita otro estilo trae también a ese. Los estilos que ya tiene se conservan, salvo que marque **Reemplazar los estilos que tienen el mismo nombre**. Importar es un solo paso para deshacer.
- En **Administrar…**, haga clic en **Exportar a fichero…** para guardar algunos de sus estilos en un fichero de plantilla (un fichero **.dotx**). Marque los estilos que desea, haga clic en **Aceptar** y elija dónde guardarlo. Otro documento puede tomarlos de ese fichero con **Importar desde fichero…**. Un estilo que necesita otro estilo trae también a ese.
- Marque **Bloqueado** en un estilo para bloquearlo. Después marque **Limitar el formato a los estilos que no están bloqueados** para activar la limitación. Mientras la limitación esté activa, nadie puede dar formato a mano al texto, por ejemplo con **Negrita** o **Centrar**, y nadie puede usar un estilo bloqueado. Sí pueden usar cualquier otro estilo. La barra de estado indica cuándo se rechaza un comando. Desmarque la casilla para desactivar la limitación. Si el documento está protegido con contraseña, primero quite esa protección en **Revisar > Restringir edición**. La limitación y los bloqueos se guardan en el fichero.
- Debajo de la lista, el cuadro **Ordenar** pone los estilos en orden. **Alfabético** los ordena por nombre. **Según recomendación** pone primero los estilos más útiles. **Por tipo** muestra los estilos de párrafo, luego los estilos vinculados y después los estilos de carácter. **Basado en** agrupa cada estilo con el estilo en el que se basa.

## Crear un estilo a partir de su texto

1. Dé formato a un párrafo como desee. Por ejemplo, póngalo en negrita, centrado y ajuste el tamaño.
2. Haga clic en **Nuevo estilo** en la parte inferior del panel, o en **Crear un estilo** en la pestaña **Inicio**.
3. Escriba un **Nombre**. Un nombre puede tener hasta 255 letras, y no puede ser el nombre de otro estilo.
4. Revise las demás opciones y haga clic en **Aceptar**.

El nuevo estilo se guarda con su documento, y el párrafo lo adopta. Un **Deshacer** (Ctrl+Z) elimina el estilo.

## Cambiar un estilo

1. Haga clic en un párrafo que tenga el estilo, o haga clic en el estilo en el panel.
2. Haga clic en **Modificar** en la parte inferior del panel, o en **Modificar estilo** en la pestaña **Inicio**.
3. Cambie lo que desee y haga clic en **Aceptar**.

El cuadro de diálogo tiene estas opciones:

- **Nombre** es el nombre del estilo. Un estilo que viene con JeffWrite conserva su nombre.
- **Basado en** es el estilo del que parte este. El nuevo estilo se parece a ese estilo, salvo por lo que cambie. Si cambia después el estilo base, este estilo lo seguirá.
- **Estilo para el párrafo siguiente** es el estilo que recibe el párrafo siguiente cuando pulsa **Entrar** al final de este. Un título suele ir seguido de Normal.
- **Fuente**, **tamaño**, **Negrita**, **Cursiva**, **Subrayado** y **color** definen el aspecto de las palabras.
- **Alineación**, **sangría**, **espaciado** e **interlineado** definen el aspecto de los párrafos.
- **Agregar a la galería de estilos** coloca el estilo en la galería de la pestaña **Inicio**.
- **Actualizar automáticamente** hace que el estilo siga su formato. Cuando cambia un párrafo que tiene este estilo, el estilo cambia para coincidir, y también lo hacen todos los demás párrafos que lo tienen.

Cuando cambia un estilo, cambian los párrafos que lo tienen. Si dio a un párrafo un aspecto propio, como una palabra en rojo, esa palabra lo conserva.

Si hace clic en **Aceptar** sin cambiar nada, el documento no se modifica.

## Actualizar un estilo para que coincida con su texto

Dé formato a un párrafo como desee. Después haga clic en **Actualizar estilo para que coincida con la selección** en la pestaña **Inicio**. El estilo de ese párrafo adopta los cambios que hizo, y todos los párrafos con ese estilo se actualizan.

## Relacionado

- [Cambiar el aspecto del texto](font)
- [Copiar un aspecto con Copiar formato](format-painter)
- [Usar JeffWrite con el teclado](keyboard)
