# Crear diapositivas a partir de un esquema
<!-- keywords: diapositivas desde esquema, importar esquema, markdown, esquema de texto, rtf, docx, títulos a diapositivas, lista con viñetas a diapositivas, empezar presentación desde texto, guardar esquema, exportar esquema -->

Puede escribir una presentación como texto sin formato y dejar que JeffDeck la convierta en diapositivas. Cada diapositiva tiene un título y un cuerpo, con el diseño Título y objetos, así que la vista Esquema, Restablecer diapositiva y los temas funcionan sobre ella como de costumbre.

## Agregar las diapositivas a la presentación abierta

1. Haga clic en **Inicio > Diapositivas**, después en la pequeña flecha situada debajo de **Nueva diapositiva**, y elija **Diapositivas desde esquema…**.
2. Elija un archivo. Puede ser un archivo de texto (.txt), un archivo Markdown (.md), un archivo de texto enriquecido (.rtf) o un archivo de documento (.docx).
3. Las diapositivas se agregan después de la diapositiva en la que se encuentra. Se agregan en un solo paso, así que una sola acción de Ctrl+Z las quita todas.

## O empezar una presentación nueva a partir de uno

Elija **Archivo > Abrir** y seleccione el archivo de esquema. JeffDeck crea una presentación nueva a partir de él. El archivo no se modifica nunca, y **Guardar** le pedirá un nombre.

## Cómo se lee el archivo

**Archivos de texto.** Una línea que empieza en el margen izquierdo es el título de una diapositiva. Las líneas situadas debajo son el texto de la diapositiva. Una línea con más sangría que la anterior pasa a un nivel más de profundidad (use tabulaciones o espacios). Una viñeta (-, * o +) o un número al principio de una línea se omite, porque la diapositiva tiene sus propias viñetas.

**Archivos Markdown.** El nivel de encabezado superior que use el archivo crea las diapositivas: # si el archivo lo tiene, y ## en caso contrario. Los encabezados de nivel inferior se convierten en líneas de texto de la diapositiva, con un nivel más de profundidad por cada paso. Una línea de tres guiones (---) inicia una diapositiva nueva. Las listas se convierten en niveles. **Negrita**, *cursiva*, `código` y los vínculos conservan su aspecto. Un bloque de código entre líneas de tres comillas inversas (```) se convierte en un cuadro de texto con fuente de máquina de escribir. Una imagen en una línea propia, escrita con la forma de imagen de Markdown (un signo de exclamación, la descripción entre corchetes y el nombre del archivo entre paréntesis), se coloca en la diapositiva, pero solo si el archivo de imagen está en la misma carpeta que el esquema o en una carpeta contenida en ella. Las imágenes de internet o de otras carpetas nunca se cargan.

**Archivos de texto enriquecido y .docx.** Los párrafos con estilo de encabezado crean las diapositivas: el estilo de encabezado superior que use el archivo es el título, los encabezados de nivel inferior son líneas del texto, y cualquier otro párrafo es una línea debajo del encabezado anterior.

## Límites

Estos límites se aplican a todos los tipos de archivo de esquema: texto, Markdown, texto enriquecido y .docx:

- Hasta 5.000 diapositivas. Lo que haya después se omite.
- Hasta 4000 caracteres por línea. Una línea más larga se corta, y lo mismo ocurre con un párrafo de Markdown cuyas líneas juntas superen ese tamaño.
- Hasta 500 líneas de texto en una diapositiva, y 100.000 líneas en total.
- El texto de una diapositiva tiene hasta 9 niveles, y una línea con más sangría se muestra en el nivel más profundo.
- Hasta 200 líneas de código en una diapositiva.
- Hasta 100 imágenes en total y 4 por diapositiva. Un archivo de imagen puede tener hasta 32 MB, y todos los archivos de imagen juntos hasta 64 MB. Una imagen tiene que ser un archivo PNG, JPEG, GIF, BMP, WebP o TIFF, sea cual sea su nombre, con hasta 16 millones de píxeles (4000 × 4000), y todas las imágenes juntas hasta 64 millones.

El tamaño del propio archivo también está limitado. Un archivo de texto o Markdown puede tener hasta 10 MB, en UTF-8 o UTF-16. Un archivo de texto enriquecido o .docx puede tener hasta 96 MB. Un archivo que no es de texto se rechaza. Las imágenes de un archivo de texto enriquecido o .docx no se colocan en las diapositivas, y JeffDeck le indica cuántas se han omitido.

Cuando se omite algo, como una imagen que no se pudo usar o texto que supera un límite, JeffDeck le indica qué ha sido. Mientras lee un archivo grande, JeffDeck sigue funcionando, y cerrar la ventana detiene la lectura.

## Guardar una copia como esquema

Elija **Archivo > Guardar una copia** y seleccione **Esquema de Markdown (*.md)**. El archivo contiene el título y el texto de cada diapositiva, con negrita, cursiva y vínculos. No incluye imágenes, formas, tablas, gráficos, otros cuadros de texto, notas del orador ni el aspecto de las diapositivas. JeffDeck se lo indica al guardar, y **Archivo > Información > Comprobar compatibilidad** también lo muestra.

## Temas relacionados

- [Agregar, cambiar y eliminar diapositivas](slides)
- [Reutilizar diapositivas de otra presentación](reuse)
- [Vistas](views)
