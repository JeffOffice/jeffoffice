# Transiciones entre diapositivas
<!-- keywords: transición, transiciones, transformación, desvanecer, empujar, barrido, dividir, revelar, cubrir, descubrir, zoom, corte, opciones de efectos, objetos, palabras, caracteres, coincidir, emparejar, nombre, dirección, duración, avance de diapositivas, al hacer clic con el mouse, después de, automático, intervalos, aplicar a todo, vista previa, estrella -->

Una **transición** es el efecto que se reproduce cuando la presentación con diapositivas pasa de una diapositiva a la siguiente. La transición pertenece a la diapositiva a la que se llega. La presentación la reproduce al avanzar hasta esa diapositiva.

## Agregar una transición

1. Haga clic en la diapositiva en el panel de diapositivas.
2. Haga clic en la pestaña **Transiciones**. En la galería **Transición a esta diapositiva**, haga clic en un efecto: **Ninguna**, **Transformación**, **Desvanecer**, **Empujar**, **Barrido**, **Dividir**, **Revelar**, **Cubrir**, **Descubrir**, **Zoom** o **Cortar**. La miniatura de la diapositiva en el panel de diapositivas muestra una pequeña estrella para indicar que tiene una transición.
3. Haga clic en **Transiciones > Vista previa > Vista previa** para verla reproducida al entrar en la diapositiva, sobre el área de la diapositiva. Presione Esc para detenerla.

Elija **Ninguna** para quitar una transición.

## Elegir cómo se reproduce

Haga clic en **Opciones de efectos** para ver las opciones del efecto:

- **Empujar**, **Barrido**, **Revelar**, **Cubrir** y **Descubrir** tienen una dirección: **Desde la derecha**, **Desde la izquierda**, **Desde abajo** o **Desde arriba**.
- **Dividir** y **Zoom** van **Hacia fuera** o **Hacia dentro**.
- **Transformación** mueve **Objetos**, **Palabras** o **Caracteres**. Consulte más abajo.
- **Desvanecer** y **Cortar** no tienen opciones. Un corte pasa por negro.

## Transformación

**Transformación** mueve lo que dos diapositivas tienen en común. Un **objeto** es un elemento de una diapositiva: una forma, una imagen, un cuadro de texto, una línea o una tabla. La presentación pasa de una diapositiva a la siguiente. Cada objeto que está en ambas diapositivas se desliza desde donde lo tiene la primera diapositiva hasta donde lo tiene la segunda. Por el camino aumenta o disminuye de tamaño, gira y cambia de color. Un objeto que solo está en la primera diapositiva desaparece gradualmente. Un objeto que solo está en la segunda aparece gradualmente.

Para crear una transformación:

1. Haga clic con el botón derecho en una diapositiva del panel de diapositivas y elija **Duplicar diapositiva**. La copia tiene los mismos objetos.
2. En la copia, mueva un objeto, agrándelo, gírelo o cambie su color. También puede cambiar su texto, o agregar y eliminar objetos.
3. Haga clic en la copia y, después, en **Transiciones > Transición a esta diapositiva > Transformación**. Una transformación dura 2 segundos. Puede cambiarlo con **Duración**.
4. Haga clic en **Vista previa** para verla, o inicie la presentación con diapositivas.

### Qué objetos van juntos

JeffDeck empareja un objeto de la primera diapositiva con un objeto de la segunda. Lo intenta de estas formas, en este orden. Cada objeto forma parte de una pareja como máximo.

1. **Un nombre que empieza por !!.** Dos objetos con el mismo nombre, si el nombre empieza por dos signos de exclamación, siempre forman pareja. Pueden ser dos objetos cualesquiera, incluso un círculo y una estrella.
2. **El mismo nombre.** Dos objetos del mismo tipo con el mismo nombre forman pareja. Un cuadro de texto y una forma son tipos distintos.
3. **El mismo texto.** Dos cuadros de texto con las mismas palabras forman pareja, sean cuales sean sus nombres. También dos formas con las mismas palabras.
4. **La misma imagen.** Dos imágenes de la misma imagen forman pareja, aunque tengan otro nombre u otro tamaño.
5. **El mismo tipo de forma.** Dos círculos forman pareja. También dos líneas o dos tablas.

Cuando varios objetos pueden formar pareja, se emparejan primero los que están más cerca entre sí.

Los nombres de los objetos proceden del archivo de presentación. Una presentación creada en otro programa conserva los nombres que este dio a sus objetos, y JeffDeck los usa.

### Cómo se mueve una pareja

- Dos objetos con la misma forma son un único objeto que se mueve. Su posición, tamaño, giro, color de relleno, color de línea y grado de transparencia cambian poco a poco. El recorte de una imagen también cambia.
- Dos objetos que no tienen la misma forma, como un círculo y un cuadrado, se mueven a las mismas posiciones mientras uno se funde en el otro.
- Si el texto de una pareja no es el mismo, el texto antiguo se funde en el nuevo.
- Los objetos que oculta una animación se omiten. Un objeto que una animación ha movido se toma desde donde terminó.

### Qué se mueve: Objetos, Palabras o Caracteres

Haga clic en **Opciones de efectos** para elegir:

- **Objetos.** El texto de una pareja permanece en su objeto. El texto antiguo se funde en el nuevo.
- **Palabras.** Una palabra que tienen ambos textos se mueve de su posición antigua a la nueva. Las demás palabras desaparecen y aparecen gradualmente. Un signo como la coma cuenta como una palabra independiente.
- **Caracteres.** Lo mismo, pero letra a letra o cifra a cifra.

**Palabras** y **Caracteres** funcionan en cuadros de texto y formas. Un texto de más de 16 000 caracteres, el texto vertical y el texto de una forma volteada se reproducen como **Objetos**. También una pareja que no tiene ninguna palabra ni carácter en común.

Transformación también es una transición en otros programas de presentaciones (las versiones de 2019 y posteriores). JeffDeck la lee y la guarda como ellos. Los programas más antiguos reproducen una transformación como un desvanecimiento.

## Establecer los intervalos

El grupo **Intervalos** establece cuánto dura la transición y cómo sale la presentación de la diapositiva.

- **Duración** es el número de segundos que tarda el efecto. Una transición nueva dura 0,7 segundos. Puede escribir de 0,01 a 59.
- **Al hacer clic con el mouse** está activado de forma predeterminada. La presentación avanza desde esta diapositiva cuando el orador hace clic. Desactívelo para avanzar con una tecla, como Espacio o Flecha derecha. Las animaciones de la diapositiva se siguen reproduciendo al hacer clic.
- **Después de** hace que la presentación avance sola tras el número de segundos que escriba. Puede usar **Después de** junto con **Al hacer clic con el mouse**: la presentación avanza con lo que ocurra primero.

**Aplicar a todo** asigna a todas las diapositivas la transición y los intervalos de la diapositiva en la que se encuentra.

Cada una de estas acciones es un paso que Ctrl+Z deshace.

## Transiciones de otros archivos

Una presentación creada en otro programa puede tener una transición para la que JeffDeck no tiene efecto. Se reproduce como un desvanecimiento y se guarda tal como estaba, por lo que sigue funcionando en el programa de origen.

## Temas relacionados

- [Pestaña Transiciones](tab-transitions)
- [Animaciones](animations)
- [Reproducir la presentación con diapositivas y usar la Vista Moderador](show)
