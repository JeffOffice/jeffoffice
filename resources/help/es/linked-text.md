# Hacer fluir texto entre cuadros de texto
<!-- keywords: vincular cuadros de texto, cuadros vinculados, crear vínculo, romper vínculo, flujo de texto, continuar artículo, desbordamiento, texto desbordado, jarra, continúa en la página, viene de la página, conectar cuadros de texto, cadena, cuadro siguiente, cuadro anterior, flujo automático, enlazar cuadros -->

Cuando un artículo es demasiado largo para un cuadro de texto, puede vincular el cuadro a un segundo cuadro. El resto del texto fluye entonces al segundo cuadro. Los cuadros vinculados forman una cadena, y todo el texto de una cadena es un solo artículo. Si modifica el texto en cualquier cuadro, las palabras vuelven a fluir por toda la cadena.

## Vincular dos cuadros

El segundo cuadro debe estar vacío y no estar ya vinculado. Dibújelo primero con **Inicio > Objetos > Dibujar cuadro de texto**.

1. Haga clic en el primer cuadro para seleccionarlo. Debe ser el último cuadro de su cadena.
2. Haga clic en **Cuadro de texto > Vinculación > Crear vínculo**. El puntero se convierte en una jarra.
3. Haga clic en el cuadro vacío. El texto sobrante se vierte en él.

También puede hacer clic en el indicador rojo de desbordamiento **A…** situado en la esquina inferior derecha de un cuadro lleno. Así se activa la misma jarra.

Si en cambio hace clic en una zona vacía de la página, JeffPub dibuja allí un cuadro nuevo, del mismo tamaño que el primero, y lo vincula.

El cuadro vacío puede estar en otra página. Mientras se muestra la jarra, haga clic en esa página en la lista de páginas y, a continuación, haga clic en el cuadro.

Si no se puede crear el vínculo, la barra de estado indica el motivo: el cuadro en el que hizo clic contiene texto o ya está vinculado.

## Cómo fluye el texto

El texto llena el primer cuadro y después el siguiente cuadro, en el orden en que los vinculó. El orden no depende de dónde estén los cuadros en la página. Si un cuadro tiene columnas, primero se llenan las columnas. Si el último cuadro sigue siendo demasiado pequeño, aparece el indicador rojo **A…** y el texto sobrante queda oculto.

## Moverse entre cuadros vinculados

Seleccione un cuadro vinculado. Un pequeño cuadro con una flecha en su esquina inferior derecha le lleva al cuadro siguiente, y otro en su esquina superior izquierda, al cuadro anterior. También puede usar **Cuadro de texto > Vinculación > Siguiente** y **Cuadro de texto > Vinculación > Anterior**. JeffPub cambia de página automáticamente.

## Romper un vínculo

1. Seleccione el cuadro situado justo antes del punto de ruptura.
2. Haga clic en **Cuadro de texto > Vinculación > Romper vínculo**.

Todo el texto permanece en la primera parte de la cadena. Los cuadros posteriores a la ruptura quedan vacíos y puede volver a vincularlos.

## Mostrar avisos "Continúa en" y "Viene de"

Un aviso indica a los lectores dónde continúa un artículo.

1. Haga clic con el botón derecho en un cuadro vinculado y elija **Formato de objeto…**.
2. Abra la pestaña **Cuadro de texto**.
3. Active **Incluir "Continúa en la página…"** para agregar un aviso en la parte inferior del cuadro. Active **Incluir "Viene de la página…"** para agregar uno en la parte superior.

El aviso dice "(Continúa en la página 4)" o "(Viene de la página 2)" en letra cursiva pequeña. JeffPub rellena el número de página. El primero solo se muestra cuando hay un cuadro a continuación, y el segundo, solo cuando hay un cuadro antes.

## Dejar que JeffPub agregue páginas

Si usa **Insertar > Texto > Insertar archivo** y el texto es demasiado largo, JeffPub le pregunta si desea continuarlo en páginas nuevas. Elija **Sí**. Se agregan páginas con un cuadro vinculado en cada una, en la misma posición y con el mismo tamaño que el último cuadro.

## Temas relacionados

- [Agregar cuadros de texto y aplicarles formato](text-boxes)
- [La pestaña Cuadro de texto](tab-text-box)
- [Agregar, mover, eliminar y asignar nombre a páginas](pages)
