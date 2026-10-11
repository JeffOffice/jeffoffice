# Recuperar el trabajo no guardado
<!-- keywords: recuperar, recuperación, autorrecuperación, autoguardado, guardado automático, bloqueo, cierre inesperado, trabajo perdido, no guardado, corte de luz, apagón, restaurar, congelado, bloqueado, cerrado por error, copia de seguridad -->

Si JeffPub se cierra de forma inesperada o se va la luz, es posible que su trabajo más reciente esté a salvo. Mientras trabaja, JeffPub guarda una copia de reserva de cada publicación que tenga cambios sin guardar. Esta función se llama **Autorrecuperación**.

## Cómo funciona la Autorrecuperación

- Cada pocos minutos, JeffPub guarda una copia de reserva de cada publicación que tenga cambios sin guardar. El valor predeterminado es cada 10 minutos.
- JeffPub guarda las copias en una carpeta de Autorrecuperación, dentro de la carpeta donde guarda sus propios datos. No es necesario que la busque usted.
- La copia de reserva nunca reemplaza el archivo real.
- La copia desaparece cuando guarda la publicación, la cierra o sale de JeffPub de la forma habitual.

La Autorrecuperación no sustituye a guardar. Si JeffPub se detiene, puede perder el trabajo de los últimos minutos. Presione Ctrl+S a menudo.

## Recuperar el trabajo

La próxima vez que inicie JeffPub después de un cierre inesperado, se abrirá automáticamente la ventana **Recuperar trabajo no guardado**. Cada línea muestra el nombre de la publicación, dónde estaba guardado el archivo (o "nunca guardado") y cuándo se creó la copia.

1. Deje activada la casilla de cada publicación que quiera recuperar. Desactive la de las demás.
2. Haga clic en **Abrir**.
3. Revise el trabajo y presione Ctrl+S.

La publicación recuperada muestra "(Recuperado)" en la barra de título hasta que la guarde. Ctrl+S pide un nombre y empieza en la carpeta del archivo original, con el nombre original. Elija el nombre original para reemplazar el archivo, o uno nuevo para conservar ambos.

Las imágenes vinculadas a archivos se buscan como si la publicación estuviera abierta en la carpeta del archivo original: las que están en esa carpeta, o en una carpeta dentro de ella, se cargan con normalidad. Una imagen vinculada a un archivo situado en cualquier otro lugar muestra su vista previa guardada y aparece marcada como **No actualizada**; selecciónela y haga clic en **Actualizar vínculo** en el [Administrador de gráficos](graphics-manager). Una publicación que nunca se guardó no tiene carpeta, por lo que todas sus imágenes vinculadas quedan a la espera de **Actualizar vínculo**.

Los demás botones de la ventana son:

- **Eliminar** borra definitivamente las copias marcadas, después de pedirle confirmación.
- **Ahora no** cierra la ventana y conserva las copias. JeffPub volverá a ofrecerlas la próxima vez que se inicie.

JeffPub solo ofrece copias de una sesión que terminó de forma inesperada. No ofrece copias de una ventana de JeffPub que siga abierta.

## Buscar el trabajo recuperado más tarde

Si cerró la ventana y quiere recuperar las copias, haga clic en **Archivo > Abrir** y, después, en **Recuperar trabajo no guardado**. Si no hay nada que recuperar, JeffPub se lo indica.

## Cambiar la frecuencia de guardado

Haga clic en **Archivo > Opciones** y, después, en la pestaña **Guardar**. Cambie **Guardar información de Autorrecuperación cada** a un número entre 1 y 120 minutos. Cuanto menor sea el número, menos podrá perder. El nuevo intervalo se aplica por completo la próxima vez que inicie JeffPub.

## Temas relacionados

- [Abrir y guardar publicaciones](open-save)
- [Cambiar la configuración de JeffPub](options)
- [Obtener ayuda e informar de problemas](support)
