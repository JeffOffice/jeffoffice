# Comprobar el diseño
<!-- keywords: comprobador de diseño, comprobar, revisar, errores, problemas, baja resolución, desbordamiento, texto desbordado, el texto no cabe, cuadro de texto vacío, fuera de la página, comprobación previa, preimpresión, comprobación final, prueba, imagen vinculada, fuente que falta, fuente no instalada -->

El **Comprobador de diseño** revisa la publicación en busca de errores frecuentes. Detecta texto que no cabe, cuadros vacíos, objetos que sobresalen de la página e imágenes que se imprimirán mal. Ejecútelo antes de imprimir o de enviar un archivo a una imprenta.

## Ejecutar el Comprobador de diseño

Haga clic en **Revisar > Comprobar > Ejecutar el comprobador de diseño**. El panel **Comprobador de diseño** se abre a la derecha. También puede hacer clic en **Archivo > Propiedades** y, después, en **Ejecutar el comprobador de diseño**.

El panel tiene dos casillas, ambas activadas al principio:

- **Ejecutar comprobaciones generales de diseño** busca problemas de diseño.
- **Ejecutar comprobaciones finales de publicación** busca aspectos importantes a la hora de imprimir.

Deje las dos activadas para ver todos los tipos de problemas. La lista se actualiza mientras trabaja. Si no hay nada que corregir, indica "No se encontraron problemas."

## Corregir un problema

1. Haga clic en un elemento de la lista bajo **Seleccione un elemento para corregirlo**.
2. Haga clic en **Ir al elemento**. O haga doble clic en el elemento.

JeffPub va a la página y selecciona el objeto. Corríjalo y el elemento desaparecerá de la lista.

## Qué detecta

| Mensaje | Qué significa | Qué hacer |
|---|---|---|
| La página no tiene contenido | La página está vacía. | Agregue algo o elimine la página. |
| El objeto está parcialmente fuera de la página | Parte de un objeto queda fuera de la página. | Muévalo, o déjelo si es un sangrado. |
| El cuadro de texto está vacío | Un cuadro de texto no contiene texto. | Escriba en él o elimínelo. |
| Artículo con texto desbordado | Un artículo es el texto que recorre varios cuadros vinculados. Este tiene más texto del que cabe. | Amplíe un cuadro, recorte texto o [vincule otro cuadro](linked-text). |
| El texto está muy reducido para ajustarse | El autoajuste, que reduce el texto para que quepa en su cuadro, lo ha dejado a menos de la mitad de su tamaño. | Amplíe el cuadro o use menos texto. |
| El marcador de posición de imagen está vacío | Un marco de imagen no tiene imagen. | Haga clic en él para agregar una, o elimínelo. |
| La imagen no tiene una escala proporcional | La imagen está estirada o comprimida. | Cambie su tamaño para recuperar su forma. |
| La imagen está vinculada, no incrustada | La imagen está ligada a un archivo y en la publicación solo se guarda una vista previa pequeña. Una imagen insertada con **Insertar y vincular** se guarda completa y no aparece en la lista. | Conserve el archivo original junto con la publicación, o haga clic en **Incrustar imagen** en el [Administrador de gráficos](graphics-manager). |
| Falta la imagen vinculada | El archivo al que apunta una imagen vinculada ya no está, así que la página solo muestra su vista previa pequeña, y eso es lo que se imprimiría. | En el Administrador de gráficos, haga clic en **Cambiar vínculo…** para buscar el archivo, o vuelva a colocarlo y haga clic en **Actualizar vínculo**. |
| La imagen vinculada solo muestra su vista previa hasta Actualizar vínculo | La imagen está vinculada a un archivo situado fuera de la carpeta de la publicación, que JeffPub no abre por sí solo. La página muestra la vista previa pequeña, y eso es lo que se imprimiría. | En el Administrador de gráficos, seleccione la imagen y haga clic en **Actualizar vínculo**. |
| La imagen tiene baja resolución | La imagen tiene menos de 150 ppp (píxeles por pulgada) a su tamaño de impresión. | Use una imagen más grande o imprímala más pequeña. |
| El objeto tiene transparencia o efectos (puede imprimirse de forma diferente) | Las zonas transparentes y los efectos pueden verse distintos en papel. | Imprima una página de prueba. |
| La fuente "nombre" no está instalada | Una fuente del texto no está en este equipo, así que se usa otra en su lugar. | Instale la fuente o elija otra. |

Cada mensaje termina con el número de página, como "(Página 2)".

## Antes de crear un archivo para la imprenta

Al crear un archivo PDF/X, JeffPub le avisa de las imágenes de baja resolución y de las fuentes que faltan, y le ofrece abrir el Comprobador de diseño. Consulte [Archivos PDF e imprentas](pdf).

## Temas relacionados

- [Archivos PDF e imprentas](pdf)
- [Imprimir una publicación](printing)
- [Hacer fluir el texto entre cuadros de texto](linked-text)
- [Agregar y ajustar imágenes](pictures)
- [Administrador de gráficos](graphics-manager)
