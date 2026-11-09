# Cambiar la configuración de JeffCrunch
<!-- keywords: recuperación, recuperar trabajo no guardado, recuperación automática, revisión, edición, autocompletar, controlador de relleno, arrastrar y colocar, posiciones decimales, opciones, configuración, preferencias, estadísticas de uso, número de instalación, privacidad, aviso de privacidad, JP_NO_TELEMETRY, eliminar estadísticas, actualizaciones, acerca de, editar en la celda, color de las líneas de cuadrícula, zoom predeterminado, ancho estándar, privacidad, información personal -->

Su configuración está en la ventana **Opciones de JeffCrunch**. Haga clic en **Archivo > Opciones** para abrirla. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## Edición

La página **Edición** cambia el modo en que funciona la escritura en las celdas.

- **Después de pulsar Entrar, mover la selección** indica adónde va la selección cuando termina de escribir una entrada: hacia abajo, hacia la derecha, hacia arriba, hacia la izquierda o a ninguna parte.
- **Ofrecer completar un texto con las celdas de arriba y de abajo** se llama autocompletar. Cuando escribe las primeras letras de un texto y solo un texto de esa columna empieza por ellas, JeffCrunch muestra el resto, seleccionado. Pulse **Entrar** para conservarlo. Siga escribiendo para ignorarlo, o pulse **Retroceso** para quitarlo. Solo mira el bloque de celdas junto a la que está escribiendo, y solo textos, no números.
- **Usar el controlador de relleno y arrastrar celdas para moverlas o copiarlas** activa el pequeño cuadrado de la esquina de la selección y el arrastre del borde de la selección. Desactívelo si mueve celdas por accidente. **Copiar** y **Pegar** siguen funcionando.
- **Mostrar un cero en las celdas que tienen valor cero (esta hoja)** activa o desactiva los ceros en la hoja que está viendo. Cuando está desactivado, una celda que contiene 0 parece vacía, pero sigue conteniendo el 0 y se suma como 0. La opción se guarda con la hoja.
- **Poner un punto decimal en los números enteros que se escriben** se llama posiciones decimales automáticas. Si establece **Posiciones decimales** en 2, al escribir 1234 se introduce 12,34. Si lo establece en -2, al escribir 5 se introduce 500. Un número que escribe con punto decimal se introduce tal como lo escribe.
- **Permitir la edición directamente en las celdas** está activada de forma predeterminada. Si la desactiva, una entrada escrita o F2 va a la barra de fórmulas, y la celda muestra el resultado solo cuando termina.

## Opciones de guardado

**Guardar información de recuperación automática cada** establece con qué frecuencia JeffCrunch guarda una copia de un libro con cambios que no ha guardado, de 1 a 120 minutos. La copia es un fichero .xlsx en una carpeta que JeffCrunch reserva para este fin, y se crea en segundo plano para que pueda seguir escribiendo. Se elimina al guardar o cerrar el libro. Si JeffCrunch se cierra inesperadamente, ofrece las copias la próxima vez que se inicia. También puede hacer clic en **Archivo > Abrir > Recuperar trabajo no guardado**. Un libro protegido con contraseña no se copia, porque la copia no quedaría protegida.

## Opciones de revisión

La página **Revisión** es compartida por las aplicaciones de JeffOffice. Decide si la ortografía se comprueba mientras escribe en una celda (desactive **Comprobar ortografía mientras escribe** para quitar las líneas rojas onduladas; **Ortografía** en la pestaña Revisar sigue funcionando), si las palabras en mayúsculas y las palabras con números se omiten al comprobar la ortografía, si la Autocorrección corrige palabras mientras escribe, si las comillas rectas se convierten en tipográficas, si los guiones, las fracciones y los ordinales se cambian, y si una dirección web se convierte en un vínculo. Consulte [Ortografía](notes-links) y [Escribir en celdas](edit-cells).

## Opciones de pantalla

- **Color de las líneas de cuadrícula** es el color de las líneas entre celdas en la pantalla. Es una opción solo para su pantalla, que no se guarda en el libro.
- **Zoom de un libro nuevo** es el tamaño con el que se ven las hojas de un libro nuevo, de 10 % a 400 %.
- **Ancho de columna estándar (esta hoja)** es el ancho de todas las columnas de esta hoja que no tienen un ancho propio. Es lo mismo que **Ancho predeterminado** en la pestaña Inicio.

## Opciones de privacidad

- **Quitar mi nombre y empresa de las propiedades de un libro cada vez que se guarde** borra el autor, quién lo guardó por última vez, la empresa y el administrador de las propiedades del libro al guardarlo. Las propiedades son los datos que un fichero guarda sobre sí mismo.

## Estadísticas de uso

**Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffCrunch. Anónimo significa que los datos no dicen nada sobre quién es usted. Cuando está activado, JeffCrunch envía estos datos una vez al día, y el día que actualiza: que es JeffCrunch, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de fichero abrió o guardó. Si JeffCrunch se cierra inesperadamente, también envía un informe de error la próxima vez que se inicia. JeffCrunch nunca envía sus ficheros, sus nombres ni nada de lo que contienen, ni nada de lo que escribe. Si está desactivado, tampoco se envía ningún informe de error.

La elección es común a todas las aplicaciones de JeffOffice: si la cambia aquí, cambia en todas. JeffCrunch le pregunta por ella la primera vez que se inicia, en una ventana llamada **Ayude a mejorar JeffCrunch**.

- **Aviso de privacidad**, junto a la casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. En él se indica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** se muestra debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, igual en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o aún no se ha generado ningún número, muestra **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas, el número se borra de su equipo, y sin él no podemos saber qué registros son suyos.
- Para desactivar las estadísticas sin abrir JeffCrunch, defina la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Entonces no se cuenta ni se envía nada, y JeffCrunch no le hace la pregunta de la primera vez.

## Relacionado

- [Bienvenido a JeffCrunch](start)
