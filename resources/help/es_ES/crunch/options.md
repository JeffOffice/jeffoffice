# Cambiar la configuración de JeffCrunch

<!-- keywords: edición, autocompletar, controlador de relleno, arrastrar y colocar, posiciones decimales, opciones, configuración, preferencias, estadísticas de uso, número de instalación, privacidad, aviso de privacidad, JP_NO_TELEMETRY, eliminar estadísticas, actualizaciones, acerca de, editar en celda, color de las líneas de cuadrícula, zoom predeterminado, ancho estándar, privacidad, información personal -->

Su configuración está en la ventana **Opciones de JeffCrunch**. Haga clic en **Archivo > Opciones** para abrirla. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## Edición

La página **Edición** cambia el modo en que funciona la escritura en las celdas.

- **Después de pulsar Entrar, mover la selección** indica adónde va la selección al terminar una entrada: hacia abajo, a la derecha, hacia arriba, a la izquierda o no se mueve.
- **Ofrecer completar un texto con las celdas de arriba y de abajo** se llama autocompletar. Cuando escribe las primeras letras de un texto y solo un texto de esa columna empieza por ellas, JeffCrunch muestra el resto, seleccionado. Pulse **Entrar** para conservarlo. Siga escribiendo para ignorarlo, o pulse **Retroceso** para quitarlo. Solo mira el bloque de celdas junto a la que está escribiendo, y solo los textos, no los números.
- **Usar el controlador de relleno y arrastrar celdas para mover o copiar** activa el pequeño cuadrado de la esquina de la selección y el arrastre del borde de la selección. Desactívelo si mueve celdas por accidente. Copiar y Pegar siguen funcionando.
- **Mostrar un cero en las celdas que tengan valor cero (esta hoja)** muestra u oculta los ceros de la hoja que está viendo. Cuando está desactivado, una celda que contiene 0 parece vacía, pero sigue conteniendo el 0 y se suma como 0. La opción se guarda con la hoja.
- **Poner un separador decimal en los números enteros que se escriben** se llama posiciones decimales automáticas. Si establece **Posiciones decimales** en 2, al escribir 1234 se introduce 12,34. Si lo establece en -2, al escribir 5 se introduce 500. Un número que escriba con separador decimal se introduce tal como lo escribió.

- **Permitir editar directamente en las celdas** está activado de forma predeterminada. Si lo desactiva, una entrada escrita o F2 va a la barra de fórmulas, y la celda muestra el resultado solo cuando termina la entrada.

## Opciones de visualización

- **Color de las líneas de cuadrícula** es el color de las líneas entre celdas en la pantalla. Es una opción solo para su pantalla, que no se guarda en el libro.
- **Zoom de un libro nuevo** indica el tamaño con que se ven las hojas de un libro nuevo, del 10 % al 400 %.
- **Ancho de columna estándar (esta hoja)** es el ancho de todas las columnas de esta hoja que no tienen un ancho propio. Es lo mismo que **Ancho predeterminado** en la ficha Inicio.

## Opciones de privacidad

- **Quitar mi nombre y la empresa de las propiedades de un libro cada vez que se guarda** borra del libro el autor, quién lo guardó por última vez, la empresa y el administrador cuando lo guarda. Las propiedades son los datos que un fichero guarda sobre sí mismo.

## Estadísticas de uso

**Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffCrunch. Anónimas significa que los datos no dicen nada sobre quién es usted. Cuando está activado, JeffCrunch envía estos datos una vez al día y el día en que se actualiza: que se trata de JeffCrunch, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de fichero abrió o guardó. Si JeffCrunch se cierra inesperadamente, también envía un informe de error la próxima vez que se inicie. JeffCrunch nunca envía sus ficheros, sus nombres ni nada de su contenido, ni lo que escriba. Si esta opción está desactivada, tampoco se envía ningún informe de error.

La elección afecta a todas las aplicaciones de JeffOffice: si la cambia aquí, cambia en todas. JeffCrunch le pregunta por ella la primera vez que se inicia, en una ventana llamada **Ayude a mejorar JeffCrunch**.

- **Aviso de privacidad**, junto a la casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. En él se explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** aparece debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, igual en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si todavía no se ha generado ningún número, aparece **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas, el número se borra de su equipo, y sin él no podemos saber cuáles de los registros son suyos.
- Para desactivar las estadísticas sin abrir JeffCrunch, establezca la variable de entorno `JP_NO_TELEMETRY` con cualquier valor antes de iniciarlo. Entonces no se cuenta ni se envía nada, y JeffCrunch no hace la pregunta de la primera vez.

## Relacionado

- [Bienvenido a JeffCrunch](start)
