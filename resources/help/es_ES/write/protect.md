# Proteger un documento y bloquear el control de cambios
<!-- keywords: proteger, protección, restringir edición, solo lectura, contraseña, bloquear control de cambios, opciones de control de cambios, nombre de usuario, iniciales, solo cambios controlados, solo comentarios, detener protección, rellenar formulario, campos de formulario -->

La protección limita lo que puede hacer la gente en un documento. Estas herramientas están en la ficha **Revisar**.

## Restringir edición

1. Haga clic en **Restringir edición** en el grupo **Proteger**.
2. Elija lo que puede hacer la gente:
   - **Sin cambios (solo lectura)**: no se puede cambiar el texto.
   - **Cambios controlados**: la gente puede editar, pero cada edición queda registrada y no se puede desactivar el control de cambios.
   - **Comentarios**: la gente puede añadir comentarios, pero no cambiar el texto.
   - **Rellenar formularios**: el texto es de solo lectura, pero la gente puede rellenar los campos de formulario que contiene.
3. Escriba una contraseña dos veces. Puede dejarla vacía. En ese caso, cualquiera puede quitar la protección.
4. Haga clic en **Aplicar protección**.

La barra de título dirá entonces «Protegido». Para quitar la protección, haga clic en **Restringir edición** de nuevo y, después, en **Detener protección**. Escriba la contraseña si la hay.

Una contraseña evita que las personas de buena fe cambien el documento por error. No es una seguridad fuerte. No use la protección para ocultar información privada.

La contraseña se guarda en el fichero como un número codificado, llamado hash. La contraseña en sí nunca se guarda. Si la olvida, JeffWrite no puede recuperarla.

## Rellenar un formulario

Un documento puede tener campos de formulario: cuadros para escribir, casillas de verificación y listas para elegir. JeffWrite los rellena, pero no añade otros nuevos.

- **Escribir en un campo.** Haga clic en él y escriba. Un campo puede admitir solo un número limitado de letras, o convertir lo que escribe en mayúsculas, o en un número o una fecha. JeffWrite lo aplica al escribir, pegar o introducir texto con un método de entrada. El texto pegado que se corta para caber conserva sus primeras letras. Un salto de línea en el texto pegado se convierte en un espacio. Un salto de línea que ya está en el campo se conserva, a menos que lo seleccione y escriba encima; escribir nunca añade uno. Un número o una fecha se escribe con su formato establecido al salir del campo: pulse Tab, pulse Intro, haga clic en otro lugar, salga con las teclas de flecha, guarde o imprima.
- **Marcar una casilla de verificación.** Haga clic en ella o desplácese hasta ella y pulse Espacio.
- **Elegir de una lista.** Haga clic en ella o desplácese hasta ella y pulse Intro. Después, elija una entrada. Si pulsa una letra, se selecciona la siguiente entrada que empiece por esa letra.
- **Moverse entre campos.** Cuando el documento está protegido para rellenar formularios, pulse Tab o Intro para ir al campo siguiente, y Mayús+Tab para volver. En cualquier documento, Intro dentro de un campo no inicia un párrafo nuevo. Solo se pueden cambiar los campos. Una parte del documento que la persona que creó el formulario dejó fuera de la protección también se puede cambiar; allí Tab es la tecla habitual, y Reemplazar todo cambia solo las coincidencias de esa parte, le indica cuántas dejó sin cambiar y advierte de que el texto de encabezados, pies de página, notas y cuadros de texto se ha dejado como está. Lo que escribe, pega, arrastra o corrige se comprueba según el lugar donde iría a parar, no según la posición del cursor. Las imágenes, formas y tablas se pueden añadir y cambiar en una parte que queda fuera de la protección, como en cualquier documento, y no se modifican en ningún otro lugar.
- **Deshacer.** Pulse Deshacer. Las letras que escribe en un campo forman un solo paso. Deshacer revierte los pasos realizados desde que empezó la protección, que son sus entradas y sus cambios en una parte que queda fuera de la protección, y nada anterior.

No se puede rellenar un campo que esté desactivado en el formulario, ni uno que el formulario calcula por sí mismo. En cualquier documento, las marcas de inicio, fin y demás de un campo nunca se eliminan por partes: seleccione el campo completo para eliminarlo. Tampoco se puede rellenar un campo en un documento de solo lectura, o abierto solo para comentarios. Con el control de cambios activado en un documento que no está protegido, lo que escribe en un campo de texto es un cambio controlado, y no se aplican la longitud máxima del texto ni las mayúsculas del campo.

## Bloquear el control de cambios

Haga clic en **Opciones de control de cambios** y, después, en **Bloquear control de cambios**. A partir de ese momento, todos los cambios quedan registrados. Nadie puede desactivar el control de cambios, ni aceptar o rechazar un cambio, sin la contraseña. Es lo mismo que proteger el documento para cambios controlados.

## Cambiar su nombre en los cambios

Haga clic en **Opciones de control de cambios** y, después, en **Opciones de control de cambios…**. Escriba su nombre y sus iniciales. Firmarán los cambios y comentarios que haga a partir de ese momento.
