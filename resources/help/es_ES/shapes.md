# Dibujar formas y líneas
<!-- keywords: forma, línea, flecha, rectángulo, círculo, óvalo, conector, curva, forma libre, relleno, contorno, sombra, 3D, modificar puntos, diagrama de flujo, figura, dibujar, trama, vincular -->

Las formas y las líneas son objetos dibujados. Puede rellenarlos con color, darles contorno y escribir texto dentro de una forma.

## Dibujar una forma

1. Haga clic en **Insertar > Ilustraciones > Formas**. (También en **Inicio > Objetos > Formas**.)
2. Elija una forma. Los grupos son Líneas, Rectángulos, Formas básicas, Flechas de bloque, Formas de ecuación, Diagrama de flujo, Estrellas y cintas, Llamadas y Botones de acción.
3. Arrastre sobre la página.

Mientras arrastra, mantenga pulsada **Mayús** para obtener una forma con proporciones iguales: un óvalo se convierte en un círculo, y un rectángulo, en un cuadrado. Mantenga pulsada **Ctrl** para dibujar desde el centro hacia fuera. Si hace clic sin arrastrar, obtendrá una forma de 2,54 cm de ancho. Después de dibujar una forma, el puntero vuelve a ser normal. Pulse **Esc** para cancelar antes de dibujar.

## Dibujar líneas y flechas

Elija **Línea**, **Flecha** o **Flecha doble** en el grupo Líneas de la galería, o en **Formato de forma > Líneas**. Arrastre sobre la página. Mantenga pulsada **Mayús** para que la línea quede horizontal, vertical o con un ángulo de 45 grados. Para cambiar una línea más adelante, arrastre el controlador de cualquiera de sus extremos.

Hay tres herramientas más para dibujar a mano alzada:

- **Curva**: haga clic en varios puntos. La línea pasa suavemente por ellos.
- **Forma libre: forma**: haga clic en puntos para obtener lados rectos, o mantenga pulsado el botón y arrastre para dibujar a mano alzada.
- **Forma libre: garabato**: mantenga pulsado el botón y dibuje un solo trazo.

Para terminar una curva o una forma libre, haga doble clic o pulse **Entrar**. Haga clic en el primer punto para cerrarla y convertirla en una forma rellena.

## Unir objetos con conectores

Una línea puede quedar pegada a otros objetos. **Conector: angular** y **Conector: curvado** dibujan conectores con codos y con curvas. Al acercarse a una forma, un cuadro de texto, una imagen o una tabla, aparecen sus puntos de unión. El extremo se ajusta al centro de un lado. Cuando mueve, cambia de tamaño o gira el objeto, la línea lo sigue. Arrastre el controlador amarillo para mover el codo de un conector angular. Arrastre una línea lejos para separarla del objeto.

## Cambiar el relleno, el contorno y los efectos

Seleccione la forma y abra la ficha **Formato de forma**.

- **Estilos de forma**: la galería ofrece aspectos rápidos.
- **Relleno de forma**: elija un color, **Sin relleno**, **Relleno de imagen…** o **Efectos de relleno…** para un degradado, una textura o una trama.
- **Contorno de forma**: elija un color. La flecha situada junto a él define el grosor, los guiones y las puntas de flecha.
- **Efectos de forma**: **Sombra**, **Reflexión**, **Iluminado**, **Bordes suaves**, **Bisel** y **Giro 3D**. **Opciones de sombra…** define el color, el desenfoque, la distancia y el ángulo de la sombra. **Giro 3D** inclina la forma en perspectiva.

Para ver todas las opciones a la vez, haga clic en el pequeño botón de la esquina del grupo **Estilos de forma** para abrir la ventana **Formato de forma**.

## Añadir texto a una forma

Haga doble clic en la forma y escriba. También puede seleccionarla y pulsar **Entrar**, o hacer clic con el botón derecho y elegir **Agregar texto**.

## Cambiar la forma de una figura

- **Cambiar forma** (en **Insertar formas**) sustituye la forma por otra.
- Un controlador amarillo en forma de rombo, en las formas que lo tienen, ajusta la forma del objeto. En un rectángulo redondeado, define el redondeo de las esquinas.
- **Modificar puntos** le permite mover las esquinas. Los cuadrados blancos marcan cada punto. Arrastre un punto para moverlo. Haga **Ctrl**+clic en un punto para eliminarlo, o en un borde para añadir uno. Pulse **Esc** cuando termine.

## Puntos de conexión para conectores

Un conector se fija a los puntos de conexión que tenga una forma (el centro de cada lado, y más en algunas formas). Para añadir uno propio, seleccione la forma, haga clic en **Modificar puntos** y haga **Mayús**+clic donde debe ir el punto. Un rombo azul lo marca. Los conectores pueden fijarse a él como a cualquier otro punto de conexión, y siguen a la forma cuando se mueve o cambia de tamaño: el punto conserva su lugar dentro de la forma (un cuarto del recorrido horizontal sigue estando a un cuarto). Haga **Mayús**+clic en un rombo para quitarlo; los conectores fijados a él quedan libres. Una forma puede tener hasta 16 puntos de conexión propios. Cada cambio es un paso de **Deshacer**, y los puntos se conservan al guardar el fichero como fichero JeffPub. Un fichero .pub conserva el conector en el lado más próximo a su punto de conexión.

## Combinar formas y convertir contornos

Seleccione dos o más formas de la misma página y haga clic en **Formato de forma > Insertar formas > Combinar formas**. La nueva forma tiene el relleno y la línea de la forma que seleccionó primero, y se coloca donde estaban las anteriores. Las formas originales desaparecen, y también el texto que contengan: la barra de estado lo indica, y si más de una de las formas tiene texto, se le pregunta una sola vez antes de eliminarlo. Un conector fijado a una forma original se fija a la nueva forma si estaba en el centro de uno de los lados de la nueva forma; en caso contrario, queda libre en el lugar donde estaba. Cada elección es un paso de **Deshacer**, y las nuevas formas quedan seleccionadas.

- **Unión** crea una sola forma con todo lo que cubren las formas.
- **Combinar** conserva lo que cubre solo una de las formas y recorta las zonas donde se superponen.
- **Restar** conserva la primera forma seleccionada sin las demás.
- **Intersecar** conserva solo lo que cubren todas las formas.
- **Fragmentar** corta las formas en todos los trozos que forman entre sí. Cada trozo es una forma independiente.

Puede combinar hasta 64 formas a la vez, y no se admite un resultado que se dividiría en más de 256 trozos. Las formas con muchísimos bordes que se cruzan tardarían demasiado, y se rechazan con un mensaje; una combinación con más de unos pocos cruces se ejecuta en segundo plano, con un cuadro en el que puede hacer clic en **Cancelar** (las formas quedan entonces como estaban). Un orificio de una forma sigue siendo un orificio, por muchas combinaciones que se hagan después. Una forma bloqueada, incluida en un grupo o que sea una obra de arte, no cambia, y la barra de estado lo indica. Una forma combinada es una forma con puntos propios, así que **Modificar puntos** permite cambiarla, y se guarda en el fichero como cualquier otra.

**Formato de forma > Insertar formas > Convertir** cambia la manera en que se dibujan las formas seleccionadas:

- **Convertir en curva** da a la forma un contorno propio, para que sus puntos puedan moverse (**Modificar puntos** hace lo mismo al empezar a editar).
- **Convertir en polígono** convierte cada curva del contorno en líneas rectas cortas.
- **Convertir en contorno** convierte la línea de la forma en una forma propia, rellena con el color de la línea. La forma necesita una línea.
- **Separar** convierte una forma formada por varias partes (como la unión de formas que no se tocan) en una forma por cada parte. Un anillo se separa en su círculo exterior y en su agujero, cada uno con relleno. El texto de una forma se queda con ella: con el primer trozo de la forma separada.

## Redirigir conectores

Un conector es una línea cuyos extremos están fijados a formas (dibújela desde un punto de conexión de una forma hasta otro de otra forma). Seleccione una forma o el conector y haga clic en **Formato de forma > Insertar formas > Redirigir conectores**. Cada conector fijado a la forma mueve cada extremo unido al punto de conexión de su forma que esté más cerca del otro extremo, de modo que el trazado sea el más corto. Es un paso de **Deshacer**. Si todos los conectores ya están en su trazado más corto, la barra de estado lo indica.

## Temas relacionados

- [Mover, alinear, agrupar y apilar objetos](arrange)
- [Agregar y ajustar imágenes](pictures)
- [Iconos, códigos de barras y otros objetos](insert-objects)
- [La ficha Formato de forma](tab-shape-format)
