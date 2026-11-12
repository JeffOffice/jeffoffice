# Cambiar la configuración de JeffDeck
<!-- keywords: opciones, configuración, preferencias, tema, modo oscuro, idioma, nombre de usuario, iniciales, regla, ajustar, ortografía, autocorrección, comillas, actualizaciones, beta, estadísticas de uso, acerca de, guardar, insertar fuentes, fuentes, personalizar teclado, personalizar la cinta de opciones, personalizar la barra de herramientas rápida, barra de herramientas rápida, agregar un comando a la barra de herramientas rápida, ocultar una pestaña, ocultar un grupo, agregar una pestaña, agregar un grupo, cambiar el nombre de una pestaña, restablecer la cinta de opciones, cambiar teclas de acceso directo, reasignar teclas, restablecer teclas -->

Su configuración está en la ventana **Opciones de JeffDeck**. Haga clic en **Archivo > Opciones** para abrirla. Tiene cinco fichas. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## General

- **Nombre de usuario** e **Iniciales** se muestran en los comentarios que escribe. Consulte [Comentarios](comments).
- **Tema** es **Usar la configuración del sistema**, **Claro** u **Oscuro**. El cambio se aplica de inmediato.
- **Idioma de presentación** es el idioma de los textos de JeffDeck. **Usar el idioma del sistema** sigue el idioma de su equipo. Otros idiomas aparecen en esta lista cuando hay traducciones disponibles. Un nuevo idioma se aplica la próxima vez que abra JeffDeck.
- **Comprobar si hay actualizaciones al iniciar JeffDeck** hace que JeffDeck busque una versión más reciente cada vez que se inicia. Si la hay, se muestra la ventana **Actualización disponible** con las novedades. Elija **Actualizar ahora**, **Más tarde** u **Omitir esta versión**. En Windows, **Actualizar ahora** descarga el programa de instalación. Instala la nueva versión de todas las aplicaciones de JeffOffice y le pide que cierre las que tenga abiertas. En otros equipos, el botón es **Abrir página de descarga**, que abre la página donde puede obtener la nueva versión. Si JeffPub también está abierto, solo uno de los dos le avisará de una nueva versión cada día.
- **Obtener versiones beta (compilaciones preliminares para pruebas)** hace que JeffDeck también le ofrezca versiones beta. Estas se publican antes que las versiones normales y pueden tener fallos.
- **Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffDeck. Anónimo significa que los datos no dicen nada sobre quién es usted. Cuando esta opción está activada, JeffDeck envía estos datos una vez al día y el día en que actualiza: que es JeffDeck, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de archivo abrió o guardó (por ejemplo, .pptx). Si JeffDeck se cierra inesperadamente, también envía un informe de error la próxima vez que se inicie. El informe indica en qué punto del programa se detuvo, para que el problema pueda encontrarse y corregirse. JeffDeck nunca envía sus archivos, sus nombres ni nada de lo que contienen, ni nada de lo que escribe. Si esta opción está desactivada, tampoco se envía ningún informe de error. JeffDeck le pregunta sobre esto la primera vez que se inicia, en una ventana llamada **Ayude a mejorar JeffDeck**. No le pregunta si ya eligió una opción en JeffPub o en la instalación.
- **Aviso de privacidad** junto a esa casilla abre nuestro aviso de privacidad en jeffoffice.com/privacy. En él se explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** se muestra debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, el mismo en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si aún no se ha generado ningún número, muestra **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas, el número se borra de su equipo y, sin él, no podemos saber qué registros son suyos.
- Para desactivar las estadísticas sin abrir JeffDeck, defina la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Así, no se cuenta ni se envía nada, y JeffDeck no le hace la pregunta de la primera vez.

**Nombre de usuario**, **Iniciales**, **Tema** e **Idioma de presentación**, así como las opciones de actualizaciones y estadísticas, son comunes a todas las aplicaciones de JeffOffice: si cambia una aquí, cambia en todas.

## Revisión

Estas casillas controlan la ortografía y las ayudas para escribir. Consulte [Escribir y dar formato al texto](text).

- **Comprobar ortografía mientras escribe**
- **Omitir palabras en MAYÚSCULAS**
- **Omitir palabras que contienen números**
- **Reemplazar texto mientras escribe (Autocorrección)** corrige errores comunes de escritura.
- **Reemplazar comillas rectas por comillas tipográficas** cambia " y ' por comillas tipográficas.
- **Aplicar formato automáticamente mientras escribe (guiones, fracciones, ordinales, listas automáticas)**
- **Convertir las direcciones web en hipervínculos mientras escribe** convierte una dirección web o de correo electrónico en un vínculo al pulsar la barra espaciadora después de ella. Deshacer elimina solo el vínculo.

## Ver

- **Mostrar la regla** muestra las reglas a lo largo de los bordes de la diapositiva. Es lo mismo que **Ver > Mostrar > Regla**, y se aplica a todas las ventanas, las abiertas ahora y las que abra después.
- **Ajustar objetos a otros objetos** hace que un objeto que arrastra se alinee con los demás objetos de la diapositiva.

## Guardar

- **Insertar fuentes en el archivo** incluye dentro del archivo, al guardarlo, las fuentes que usan sus diapositivas, para que se vean igual en un equipo que no tenga esas fuentes. Está activada hasta que la desmarque. Solo incluye las fuentes que vienen con JeffDeck, y aumenta el tamaño del archivo. El tema Abrir y guardar presentaciones tiene los detalles, en "Fuentes en el archivo".
- **Quitar la información personal de este archivo al guardarlo** elimina del archivo que se escribe (un archivo .pptx u .odp, una plantilla y una presentación enviada por correo electrónico) el autor, la persona que guardó el archivo por última vez, la empresa, el administrador, y los nombres, iniciales y horas de los comentarios. El contenido del archivo no cambia. Las propiedades personalizadas del archivo se mantienen tal cual, y el documento que tiene abierto no se modifica. Es la misma opción en todas las aplicaciones de JeffOffice: si la cambia en una, cambia en todas. La misma casilla está en la página **Información**.

## Personalizar

La ficha **Personalizar** tiene tres páginas: **Cinta de opciones**, **Barra de herramientas rápida** y **Teclado**. La página **Teclado** es donde elige las teclas de acceso directo de los comandos de JeffDeck. Consulte "Elegir sus propias teclas de acceso directo" en el tema sobre el teclado.

### Personalizar la cinta de opciones

La página **Cinta de opciones** muestra las pestañas de la cinta, cada una con sus grupos.

- **Ocultar o mostrar una pestaña o un grupo.** Desmarque la casilla junto a ellos para ocultarlos, y márquela para volver a mostrarlos. Un grupo oculto no ocupa espacio, y sus comandos siguen funcionando con sus teclas de acceso directo. **Inicio** no tiene casilla, porque siempre permanece, y tampoco el botón **Archivo**. Una pestaña que aparece solo cuando hay algo seleccionado, como una imagen o una tabla, está al final de la lista y no tiene casilla.
- **Crear su propia pestaña o grupo.** Escriba un nombre en el cuadro **Nombre** y haga clic en **Nueva pestaña**. Para crear un grupo, elija una pestaña y haga clic en **Nuevo grupo**. Un grupo propio puede ir en cualquier pestaña, incluida una pestaña que aparece solo cuando hay algo seleccionado.
- **Poner comandos en su grupo.** Elija su grupo, busque un comando en la lista **Comandos** (escriba en el cuadro **Buscar comandos** para reducir la lista) y haga clic en **Agregar comando**. Puede agregar cualquier comando que tenga nombre. Un comando solo puede estar una vez en un grupo.
- **Cambiar el nombre.** Elija una pestaña, un grupo o un comando propio, escriba el nuevo nombre y haga clic en **Cambiar nombre**. Un comando al que cambie el nombre hace lo mismo, y el nuevo nombre es solo el texto de su botón. Para devolver a un comando su nombre original, cámbiele el nombre por uno vacío.
- **Reordenar.** Elija una pestaña y haga clic en **Subir** o **Bajar**. Puede mover las pestañas propias de la cinta y también las suyas, pero no las pestañas que aparecen solo cuando hay algo seleccionado. Sus grupos y sus comandos se mueven de la misma forma dentro de su pestaña o grupo. Los grupos propios de la cinta mantienen su orden.
- **Quitar.** Elija una pestaña, un grupo o un comando propio y haga clic en **Quitar**.

**Restablecer uno** devuelve la pestaña elegida: se muestra, se muestran sus grupos y desaparecen los grupos que agregó a ella. Una pestaña propia se elimina. **Restablecer todo** devuelve toda la cinta, incluido el orden de las pestañas.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffDeck muestran su cinta, y también todas las ventanas que abra después. JeffDeck guarda solo lo que difiere de la cinta habitual, de modo que una pestaña o grupo que añada una versión posterior aparece por sí mismo. Sus pestañas y grupos reciben letras de acceso (las letras que se muestran al pulsar **Alt**) que no coinciden con las de los demás. Puede tener 20 pestañas propias, 30 grupos propios en una pestaña y 60 comandos en un grupo.

### Personalizar la barra de herramientas rápida

La página **Barra de herramientas rápida** sirve para la pequeña barra de herramientas situada en la parte superior izquierda de la ventana. Su lista de la izquierda muestra lo que hay ahora en la barra, y la lista **Comandos** de la derecha contiene todos los comandos que tienen nombre (escriba en el cuadro **Buscar comandos** para reducirla, igual que en la página **Cinta de opciones**).

- **Agregar un comando.** Elíjalo en la lista **Comandos** y haga clic en **Agregar comando**. Se coloca al final de la barra. Un comando que abre un menú conserva su menú en la barra. Un comando solo puede estar una vez en la barra.
- **Clic con el botón derecho.** También puede hacer clic con el botón derecho en cualquier comando de la cinta y elegir **Agregar a la barra de herramientas rápida**, o hacer clic con el botón derecho en un botón de la barra y elegir **Quitar de la barra de herramientas rápida**. El mismo menú tiene **Personalizar la barra de herramientas rápida** y **Personalizar la cinta de opciones**, que abren esta página.
- **Agregar un separador.** Haga clic en **Agregar separador** para poner una línea fina al final, y después muévala donde quiera.
- **Reordenar.** Elija un comando o un separador en la lista de la izquierda y haga clic en **Subir** o **Bajar**.
- **Quitar.** Elíjalo y haga clic en **Quitar**.

**Restablecer uno** devuelve el comando elegido: a su lugar habitual en JeffDeck, o lo quita de la barra si JeffDeck no lo suele tener allí. **Restablecer todo** devuelve la barra de herramientas con la que viene JeffDeck. Restablecer todo en esta página no cambia su cinta, y restablecer todo en la página **Cinta de opciones** no cambia su barra de herramientas.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffDeck muestran su barra, y también todas las ventanas que abra después. La barra puede tener 100 comandos y separadores en total. Cuando hay más botones de los que caben en un tercio del ancho de la ventana, los restantes quedan detrás de un botón **»** al final de la barra, de modo que el título de la ventana nunca se desplaza fuera de la pantalla. JeffDeck conserva en su lista un comando que una versión posterior ya no tiene, y lo vuelve a mostrar si el comando regresa. Los botones reciben letras de acceso: los números del 1 al 9 y luego dos letras, en el orden en que están en la barra. La barra siempre se sitúa en la parte superior izquierda, junto al botón **Archivo**.

## Temas relacionados

- [Ficha Ver](tab-view)
- [Recuperar trabajo no guardado](recover)
- [La pantalla Archivo](file-screen)
