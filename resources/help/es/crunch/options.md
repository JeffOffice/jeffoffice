# Cambiar la configuración de JeffCrunch
<!-- keywords: edición, autocompletar, controlador de relleno, arrastrar y colocar, posiciones decimales, opciones, configuración, preferencias, estadísticas de uso, número de instalación, privacidad, aviso de privacidad, JP_NO_TELEMETRY, eliminar estadísticas, actualizaciones, acerca de -->

Su configuración está en la ventana **Opciones de JeffCrunch**. Haga clic en **Archivo > Opciones** para abrirla. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

## Edición

La página **Edición** cambia el modo en que funciona la escritura en las celdas.

- **Después de presionar Entrar, mover la selección** indica adónde pasa la selección al terminar de escribir una entrada: abajo, a la derecha, arriba, a la izquierda o no se mueve.
- **Ofrecer completar un texto con las celdas superior e inferior** se llama Autocompletar. Cuando escribe las primeras letras de un texto, y solo un texto de esa columna empieza así, JeffCrunch muestra el resto, seleccionado. Presione **Entrar** para conservarlo. Siga escribiendo para ignorarlo, o presione **Retroceso** para quitarlo. Solo busca en el bloque de celdas contiguo a la que está escribiendo, y solo en textos, no en números.
- **Usar el controlador de relleno y arrastrar celdas para mover o copiar** activa el pequeño cuadrado de la esquina de la selección y el arrastre del borde de la selección. Desactívelo si mueve celdas por accidente. Copiar y Pegar siguen funcionando.
- **Insertar un separador decimal en los números enteros escritos** se llama posiciones decimales automáticas. Si establece **Posiciones decimales** en 2, al escribir 1234 se introduce 12,34. Si lo establece en -2, al escribir 5 se introduce 500. Un número que escriba con separador decimal se introduce tal como lo escribe.

## Estadísticas de uso

**Enviar estadísticas de uso anónimas** le permite compartir datos sobre cómo se usa JeffCrunch. Anónimo significa que los datos no revelan quién es usted. Cuando esta opción está activada, JeffCrunch envía estos datos una vez al día y el día en que se actualiza: que se trata de JeffCrunch, su versión, su sistema operativo e idioma, cuántas veces se ha iniciado, con qué frecuencia se usa cada comando y qué tipo de archivo abrió o guardó. Si JeffCrunch se cierra de forma inesperada, también envía un informe de error la próxima vez que se inicie. JeffCrunch nunca envía sus archivos, sus nombres ni nada de lo que contienen, ni lo que escribe. Si esta opción está desactivada, tampoco se envía ningún informe de error.

La elección afecta a todas las aplicaciones de JeffOffice: si la cambia aquí, cambia en todas. JeffCrunch le pregunta por esta opción la primera vez que se inicia, en una ventana llamada **Ayudar a mejorar JeffCrunch**.

- **Aviso de privacidad**, junto a la casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. En él se explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** se muestra debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, igual en todas las aplicaciones de JeffOffice. No dice nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas, o si todavía no se ha generado ningún número, muestra **ninguno**.
- Para que se eliminen las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas, el número se borra de su equipo, y sin él no podemos saber qué registros son suyos.
- Para desactivar las estadísticas sin abrir JeffCrunch, establezca la variable de entorno `JP_NO_TELEMETRY` en cualquier valor antes de iniciarlo. Entonces no se cuenta ni se envía nada, y JeffCrunch no hace la pregunta de la primera vez.

## Relacionado

- [Bienvenido a JeffCrunch](start)
