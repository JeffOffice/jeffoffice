# Lectores de pantalla, alto contraste y texto alternativo
<!-- keywords: accesibilidad, lector de pantalla, narrador, ciego, baja visión, alto contraste, modo oscuro, tema oscuro, contorno de enfoque, texto alternativo, descripción de imagen, zoom, ampliar, solo teclado -->

JeffPub está diseñado para que las personas que usan lectores de pantalla, el teclado o colores de alto contraste también puedan usarlo.

## Lectores de pantalla

Un **lector de pantalla** es un programa que lee en voz alta lo que aparece en la pantalla. JeffPub funciona con lectores de pantalla como Narrador en Windows. Cada control tiene un nombre y una descripción. Esto incluye las pestañas, los botones, las galerías y los botones de color de la cinta de opciones, la lista de páginas, la página, los paneles de tareas y la barra de estado.

Los objetos de la página también se leen. En la página, pulse **Tab** para seleccionar el siguiente objeto: el lector de pantalla indica de qué tipo es y qué contiene, por ejemplo "Cuadro de texto: Oferta de primavera" o "Imagen: un granero rojo", y después dónde está y qué tamaño tiene. Una imagen sin texto alternativo se lee como "Imagen, sin texto alternativo", para que sepa cuáles todavía lo necesitan. Pulse **Entrar** para escribir en un cuadro de texto; entonces el lector de pantalla puede leer su texto, la línea o la palabra donde está el cursor, y lo que seleccione.

- La lista de páginas lee cada página como "Página 1", "Página 2", y así sucesivamente. Si ha dado nombre a una página, el nombre aparece después, como en "Página 3, Portada".
- El área de la página se llama "Página". Su descripción le indica las teclas que puede usar: Tab selecciona el siguiente objeto, las teclas de flecha lo mueven, Entrar edita su texto y Suprimir lo elimina.
- Las pestañas se leen como pestañas de la cinta de opciones. Las pestañas que aparecen al seleccionar algo también indican su grupo, como Herramientas de imagen.

## Usar el teclado

Puede llegar a la cinta de opciones, a la lista de páginas y a los objetos de la página solo con el teclado. Pulse F6 para moverse entre la cinta de opciones, la lista de páginas y la página. Pulse Alt para ver las KeyTips. Un contorno de color muestra qué control tiene el foco. Consulte [Usar JeffPub con el teclado](keyboard).

## Cambiar los colores

- **Claro u oscuro**: haga clic en **Archivo > Opciones** y, en la ficha **General**, establezca **Tema** en **Claro**, **Oscuro** o **Usar la configuración del sistema**. El cambio se aplica de inmediato.
- **Alto contraste**: en Windows, cuando activa un tema de alto contraste, JeffPub lo sigue. Usa los colores que Windows haya elegido, y cambia al activar o desactivar el tema.

## Hacer las imágenes más fáciles de entender

El **texto alternativo** es una descripción breve de una imagen u otro objeto. Los lectores de pantalla lo leen en voz alta. JeffPub también lo incluye en las páginas web y los libros electrónicos que guarde.

1. Haga clic con el botón derecho en la imagen o el objeto.
2. Haga clic en **Formato de objeto…**.
3. Haga clic en la ficha **Texto alternativo**.
4. Escriba una descripción en el cuadro **Texto alternativo**.
5. Haga clic en **Aceptar**.

Diga en pocas palabras lo que muestra la imagen, por ejemplo "Dos niños plantando un árbol". Los iconos y los códigos de barras que inserte reciben texto alternativo por sí mismos.

## Comprobar la página

Haga clic en **Revisar > Comprobar accesibilidad** para ver una lista de lo que puede resultar difícil de usar para algunas personas. Además de las imágenes y formas sin texto alternativo, detecta tres cosas más.

- **Texto difícil de leer.** El texto necesita suficiente *contraste* con lo que hay detrás: el relleno de su cuadro o el color de página cuando el cuadro no tiene relleno. El comprobador calcula el contraste como una proporción. Una proporción de 4,5 a 1 basta para el texto normal, y 3 a 1 basta para el texto grande (18 puntos o más, o 14 puntos o más en negrita). Por ejemplo, el gris #777777 sobre blanco da 4,48 a 1, apenas por debajo, y #767676 da 4,54 a 1. Elija un color de texto más oscuro o un fondo más claro. Cuando el texto está sobre una imagen, el comprobador no puede determinarlo y añade un consejo para que revise el contraste a simple vista.
- **Objetos que se leen en otro orden.** Un lector de pantalla y un PDF etiquetado leen los objetos de una página desde el fondo de la pila hasta el frente. Si ese orden no es de arriba abajo y, después, de izquierda a derecha, el comprobador enumera como consejos los objetos que están fuera de lugar. Haga clic en **Orden de lectura** para ver los objetos en el orden en que se leen. Seleccione uno y haga clic en **Leer antes** o **Leer después**, o arrástrelo. Cada movimiento es un paso de Deshacer, y también cambia qué objeto queda delante.
- **Una tabla sin fila de encabezado.** Un lector de pantalla usa la primera fila de una tabla para nombrar sus columnas. Una tabla cuya primera fila no está marcada como fila de encabezado aparece en la lista como advertencia. Haga clic en el problema y, después, en **Usar la primera fila como encabezado** para activar la fila de encabezado. Es un paso de Deshacer.

## Hacer la página más grande

Acerque la vista para leer texto pequeño. Use los controles de **Zoom** de la barra de estado, mantenga pulsado Ctrl y gire la rueda del ratón, o elija un tamaño en el cuadro **Zoom** de la ficha **Vista**. El control deslizante de zoom va de aproximadamente el 10 % al 1600 %.

## Temas relacionados

- [Usar JeffPub con el teclado](keyboard)
- [Agregar y ajustar imágenes](pictures)
- [Cambiar la configuración de JeffPub](options)
- [Guardar como imagen, página web o libro electrónico](export)
