# Cambiar la configuración de JeffCrunch
<!-- keywords: opciones de fórmulas, configuración de cálculo, cambio máximo, iteración, recuperación, recuperar trabajo no guardado, recuperación automática, revisión, edición, autocompletar, controlador de relleno, arrastrar y colocar, posiciones decimales, opciones, configuración, preferencias, estadísticas de uso, número de instalación, privacidad, aviso de privacidad, JP_NO_TELEMETRY, eliminar estadísticas, actualizaciones, acerca de, editar en la celda, color de las líneas de cuadrícula, zoom predeterminado, ancho estándar, privacidad, información personal, personalizar teclado, personalizar la cinta de opciones, personalizar la barra de herramientas rápida, barra de herramientas rápida, agregar un comando a la barra de herramientas rápida, ocultar una ficha, ocultar un grupo, agregar una ficha, agregar un grupo, cambiar el nombre de una ficha, restablecer la cinta de opciones, cambiar métodos abreviados de teclado, reasignar teclas, restablecer teclas -->

Su configuración está en la ventana **Opciones de JeffCrunch**. Haga clic en **Archivo > Opciones** para abrirla. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## General

**Mostrar minibarra de herramientas al seleccionar** está activada de forma predeterminada. Decide si la pequeña barra de formato aparece flotando junto al texto seleccionado y en la parte superior del menú contextual de las celdas (véase [Escribir en celdas](edit-cells)). La elección se comparte con las aplicaciones de JeffOffice.

## Edición

La página **Edición** cambia el modo en que funciona la escritura en las celdas.

- **Después de presionar Entrar, mover la selección** indica adónde va la selección cuando termina de introducir un dato: abajo, a la derecha, arriba, a la izquierda o no se mueve.
- **Ofrecer completar un texto con las celdas de arriba y de abajo** se llama Autocompletar. Cuando escribe las primeras letras de un texto y solo un texto de esa columna empieza así, JeffCrunch muestra el resto, seleccionado. Presione **Entrar** para conservarlo. Siga escribiendo para ignorarlo, o presione **Retroceso** para quitarlo. Solo mira el bloque de celdas contiguo a la que está escribiendo, y solo textos, no números.
- **Usar el controlador de relleno y arrastrar celdas para mover o copiar** activa el pequeño cuadrado de la esquina de la selección y el arrastre del borde de la selección. Desactívelo si mueve celdas por accidente. Copiar y Pegar siguen funcionando.
- **Mostrar un cero en las celdas que tienen valor cero (esta hoja)** activa o desactiva los ceros en la hoja que está viendo. Cuando está desactivado, una celda que contiene 0 parece vacía, pero sigue conteniendo el 0 y se suma como 0. La elección se guarda con la hoja.
- **Poner un punto decimal en los números enteros que se escriben** se llama posiciones decimales automáticas. Si establece **Posiciones** en 2, al escribir 1234 se introduce 12,34. Si lo establece en -2, al escribir 5 se introduce 500. Un número que escribe con punto decimal se introduce tal como lo escribe.
- **Permitir editar directamente en las celdas** está activada de forma predeterminada. Cuando la desactiva, un dato escrito o F2 va a la barra de fórmulas, y la celda muestra el resultado solo cuando termina.

## Opciones de fórmulas

La página **Fórmulas** establece cómo calcula sus fórmulas el libro abierto: **Cálculo** (automático, automático excepto tablas de datos, o manual), si las fórmulas se calculan antes de guardar el libro cuando el cálculo es manual, si una fórmula que se hace referencia a sí misma se calcula mediante iteraciones, el número máximo de iteraciones y el cambio que detiene las iteraciones. Estos ajustes pertenecen al libro, no a usted: se guardan en su archivo y no se conservan para otros libros, y las opciones quedan desactivadas cuando no hay ningún libro abierto. El cambio se muestra con todos sus dígitos, y puede escribirlo como número, por ejemplo 0,0000001 o 1e-7. Solo se guarda un ajuste que haya cambiado: **Aceptar** sin cambios no añade ningún paso de Deshacer y deja el libro como estaba. Cada cambio es un paso de Deshacer. El formato de libro nuevo y el archivo .xls antiguo conservan todos los ajustes. Una copia en .ods conserva los ajustes de iteración, pero no tiene dónde guardar el cálculo manual, de modo que la copia calcula las fórmulas a medida que cambian, y el informe que aparece antes de guardar la copia lo indica. Los mismos ajustes están en la ficha **Fórmulas**.

## Opciones de guardado

**Guardar información de recuperación automática cada** establece cada cuánto tiempo JeffCrunch guarda una copia de un libro con cambios que no ha guardado, de 1 a 120 minutos. La copia es un archivo .xlsx en una carpeta que JeffCrunch reserva para este fin, y se crea en segundo plano para que pueda seguir escribiendo. Se elimina cuando guarda o cierra el libro. Si JeffCrunch se cierra inesperadamente, ofrece las copias la próxima vez que se inicia. También puede hacer clic en **Archivo > Abrir > Recuperar trabajo no guardado**. Un libro protegido con contraseña no se copia, porque la copia no estaría protegida.

## Opciones de revisión

La página **Revisión** es compartida por las aplicaciones de JeffOffice. Decide si la ortografía se comprueba mientras escribe en una celda (desactive **Comprobar ortografía mientras escribe** para eliminar las líneas rojas onduladas de todas las ventanas abiertas y de las demás aplicaciones; **Ortografía** en la ficha Revisar sigue funcionando), si se ignoran las palabras en mayúsculas y las palabras con números al comprobar la ortografía, si la Autocorrección corrige palabras mientras escribe, si las comillas rectas se convierten en tipográficas, si se cambian guiones, fracciones y ordinales, y si una dirección web se convierte en un vínculo. Véanse [Ortografía](notes-links) y [Escribir en celdas](edit-cells).

## Opciones de pantalla

- **Color de las líneas de cuadrícula** es el color de las líneas entre celdas en su pantalla. Es una elección solo para su pantalla, que no se guarda en el libro.
- **Zoom de un libro nuevo** indica el tamaño con el que se ven las hojas de un libro nuevo, de 10 % a 400 %.
- **Ancho de columna estándar (esta hoja)** es el ancho de cada columna de esta hoja que no tiene un ancho propio. Es lo mismo que **Ancho predeterminado** en la ficha Inicio.

## Personalizar

La ficha **Personalizar** tiene tres páginas: **Cinta de opciones**, **Barra de herramientas rápida** y **Teclado**. La página **Teclado** es donde elige los métodos abreviados de teclado de los comandos de JeffCrunch. Véase «Elegir sus propios métodos abreviados de teclado» en el tema sobre el teclado.

### Personalizar la cinta de opciones

La página **Cinta de opciones** muestra las fichas de la cinta, cada una con sus grupos.

- **Ocultar o mostrar una ficha o un grupo.** Desmarque la casilla junto a ella para ocultarla, y márquela para volver a mostrarla. Un grupo oculto no ocupa espacio, y sus comandos siguen funcionando con sus métodos abreviados de teclado. **Inicio** no tiene casilla, porque siempre permanece, y tampoco el botón **Archivo**. Una ficha que aparece solo cuando hay algo seleccionado, como una imagen o una tabla, está al final de la lista y no tiene casilla.
- **Crear su propia ficha o grupo.** Escriba un nombre en el cuadro **Nombre** y haga clic en **Nueva ficha**. Para crear un grupo, elija una ficha y haga clic en **Nuevo grupo**. Un grupo propio puede ir en cualquier ficha, incluida una ficha que aparece solo cuando hay algo seleccionado.
- **Poner comandos en su grupo.** Elija su grupo, busque un comando en la lista **Comandos** (escriba en el cuadro **Buscar comandos** para reducir la lista) y haga clic en **Agregar comando**. Puede incluirse cualquier comando que tenga nombre. Un comando solo puede estar una vez en un grupo.
- **Cambiar el nombre.** Elija una ficha, grupo o comando propio, escriba el nuevo nombre y haga clic en **Cambiar nombre**. Un comando al que cambia el nombre hace lo mismo, y el nuevo nombre es solo el texto de su botón. Para cambiar el nombre de un comando con un nombre vacío y devolverle su nombre original, use un nombre vacío.
- **Reordenar.** Elija una ficha y haga clic en **Subir** o **Bajar**. Puede mover las fichas propias de la cinta, además de las suyas, pero no las fichas que aparecen solo cuando hay algo seleccionado. Sus grupos y sus comandos se mueven del mismo modo dentro de su ficha o grupo. Los grupos propios de la cinta conservan su orden.
- **Quitar.** Elija una ficha, grupo o comando propio y haga clic en **Quitar**.

**Restablecer una** devuelve la ficha elegida a su estado habitual: se muestra, se muestran sus grupos, y se eliminan los grupos que añadió. Una ficha propia se elimina. **Restablecer todo** devuelve la cinta completa, incluido el orden de las fichas.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffCrunch muestran su cinta, y también todas las ventanas que abra después. JeffCrunch guarda solo lo que difiere de la cinta habitual, de modo que una ficha o grupo que añada una versión posterior aparece por sí solo. Sus fichas y grupos reciben KeyTips (las letras que aparecen al presionar **Alt**) que no coinciden con los demás. Puede tener 20 fichas propias, 30 grupos propios en una ficha y 60 comandos en un grupo.

### Personalizar la barra de herramientas rápida

La página **Barra de herramientas rápida** sirve para la pequeña barra de herramientas situada arriba a la izquierda de la ventana. Su lista izquierda muestra lo que hay ahora en la barra, y la lista **Comandos** de la derecha tiene todos los comandos que tienen nombre (escriba en el cuadro **Buscar comandos** para reducirla, igual que en la página **Cinta de opciones**).

- **Agregar un comando.** Elíjalo en la lista **Comandos** y haga clic en **Agregar comando**. Se añade al final de la barra. Un comando que abre un menú conserva su menú en la barra. Un comando solo puede estar una vez en la barra.
- **Clic derecho.** También puede hacer clic derecho en cualquier comando de la cinta y elegir **Agregar a la barra de herramientas rápida**, o hacer clic derecho en un botón de la barra y elegir **Quitar de la barra de herramientas rápida**. El mismo menú tiene **Personalizar la barra de herramientas rápida** y **Personalizar la cinta de opciones**, que abren esta página.
- **Colocarla debajo de la cinta.** Marque **Mostrar la barra de herramientas rápida debajo de la cinta de opciones** para colocar la barra en una fila bajo la cinta, en lugar de arriba a la izquierda. Los dos menús de clic derecho también tienen **Mostrar la barra de herramientas rápida debajo de la cinta de opciones**. JeffCrunch recuerda la elección.
- **Agregar un separador.** Haga clic en **Agregar separador** para poner una línea fina al final, y luego muévala donde quiera.
- **Reordenar.** Elija un comando o un separador en la lista izquierda y haga clic en **Subir** o **Bajar**.
- **Quitar.** Elíjalo y haga clic en **Quitar**.

**Restablecer una** devuelve el comando elegido a su sitio habitual en JeffCrunch, o lo quita de la barra si JeffCrunch no suele tenerlo allí. **Restablecer todo** devuelve la barra con la que viene JeffCrunch. Restablecer todo en esta página no cambia su cinta, y Restablecer todo en la página **Cinta de opciones** no cambia su barra.

Nada cambia hasta que haga clic en **Aceptar**. Entonces, todas las ventanas abiertas de JeffCrunch muestran su barra, y también todas las ventanas que abra después. La barra puede tener 100 comandos y separadores en total. Cuando hay más botones de los que caben en un tercio del ancho de la ventana, el resto espera tras un botón **»** al final de la barra, de modo que el título de la ventana nunca se desplaza fuera de la pantalla. JeffCrunch conserva en su lista un comando que una versión posterior ya no tiene, y lo vuelve a mostrar si el comando regresa. Los botones reciben KeyTips, los números del 1 al 9 y después dos letras, en el orden en que están en la barra. La barra siempre se sitúa arriba a la izquierda, junto al botón **Archivo**.

## Opciones de privacidad

- **Quitar la información personal de este archivo al guardarlo** elimina el autor, la persona que guardó el archivo por última vez, la organización, el responsable, y los nombres y las horas de las notas y comentarios, del archivo que se escribe (un libro, una copia y un libro enviado por correo electrónico). Lo que dice el archivo se mantiene igual. Las propiedades personalizadas del archivo no se modifican, y el documento que tiene abierto no cambia. Es la misma opción en todas las aplicaciones de JeffOffice: si la cambia en una, cambia en todas. Para hacerlo una sola vez, use **Inspeccionar libro** en [Información](info). El mismo cuadro está en la página **Información**.

## Estadísticas de uso

**Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffCrunch. Anónimas significa que los datos no dicen nada sobre quién es usted. Cuando está activado, JeffCrunch envía estos datos una vez al día, y el día que actualiza: que es JeffCrunch, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de archivo abrió o guardó. Si JeffCrunch alguna vez se cierra inesperadamente, también envía un informe de error la próxima vez que se inicia. JeffCrunch nunca envía sus archivos, sus nombres ni nada de lo que contienen, ni lo que escribe. Si esta opción está desactivada, tampoco se envía ningún informe de error.

La elección es común a todas las aplicaciones de JeffOffice: si la cambia aquí, cambia en cada una de ellas. JeffCrunch le pregunta por ella la primera vez que se inicia, en una ventana llamada **Ayude a mejorar JeffCrunch**.

- **Aviso de privacidad**, junto a la casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. Explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** se muestra bajo la casilla. Es un número aleatorio generado para su copia de JeffOffice, el mismo en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si aún no se ha generado ningún número, muestra **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas se borra el número de su equipo, y sin él no podemos saber qué registros son suyos.
- Para desactivar las estadísticas sin abrir JeffCrunch, defina la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Entonces no se cuenta ni se envía nada, y JeffCrunch no hace la pregunta de la primera vez.

## Seguridad

La ficha **Seguridad** decide qué hace JeffCrunch con los elementos a los que puede apuntar un archivo: archivos vinculados, películas y sonidos vinculados, y archivos de Internet. Las macros se guardan en un archivo y nunca se ejecutan, y un vínculo que iniciaría un programa siempre se rechaza. Véase [Seguridad](security).

## Relacionado

- [Bienvenido a JeffCrunch](start)
