# Recuperar trabajo no guardado
<!-- keywords: recuperar, recuperación, autorrecuperación, autorecuperación, autoguardado, guardado automático, bloqueo, cierre inesperado, trabajo perdido, no guardado, corte de luz, apagón, restaurar, congelado, cerrado por error, copia de seguridad -->

Si JeffDeck se cierra de forma inesperada o se va la luz, es posible que su trabajo más reciente siga a salvo. Mientras trabaja, JeffDeck guarda una copia de reserva de cada presentación que tenga cambios sin guardar. Esto se llama **Autorrecuperación**.

## Cómo funciona la Autorrecuperación

- Cada 10 minutos, JeffDeck guarda una copia de reserva de cada presentación que tenga cambios sin guardar.
- JeffDeck guarda las copias en una carpeta de Autorrecuperación, dentro de la carpeta donde guarda sus propios datos. No tiene que buscarlas usted.
- La copia de reserva nunca reemplaza el archivo real.
- La copia desaparece al guardar la presentación, cerrarla o salir de JeffDeck de la forma normal.

La Autorrecuperación no sustituye a guardar. Si JeffDeck se detiene, puede perder el trabajo de los últimos minutos. Presione Ctrl+S a menudo.

## Recuperar el trabajo

La próxima vez que inicie JeffDeck tras un cierre inesperado, se abre automáticamente la ventana **Recuperar trabajo no guardado**. Cada línea muestra el nombre de la presentación, dónde estaba guardado el archivo (o "nunca guardado") y cuándo se creó la copia.

1. Deje activada la casilla de cada presentación que quiera recuperar. Desactive la de las demás.
2. Haga clic en **Abrir**.
3. Revise el trabajo y presione Ctrl+S.

Una presentación recuperada se abre como trabajo no guardado. Ctrl+S le pide un nombre y empieza en la carpeta del archivo original, con el nombre original. Elija el nombre original para reemplazar el archivo, o elija un nombre nuevo para conservar ambos.

Los otros botones de la ventana son:

- **Eliminar** quita definitivamente las copias marcadas, después de pedirle confirmación.
- **Ahora no** cierra la ventana y conserva las copias. JeffDeck se las vuelve a ofrecer la próxima vez que se inicie.

JeffDeck solo ofrece copias de una sesión que terminó de forma inesperada. No ofrece copias de una ventana de JeffDeck que siga abierta.

## Encontrar el trabajo recuperado más tarde

Si cerró la ventana y quiere recuperar las copias, haga clic en **Archivo > Abrir** y, después, en **Recuperar trabajo no guardado**. Si no hay nada que recuperar, JeffDeck se lo indica.

## Temas relacionados

- [Abrir y guardar presentaciones](open-save)
- [La pantalla Archivo](file-screen)
- [Cambiar la configuración de JeffDeck](options)
