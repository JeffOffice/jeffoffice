# La pantalla Archivo

<!-- keywords: archivo, pantalla archivo, vista backstage, nuevo, abrir, guardar, guardar como, imprimir, exportar, cerrar, opciones, acerca de, versión, licencia, informar de un problema, código fuente, descargas, volver a la presentación, información, propiedades, título, autor, asunto, palabras clave, categorías, comentarios, empresa, administrador, propiedades personalizadas, marcar como final, final, solo lectura, contraseña, cifrar, proteger, contraseña de apertura, inspeccionar presentación, información personal, notas del orador, diapositivas ocultas, fuera de la diapositiva, historial de versiones, versiones, guardar una versión, volver, restaurar, mostrar en carpeta, copiar ruta, comprobar compatibilidad, guardar de todos modos, no volver a preguntar, abrir siempre como solo lectura, fichero, pantalla fichero -->

El botón **Archivo**, a la izquierda de las pestañas, abre la pantalla Archivo. Ocupa toda la ventana y reúne todo lo que actúa sobre el fichero completo: iniciar, abrir, guardar, imprimir y exportar. Pulse Esc, o haga clic en **Volver a la presentación**, para regresar a sus diapositivas. **Alt+A** también la abre.

A la izquierda hay una lista. Algunas entradas abren una página, y otras realizan su función de inmediato.

## Inicio

- **Nuevo** muestra una **Presentación en blanco** para hacer clic en ella. Consulte [Abrir y guardar presentaciones](open-save).
- **Abrir** tiene **Examinar** para elegir un fichero, **Recuperar trabajo no guardado** para recuperar las copias que guardó la recuperación automática, y las listas **Anclados** y **Presentaciones recientes**. Consulte [Abrir y guardar presentaciones](open-save) y [Recuperar trabajo no guardado](recover).

## Esta presentación

- **Información** muestra los datos de la presentación y la protege antes de compartirla. Consulte la sección Información más abajo.
- **Guardar** y **Guardar como** guardan el fichero, y la pantalla se cierra al terminar. Consulte [Abrir y guardar presentaciones](open-save).
- **Imprimir** es la página Imprimir, con vista previa. Consulte [Imprimir, PDF e imágenes de diapositivas](printing).
- **Exportar** tiene **Exportar a PDF**, **Exportar diapositiva como imagen** y **Crear un correo electrónico**. Consulte [Imprimir, PDF e imágenes de diapositivas](printing).
- **Cerrar** cierra la ventana de la presentación. JeffDeck pregunta antes si desea guardar los cambios.

## Información

Haga clic en **Información** para ver los datos de su presentación y protegerla antes de compartirla.

En la parte superior, la página muestra dónde está guardado el fichero. Haga clic en **Mostrar en carpeta** para abrir esa carpeta. Haga clic en **Copiar ruta** para copiar la ruta y pegarla en un correo electrónico o en un chat.

### Propiedades

Las propiedades son datos de la presentación, como su título o quién la escribió. La página tiene un cuadro para cada una: **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Empresa** y **Administrador**. Escriba en un cuadro y pulse **Tab** o **Entrar**. JeffDeck conserva el cambio y lo guarda la próxima vez que guarde la presentación. **Ctrl+Z** lo deshace.

Debajo de los cuadros, la página muestra cuándo se creó, guardó e imprimió la presentación, cuántas diapositivas tiene, cuántas están ocultas y cuántas tienen notas del orador. Haga clic en **Propiedades personalizadas** para crear propiedades propias, como **Proyecto** o **Cliente**. Haga clic en **Agregar**, escriba un nombre, elija qué tipo de valor es (texto, número entero, número, sí o no, o fecha) y escriba el valor.

### Abrir siempre como solo lectura

Haga clic en **Abrir siempre como solo lectura** para que la presentación se abra solo para lectura cada vez, de modo que nadie la cambie por accidente. La elección se guarda con el fichero al guardarlo. La próxima vez que alguien abra el fichero, la barra de estado indicará que está abierto para lectura. Para editarlo, abra **Archivo > Información** y haga clic en **Se abre siempre como solo lectura**. Es un solo clic, así que es un recordatorio y no un bloqueo. Otros programas no leen esta elección, y un fichero .odp no puede guardarla.

### Marcar como final

Haga clic en **Marcar como final** para indicar a los lectores que la presentación está terminada. JeffDeck guarda la presentación y después la mantiene solo para lectura. Cuando alguien la abre, la barra de título indica que es de solo lectura y una nota le informa de que se marcó como final. Es una forma suave de decir «por favor, no lo cambie». No es un bloqueo, porque cualquiera puede volver a elegir **Marcado como final** para editarla. Un fichero **.odp** no puede guardar la marca, así que guarde antes la presentación como fichero **.pptx**.

### Cifrar con contraseña

Haga clic en **Cifrar con contraseña** para que se pida una contraseña cada vez que se abra el fichero. Escriba la contraseña dos veces y haga clic en **Proteger**. El fichero se codifica al guardarlo, de modo que nadie puede leerlo sin la contraseña.

- Si olvida la contraseña, **no se puede recuperar**. Guárdela en un lugar seguro.
- Para cambiar la contraseña, haga clic de nuevo en la misma tarjeta. Haga clic en **Quitar contraseña** para eliminarla. El cambio se guarda la próxima vez que guarde.
- Las contraseñas funcionan con ficheros **.pptx**, **.potx** y **.ppsx**, y con ficheros **.odp** y **.otp**. Un fichero .odp se protege de la forma que describe el estándar OpenDocument, de modo que otros programas que siguen el estándar también pueden abrirlo con la contraseña.
- Las versiones que guardó y la copia de seguridad siguen a la contraseña: están protegidas con ella y cambian cuando usted la cambia o la quita.
- Mientras una presentación tiene contraseña, JeffDeck no guarda copia de recuperación de ella, porque esa copia no estaría protegida. Guarde con frecuencia.

Para abrir un fichero protegido, ábralo como de costumbre. JeffDeck le pedirá la contraseña. Si no es correcta, puede intentarlo de nuevo o hacer clic en **Cancelar**. Si el fichero está dañado, JeffDeck se lo indicará.

### Comprobar compatibilidad

Haga clic en **Comprobar compatibilidad** para ver qué no puede guardar un fichero **.odp** de su presentación, sin guardar nada. La lista procede del mismo lugar que usa JeffDeck al guardar, así que es exacta. Si no aparece nada, no se pierde nada.

Al guardar como fichero .odp o .otp, JeffDeck muestra esa lista **antes** de escribir nada. Elija **Guardar de todos modos**, **Guardar como .pptx** (el fichero .pptx se guarda junto a él, y no se pierde nada) o **Cancelar**. Marque **No volver a preguntar para este fichero** y JeffDeck dejará de preguntar por ese fichero hasta que cierre el programa.

### Inspeccionar presentación

Haga clic en **Inspeccionar presentación** antes de enviar una presentación a alguien. Muestra lo que contiene el fichero que quizá no desee compartir, y cuántos elementos de cada tipo encontró:

- **Comentarios** son notas que otras personas dejaron en las diapositivas.
- **Notas del orador** son las notas que escribió para la persona que da la charla.
- **Propiedades del documento e información personal** son el autor, la persona que lo guardó por última vez, la empresa y el administrador.
- **Diapositivas ocultas** son diapositivas que la presentación omite. **Mostrar todo** las vuelve a mostrar.
- **Contenido fuera de las diapositivas** es texto, imágenes y formas que quedan fuera de una diapositiva. No se muestran, pero están en el fichero.
- **Encabezados y pies de página** son el texto del pie y la fecha fija al pie de las diapositivas. **Eliminar todo** borra ese texto y deja los lugares, de modo que el número de diapositiva se conserva. No se revisan el encabezado, el pie ni la fecha de las páginas de notas y de los documentos impresos.
- **Datos XML personalizados** son datos que otros programas añadieron al fichero y que no se ven en una diapositiva. JeffDeck no los conserva, así que una presentación que guarda no los tiene. El recuento corresponde a lo que tenía el fichero al abrirlo. **Eliminar todo** lo deja así para la presentación que tiene abierta.

Haga clic en **Quitar todo** junto a un tipo para eliminarlo. Puede deshacerlo con **Ctrl+Z**. Guarde la presentación después para conservar la copia limpia.

**Quitar la información personal de este fichero al guardarlo**, debajo de las tarjetas, lo hace cada vez que guarda, para cualquier fichero, sin cambiar el documento que tiene abierto. Es la misma casilla que en **Opciones**, y es igual en todas las aplicaciones de JeffOffice.

### Historial de versiones

Haga clic en **Historial de versiones** para guardar copias con nombre de la presentación en su equipo. No se envía nada a ningún sitio. Guarde antes la presentación, porque una versión pertenece a un fichero. Haga clic en **Guardar una versión…** y escriba un nombre. Más tarde, elija una versión y haga clic en **Abrir una copia** para verla, **Volver a esta versión** para ponerla en la ventana (la presentación tal como está ahora se guarda antes como versión), o **Eliminar**. JeffDeck conserva las últimas 50.

### Comprobar accesibilidad

Haga clic en **Comprobar accesibilidad** para abrir la lista de elementos que pueden ser difíciles de leer o de usar para algunas personas. Es la misma comprobación que **Revisar > Revisión > Comprobar accesibilidad**.

## JeffDeck

- **Opciones** abre la ventana **Opciones de JeffDeck**. Consulte [Cambiar la configuración de JeffDeck](options).
- **Acerca de** muestra la versión de JeffDeck y de Qt, con la que está compilado JeffDeck. También enumera las aplicaciones de JeffOffice, con vínculos al **Código fuente**, a las **Descargas** y a **Informar de un problema**. Cada vínculo abre la página del proyecto en su navegador. Al final están la licencia y los avisos: JeffDeck es software libre bajo la Licencia Pública General de GNU, versión 3, y se distribuye sin ninguna garantía.

## Relacionado

- [Abrir y guardar presentaciones](open-save)
- [Imprimir, PDF e imágenes de diapositivas](printing)
- [Cambiar la configuración de JeffDeck](options)
