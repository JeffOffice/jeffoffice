# Guardar un bloque y volver a insertarlo
<!-- keywords: bloques guardados, texto repetido, firma, membrete, atajo de texto, atajos de texto, reutilizar texto, guardar selección, insertar de nuevo, F3, biblioteca -->

Un **bloque** es una parte de un documento que guarda para poder volver a insertarla: una firma, un párrafo que usa en todas las cartas, un membrete o una imagen con su título. Un bloque conserva su texto con sus fuentes, colores, estilos, viñetas y numeración, imágenes, vínculos, campos y tablas (filas, columnas, anchos, bordes, sombreado, celdas combinadas, una fila de encabezado que se repite y el estilo de la tabla). No conserva comentarios, notas al pie, notas al final ni saltos de sección (véase Información útil).

## Guardar un bloque

1. Seleccione lo que desea guardar.
2. Elija **Insertar > Bloques guardados > Guardar selección como bloque**.
3. Escriba un **Nombre**. También puede agregar una **Categoría** y una **Descripción**, que le ayudan a encontrar el bloque más adelante.
4. En **Galería**, elija **Bloques guardados** (un bloque que selecciona desde una galería) o **Atajos de texto** (un bloque que se inserta escribiendo su nombre).
5. En **Colocar en**, elija cómo se inserta el bloque en un documento: **Tal cual, en el cursor**, **En un párrafo propio** o **En una página propia**.
6. Elija **Guardar**.

Si ya hay un bloque guardado con ese nombre, JeffWrite le pregunta si desea reemplazarlo.

## Insertar un bloque

1. Coloque el cursor donde debe ir el bloque. Si antes selecciona palabras, el bloque ocupará su lugar.
2. Elija **Insertar > Bloques guardados** y haga clic en el bloque. Escriba en el cuadro de búsqueda para encontrar un bloque por su nombre, categoría o descripción.

Insertar un bloque se puede deshacer en un solo paso. Si el bloque usa un estilo que el documento no tiene, este se agrega; un estilo que el documento ya tiene con el mismo nombre nunca se modifica. Un campo del bloque, como el número de página, se actualiza para el documento en el que está. El primer y el último párrafo de un bloque toman el aspecto de los párrafos a los que se unen, igual que al pegar.

## Atajos de texto

Un atajo de texto es un bloque con un nombre corto. Escriba el nombre y pulse **F3** (o elija **Insertar > Bloques guardados > Expandir acceso directo de texto**), y el nombre se sustituye por el bloque. El nombre debe empezar una palabra: «firma» funciona después de un espacio, pero no dentro de «diseñar».

## Administrar bloques guardados

Elija **Insertar > Bloques guardados > Administrar bloques guardados**. Puede ver cada bloque, cambiar su nombre, categoría, descripción o la forma en que se inserta con **Editar propiedades**, y **Eliminar** el bloque. **Deshacer eliminación** restaura lo que eliminó hasta que cierre el cuadro. **Exportar** guarda sus bloques en un fichero, y **Importar** agrega los bloques de un fichero de ese tipo; un bloque cuyo nombre ya tiene se deja como está.

## Información útil

- Sus bloques guardados se conservan en un solo fichero de este equipo, solo para su cuenta: no están en sus documentos y no se envían a ningún sitio. El fichero es `library.jblocks`, en la carpeta `Saved Blocks` dentro de su carpeta de datos: `AppData\Local\JeffOffice` en Windows, `Library/Application Support/JeffOffice` en un Mac y `.local/share/JeffOffice` en Linux. Todos los programas de JeffOffice de su equipo comparten este fichero, y dos ventanas pueden agregar bloques al mismo tiempo sin que se pierdan los de cada una.
- Un bloque puede ocupar hasta 8 megabytes, y la biblioteca admite hasta 5000 bloques.
- Una tabla se guarda como tabla cuando la selección incluye toda la tabla. Si la selección incluye solo una parte, esa parte se guarda como texto, con un párrafo por cada celda, y JeffWrite se lo indica antes de guardarla.
- Un marcador de un bloque recibe un nombre propio al insertar el bloque: si su documento ya tiene un marcador con ese nombre, el nuevo se llama con «_2», «_3», etc. detrás.
- Los comentarios no se incluyen. Las notas al pie y las notas al final tampoco. Los saltos de sección no se conservan. Las formas y los cuadros de texto sí se conservan y se insertan de nuevo con el bloque.
- Los cambios registrados se aceptan al guardar el bloque: las palabras insertadas se conservan, y las palabras eliminadas no están en el bloque, así que un bloque nunca contiene palabras registradas de otra persona.
- Un documento nunca agrega bloques a su biblioteca. Solo lo hace usted.
