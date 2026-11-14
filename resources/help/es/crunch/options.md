# Cambiar la configuración de JeffCrunch
<!-- keywords: recuperación, recuperar trabajo no guardado, recuperación automática, revisión, edición, autocompletar, controlador de relleno, arrastrar y colocar, posiciones decimales, opciones, configuración, preferencias, estadísticas de uso, número de instalación, privacidad, aviso de privacidad, JP_NO_TELEMETRY, eliminar estadísticas, actualizaciones, acerca de, editar en la celda, color de las líneas de cuadrícula, zoom predeterminado, ancho estándar, privacidad, información personal, personalizar teclado, personalizar la cinta de opciones, personalizar la barra de herramientas rápida, barra de herramientas rápida, agregar un comando a la barra de herramientas rápida, ocultar una pestaña, ocultar un grupo, agregar una pestaña, agregar un grupo, cambiar el nombre de una pestaña, restablecer la cinta de opciones, cambiar teclas de acceso directo, reasignar teclas, restablecer teclas -->

Su configuración está en la ventana **Opciones de JeffCrunch**. Haga clic en **Archivo > Opciones** para abrirla. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## Edición

La página **Edición** cambia el modo en que funciona la escritura en las celdas.

- **Después de presionar Entrar, mover la selección** indica adónde va la selección al terminar una entrada: abajo, a la derecha, arriba, a la izquierda o no se mueve.
- **Ofrecer completar un texto con las celdas de arriba y de abajo** se denomina Autocompletar. Cuando escribe las primeras letras de un texto y solo un texto de esa columna empieza por ellas, JeffCrunch muestra el resto, seleccionado. Presione **Entrar** para conservarlo. Siga escribiendo para ignorarlo, o presione **Retroceso** para quitarlo. Solo mira el bloque de celdas junto a la que está escribiendo, y solo textos, no números.
- **Usar el controlador de relleno y arrastrar celdas para mover o copiar** activa el pequeño cuadrado de la esquina de la selección y el arrastre del borde de la selección. Desactívelo si mueve celdas por accidente. Copiar y Pegar siguen funcionando.
- **Mostrar un cero en las celdas que tienen valor cero (esta hoja)** activa o desactiva los ceros en la hoja que está viendo. Cuando está desactivado, una celda que contiene 0 parece vacía, pero sigue conteniendo el 0 y se suma como 0. La opción se guarda con la hoja.
- **Poner un punto decimal en los números enteros escritos** es la función de posiciones decimales automáticas. Si establece **Posiciones** en 2, al escribir 1234 se introduce 12,34. Si la establece en -2, al escribir 5 se introduce 500. Un número que escriba con separador decimal se introduce tal como lo escribió.

- **Permitir editar directamente en las celdas** está activado de forma predeterminada. Si lo desactiva, una entrada escrita o F2 va a la barra de fórmulas, y la celda muestra el resultado solo al terminar.

## Opciones de guardado

**Guardar información de recuperación automática cada** indica cada cuántos minutos JeffCrunch guarda una copia de un libro con cambios que no ha guardado, de 1 a 120 minutos. La copia es un archivo .xlsx en una carpeta que JeffCrunch reserva para ello, y se crea en segundo plano para que pueda seguir escribiendo. Se elimina al guardar o cerrar el libro. Si JeffCrunch se cierra inesperadamente, ofrece las copias la próxima vez que se inicia. También puede hacer clic en **Archivo > Abrir > Recuperar trabajo no guardado**. Un libro protegido con contraseña no se copia, porque la copia no estaría protegida.

## Opciones de revisión

La página **Revisión** es compartida por las aplicaciones de JeffOffice. Decide si la ortografía se revisa mientras escribe en una celda (desactive **Comprobar ortografía mientras escribe** para quitar las líneas rojas onduladas de todas las ventanas abiertas y de las demás aplicaciones; **Ortografía** en la pestaña Revisar sigue funcionando), si se ignoran las palabras en mayúsculas y las palabras con números al revisar la ortografía, si Autocorrección corrige las palabras mientras escribe, si las comillas rectas se convierten en comillas tipográficas, si se cambian guiones, fracciones y ordinales, y si una dirección web se convierte en vínculo. Consulte [Ortografía](notes-links) y [Escribir en celdas](edit-cells).

## Opciones de pantalla

- **Color de las líneas de cuadrícula** es el color de las líneas entre celdas en su pantalla. Es una opción solo para su pantalla, no se guarda en el libro.
- **Zoom de un libro nuevo** indica el tamaño con el que se ven las hojas de un libro nuevo, de 10 % a 400 %.
- **Ancho de columna estándar (esta hoja)** es el ancho de cada columna de esta hoja que no tiene un ancho propio. Es lo mismo que **Ancho predeterminado** en la pestaña Inicio.

## Personalizar

La pestaña **Personalizar** tiene tres páginas: **Cinta de opciones**, **Barra de herramientas rápida** y **Teclado**. La página **Teclado** es donde elige las teclas de método abreviado de los comandos de JeffCrunch. Consulte «Elegir sus propias teclas de método abreviado» en el tema del teclado.

### Personalizar la cinta de opciones

La página **Cinta de opciones** muestra las pestañas de la cinta, cada una con sus grupos.

- **Ocultar o mostrar una pestaña o un grupo.** Desmarque la casilla junto a ella para ocultarla, y márquela para mostrarla de nuevo. Un grupo oculto no ocupa espacio, y sus comandos siguen funcionando con sus teclas de acceso directo. **Inicio** no tiene casilla, porque siempre permanece, y tampoco el botón **Archivo**. Una pestaña que aparece solo cuando hay algo seleccionado, como una imagen o una tabla, está al final de la lista y no tiene casilla.
- **Crear su propia pestaña o grupo.** Escriba un nombre en el cuadro **Nombre** y haga clic en **Nueva pestaña**. Para crear un grupo, elija una pestaña y haga clic en **Nuevo grupo**. Un grupo propio puede ir en cualquier pestaña, incluida una que aparezca solo cuando hay algo seleccionado.
- **Poner comandos en su grupo.** Elija su grupo, busque un comando en la lista **Comandos** (escriba en el cuadro **Buscar comandos** para reducir la lista) y haga clic en **Agregar**. Puede ir cualquier comando que tenga nombre. Un comando solo puede estar una vez en un grupo.
- **Cambiar el nombre.** Elija una pestaña, grupo o comando propio, escriba el nuevo nombre y haga clic en **Cambiar nombre**. Un comando al que cambia el nombre hace lo mismo, y el nuevo nombre es solo el texto de su botón. Para cambiar el nombre de un comando por uno vacío y devolverle su nombre original, déjelo vacío.
- **Reordenar.** Elija una pestaña y haga clic en **Subir** o **Bajar**. Puede mover las pestañas propias de la cinta y también las suyas, pero no las pestañas que aparecen solo cuando hay algo seleccionado. Sus grupos y sus comandos se mueven de la misma manera dentro de su pestaña o grupo. Los grupos propios de la cinta mantienen su orden.
- **Quitar.** Elija una pestaña, grupo o comando propio y haga clic en **Quitar**.

**Restablecer uno** devuelve la pestaña elegida a su estado original: se muestra, se muestran sus grupos y desaparecen los grupos que agregó. Una pestaña propia se elimina. **Restablecer todo** devuelve la cinta completa a su estado original, incluido el orden de las pestañas.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffCrunch muestran su cinta, y también todas las ventanas que abra después. JeffCrunch guarda solo lo que difiere de la cinta habitual, de modo que una pestaña o grupo que añada una versión posterior aparecerá por sí solo. Sus pestañas y grupos reciben teclas de acceso (las letras que aparecen al presionar **Alt**) que no coinciden con las demás. Puede tener 20 pestañas propias, 30 grupos propios en una pestaña y 60 comandos en un grupo.

### Personalizar la barra de herramientas rápida

La página **Barra de herramientas rápida** es para la pequeña barra de herramientas de la parte superior izquierda de la ventana. Su lista izquierda muestra lo que hay ahora en la barra, y la lista **Comandos** de la derecha tiene todos los comandos que tienen nombre (escriba en el cuadro **Buscar comandos** para reducirla, igual que en la página **Cinta de opciones**).

- **Agregar un comando.** Elíjalo en la lista **Comandos** y haga clic en **Agregar**. Se añade al final de la barra. Un comando que abre un menú conserva su menú en la barra. Un comando solo puede estar una vez en la barra.
- **Clic derecho.** También puede hacer clic derecho en cualquier comando de la cinta y elegir **Agregar a la barra de herramientas rápida**, o hacer clic derecho en un botón de la barra y elegir **Quitar de la barra de herramientas rápida**. El mismo menú tiene **Personalizar la barra de herramientas rápida** y **Personalizar la cinta de opciones**, que abren esta página.
- **Mover debajo de la cinta.** Marque **Mostrar la barra de herramientas rápida debajo de la cinta de opciones** para colocar la barra en una fila bajo la cinta en lugar de en la parte superior izquierda. Los dos menús de clic derecho también tienen **Mostrar la barra de herramientas rápida debajo de la cinta de opciones**. JeffCrunch recuerda la elección.
- **Agregar un separador.** Haga clic en **Agregar separador** para poner una línea fina al final, y luego muévala donde quiera.
- **Reordenar.** Elija un comando o un separador en la lista izquierda y haga clic en **Subir** o **Bajar**.
- **Quitar.** Elíjalo y haga clic en **Quitar**.

**Restablecer uno** devuelve el comando elegido a su lugar habitual, o lo quita de la barra cuando JeffCrunch no lo suele tener allí. **Restablecer todo** devuelve la barra con la que viene JeffCrunch. Restablecer todo en esta página no cambia su cinta, y Restablecer todo en la página **Cinta de opciones** no cambia su barra.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffCrunch muestran su barra, y también todas las ventanas que abra después. La barra puede tener 100 comandos y separadores en total. Cuando hay más botones de los que caben en un tercio del ancho de la ventana, el resto espera tras un botón **»** al final de la barra, de modo que el título de la ventana nunca queda fuera de la pantalla. JeffCrunch conserva en su lista un comando que una versión posterior ya no tiene, y lo muestra de nuevo si el comando vuelve. Los botones reciben teclas de acceso, los números del 1 al 9 y luego dos letras, en el orden en que están en la barra. La barra siempre se sitúa en la parte superior izquierda, junto al botón **Archivo**.

## Opciones de privacidad

- **Quitar la información personal de este archivo al guardarlo** elimina el autor, la persona que guardó el archivo por última vez, la empresa, el responsable y los nombres y horas de las notas y comentarios, del archivo que se escribe (un libro, una copia y un libro enviado por correo electrónico). Lo que dice el archivo se mantiene igual. Las propiedades personalizadas del archivo se dejan como están, y el documento que tiene abierto no cambia. Es la misma opción en todas las aplicaciones de JeffOffice: si la cambia en una, cambia en todas. Para hacerlo una sola vez, use **Inspeccionar libro** en [Información](info). La misma casilla está en la página **Información**.

## Estadísticas de uso

**Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffCrunch. Anónimo significa que los datos no dicen nada sobre quién es usted. Cuando está activado, JeffCrunch envía estos datos una vez al día, y el día en que se actualiza: que es JeffCrunch, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de archivo abrió o guardó. Si JeffCrunch llega a cerrarse inesperadamente, también envía un informe de error la próxima vez que se inicia. JeffCrunch nunca envía sus archivos, sus nombres ni nada de lo que contienen, ni lo que escribe. Si esta opción está desactivada, tampoco se envía ningún informe de error.

La elección es común a todas las aplicaciones de JeffOffice: si la cambia aquí, cambia en cada una de ellas. JeffCrunch le pregunta por ello la primera vez que se inicia, en una ventana llamada **Ayúdenos a mejorar JeffCrunch**.

- **Aviso de privacidad**, junto a la casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. Explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** aparece debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, igual en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si todavía no se ha generado ningún número, muestra **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas se borra el número de su equipo, y sin él no podemos saber qué registros son suyos.
- Para desactivar las estadísticas sin abrir JeffCrunch, establezca la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Entonces no se cuenta ni se envía nada, y JeffCrunch no hace la pregunta de la primera vez.

## Temas relacionados

- [Bienvenido a JeffCrunch](start)
