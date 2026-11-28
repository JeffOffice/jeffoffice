# Cambiar la configuración de JeffDeck

<!-- keywords: opciones, configuración, preferencias, tema, modo oscuro, idioma, nombre de usuario, iniciales, regla, ajustar, ortografía, autocorrección, comillas, actualizaciones, versión beta, estadísticas de uso, acerca de, guardar, insertar fuentes, fuentes, personalizar teclado, personalizar la cinta de opciones, personalizar la barra de herramientas rápida, barra de herramientas rápida, agregar un comando a la barra de herramientas rápida, ocultar una ficha, ocultar un grupo, agregar una ficha, agregar un grupo, cambiar el nombre de una ficha, restablecer la cinta de opciones, cambiar teclas de acceso directo, reasignar teclas, restablecer teclas -->

Su configuración está en la ventana **Opciones de JeffDeck**. Haga clic en **Archivo > Opciones** para abrirla. Tiene siete fichas. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## General

- **Nombre de usuario** e **Iniciales** se muestran en los comentarios que escribe. Consulte [Comentarios](comments).
- **Tema** es **Usar la configuración del sistema**, **Claro** u **Oscuro**. El cambio se muestra al momento.
- **Idioma de presentación** es el idioma de los textos de JeffDeck. **Usar el idioma del sistema** sigue el idioma de su equipo. Los demás idiomas aparecen en esta lista cuando hay traducciones disponibles. Un idioma nuevo se aplica la próxima vez que abra JeffDeck.
- **Buscar actualizaciones al iniciar JeffDeck** hace que JeffDeck busque una versión más reciente cada vez que se inicia. Si la hay, se abre la ventana **Actualización disponible** con las novedades. Elija **Actualizar ahora**, **Más tarde** u **Omitir esta versión**. En Windows, **Actualizar ahora** descarga el programa de instalación. Instala la nueva versión de todas las aplicaciones de JeffOffice y le pide que cierre las que estén abiertas. En otros equipos, el botón es **Abrir página de descarga**, que abre la página donde puede obtener la nueva versión. Si JeffPub también está abierta, solo una de las dos le avisará de una nueva versión cada día.
- **Obtener versiones beta (compilaciones preliminares para pruebas)** hace que JeffDeck ofrezca también las versiones beta. Estas se publican antes que las versiones habituales y pueden tener fallos.
- **Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffDeck. Anónimo significa que los datos no dicen nada sobre usted. Cuando está activada, JeffDeck envía estos datos una vez al día y el día en que actualiza: que es JeffDeck, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de archivo abrió o guardó (por ejemplo, .pptx). Si JeffDeck se cierra inesperadamente, también envía un informe de error la próxima vez que se inicie. El informe indica en qué punto del programa se detuvo, para que el problema pueda localizarse y corregirse. JeffDeck nunca envía sus archivos, sus nombres ni su contenido, ni nada de lo que escriba. Si esta opción está desactivada, tampoco se envía ningún informe de error. JeffDeck le pregunta por este asunto la primera vez que se inicia, en una ventana llamada **Ayude a mejorar JeffDeck**. No le pregunta si ya eligió una opción en JeffPub o en la instalación.
- **Aviso de privacidad**, junto a esta casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. En él se explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** se muestra debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, igual en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si todavía no se ha generado ningún número, aparece **ninguno**.
- Para que se borren las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas se borra el número de su equipo y, sin él, no podemos saber cuáles son sus registros.
- Para desactivar las estadísticas sin abrir JeffDeck, defina la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Así no se cuenta ni se envía nada, y JeffDeck no hace la pregunta de la primera vez.

**Nombre de usuario**, **Iniciales**, **Tema** e **Idioma de presentación**, así como las opciones de actualizaciones y de estadísticas, son comunes a todas las aplicaciones de JeffOffice: si cambia una aquí, cambiará en todas las demás.

## Revisión

Estas casillas controlan la revisión ortográfica y la ayuda al escribir. Consulte [Escribir y dar formato al texto](text).

- **Comprobar ortografía mientras escribe**
- **Comprobar gramática mientras escribe** marca con una línea azul de puntos los errores claros de gramática y de estilo en texto en inglés, en todas sus variantes regionales. El texto en otros idiomas solo se revisa ortográficamente.
- **Configuración de gramática** abre una lista de los tipos de problemas que buscar: gramática, puntuación, mayúsculas, palabras confundidas, repeticiones y estilo (que está desactivado hasta que lo active).
- **Ignorar palabras en MAYÚSCULAS**
- **Ignorar palabras que contienen números**
- **Reemplazar texto mientras escribe (Autocorrección)** corrige errores de escritura habituales. En **Opciones de autocorrección**, la ficha **Símbolos matemáticos** tiene un interruptor, **Reemplazar los nombres de los símbolos matemáticos mientras escribe**, que está desactivado hasta que lo active. Con él activado, una barra invertida seguida de un nombre, como \alpha, se convierte en el símbolo (α) al escribir un espacio o un signo después. El nombre debe empezar una palabra, de modo que una ruta de carpeta como C:\temp\pi se conserva tal como la escribió. La ficha muestra todos los nombres. JeffDeck, JeffWrite y JeffPub los cambian; JeffCrunch no cambia nada en una celda.
- **Reemplazar comillas rectas por comillas tipográficas** cambia " y ' por comillas curvas.
- **Aplicar formato automático mientras escribe (guiones, fracciones, ordinales, listas automáticas)**
- **Convertir las direcciones web en hipervínculos mientras escribe** convierte una dirección web o de correo electrónico en un vínculo al pulsar la barra espaciadora después. **Deshacer** elimina solo el vínculo.

## Vista

- **Mostrar la regla** muestra las reglas a lo largo de los bordes de la diapositiva. Es lo mismo que **Vista > Mostrar > Regla**, y se aplica a todas las ventanas, las abiertas ahora y las que abra después.
- **Ajustar objetos a otros objetos** hace que un objeto que arrastra se alinee con los demás objetos de la diapositiva.

## Opciones avanzadas

- **Usar la tecla Insertar para activar y desactivar el modo de sobrescritura** está desactivada al principio. Cuando está activada, la tecla **Insertar** alterna entre dos formas de escribir en un cuadro de texto. Normalmente, cada letra que escribe empuja hacia la derecha las letras que siguen. En modo de **sobrescritura**, cada letra que escribe reemplaza la letra que está detrás del cursor, hasta el final del párrafo. La barra de estado muestra **Insertar** o **Sobrescribir** para que sepa en qué modo está. Pulse **Insertar** de nuevo para volver al modo anterior.

## Guardar

- **Insertar fuentes en el archivo** incluye dentro del archivo, al guardar, las fuentes que usan sus diapositivas, para que se vean igual en un equipo que no tenga esas fuentes. Está activada hasta que la desmarque. Solo incluye las fuentes que vienen con JeffDeck, y aumenta el tamaño del archivo. El tema Abrir y guardar presentaciones tiene los detalles, en «Fuentes en el archivo».
- **Quitar la información personal de este archivo al guardarlo** elimina del archivo que se escribe (un archivo .pptx o .odp, una plantilla y una presentación enviada por correo electrónico) el autor, la persona que guardó el archivo por última vez, la empresa, el responsable y los nombres, las iniciales y las horas de los comentarios. El contenido del archivo no cambia. Las propiedades personalizadas del archivo se conservan tal como están, y el documento que tiene abierto no se modifica. Es la misma opción en todas las aplicaciones de JeffOffice: si la cambia en una, cambia en todas. La misma casilla está en la página **Información**.

## Personalizar

La ficha **Personalizar** tiene tres páginas: **Cinta de opciones**, **Barra de herramientas rápida** y **Teclado**. La página **Teclado** es donde elige las teclas de acceso directo de los comandos de JeffDeck. Consulte «Elegir sus propias teclas de acceso directo» en el tema sobre el teclado.

### Personalizar la cinta de opciones

La página **Cinta de opciones** muestra las fichas de la cinta, cada una con sus grupos.

- **Ocultar o mostrar una ficha o un grupo.** Desmarque la casilla que hay junto a él para ocultarlo, y márquela para volver a mostrarlo. Un grupo oculto no ocupa espacio, y sus comandos siguen funcionando con sus teclas de acceso directo. **Inicio** no tiene casilla, porque siempre permanece, y tampoco el botón **Archivo**. Una ficha que aparece solo cuando se selecciona algo, como una imagen o una tabla, está al final de la lista y no tiene casilla.
- **Crear su propia ficha o grupo.** Escriba un nombre en el cuadro **Nombre** y haga clic en **Nueva ficha**. Para crear un grupo, elija una ficha y haga clic en **Nuevo grupo**. Un grupo propio puede ir en cualquier ficha, incluida una ficha que aparece solo cuando se selecciona algo.
- **Poner comandos en su grupo.** Elija su grupo, busque un comando en la lista **Comandos** (escriba en el cuadro **Buscar comandos** para reducir la lista) y haga clic en **Agregar comando**. Puede incluirse cualquier comando que tenga nombre. Un comando solo puede estar una vez en un mismo grupo.
- **Cambiar el nombre.** Elija una ficha, un grupo o un comando propio, escriba el nuevo nombre y haga clic en **Cambiar nombre**. Un comando al que cambia el nombre hace lo mismo, y el nuevo nombre es solo el texto de su botón. Para cambiar el nombre de un comando a uno vacío y devolverle su nombre original, use un nombre vacío.
- **Reordenar.** Elija una ficha y haga clic en **Subir** o **Bajar**. Puede mover las fichas propias de la cinta y también sus fichas, pero no las que aparecen solo cuando se selecciona algo. Sus grupos y sus comandos se mueven del mismo modo dentro de su ficha o grupo. Los grupos propios de la cinta mantienen su orden.
- **Quitar.** Elija una ficha, un grupo o un comando propio y haga clic en **Quitar**.

**Restablecer uno** devuelve la ficha elegida a su estado habitual: se muestra, se muestran sus grupos y desaparecen los grupos que añadió. Una ficha propia se elimina. **Restablecer todo** devuelve la cinta completa, incluido el orden de las fichas.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffDeck muestran su cinta, y también todas las ventanas que abra después. JeffDeck guarda solo lo que difiere de la cinta habitual, de modo que una ficha o un grupo que añada una versión posterior aparece por sí solo. Sus fichas y grupos tienen KeyTips (las letras que aparecen al pulsar **Alt**) que no coinciden con las de los demás. Puede tener 20 fichas propias, 30 grupos propios en una ficha y 60 comandos en un grupo.

### Personalizar la barra de herramientas rápida

La página **Barra de herramientas rápida** sirve para la pequeña barra de herramientas situada arriba a la izquierda de la ventana. Su lista de la izquierda muestra lo que hay ahora en la barra, y la lista **Comandos** de la derecha tiene todos los comandos que tienen nombre (escriba en el cuadro **Buscar comandos** para reducirla, igual que en la página **Cinta de opciones**).

- **Agregar un comando.** Elíjalo en la lista **Comandos** y haga clic en **Agregar comando**. Se añade al final de la barra. Un comando que abre un menú conserva su menú en la barra. Un comando solo puede estar una vez en la barra.
- **Clic con el botón derecho.** También puede hacer clic con el botón derecho en cualquier comando de la cinta y elegir **Agregar a la barra de herramientas rápida**, o hacer clic con el botón derecho en un botón de la barra y elegir **Quitar de la barra de herramientas rápida**. El mismo menú tiene **Personalizar la barra de herramientas rápida** y **Personalizar la cinta de opciones**, que abren esta página.
- **Mover debajo de la cinta.** Marque **Mostrar la barra de herramientas rápida debajo de la cinta de opciones** para colocar la barra en una fila debajo de la cinta, en lugar de arriba a la izquierda. Los dos menús del clic derecho también tienen **Mostrar la barra de herramientas rápida debajo de la cinta de opciones**. JeffDeck recuerda la elección.
- **Agregar un separador.** Haga clic en **Agregar separador** para añadir una línea fina al final, y después muévala donde quiera.
- **Reordenar.** Elija un comando o un separador en la lista de la izquierda y haga clic en **Subir** o **Bajar**.
- **Quitar.** Elíjalo y haga clic en **Quitar**.

**Restablecer uno** devuelve el comando elegido a su posición habitual en JeffDeck o, si JeffDeck no lo suele tener en la barra, lo quita de ella. **Restablecer todo** devuelve la barra con la que viene JeffDeck. Restablecer todo en esta página no cambia su cinta, y restablecer todo en la página **Cinta de opciones** no cambia su barra.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffDeck muestran su barra, y también todas las ventanas que abra después. La barra puede tener en total 100 comandos y separadores. Cuando hay más botones de los que caben en un tercio del ancho de la ventana, los restantes quedan detrás de un botón **»** al final de la barra, de modo que el título de la ventana nunca se desplaza fuera de la pantalla. JeffDeck conserva en su lista un comando que una versión posterior ya no tenga, y lo vuelve a mostrar si el comando regresa. Los botones tienen KeyTips: los números del 1 al 9 y después dos letras, en el orden en que aparecen en la barra. La barra siempre está arriba a la izquierda, junto al botón **Archivo**.

## Seguridad

La ficha **Seguridad** decide qué hace JeffDeck con los elementos a los que puede apuntar un archivo: archivos vinculados, películas y sonidos vinculados, y archivos de Internet. Las macros se guardan en un archivo y nunca se ejecutan, y un vínculo que iniciaría un programa siempre se rechaza. Consulte [Seguridad](security).

## Temas relacionados

- [Ficha Vista](tab-view)
- [Recuperar trabajo no guardado](recover)
- [La pantalla Archivo](file-screen)
