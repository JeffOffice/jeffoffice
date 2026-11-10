# Cambiar la configuración de JeffCrunch
<!-- keywords: recuperación, recuperar trabajo no guardado, recuperación automática, revisión, edición, autocompletar, controlador de relleno, arrastrar y colocar, posiciones decimales, opciones, configuración, preferencias, estadísticas de uso, número de instalación, privacidad, aviso de privacidad, JP_NO_TELEMETRY, eliminar estadísticas, actualizaciones, acerca de, editar en celda, color de las líneas de cuadrícula, zoom predeterminado, ancho estándar, privacidad, información personal -->

Su configuración está en la ventana **Opciones de JeffCrunch**. Haga clic en **Archivo > Opciones** para abrirla. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## Edición

La página **Edición** cambia el modo en que funciona la escritura en las celdas.

- **Después de presionar Entrar, mover la selección** indica adónde va la selección cuando termina una entrada: hacia abajo, a la derecha, hacia arriba, a la izquierda o no se mueve.
- **Ofrecer completar un texto con las celdas de arriba y de abajo** se llama Autocompletar. Cuando escribe las primeras letras de un texto, y solo un texto de esa columna empieza por ellas, JeffCrunch muestra el resto, seleccionado. Presione **Entrar** para conservarlo. Siga escribiendo para ignorarlo, o presione **Retroceso** para quitarlo. Solo se fija en el bloque de celdas junto a la que está escribiendo, y solo en textos, no en números.
- **Usar el controlador de relleno y arrastrar celdas para moverlas o copiarlas** activa el pequeño cuadrado de la esquina de la selección y el arrastre del borde de la selección. Desactívelo si mueve celdas por accidente. Copiar y Pegar siguen funcionando.
- **Mostrar un cero en las celdas que tienen valor cero (esta hoja)** activa o desactiva los ceros en la hoja que está viendo. Cuando está desactivado, una celda que contiene 0 parece vacía, pero sigue conteniendo el 0 y se suma como 0. La opción se guarda con la hoja.
- **Poner un punto decimal en los números enteros que se escriben** se llama posiciones decimales automáticas. Si establece **Posiciones** en 2, al escribir 1234 se introduce 12,34. Si lo establece en -2, al escribir 5 se introduce 500. Un número que escribe con punto decimal se introduce tal como lo escribe.

- **Permitir editar directamente en las celdas** está activado de forma predeterminada. Si lo desactiva, lo que escriba o F2 va a la barra de fórmulas, y la celda muestra el resultado solo cuando termina la entrada.

## Opciones de guardado

**Guardar información de recuperación automática cada** indica cada cuánto JeffCrunch guarda una copia de un libro con cambios que no ha guardado, de 1 a 120 minutos. La copia es un archivo .xlsx en una carpeta que JeffCrunch reserva para este fin, y se crea en segundo plano para que pueda seguir escribiendo. Se elimina cuando guarda o cierra el libro. Si JeffCrunch se cierra inesperadamente, ofrece las copias la próxima vez que se inicia. También puede hacer clic en **Archivo > Abrir > Recuperar trabajo no guardado**. Un libro protegido con contraseña no se copia, porque la copia no estaría protegida.

## Opciones de revisión

La página **Revisión** es compartida por las aplicaciones de JeffOffice. Decide si la ortografía se comprueba mientras escribe en una celda (desactive **Comprobar ortografía mientras escribe** para quitar las líneas rojas onduladas de todas las ventanas abiertas y de las demás aplicaciones; **Ortografía** en la pestaña Revisar sigue funcionando), si se ignoran las palabras en mayúsculas y las que contienen números al comprobar la ortografía, si la Autocorrección corrige palabras mientras escribe, si las comillas rectas se convierten en tipográficas, si los guiones, las fracciones y los ordinales se cambian, y si una dirección web se convierte en un vínculo. Consulte [Ortografía](notes-links) y [Escribir en celdas](edit-cells).

## Opciones de visualización

- **Color de las líneas de cuadrícula** es el color de las líneas entre celdas en su pantalla. Es una opción solo para su pantalla y no se guarda en el libro.
- **Zoom de un libro nuevo** indica el tamaño con que se ven las hojas de un libro nuevo, de 10 % a 400 %.
- **Ancho de columna estándar (esta hoja)** es el ancho de cada columna de esta hoja que no tiene un ancho propio. Es lo mismo que **Ancho predeterminado** en la pestaña Inicio.

## Opciones de privacidad

- **Quitar mi nombre y mi empresa de las propiedades de un libro cada vez que se guarde** borra del libro, al guardarlo, el autor, quien lo guardó por última vez, la empresa y el administrador. Las propiedades son los datos que un archivo guarda sobre sí mismo.

## Estadísticas de uso

**Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffCrunch. Anónimas significa que los datos no dicen nada sobre quién es usted. Cuando está activado, JeffCrunch envía estos datos una vez al día, y el día en que se actualiza: que es JeffCrunch, su versión, su sistema operativo y su idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de archivo abrió o guardó. Si JeffCrunch se cierra inesperadamente, también envía un informe de error la próxima vez que se inicia. JeffCrunch nunca envía sus archivos, sus nombres ni nada de lo que contienen, ni lo que haya escrito. Si esta opción está desactivada, tampoco se envía ningún informe de error.

La elección es común a todas las aplicaciones de JeffOffice: si la cambia aquí, cambia en todas ellas. JeffCrunch le pregunta por ella la primera vez que se inicia, en una ventana llamada **Ayude a mejorar JeffCrunch**.

- **Aviso de privacidad**, junto a la casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. En él se explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** aparece debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, igual en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si todavía no se ha generado ningún número, muestra **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas se borra el número de su equipo, y sin él no podemos saber cuáles son sus registros.
- Para desactivar las estadísticas sin abrir JeffCrunch, establezca la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Entonces no se cuenta ni se envía nada, y JeffCrunch no hace la pregunta de la primera vez.

## Temas relacionados

- [Bienvenido a JeffCrunch](start)
