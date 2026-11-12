# PDF y imprentas

<!-- keywords: pdf, crear pdf, guardar como pdf, exportar pdf, pdf/x, pdf/a, archivar, archivo, imprenta, impresión comercial, cmyk, sangrado, marcas de recorte, perfil de color, color directo, sobreimpresión, contraseña, cifrar, proteger con contraseña -->

Un **PDF** es un archivo que se ve igual en cualquier equipo. Use uno para enviar una publicación por correo electrónico, publicarla en internet o enviarla a una imprenta.

## Crear un PDF

1. Haga clic en **Archivo > Exportar** y, después, en **Crear PDF**.
2. En **Optimizar para**, elija una calidad. **ppp** significa puntos por pulgada. Cuantos más puntos, más nítidas serán las imágenes y mayor será el archivo.
3. En **Páginas**, elija **Todas las páginas**, **Página actual** o **Páginas desde** una página **hasta** otra.
4. Haga clic en **Guardar PDF…**, escriba un nombre y haga clic en **Guardar**.

| Opción | Imágenes | Ideal para |
|---|---|---|
| Tamaño mínimo | 96 ppp | Ver en pantalla |
| Estándar | 150 ppp | Compartir en internet |
| Impresión de alta calidad | 300 ppp | Impresoras de escritorio |
| Imprenta comercial | Resolución completa | Una imprenta. Añade marcas de recorte, marcas de sangrado, marcas de registro, barras de color e información del trabajo. |

Otras opciones:

- **Incluir propiedades del documento** pone el título, el autor, el asunto y las palabras clave en el PDF.
- **Etiquetas de estructura del documento para la accesibilidad** (activada de forma predeterminada) crea un PDF *etiquetado*. Las etiquetas indican a un lector de pantalla qué es un encabezado, un párrafo, una imagen o una tabla, y el orden en que debe leerlos: el orden de los objetos de cada página, que puede ver y cambiar en la lista **Orden de lectura** del Comprobador de accesibilidad. Una imagen se lee con su texto alternativo. Un archivo PDF/X no tiene etiquetas.
- **Crear marcadores** (activada de forma predeterminada) añade un marcador en el PDF por cada página. Es el nombre de la página, o "Página 3" si la página no tiene nombre. Un lector de PDF los muestra junto a las páginas.
- **Abrir los PDF después de guardarlos** muestra el PDF en su visor de inmediato.
- **Hojas de folleto** aparece en los folletos. Pone dos páginas en cada cara en orden de plegado. Desmárquela para tener una página por página del PDF.

## PDF/A para conservar registros

**PDF/A** es un PDF pensado para conservarse durante años. Incluye todas las fuentes y usa colores estándar, sin efectos de transparencia. Marque **PDF/A para archivado a largo plazo** o haga clic en **Archivo > Exportar** y, después, en **Crear PDF/A para archivado**.

## PDF/X para una imprenta

**PDF/X** es un PDF más estricto que piden las imprentas. Los colores se indican como cantidades de tinta, las fuentes van incluidas y el corte y el sangrado están marcados. El **sangrado** es imagen o color adicional que sobrepasa el borde de la página, de modo que al recortar no quede ningún borde blanco.

1. Haga clic en **Archivo > Exportar** y, después, en **Crear PDF/X para una imprenta comercial**. O marque **PDF/X para una imprenta comercial** en la ventana **Crear PDF**. No se puede usar junto con PDF/A.
2. En **Condición de impresión**, elija la que indique su imprenta.
3. Si la imprenta pide marcas de recorte y sangrado, elija también **Imprenta comercial** en **Optimizar para**.
4. Haga clic en **Guardar PDF…**.

Las condiciones son:

- **PDF/X-1a** para offset web de EE. UU. (SWOP) u offset europeo (FOGRA39). Aplana los efectos de transparencia.
- **PDF/X-4** para papel estucado de alta calidad (FOGRA51, PSO Coated v3) o con el perfil de color de su impresora (un archivo .icc que usted elige). Conserva los efectos de transparencia.

El perfil de color PSO Coated v3 no viene con JeffPub. La primera vez que lo use, JeffPub le pedirá permiso antes de descargar el perfil, una sola vez, de la European Color Initiative.

Antes de crear el archivo, JeffPub comprueba si hay imágenes por debajo de 150 ppi (píxeles por pulgada) en su tamaño impreso y si faltan fuentes en este equipo. Haga clic en **Crear PDF de todos modos**, **Abrir comprobador de diseño** o **Cancelar**.

## Proteger un PDF con contraseña

En la ventana **Crear PDF**, haga clic en **Cifrar con contraseña…**. Escriba la contraseña dos veces. Quien abra el PDF tendrá que escribirla. La contraseña protege de verdad el archivo: las páginas se cifran con un cifrado robusto (AES-256), y nadie puede leerlas sin ella.

También puede marcar **Restringir la impresión, la copia y la edición**, escribir una segunda contraseña y elegir qué se permite. Esas restricciones son solo una petición. La mayoría de los lectores de PDF las respetan, pero no impiden a quien quiera saltárselas, así que no confíe en ellas para mantener un archivo privado.

Una contraseña olvidada no se puede recuperar, así que guárdela en un lugar seguro. La contraseña se usa solo para ese PDF y nunca se guarda. Un PDF con contraseña no puede ser PDF/A ni PDF/X, por lo que esas dos opciones se desactivan mientras haya una contraseña. En **Archivo > Compartir**, **Cifrar con contraseña…** encima de **Enviar como PDF** pone una contraseña al PDF que se envía por correo electrónico.

Al colocar un PDF con contraseña como imagen, JeffPub le pide la contraseña. Si es incorrecta, JeffPub vuelve a pedirla; haga clic en **Cancelar** para detenerse. La página de su publicación es una copia y no necesita contraseña. Si el propietario del PDF ha pedido que no se copie, JeffPub lo coloca igualmente y muestra una nota al respecto; esa petición no es un bloqueo.

Una imagen vinculada a un PDF con contraseña aparece como **No actualizada** al abrir la publicación, y JeffPub nunca pide la contraseña por su cuenta. Haga clic en **Actualizar vínculo** para que se la pida.

## Configurar los colores para una imprenta

Haga clic en **Archivo > Información** y, después, en la tarjeta **Opciones de impresión comercial**:

- **Modelo de color** es **RGB**, **Un solo color (directo)**, **Colores directos**, **Colores de cuatricromía (CMYK)** o **Cuatricromía y colores directos**. Los colores de cuatricromía son las tintas cian, magenta, amarillo y negro. Un color directo es una tinta especial que mezcla la imprenta. Añada cada uno en **Colores directos**.
- **Sobreimpresión del negro en archivos para imprenta (PDF/X)** hace que el negro se imprima sobre los colores que tiene debajo, de modo que, si una plancha se desplaza, no quede ningún borde blanco. Usted elige lo que cubre: texto por debajo de un tamaño, líneas o rellenos.

## Temas relacionados

- [Imprimir una publicación](printing)
- [Guardar como imagen, página web o libro electrónico](export)
- [Comprobar el diseño](design-checker)
