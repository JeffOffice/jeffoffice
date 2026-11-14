# Cambiar la configuración de JeffDeck
<!-- keywords: opciones, configuración, preferencias, tema, modo oscuro, idioma, nombre de usuario, iniciales, regla, ajustar, ortografía, autocorrección, comillas, actualizaciones, beta, estadísticas de uso, acerca de, guardar, insertar fuentes, fuentes, personalizar teclado, personalizar la cinta de opciones, personalizar la barra de herramientas rápida, barra de herramientas rápida, agregar un comando a la barra de herramientas rápida, ocultar una ficha, ocultar un grupo, agregar una ficha, agregar un grupo, cambiar el nombre de una ficha, restablecer la cinta de opciones, cambiar teclas de acceso directo, reasignar teclas, restablecer teclas -->

La configuración está en la ventana **Opciones de JeffDeck**. Haga clic en **Archivo > Opciones** para abrirla. Tiene cinco fichas. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## General

- **Nombre de usuario** e **Iniciales** se muestran en los comentarios que escribe. Consulte [Comentarios](comments).
- **Tema** es **Usar la configuración del sistema**, **Claro** u **Oscuro**. El cambio se muestra al instante.
- **Idioma de presentación** es el idioma de los textos de JeffDeck. **Usar el idioma del sistema** sigue el idioma de su equipo. Otros idiomas aparecen en esta lista cuando hay traducciones disponibles. Un idioma nuevo se aplica la próxima vez que abra JeffDeck.
- **Buscar actualizaciones al iniciar JeffDeck** hace que JeffDeck busque una versión más reciente cada vez que se inicia. Si la hay, aparece una ventana **Actualización disponible** que muestra las novedades. Elija **Actualizar ahora**, **Más tarde** u **Omitir esta versión**. En Windows, **Actualizar ahora** descarga el programa de instalación. Instala la nueva versión de todas las aplicaciones de JeffOffice y le pide que cierre las que estén abiertas. En otros equipos, el botón es **Abrir página de descarga**. Abre la página donde puede obtener la nueva versión. Si JeffPub también está abierto, solo una de las dos aplicaciones le avisará de una nueva versión cada día.
- **Obtener versiones beta (compilaciones preliminares para pruebas)** hace que JeffDeck también le ofrezca versiones beta. Estas se publican antes que las versiones normales. Pueden tener defectos.
- **Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffDeck. Anónimo significa que los datos no dicen nada sobre usted. Cuando esta opción está activada, JeffDeck envía estos datos una vez al día, y el día en que actualiza: que es JeffDeck, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de archivo abrió o guardó (por ejemplo, .pptx). Si JeffDeck se cierra inesperadamente, también envía un informe de error la próxima vez que se inicie. El informe indica en qué punto del programa se detuvo, para que el problema pueda encontrarse y corregirse. JeffDeck nunca envía sus archivos, sus nombres ni nada de su contenido, ni lo que escribe. Si esta opción está desactivada, tampoco se envía ningún informe de error. JeffDeck le pregunta por esto la primera vez que se inicia, en una ventana llamada **Ayudar a mejorar JeffDeck**. No le pregunta si ya lo eligió en JeffPub o en la instalación.
- **Aviso de privacidad**, junto a esta casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. En él se explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** se muestra debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, igual en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si todavía no se ha generado ningún número, muestra **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas, el número se borra de su equipo, y sin él no podemos saber cuáles son sus registros.
- Para desactivar las estadísticas sin abrir JeffDeck, defina la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Entonces no se cuenta ni se envía nada, y JeffDeck no hace la pregunta de la primera ejecución.

**Nombre de usuario**, **Iniciales**, **Tema** e **Idioma de presentación**, así como las opciones de actualizaciones y estadísticas, son comunes a todas las aplicaciones de JeffOffice: si cambia una aquí, el cambio se aplica en todas.

## Revisión

Estas casillas controlan la ortografía y las ayudas para escribir. Consulte [Escribir y dar formato al texto](text).

- **Comprobar ortografía mientras escribe**
- **Comprobar gramática mientras escribe** marca con una línea azul de puntos los errores claros de gramática y de estilo en textos en inglés, francés, alemán y español.
- **Configuración de gramática** abre una lista de los tipos de problemas que se buscan: gramática, puntuación, uso de mayúsculas, palabras confundidas, repeticiones y estilo (que está desactivado hasta que lo active).
- **Omitir palabras en MAYÚSCULAS**
- **Omitir palabras que contienen números**
- **Reemplazar texto mientras escribe (Autocorrección)** corrige los errores de escritura más comunes.
- **Reemplazar comillas rectas por comillas tipográficas** cambia " y ' por comillas curvas.
- **Aplicar formato automático mientras escribe (guiones, fracciones, ordinales, listas automáticas)**
- **Convertir las direcciones web en hipervínculos mientras escribe** convierte una dirección web o de correo electrónico en un hipervínculo al pulsar la barra espaciadora después de ella. Deshacer solo elimina el hipervínculo.

## Vista

- **Mostrar reglas** muestra las reglas a lo largo de los bordes de la diapositiva. Es lo mismo que **Vista > Mostrar > Regla**, y se aplica a todas las ventanas, las actuales y las que abra más adelante.
- **Ajustar objetos a otros objetos** hace que un objeto que arrastra se alinee con los demás objetos de la diapositiva.

## Guardar

- **Insertar fuentes en el archivo** incluye dentro del archivo, al guardarlo, las fuentes que usan sus diapositivas, para que se vean igual en un equipo que no tenga esas fuentes. Está activada hasta que la desmarque. Solo incluye las fuentes que vienen con JeffDeck, y hace el archivo más grande. El tema Abrir y guardar presentaciones tiene más detalles, en «Fuentes en el archivo».
- **Quitar la información personal de este archivo al guardarlo** elimina del archivo que se escribe (un archivo .pptx u .odp, una plantilla y una presentación enviada por correo electrónico) el autor, la persona que guardó el archivo por última vez, la empresa, el administrador y los nombres, las iniciales y las horas de los comentarios. El contenido del archivo no cambia. Las propiedades personalizadas del archivo se mantienen tal como están, y el documento que tiene abierto no se modifica. Es la misma opción en todas las aplicaciones de JeffOffice: si la cambia en una, cambia en todas. La misma casilla está en la página **Información**.

## Personalizar

La ficha **Personalizar** tiene tres páginas: **Cinta de opciones**, **Barra de herramientas rápida** y **Teclado**. La página **Teclado** es donde elige las teclas de acceso directo de los comandos de JeffDeck. Consulte «Elegir sus propias teclas de acceso directo» en el tema sobre el teclado.

### Personalizar la cinta de opciones

La página **Cinta de opciones** muestra las fichas de la cinta, cada una con sus grupos.

- **Ocultar o mostrar una ficha o un grupo.** Desmarque la casilla junto a él para ocultarlo, y márquela para mostrarlo de nuevo. Un grupo oculto no ocupa espacio, y sus comandos siguen funcionando con sus teclas de acceso directo. **Inicio** no tiene casilla, porque siempre se muestra, y tampoco el botón **Archivo**. Una ficha que aparece solo cuando se selecciona algo, como una imagen o una tabla, está al final de la lista y no tiene casilla.
- **Crear una ficha o un grupo propio.** Escriba un nombre en el cuadro **Nombre** y haga clic en **Nueva ficha**. Para crear un grupo, elija una ficha y haga clic en **Nuevo grupo**. Un grupo propio puede ir en cualquier ficha, incluida una ficha que aparece solo cuando se selecciona algo.
- **Poner comandos en su grupo.** Elija su grupo, busque un comando en la lista **Comandos** (escriba en el cuadro **Buscar comandos** para reducir la lista) y haga clic en **Agregar comando**. Puede ir en un grupo cualquier comando que tenga nombre. Un comando solo puede estar una vez en un grupo.
- **Cambiar el nombre.** Elija una ficha, un grupo o un comando propio, escriba el nombre nuevo y haga clic en **Cambiar nombre**. Un comando al que cambia el nombre hace lo mismo, y el nombre nuevo es solo el texto de su botón. Para devolver a un comando su nombre original, cambie su nombre por uno vacío.
- **Reordenar.** Elija una ficha y haga clic en **Subir** o **Bajar**. Puede mover tanto las fichas propias de la cinta como las suyas, pero no las fichas que aparecen solo cuando se selecciona algo. Sus grupos y sus comandos se mueven de la misma forma dentro de su ficha o grupo. Los grupos propios de la cinta mantienen su orden.
- **Quitar.** Elija una ficha, un grupo o un comando propio y haga clic en **Quitar**.

**Restablecer uno** devuelve la ficha elegida a su estado original: se muestra, se muestran sus grupos y desaparecen los grupos que agregó. Una ficha propia se elimina. **Restablecer todo** devuelve la cinta completa a su estado original, incluido el orden de las fichas.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffDeck muestran su cinta, y también todas las ventanas que abra después. JeffDeck guarda solo lo que difiere de la cinta habitual, de modo que una ficha o un grupo que añada una versión posterior aparece por sí solo. Sus fichas y grupos tienen KeyTips (las letras que se muestran al pulsar **Alt**) que no coinciden con los demás. Puede tener 20 fichas propias, 30 grupos propios en una ficha y 60 comandos en un grupo.

### Personalizar la barra de herramientas rápida

La página **Barra de herramientas rápida** sirve para la pequeña barra de herramientas situada arriba a la izquierda de la ventana. Su lista de la izquierda muestra lo que hay ahora en la barra, y la lista **Comandos** de la derecha tiene todos los comandos que tienen nombre (escriba en el cuadro **Buscar comandos** para reducirla, igual que en la página **Cinta de opciones**).

- **Agregar un comando.** Elíjalo en la lista **Comandos** y haga clic en **Agregar comando**. Se coloca al final de la barra. Un comando que abre un menú conserva su menú en la barra. Un comando solo puede estar una vez en la barra.
- **Clic derecho.** También puede hacer clic derecho en cualquier comando de la cinta y elegir **Agregar a la barra de herramientas rápida**, o hacer clic derecho en un botón de la barra y elegir **Quitar de la barra de herramientas rápida**. El mismo menú tiene **Personalizar la barra de herramientas rápida** y **Personalizar la cinta de opciones**, que abren esta página.
- **Mover la barra debajo de la cinta.** Marque **Mostrar la barra de herramientas rápida debajo de la cinta de opciones** para colocar la barra en una fila debajo de la cinta, en lugar de arriba a la izquierda. Los dos menús del clic derecho también tienen **Mostrar la barra de herramientas rápida debajo de la cinta de opciones**. JeffDeck recuerda la opción.
- **Agregar un separador.** Haga clic en **Agregar separador** para colocar una línea fina al final, y luego muévala donde quiera.
- **Reordenar.** Elija un comando o un separador en la lista de la izquierda y haga clic en **Subir** o **Bajar**.
- **Quitar.** Elíjalo y haga clic en **Quitar**.

**Restablecer uno** devuelve el comando elegido a su posición habitual, o lo quita de la barra si JeffDeck no suele tenerlo allí. **Restablecer todo** devuelve la barra con la que viene JeffDeck. Restablecer todo en esta página no cambia su cinta, y restablecer todo en la página **Cinta de opciones** no cambia su barra.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffDeck muestran su barra, y también todas las ventanas que abra después. La barra puede tener 100 comandos y separadores en total. Cuando hay más botones de los que caben en un tercio del ancho de la ventana, los demás quedan detrás de un botón **»** al final de la barra, de modo que el título de la ventana nunca queda fuera de la pantalla. JeffDeck conserva en su lista un comando que una versión posterior ya no tenga, y lo vuelve a mostrar si el comando regresa. Los botones tienen KeyTips: los números del 1 al 9 y luego dos letras, en el orden en que están en la barra. La barra siempre se encuentra arriba a la izquierda, junto al botón **Archivo**.

## Temas relacionados

- [Ficha Vista](tab-view)
- [Recuperar trabajo no guardado](recover)
- [La pantalla Archivo](file-screen)
