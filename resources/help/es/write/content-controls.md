# Crear formularios con controles de contenido
<!-- keywords: control de contenido, controles de contenido, formulario, formularios, rellenar, texto con formato, control de imagen, cuadro combinado, sección repetible, agregar elemento, texto sin formato, lista desplegable, selector de fecha, calendario, casilla de verificación, casilla, título, etiqueta, marcador de posición, bloquear, no se puede eliminar, propiedades -->

Un **control de contenido** es un cuadro de su texto que una persona rellena: una línea de texto, párrafos con formato, una imagen, una opción de una lista, una fecha de un calendario, una casilla de verificación o una sección que puede repetirse. Utilícelos para crear un formulario que otros puedan completar sin cambiar el resto del documento.

## Insertar un control

1. Coloque el cursor donde debe ir el control.
2. Elija **Insertar > Control de contenido** y, a continuación, seleccione el tipo:
   - **Texto con formato:** un cuadro para palabras con formato. Si hay párrafos seleccionados, o si está en un párrafo vacío, el control contiene párrafos completos, incluidas listas y tablas, y la tecla Intro crea un nuevo párrafo dentro del control. Si se crea en un párrafo que tiene otras palabras, el control contiene las palabras de ese párrafo, y al pulsar Intro allí, el párrafo se divide en dos controles.
   - **Texto sin formato:** un cuadro para palabras. Muestra el marcador de posición «Escriba texto» hasta que alguien escriba. Si hay palabras seleccionadas, el control se crea alrededor de ellas.
   - **Imagen:** un lugar para una imagen. Muestra «Agregar una imagen» hasta que se inserte una.
   - **Cuadro combinado:** una lista de opciones que también admite lo que escriba. Muestra «Elija o escriba un elemento» hasta que se seleccione o escriba uno.
   - **Lista desplegable:** una lista de opciones. Muestra «Elija un elemento» hasta que se seleccione uno.
   - **Selector de fecha:** una fecha tomada de un calendario. Muestra «Elija una fecha» hasta que se seleccione una.
   - **Casilla de verificación:** una casilla pequeña que se marca o se desmarca.
   - **Sección repetible:** el párrafo donde está el cursor (o los párrafos seleccionados) pasa a ser el primer elemento de una sección que puede tener más elementos.

Un control recibe un sombreado claro para que pueda ver dónde empieza y dónde termina. Cuando el cursor está dentro de uno, aparece con un contorno, y su título figura en una pestaña situada encima. Insertar un control se puede deshacer en un solo paso.

## Rellenar un control

- **Texto con formato:** haga clic en el marcador de posición y escriba. Todo lo que puede hacer con el texto, como negrita, listas y tablas, también puede hacerlo dentro de un control que contiene párrafos.
- **Imagen:** haga clic en el control, o pulse Intro con el cursor dentro, y elija un archivo de imagen; también puede arrastrar un archivo de imagen sobre él. Una imagen que sustituye a otra toma el tamaño de la primera, manteniendo las proporciones. Un control de imagen empieza con un cuadrado de 5 cm de lado. **Quitar imagen** (en el menú Control de contenido) devuelve el marcador de posición.
- **Cuadro combinado:** haga clic en el marcador de posición y escriba, o haga doble clic en el control (o pulse Alt+Flecha abajo con el cursor dentro) y haga clic en un elemento.
- **Sección repetible:** haga clic en el botón **+** al final de la sección, o elija **Agregar elemento**, para añadir un elemento después del último. Cada elemento nuevo es una copia del último con todos sus controles vacíos de nuevo. **Quitar elemento** elimina el elemento donde está el cursor; una sección conserva al menos un elemento y admite como máximo 1000. Con el control de cambios activado, un elemento agregado es una inserción registrada y un elemento quitado es una eliminación registrada, para que un revisor pueda verlos, y Rechazar todo lo devuelve todo a su estado original. Con el control de cambios activado, no se puede agregar ni quitar un elemento de una sección que termina el documento: desactive el control de cambios o coloque un párrafo después de la sección.
- **Texto sin formato:** haga clic en el marcador de posición y escriba. Lo que escriba ocupa su lugar. Pulse Mayús+Intro para un salto de línea; la tecla Intro no crea un nuevo párrafo dentro de un control.
- **Lista desplegable:** haga clic en el control (o pulse Intro o Alt+Flecha abajo con el cursor dentro) y haga clic en un elemento. El elemento que eligió aparece marcado la próxima vez.
- **Selector de fecha:** haga clic en el control y, a continuación, haga clic en un día del calendario. **Hoy** inserta la fecha de hoy. **Borrar** devuelve el marcador de posición.
- **Casilla de verificación:** haga clic en ella, o pulse la barra espaciadora con el cursor dentro, para marcarla o desmarcarla.

Una lista desplegable, un selector de fecha, una casilla de verificación y un control de imagen solo contienen lo que se elige para ellos: no puede escribir en ellos.

## Cambiar las propiedades de un control

Coloque el cursor en el control y elija **Insertar > Control de contenido > Propiedades**. Puede establecer:

- **Título** y **Etiqueta:** el título aparece en la pestaña situada encima del control. La etiqueta es un nombre corto que otros programas pueden usar para encontrar el control.
- **Elementos de la lista** (lista desplegable y cuadro combinado): Agregar, cambiar el nombre (haga doble clic en un elemento), Quitar, Subir y Bajar. Una lista admite hasta 1000 elementos.
- **Formato de fecha** (selector de fecha): elija un formato o escriba el suyo, como MMMM d, yyyy. El cuadro muestra cómo se ve la fecha de hoy con ese formato.
- **El control no se puede eliminar:** el control permanece, aunque se puede cambiar lo que contiene.
- **El contenido no se puede editar:** el control puede eliminarse, pero no se puede cambiar lo que contiene. Esto incluye escribir, pegar, arrastrar, Buscar y reemplazar, un cambio ortográfico, un nuevo aspecto de las palabras, Copiar formato, los comandos de tabla, y aceptar o rechazar un cambio registrado dentro de él. Un cambio dentro del control que se registró antes del bloqueo se resolverá más adelante.
- **No se pueden agregar ni quitar elementos** (sección repetible): Agregar elemento y Quitar elemento no hacen nada y le indican el motivo. Coloque el cursor en cualquier elemento para ver este cuadro para toda la sección.

Elija **Aceptar** para aplicar los cambios como un único paso que se puede deshacer.

## Conviene saber

- Un control que contiene párrafos completos mantiene su texto separado de los párrafos que lo rodean. Si no se puede eliminar, o si su contenido no se puede editar, las teclas Retroceso y Supr en su borde no hacen nada, porque moverían palabras hacia dentro o hacia fuera del control. Si no tiene bloqueo, unir un párrafo al cruzar su borde deja el párrafo unido del lado donde está el cursor, y el control conserva sus demás párrafos; un control con un solo párrafo nunca se queda sin ninguno, así que el párrafo unido permanece en él.
- Un bloqueo de un control alrededor de filas o celdas completas de una tabla se conserva en el archivo, pero JeffWrite no lo mantiene. Lo mismo ocurre con un control que comienza con una tabla.
- Un documento protegido para rellenar formularios es de solo lectura en todo su contenido, así que no se puede rellenar ningún control en él, y la tecla Tab no se mueve entre controles.
- Un documento admite hasta 10 000 controles, y los controles pueden anidarse hasta 8 niveles. Una sección repetible y sus elementos cuentan como controles.
- Los controles de archivos de otros programas se muestran y se conservan tal como estaban. Un tipo que JeffWrite no puede crear (como una galería de bloques de creación, una ecuación o un grupo) se mantiene en su lugar; puede cambiar su título, su etiqueta y sus bloqueos, pero no su contenido.
- Los controles se guardan en archivos `.docx`. En otros tipos de archivo, las palabras se conservan pero el control no. Comprobar compatibilidad lo indica y nombra los tipos sin equivalente: secciones repetibles, controles de imagen y cuadros combinados.
