# Imprimir y crear un PDF
<!-- keywords: páginas por hoja, varias páginas por hoja, n-up, páginas impares, páginas pares, ambas caras, impresión a doble cara, dúplex, imprimir, impresora, vista previa, pdf, exportar, copias, intercalar, páginas, intervalo de páginas, selección, libro completo, escala, ajustar, correo electrónico, saltos de página, líneas discontinuas, guardar como pdf, escalar al papel, tamaño de papel, vista previa de la hoja, contraseña, cifrar, proteger con contraseña -->

Elija **Archivo > Imprimir** (o presione **Ctrl+P**). La página se abre con una vista previa de las páginas que saldrán, una a una. Use las flechas que hay debajo para pasar de una página a otra, y **Ajustar página** o los botones de zoom para ver más o menos de una página. La vista previa se dibuja igual que la impresora y el PDF, así que lo que ve es lo que obtiene.

## Qué imprimir

- **Qué**: la hoja en la que está (o todas las hojas cuyas pestañas haya elegido con Ctrl o Mayús, una tras otra), el libro completo (todas las hojas que se muestran) o la selección. Las hojas ocultas no se incluyen. Una hoja de gráfico ocupa una página, con su gráfico tan grande como permita el espacio de la página dentro de los márgenes.
- **Páginas**: todas, o algunas, como **1-3, 5**.
- **Qué páginas**: **Todas las páginas**, **Solo páginas impares** o **Solo páginas pares**. Use esta opción para imprimir por las dos caras del papel a mano cuando su impresora no pueda hacerlo. Primero imprima las páginas impares. Cuando termine, dé la vuelta a la pila, colóquela de nuevo en la bandeja de papel e imprima otra vez con **Solo páginas pares**.
- **Caras**: **Imprimir a una cara**, **Imprimir a doble cara, voltear por el borde largo**, **Imprimir a doble cara, voltear por el borde corto** o **Imprimir a doble cara manualmente**. El borde largo sirve para páginas que se pasan como las de un libro. El borde corto sirve para páginas que se pasan como las de un bloc de notas. Si su impresora no puede imprimir a doble cara, esas dos opciones aparecen atenuadas. Señálelas para ver el motivo. **Manualmente** es para ese tipo de impresora: JeffCrunch imprime la primera, la tercera, la quinta, etc., de las páginas, o de las hojas si imprime varias páginas por hoja. Después, un mensaje le pide que dé la vuelta a la pila y la coloque de nuevo en la bandeja de papel. Al hacer clic en **Aceptar**, imprime las demás, desde la última página hacia atrás, para que caigan en el reverso. Si imprime solo una página, no hay nada que voltear. Todas estas opciones aparecen atenuadas al guardar un PDF, porque un archivo no tiene caras. JeffCrunch recuerda su elección para la próxima vez.
- **Páginas por hoja**: imprima 1, 2, 4, 6, 9 o 16 páginas, reducidas, en cada hoja de papel. Cada página recibe un marco fino. La vista previa sigue mostrando una página a tamaño completo.
- **Copias** e **Intercalar** (un juego completo cada vez, 1 2 3 1 2 3, en lugar de las copias de cada página juntas).
- **Orientación**, **Papel** y **Márgenes**, para usar en esta impresión en lugar de lo que indique cada hoja.
- **Escala**: según cada hoja, tamaño real, toda la hoja en una página, todas las columnas en una página o todas las filas en una página.
- **Ignorar área de impresión**: imprime todo lo que tenga contenido.
- **Configurar página** abre el cuadro de diálogo de la hoja en la que está.

Debajo de la vista previa, JeffCrunch le indica cuando algo necesita su atención: una hoja que necesita más páginas de las que puede generar, una hoja reducida tanto que las letras son muy pequeñas, o una hoja sin nada.

## Escalar a un tamaño de papel

**Escalar al papel** reduce o amplía cada página para que quepa en el papel que elija, manteniendo su forma y centrándola. Úselo para imprimir en un papel que su impresora no tiene, por ejemplo una página configurada para A4 en papel carta. **Sin escalar** imprime cada página en su propio papel. La vista previa muestra la hoja de papel que saldrá.

## Varias páginas por hoja

**Páginas por hoja** coloca 2, 4, 6, 9 o 16 páginas en cada hoja de papel, cada una más pequeña y con marco. La vista previa muestra la hoja completa, y la etiqueta debajo indica qué hoja está viendo, por ejemplo **Hoja 1 de 2**. Use **Siguiente** y **Anterior** para ver las demás hojas.

## La impresora

Elija una impresora de la lista, o **Guardar como PDF**. **Configuración de la impresora** abre el cuadro de diálogo del propio sistema para las demás opciones de la impresora, como las bandejas de papel. **Imprimir** envía las páginas.

## Exportar a PDF

**Archivo > Exportar** crea un PDF de la hoja en la que está, o de todo el libro, sin elegir una impresora. Las letras siguen siendo letras (no imágenes), así que se pueden buscar y se ven nítidas al hacer zoom.

**Exportar todo el libro como PDF/A** crea un PDF con el formato pensado para conservar documentos durante años. Todas las fuentes se incluyen dentro del archivo, de modo que se verá igual dentro de mucho tiempo. Los programas de archivo de documentos piden este formato.

**Crear un correo electrónico** crea un archivo de mensaje con su libro adjunto, o con un PDF del libro. No se envía nada. Abra el archivo en su programa de correo, añada a quién va dirigido y envíelo. Un libro con contraseña sigue bloqueado en el mensaje: quien lo reciba necesita la contraseña para abrirlo. La contraseña nunca se incluye en el mensaje. Un PDF del libro solo tiene contraseña si hace clic en **Cifrar con contraseña…** en el cuadro, igual que con un PDF exportado. Comunique la contraseña a la persona por otro medio distinto del mensaje.

## Proteger un PDF con contraseña

En **Archivo > Exportar**, haga clic en **Cifrar con contraseña…** antes de elegir un comando de PDF. Solo se usa para el siguiente PDF. Escriba la contraseña dos veces. Quien abra después el PDF tendrá que escribirla. La contraseña protege de verdad el archivo: las páginas se cifran con un cifrado robusto (AES-256), y nadie puede leerlas sin ella.

También puede marcar **Restringir impresión, copia y edición**, indicar una segunda contraseña y elegir qué se permite. Esas limitaciones son solo una petición. La mayoría de los lectores de PDF las respetan, pero no impiden a quien quiera saltárselas, así que no confíe en ellas para mantener un archivo en privado.

Si olvida la contraseña, no se puede recuperar, así que guárdela en un lugar seguro. La contraseña se usa solo para ese PDF y nunca se guarda. El comando **Exportar todo el libro como PDF/A** aparece atenuado mientras hay una contraseña establecida, porque un archivo PDF/A no puede tenerla.

## Saltos de página en pantalla

Después de abrir la pantalla de Imprimir, la hoja en pantalla muestra dónde empiezan las páginas: una línea discontinua para un salto que calculó JeffCrunch, y una línea continua para uno que haya establecido usted. Se actualizan al cambiar la hoja. Desactívelas con **Vista > Saltos de página**. (La vista **Saltos de página**, descrita en [Configurar las páginas](page-layout), siempre las muestra.)

## Lo que todavía no se imprime

Las notas (comentarios) se imprimen después de la última página de la hoja cuando la configuración de página de la hoja indica **Enumerarlas tras la última página**; todavía no se imprimen donde están en las celdas. Los formatos condicionales todavía no se muestran en pantalla, así que tampoco se imprimen.
