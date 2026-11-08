# Cambiar la configuración de JeffPub
<!-- keywords: opciones, configuración, preferencias, ajustes, tema, modo oscuro, unidades, pulgadas, centímetros, recuperación automática, copia de seguridad, idioma, nombre de usuario, desplazar, actualizaciones, beta, estadísticas de uso, acerca de -->

La configuración está en la ventana **Opciones de JeffPub**. Haga clic en **Archivo > Opciones** para abrirla. Tiene cuatro pestañas. Haga clic en **Aceptar** para conservar los cambios o en **Cancelar** para descartarlos.

## General

- **Nombre de usuario** e **Iniciales** se guardan como autor de las publicaciones nuevas y se muestran en los comentarios.
- **Tema** puede ser **Usar la configuración del sistema**, **Claro** u **Oscuro**. El cambio se aplica al instante.
- **Idioma de presentación** es el idioma de los textos de JeffPub. **Usar el idioma del sistema** sigue el idioma del equipo. En esta lista aparecen otros idiomas cuando hay traducciones disponibles. El nuevo idioma se aplica la próxima vez que abra JeffPub.
- **Enviar estadísticas de uso anónimas** y **Buscar actualizaciones al iniciar JeffPub** son las mismas opciones que hay en la página **Acerca de** (más abajo).

**Nombre de usuario**, **Iniciales**, **Tema** e **Idioma de presentación**, así como las opciones de actualizaciones y estadísticas, son comunes a todas las aplicaciones de JeffOffice: si cambia una aquí, cambia en todas ellas.

## Revisión

Estas casillas controlan la ayuda con la ortografía y la escritura. Vea [Revisar la ortografía y la división de palabras](proofing).

- **Revisar la ortografía mientras escribe**
- **Omitir palabras en MAYÚSCULAS**
- **Omitir palabras que contienen números**
- **Reemplazar texto mientras escribe (Autocorrección)** corrige errores de escritura habituales.
- **Reemplazar comillas rectas por comillas tipográficas** convierte " y ' en comillas tipográficas.
- **Autoformato mientras escribe (guiones, fracciones, ordinales, listas automáticas)**

## Guardar

- **Guardar información de recuperación automática cada** establece con qué frecuencia JeffPub crea copias de reserva del trabajo no guardado, de 1 a 120 minutos. Vea [Recuperar trabajo no guardado](recover).
- **Crear siempre una copia de seguridad** conserva el fichero anterior como "Copia de seguridad de" seguido de su nombre cada vez que guarda.

## Avanzado

- **Unidades de medida** puede ser **Pulgadas**, **Centímetros**, **Milímetros**, **Puntos** o **Picas**. Determina las reglas y todos los cuadros de medidas. Mientras no elija ninguna, JeffPub usa pulgadas si la región del equipo mide en pulgadas, como en Estados Unidos, y centímetros en el resto. Un punto equivale a 1/72 de pulgada y una pica, a 12 puntos.
- **Las teclas de dirección desplazan los objetos en** establece cuánto se mueve un objeto seleccionado con cada pulsación de una tecla de dirección.
- **Mostrar este número de publicaciones recientes** indica cuántos nombres muestra la página **Abrir**, de 0 a 50.
- **Al seleccionar, seleccionar automáticamente la palabra completa** selecciona palabras completas al arrastrar sobre el texto.
- **Permitir arrastrar y colocar texto** le permite mover el texto seleccionado arrastrándolo.
- **Dividir palabras automáticamente en los cuadros de texto nuevos** activa la división de palabras, que corta las palabras largas al final de la línea, en los cuadros de texto que agregue.

## Actualizaciones, estadísticas y su nombre

Haga clic en **Archivo > Acerca de** para encontrar estas opciones. Las dos casillas también están en la pestaña **General** de **Archivo > Opciones**, junto con una tercera, **Obtener versiones beta**.

- **Buscar actualizaciones** busca ahora una versión más reciente. Si hay una disponible, la ventana **Actualización disponible** muestra las novedades. Elija **Actualizar ahora**, **Más tarde** u **Omitir esta versión**.
- **Obtener versiones beta (compilaciones preliminares para pruebas)** hace que JeffPub ofrezca también versiones beta, que salen antes que las normales y pueden tener algunos fallos.
- **Buscar actualizaciones automáticamente** hace que JeffPub las busque cada vez que se inicia.
- **Enviar estadísticas de uso anónimas** le permite decidir si comparte datos sobre cómo se usa JeffPub. Si está activada, JeffPub los envía una vez al día y el día en que se actualiza: su versión, su sistema operativo e idioma, y la frecuencia con que se usa cada comando. Si JeffPub se cierra inesperadamente, también envía un informe de bloqueo la siguiente vez que se inicia. El informe indica en qué parte del programa se detuvo, para poder encontrar y corregir el problema. Nunca envía sus ficheros, sus nombres ni su contenido, ni nada de lo que haya escrito. Haga clic en **Qué se envía** para obtener más información. JeffPub pregunta por esto la primera vez que se inicia, en una ventana llamada **Ayude a mejorar JeffPub**. En Windows, puede que el programa de instalación pregunte antes. La opción se aplica a todas las aplicaciones de JeffOffice: JeffDeck envía el mismo tipo de datos sobre sí mismo, solo si eligió compartirlos.
- **Aviso de privacidad**, junto a esa casilla, abre nuestro aviso de privacidad en jeffoffice.com/privacy. Explica qué se envía, adónde va y cuánto tiempo lo conservamos. Conservamos las estadísticas durante 24 meses.
- **Número de instalación** aparece debajo de la casilla. Es un número aleatorio generado para su copia de JeffOffice, el mismo en todas las aplicaciones de JeffOffice. No revela nada sobre usted. Haga clic en **Copiar** para copiarlo. Si las estadísticas están desactivadas o aún no se ha generado ningún número, se muestra **ninguno**.
- Para que eliminemos las estadísticas que ya tenemos, envíe su número de instalación a info@jeffoffice.com **antes** de desactivar las estadísticas. Al desactivarlas se borra el número del equipo y, sin él, no podemos saber qué registros son suyos.
- Para desactivar las estadísticas de forma permanente sin abrir JeffPub, asigne cualquier valor a la variable de entorno `JP_NO_TELEMETRY` antes de iniciarlo. Así no se cuenta ni se envía nada, y JeffPub no hace la pregunta del primer inicio.
- **Su nombre** es el mismo nombre que **Nombre de usuario**, más arriba.

## Temas relacionados

- [Recuperar trabajo no guardado](recover)
- [Revisar la ortografía y la división de palabras](proofing)
- [Obtener ayuda e informar de problemas](support)
