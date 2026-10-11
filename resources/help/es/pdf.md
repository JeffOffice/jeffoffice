# Archivos PDF e imprentas
<!-- keywords: pdf, crear pdf, guardar como pdf, exportar pdf, pdf/x, pdf/a, archivar, archivado, imprenta, imprenta comercial, cmyk, cuatricromía, sangrado, marcas de recorte, perfil de color, color directo, sobreimpresión -->

Un **PDF** es un archivo que se ve igual en cualquier equipo. Úselo para enviar una publicación por correo electrónico, publicarla en Internet o enviarla a una imprenta.

## Crear un PDF

1. Haga clic en **Archivo > Exportar** y, a continuación, en **Crear PDF**.
2. En **Optimizar para**, elija una calidad. **ppp** significa puntos por pulgada. Más puntos producen imágenes más nítidas y archivos más grandes.
3. En **Páginas**, elija **Todas las páginas**, **Página actual** o **Páginas desde** una página **hasta** otra.
4. Haga clic en **Guardar PDF…**, escriba un nombre y haga clic en **Guardar**.

| Opción | Imágenes | Ideal para |
|---|---|---|
| Tamaño mínimo | 96 ppp | Ver en pantalla |
| Estándar | 150 ppp | Compartir en línea |
| Impresión de alta calidad | 300 ppp | Impresoras de escritorio |
| Imprenta comercial | Resolución completa | Una imprenta. Agrega marcas de recorte, marcas de sangrado, marcas de registro, barras de color e información del trabajo. |

Otras opciones:

- **Incluir propiedades del documento** incluye en el PDF el título, el autor, el asunto y las palabras clave.
- **Abrir el PDF después de guardarlo** muestra el PDF en el visor inmediatamente.
- **Hojas de folleto** aparece en los folletos. Coloca dos páginas en cada cara en el orden de plegado. Desactívela para obtener una página por cada página del PDF.

## PDF/A para conservar documentos

**PDF/A** es un PDF diseñado para durar años. Contiene todas las fuentes y usa colores estándar, sin efectos de transparencia. Active **PDF/A para archivado a largo plazo**, o haga clic en **Archivo > Exportar** y, a continuación, en **Crear PDF/A para archivado**.

## PDF/X para una imprenta

**PDF/X** es un PDF más estricto que piden las imprentas. Los colores se indican como cantidades de tinta, las fuentes van incluidas y el corte y el sangrado están marcados. El **sangrado** es la parte adicional de imagen o color que sobrepasa el borde de la página, para que al cortar no quede ningún borde blanco.

1. Haga clic en **Archivo > Exportar** y, a continuación, en **Crear PDF/X para una imprenta comercial**. O bien active **PDF/X para una imprenta comercial** en la ventana **Crear PDF**. No se puede usar junto con PDF/A.
2. En **Condición de impresión**, elija la que indique su imprenta.
3. Si la imprenta quiere marcas de recorte y sangrado, elija también **Imprenta comercial** en **Optimizar para**.
4. Haga clic en **Guardar PDF…**.

Las condiciones son:

- **PDF/X-1a** para offset rotativo de EE. UU. (SWOP) u offset europeo (FOGRA39). Acopla los efectos de transparencia.
- **PDF/X-4** para papel estucado de alta calidad (FOGRA51, PSO Coated v3), o con el perfil de color propio de su imprenta (un archivo .icc que usted elige). Conserva los efectos de transparencia.

El perfil de color PSO Coated v3 no se incluye con JeffPub. La primera vez que lo use, JeffPub le preguntará antes de descargar el perfil, una sola vez, de la European Color Initiative.

Antes de crear el archivo, JeffPub comprueba si hay imágenes de menos de 150 ppp (píxeles por pulgada) en su tamaño de impresión y fuentes que falten en este equipo. Haga clic en **Crear PDF de todos modos**, **Abrir comprobador de diseño** o **Cancelar**.

## Configurar los colores para una imprenta

Haga clic en **Archivo > Propiedades** y, a continuación, en la tarjeta **Configuración de impresión comercial**:

- **Modelo de color** puede ser **RGB**, **Un solo color (directo)**, **Colores directos**, **Colores de cuatricromía (CMYK)** o **Cuatricromía y colores directos**. Los colores de cuatricromía son las tintas cian, magenta, amarilla y negra. Un color directo es una tinta especial que mezcla la imprenta. Agregue cada uno en **Colores directos**.
- **Sobreimpresión del negro en archivos para imprenta (PDF/X)** hace que el negro se imprima sobre los colores que tiene debajo, para que una plancha desplazada no deje ningún borde blanco. Usted elige a qué se aplica: al texto inferior a un tamaño, a las líneas o a los rellenos.

## Temas relacionados

- [Imprimir una publicación](printing)
- [Guardar como imagen, página web o libro electrónico](export)
- [Comprobar el diseño](design-checker)
