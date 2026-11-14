# Cambiar la configuración de JeffCrunch
<!-- keywords: recuperación, recuperar trabajo sin guardar, recuperación automática, revisión, edición, autocompletar, controlador de relleno, arrastrar y colocar, posiciones decimales, opciones, configuración, preferencias, estadísticas de uso, número de instalación, privacidad, aviso de privacidad, JP_NO_TELEMETRY, eliminar estadísticas, actualizaciones, acerca de, editar en la celda, color de las líneas de cuadrícula, zoom predeterminado, ancho estándar, información personal, personalizar teclado, personalizar la cinta de opciones, personalizar la barra de herramientas rápida, barra de herramientas rápida, agregar un comando a la barra de herramientas rápida, ocultar una pestaña, ocultar un grupo, agregar una pestaña, agregar un grupo, cambiar el nombre de una pestaña, restablecer la cinta de opciones, cambiar métodos abreviados de teclado, reasignar teclas, restablecer teclas -->

Su configuración está en la ventana **Opciones de JeffCrunch**. Haga clic en **Archivo > Opciones** para abrirla. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## Edición

La página **Edición** cambia el modo en que funciona la escritura en las celdas.

- **Después de presionar Entrar, mover la selección** indica adónde va la selección al terminar una entrada: hacia abajo, hacia la derecha, hacia arriba, hacia la izquierda o no se mueve.
- **Ofrecer completar un texto a partir de las celdas superiores e inferiores** se llama Autocompletar. Cuando escribe las primeras letras de un texto y solo un texto de esa columna empieza por ellas, JeffCrunch muestra el resto, seleccionado. Presione **Entrar** para conservarlo. Siga escribiendo para ignorarlo, o presione **Retroceso** para quitarlo. Solo mira el bloque de celdas junto a la que está escribiendo, y solo textos, no números.
- **Usar el controlador de relleno y arrastrar celdas para moverlas o copiarlas** activa el pequeño cuadrado de la esquina de la selección y el arrastre del borde de la selección. Desactívelo si mueve celdas por accidente. **Copiar** y **Pegar** siguen funcionando.
- **Mostrar un cero en las celdas que tienen valor cero (esta hoja)** activa o desactiva los ceros en la hoja que está viendo. Cuando está desactivado, una celda que contiene 0 parece vacía, pero sigue conteniendo el 0 y se suma como 0. La opción se guarda con la hoja.
- **Poner un punto decimal en los números enteros que se escriben** se llama posiciones decimales automáticas. Si establece **Posiciones** en 2, al escribir 1234 se introduce 12,34. Si lo establece en -2, al escribir 5 se introduce 500. Un número que escribe con punto decimal se introduce tal como lo escribe.
- **Permitir editar directamente en las celdas** está activado de forma predeterminada. Si lo desactiva, una entrada escrita o F2 va a la barra de fórmulas, y la celda muestra el resultado solo cuando termina la entrada.

## Opciones de guardado

**Guardar información de recuperación automática cada** indica cada cuántos minutos JeffCrunch guarda una copia de un libro con cambios que no ha guardado, de 1 a 120 minutos. La copia es un archivo .xlsx en una carpeta que JeffCrunch reserva para este fin, y se crea en segundo plano para que pueda seguir escribiendo. Se elimina al guardar o cerrar el libro. Si JeffCrunch se cierra inesperadamente, ofrece las copias la próxima vez que se inicie. También puede hacer clic en **Archivo > Abrir > Recuperar trabajo no guardado**. Un libro protegido con contraseña no se copia, porque la copia no estaría protegida.

## Opciones de revisión

La página **Revisión** es compartida por las aplicaciones de JeffOffice. Decide si la ortografía se revisa mientras escribe en una celda (desactive **Comprobar ortografía mientras escribe** para eliminar las líneas rojas onduladas de todas las ventanas abiertas y de las demás aplicaciones; **Ortografía** en la pestaña Revisar sigue funcionando), si se ignoran al revisar la ortografía las palabras en mayúsculas y las que contienen números, si la Autocorrección corrige las palabras mientras escribe, si las comillas rectas se convierten en tipográficas, si se cambian guiones, fracciones y ordinales, y si una dirección web se convierte en un vínculo. Consulte [Ortografía](notes-links) y [Escribir en celdas](edit-cells).

## Opciones de pantalla

- **Color de las líneas de cuadrícula** es el color de las líneas entre celdas en la pantalla. Es una opción solo para su pantalla, que no se guarda en el libro.
- **Zoom de un libro nuevo** es el tamaño con que se ven las hojas de un libro nuevo, de 10 % a 400 %.
- **Ancho de columna estándar (esta hoja)** es el ancho de todas las columnas de esta hoja que no tienen un ancho propio. Es lo mismo que **Ancho predeterminado** en la pestaña Inicio.

## Personalizar

La pestaña **Personalizar** tiene tres páginas: **Cinta de opciones**, **Barra de herramientas rápida** y **Teclado**. La página **Teclado** es donde elige las teclas de método abreviado de los comandos de JeffCrunch. Consulte «Elegir sus propias teclas de método abreviado» en el tema del teclado.

### Personalizar la cinta de opciones

La página **Cinta de opciones** muestra las pestañas de la cinta, cada una con sus grupos.

- **Ocultar o mostrar una pestaña o un grupo.** Desmarque la casilla junto a ella para ocultarla, y márquela para volver a mostrarla. Un grupo oculto no ocupa espacio, y sus comandos siguen funcionando con sus teclas de método abreviado. **Inicio** no tiene casilla, porque siempre permanece, y tampoco el botón **Archivo**. Una pestaña que aparece solo cuando hay algo seleccionado, como una imagen o una tabla, está al final de la lista y no tiene casilla.
- **Crear una pestaña o un grupo propio.** Escriba un nombre en el cuadro **Nombre** y haga clic en **Nueva pestaña**. Para crear un grupo, elija una pestaña y haga clic en **Nuevo grupo**. Un grupo propio puede ir en cualquier pestaña, incluida una que aparece solo cuando hay algo seleccionado.
- **Poner comandos en su grupo.** Elija su grupo, busque un comando en la lista **Comandos** (escriba en el cuadro **Buscar comandos** para reducir la lista) y haga clic en **Agregar comando**. Puede incluirse cualquier comando que tenga nombre. Un comando solo puede estar una vez en un grupo.
- **Cambiar el nombre.** Elija una pestaña, grupo o comando propio, escriba el nuevo nombre y haga clic en **Cambiar nombre**. Un comando renombrado hace lo mismo, y el nuevo nombre es solo el texto de su botón. Para devolver a un comando su nombre original, cámbiele el nombre por uno vacío.
- **Reordenar.** Elija una pestaña y haga clic en **Subir** o **Bajar**. Puede mover las pestañas propias de la cinta, además de las suyas, pero no las pestañas que aparecen solo cuando hay algo seleccionado. Sus grupos y sus comandos se mueven de la misma forma dentro de su pestaña o grupo. Los grupos propios de la cinta conservan su orden.
- **Quitar.** Elija una pestaña, grupo o comando propio y haga clic en **Quitar**.

**Restablecer uno** devuelve la pestaña elegida a su estado original: se muestra, se muestran sus grupos y desaparecen los grupos que agregó. Una pestaña propia se elimina. **Restablecer todo** devuelve la cinta completa, incluido el orden de las pestañas.

No se cambia nada hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffCrunch muestran su cinta, y también todas las ventanas que abra después. JeffCrunch guarda solo lo que difiere de la cinta habitual, de modo que una pestaña o grupo que añada una versión posterior aparece por sí solo. Sus pestañas y grupos reciben KeyTips (las letras que se muestran al presionar **Alt**) que no coinciden con las demás. Puede tener 20 pestañas propias, 30 grupos propios en una pestaña y 60 comandos en un grupo.

### Personalizar la barra de herramientas rápida

La página **Barra de herramientas rápida** sirve para la pequeña barra de herramientas de la parte superior izquierda de la ventana. Su lista de la izquierda muestra lo que hay ahora en la barra, y la lista **Comandos** de la derecha tiene todos los comandos que tienen nombre (escriba en el cuadro **Buscar comandos** para reducirla, igual que en la página **Cinta de opciones**).

- **Agregar un comando.** Elíjalo en la lista **Comandos** y haga clic en **Agregar comando**. Se coloca al final de la barra. Un comando que abre un menú conserva su menú en la barra. Un comando solo puede estar una vez en la barra.
- **Clic con el botón derecho.** También puede hacer clic con el botón derecho en cualquier comando de la cinta y elegir **Agregar a la barra de herramientas rápida**, o hacer clic con el botón derecho en un botón de la barra y elegir **Quitar de la barra de herramientas rápida**. El mismo menú tiene **Personalizar la barra de herramientas rápida** y **Personalizar la cinta de opciones**, que abren esta página.
- **Mostrarla debajo de la cinta.** Marque **Mostrar la barra de herramientas rápida debajo de la cinta de opciones** para colocar la barra en una fila debajo de la cinta, en lugar de en la parte superior izquierda. Los dos menús del clic derecho también tienen **Mostrar la barra de herramientas rápida debajo de la cinta de opciones**. JeffCrunch recuerda la elección.
- **Agregar un separador.** Haga clic en **Agregar separador** para colocar una línea fina al final, y luego muévala donde quiera.
- **Reordenar.** Elija un comando o un separador en la lista de la izquierda y haga clic en **Subir** o **Bajar**.
- **Quitar.** Elíjalo y haga clic en **Quitar**.

**Restablecer uno** devuelve el comando elegido a su posición habitual, o lo quita de la barra si JeffCrunch no lo tiene allí normalmente. **Restablecer todo** devuelve la barra con la que viene JeffCrunch. Restablecer todo en esta página no cambia su cinta, y Restablecer todo en la página **Cinta de opciones** no cambia su barra.

No se cambia nada hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffCrunch muestran su barra, y también todas las ventanas que abra después. La barra puede tener 100 comandos y separadores en total. Cuando hay más botones de los que caben en un tercio del ancho de la ventana, los demás quedan detrás de un botón **»** al final de la barra, de modo que el título de la ventana nunca se sale de la pantalla. JeffCrunch conserva en su lista un comando que una versión posterior ya no tiene, y lo vuelve a mostrar si el comando regresa. Los botones reciben KeyTips: los números del 1 al 9 y luego dos letras, en el orden en que están en la barra. La barra siempre se sitúa en la parte superior izquierda, junto al botón **Archivo**.

## Opciones de privacidad

- **Quitar la información personal de este archivo al guardarlo** elimina el autor, la persona que guardó el archivo por última vez, la empresa, el responsable, y los nombres y horas de las notas y comentarios, del archivo que se escribe (un libro, una copia y un libro enviado por correo electrónico). Lo que dice el archivo no cambia. Las propiedades personalizadas del archivo se dejan como están, y el documento que tiene abierto no se modifica. Es la misma opción en todas las aplicaciones de JeffOffice: si la cambia en una, cambia en todas. Para hacerlo una sola vez, use **Inspeccionar libro** en [Información](info). La misma casilla está en la página **Información**.

## Estadísticas de uso

**Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffCrunch. Anónimas significa que los datos no dicen nada sobre quién es usted. Cuando está activada, JeffCrunch envía estos datos una vez al día, y el día en que se actualiza: que es JeffCrunch, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de archivo abrió o guardó. Si JeffCrunch llega a cerrarse inesperadamente, también envía un informe de error la próxima vez que se inicie. JeffCrunch nunca envía sus archivos, sus nombres ni nada de lo que contienen, ni lo que escribe. Si está desactivada, tampoco se envía ningún informe de error.

La elección es común a todas las aplicaciones de JeffOffice: si la cambia aquí, cambia en cada una de ellas. JeffCrunch le pregunta por ella la primera vez que se inicia, en una ventana llamada **Ayude a mejorar JeffCrunch**.

- **Aviso de privacidad**, junto a la casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. En él se indica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** se muestra debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, igual en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si todavía no se ha generado ningún número, aparece **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas, el número se borra de su equipo, y sin él no podemos saber qué registros son suyos.
- Para desactivar las estadísticas sin abrir JeffCrunch, establezca la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Entonces no se cuenta ni se envía nada, y JeffCrunch no hace la pregunta de la primera ejecución.

## Relacionado

- [Bienvenido a JeffCrunch](start)
