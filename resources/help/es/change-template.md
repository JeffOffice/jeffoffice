# Cambiar la plantilla de una publicación
<!-- keywords: cambiar plantilla, aplicar plantilla, otro diseño, nuevo diseño, cambiar diseño, cambiar estilo, contenido adicional, texto sobrante, imágenes sobrantes, marcador de posición, funciones, crear una publicación nueva, mover texto e imágenes -->

**Cambiar plantilla** da a la publicación abierta el aspecto de otro diseño y traslada a él el texto y las imágenes. No tiene que volver a escribir nada. Todo se hace en un solo paso que puede deshacer.

## Cambiar la plantilla

1. Haga clic en **Diseño de página > Plantilla > Cambiar plantilla**. La vista Archivo se abre con una galería de diseños titulada **Cambiar plantilla**.
2. Elija un diseño igual que para una publicación nueva: seleccione un grupo a la izquierda o escriba en el cuadro **Buscar plantillas** y, a continuación, haga clic en un diseño. Consulte [Crear una publicación nueva](create).
3. Si lo desea, use el panel de la derecha. **Combinación de colores** y **Combinación de fuentes** empiezan en **(mantener actual)**, que conserva los colores y las fuentes que tiene. Elija ahí una combinación solo si quiere que el nuevo diseño los cambie. **Información empresarial** selecciona el conjunto de datos de contacto que rellena el diseño. **Incluir logotipo** e **Incluir dirección postal** solo aparecen en los diseños que los usan.
4. Haga clic en **Cambiar plantilla**. También puede hacer doble clic en el diseño.
5. Elija dónde se aplica el nuevo diseño y haga clic en **Aceptar**:
   - **Aplicar la plantilla a la publicación actual** cambia la publicación abierta.
   - **Crear una publicación nueva con mi texto y mis gráficos** abre una segunda ventana con el resultado. La publicación de partida no se modifica.

La galería solo muestra diseños. Para cambiar el tamaño del papel de una publicación sin cambiar su diseño, use [Configurar página](page-setup).

## Adónde van el texto y las imágenes

Un **artículo** es el texto de un cuadro de texto o de una cadena de cuadros de texto vinculados. Cada cuadro de texto de un diseño tiene una **función**, que indica para qué sirve el cuadro. Las funciones son:

- Título (el titular) y subtítulo
- Encabezado, para texto grande en el interior de la publicación
- Texto del cuerpo
- Fecha, que también admite un lugar o una hora
- Dirección y datos de contacto: la dirección postal, el teléfono, el correo electrónico y la dirección web
- Organización, que es el nombre y el lema de su organización
- Persona, que es el nombre y el cargo de una persona
- Título de imagen y cita destacada
- Dirección de envío, el bloque que rellena la combinación de correspondencia
- Texto breve, para etiquetas pequeñas

Los diseños que incluye JeffPub ya tienen asignadas estas funciones. Para un cuadro que haya dibujado usted o que proceda de un archivo .pub, JeffPub deduce la función a partir de su texto (una fecha, un número de teléfono), su estilo (Título o Cita) y su tamaño.

Cada artículo va al cuadro del nuevo diseño que tiene la misma función. Por ejemplo, el artículo que escribió en el cuerpo de un volante va al cuadro de cuerpo de un boletín, y el nombre de su organización va a la cabecera. Si el nuevo diseño tiene varios cuadros con la misma función, los artículos los rellenan en orden de lectura: página a página, de arriba abajo y de izquierda a derecha.

El artículo conserva sus palabras, así como la negrita, la cursiva, los vínculos y los campos. Adopta la tipografía, el color y el espaciado del nuevo diseño, de modo que se integra con el resto. El titular de un diseño que usa texto artístico toma el primer párrafo del artículo de título. Los párrafos siguientes van a Contenido adicional.

Las imágenes van a los **marcadores de posición de imagen** del diseño, que son las imágenes provisionales que muestra un diseño hasta que agrega las suyas. La primera imagen ocupa el primer marcador de posición en orden de lectura, la siguiente ocupa el siguiente, y así sucesivamente. Cada imagen se recorta para rellenar su marco y conserva su brillo, su contraste y su color modificado. Los marcadores de posición para los que no tiene imagen conservan la imagen provisional del diseño.

Algunas cosas se quedan como están:

- El texto que nunca sobrescribió es el texto de ejemplo del diseño anterior. No se traslada, y el nuevo diseño muestra ahí su propio texto de ejemplo.
- El tamaño de página, los márgenes, las páginas y las páginas maestras proceden del nuevo diseño. Si una publicación tiene más páginas que el diseño, se conservan las páginas del diseño y el resto del contenido va a Contenido adicional. JeffPub no agrega páginas para él.
- Las tablas, formas, líneas, objetos de texto artístico y grupos de objetos que creó el diseño anterior y que usted no modificó se quedan con él. Todo lo que dibujó y todo lo que modificó (una tabla con una celda en la que escribió, una forma cuyo texto editó, una línea que movió) va completo a Contenido adicional.
- La información empresarial, las propiedades del documento, los estilos, la lista de combinación de correspondencia y los objetos del área de trabajo se quedan con la publicación.

## Contenido adicional

No se pierde nada de lo que creó. El contenido que no tiene lugar en el nuevo diseño espera en el panel **Contenido adicional**, que se abre junto a la página al terminar el cambio. En él aparece cada artículo, imagen, tabla, forma, línea y grupo sobrante.

- Haga clic en un elemento y luego en **Colocar en la página** para ponerlo en la página que está viendo. Un artículo se convierte en un cuadro de texto ajustado a su texto, una tabla o un grupo vuelve completo, y una imagen u objeto demasiado grande se reduce para que quepa en la página. También puede hacer doble clic en un elemento o arrastrarlo a la página.
- Haga clic en un elemento y luego en **Descartar** para quitarlo definitivamente. (Aún puede deshacerlo).

El panel permanece con la publicación hasta que la cierra, y se guarda en el archivo de la publicación. (Un archivo .pub no tiene lugar para él, así que al guardar como .pub no se incluye). Para volver a abrirlo, haga clic en **Diseño de página > Plantilla > Contenido adicional**. El botón aparece atenuado cuando no queda nada.

## Deshacer

**Deshacer** (Ctrl+Z) restaura el diseño anterior y vuelve a sacar los elementos sobrantes de Contenido adicional. Colocar o descartar un elemento es un paso independiente.

## Temas relacionados

- [Crear una publicación nueva](create)
- [Combinaciones de colores y de fuentes](color-schemes)
- [Tamaño de página, márgenes y diseño](page-setup)
- [La pestaña Diseño de página](tab-page-design)
