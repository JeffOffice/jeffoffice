# Guardar un bloque y volver a insertarlo
<!-- keywords: bloques guardados, texto repetido, firma, membrete, atajo de texto, atajos de texto, reutilizar texto, guardar selección, insertar de nuevo, F3, biblioteca -->

Un **bloque** es una parte de un documento que guarda para poder volver a insertarla: una firma, un párrafo que usa en todas sus cartas, un membrete o una imagen con su título. Un bloque conserva su texto con sus fuentes, colores, estilos, viñetas y numeración, imágenes, vínculos, campos y tablas (filas, columnas, anchos, bordes, sombreado, celdas combinadas, una fila de encabezado que se repite y el estilo de la tabla). No conserva comentarios, notas al pie, notas al final ni saltos de sección (véase Información útil).

## Guardar un bloque

1. Seleccione lo que desea guardar.
2. Elija **Insertar > Bloques guardados > Guardar selección como bloque**.
3. Escriba un **Nombre**. También puede agregar una **Categoría** y una **Descripción**, que le ayudarán a encontrar el bloque más adelante.
4. En **Galería**, elija **Bloques guardados** (un bloque que se selecciona desde una galería) o **Atajos de texto** (un bloque que se inserta escribiendo su nombre).
5. En **Colocar**, elija cómo se inserta el bloque en un documento: **Tal cual, en el cursor**, **En un párrafo propio** o **En una página propia**.
6. Elija **Guardar**.

Si ya hay un bloque guardado con ese nombre, JeffWrite le pregunta si desea reemplazarlo.

## Insertar un bloque

1. Coloque el cursor donde debe ir el bloque. Si antes selecciona palabras, el bloque ocupa su lugar.
2. Elija **Insertar > Bloques guardados** y, a continuación, haga clic en el bloque. Escriba en el cuadro de búsqueda para encontrar un bloque por su nombre, categoría o descripción.

Insertar un bloque se puede deshacer en un solo paso. Si el bloque usa un estilo que su documento no tiene, ese estilo se agrega; un estilo que su documento ya tiene con el mismo nombre nunca se modifica. Un campo del bloque, como el número de página, se actualiza para el documento en el que está. El primer y el último párrafo de un bloque adoptan el aspecto de los párrafos a los que se unen, igual que al pegar.

## Sus propias portadas, tablas y cuadros de texto

Su propia portada, tabla o cuadro de texto puede guardarse en una galería propia, debajo de las integradas.

- **Portada.** Seleccione la portada y su salto de página (o coloque el cursor en cualquier punto sin seleccionar nada, si el documento ya tiene una portada) y elija **Insertar > Portada > Sus portadas > Guardar selección en la galería de portadas**.
- **Tabla.** Seleccione la tabla completa y elija **Insertar > Tabla > Tablas predefinidas > Mis tablas > Guardar selección en la galería de tablas**.
- **Cuadro de texto.** Haga clic en el cuadro de texto para seleccionarlo y elija **Insertar > Cuadro de texto > Mis cuadros de texto > Guardar selección en la galería de cuadros de texto**.

Escriba un nombre y haga clic en **Guardar**. Después aparecerá en **Mis portadas**, **Mis tablas** o **Mis cuadros de texto**. Haga clic en él para insertarlo. Una portada se coloca al principio del documento, en una página propia, y sustituye a la portada que tenga el documento. Una tabla o un cuadro de texto se inserta en el cursor. **Administrar bloques guardados** también muestra estas galerías, para que pueda cambiarles el nombre, modificarlas, eliminarlas, exportarlas e importarlas.

## Atajos de texto

Un atajo de texto es un bloque con un nombre corto. Escriba el nombre, pulse **F3** (o elija **Insertar > Bloques guardados > Expandir atajo de texto**) y el nombre se sustituye por el bloque. El nombre debe empezar una palabra: «firma» funciona después de un espacio, pero no dentro de «diseñar».

## Administrar bloques guardados

Elija **Insertar > Bloques guardados > Administrar bloques guardados**. Verá cada bloque y podrá cambiar su nombre, categoría, descripción o forma de insertarlo con **Editar propiedades**, y **Eliminar** un bloque. **Deshacer eliminación** restaura lo que eliminó mientras el cuadro esté abierto. **Exportar** guarda sus bloques en un archivo, y **Importar** agrega los bloques de un archivo de este tipo; un bloque cuyo nombre ya tiene se deja tal como está.

## Información útil

- Sus bloques guardados se conservan en un único archivo de este equipo, solo para su cuenta: no están en sus documentos ni se envían a ningún sitio. El archivo es `library.jblocks`, en la carpeta `Saved Blocks` dentro de su carpeta de datos: `AppData\Local\JeffOffice` en Windows, `Library/Application Support/JeffOffice` en un Mac y `.local/share/JeffOffice` en Linux. Todos los programas de JeffOffice de su equipo comparten este archivo, y dos ventanas pueden agregar bloques al mismo tiempo sin que se pierdan los de cada una.
- Un bloque puede tener hasta 8 megabytes, y la biblioteca admite hasta 5000 bloques.
- Una tabla se guarda como tabla cuando la selección incluye toda la tabla. Si la selección incluye solo una parte de una tabla, esa parte se guarda como texto, con un párrafo por cada celda, y JeffWrite le avisa antes de guardarla.
- Un marcador de un bloque recibe un nombre propio al insertar el bloque: si su documento ya tiene un marcador con ese nombre, el nuevo se llama con «_2», «_3» y así sucesivamente.
- Los comentarios no se incluyen. Las notas al pie y las notas al final tampoco. Los saltos de sección no se conservan. Las formas y los cuadros de texto se conservan y se insertan de nuevo con el bloque.
- Los cambios controlados se aceptan al guardar el bloque: el texto insertado se conserva y el texto eliminado no forma parte del bloque, de modo que un bloque nunca contiene texto de cambios controlados de otra persona.
- Un documento nunca agrega bloques a su biblioteca. Solo lo hace usted.
