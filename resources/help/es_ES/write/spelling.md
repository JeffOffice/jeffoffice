# Revisar la ortografía
<!-- keywords: ortografía, revisar ortografía, corrector ortográfico, corrector, palabra mal escrita, errores ortográficos, línea roja, línea ondulada, subrayado ondulado, sugerencias, omitir, ignorar, ignorar todo, agregar al diccionario, diccionario personalizado, diccionario, idioma, revisión, F7, revisar, comprobar ortografía mientras escribe, mayúsculas, números, gramática, comprobar gramática, estilo, línea azul, línea punteada, comprobar gramática mientras escribe -->

JeffWrite revisa la ortografía mientras escribe. Una palabra que no está en el diccionario recibe una línea roja ondulada por debajo. La revisión se hace en su propio equipo, sin conexión a internet, y conoce 31 idiomas y variantes regionales.

## Mientras escribe

- Una línea roja ondulada indica que el diccionario no tiene la palabra. Es posible que la palabra sea correcta. Los nombres y las palabras nuevas suelen faltar en el diccionario.
- La palabra que está escribiendo no se marca hasta que pasa a otra, así que una palabra a medio escribir no parpadea.
- Las líneas solo se muestran en la pantalla. Nunca se imprimen ni aparecen en un PDF.

Para corregir una palabra, haga clic con el botón derecho en ella. El menú muestra estas opciones por encima de las habituales:

- Hasta seis **sugerencias**, la mejor primero, en negrita. Haga clic en una para reemplazar la palabra. Es un solo paso, así que **Ctrl+Z** lo deshace.
- **Ignorar todo** deja esa palabra sin marcar, en todos sus usos, hasta que cierre JeffWrite.
- **Agregar al diccionario** enseña la palabra a JeffWrite de forma permanente. La palabra se guarda en un fichero pequeño dentro de su carpeta de JeffOffice, y todas las aplicaciones de JeffOffice la usan. Para ver o cambiar su lista de palabras, consulte Sus propias palabras, más abajo.

Si el diccionario no tiene ninguna sugerencia, el menú lo indica.

## Recorrer las palabras mal escritas

Pulse **F7** o haga clic en **Revisar > Ortografía**. El panel Ortografía se abre a la derecha. Muestra la primera palabra mal escrita después del cursor, seleccionada en su texto, con la línea en la que está y una lista de sugerencias.

- **Cambiar** reemplaza la palabra por la sugerencia elegida. Haga doble clic en una sugerencia para hacer lo mismo.
- **Cambiar todas** hace esto con todas las copias de la palabra en los lugares a los que va el panel, indicados más abajo. Es un solo paso, así que **Ctrl+Z** deshace todos los cambios, estén donde estén. Al terminar, el panel indica cuántas palabras ha cambiado y en cuántos lugares. El texto, cada cuadro de texto, cada nota y cada encabezado o pie de página cuentan como un lugar cada uno.
- **Omitir** pasa de esta palabra. Su línea roja permanece.
- **Ignorar todo** y **Agregar al diccionario** funcionan igual que en el menú contextual.

El panel empieza donde está el cursor: en el texto, o en el cuadro de texto, nota al pie, encabezado o pie de página que esté editando. Primero recorre el texto del documento, incluidas las tablas; después los cuadros de texto, después las notas al pie y, por último, los encabezados y pies de página. Cuando una palabra está en un cuadro de texto, una nota al pie, un encabezado o un pie de página, JeffWrite lo abre para editarlo, igual que hace Buscar, para que vea la palabra donde está. Un encabezado o pie de página que no aparece en ninguna página se omite, y **Cambiar todas** tampoco lo modifica. Desde el cursor, el panel llega hasta el final de ese lugar, sigue por los demás en este orden y continúa desde el principio hasta el lugar donde empezó. **Cambiar** es un paso que puede deshacerse, y **Cambiar todas** busca en todos los lugares a los que va el panel y también es un solo paso. Una palabra de una nota al final puede ignorarse o agregarse, pero no cambiarse. Cuando no quedan más, el panel indica que la comprobación ortográfica ha terminado. Después pasa a los problemas de gramática y estilo (consulte Comprobar gramática, más abajo).

## Contar los errores en la barra de estado

La barra de estado muestra cuántos errores de ortografía y gramática tiene el documento, por ejemplo **3 errores de revisión**. Haga clic en ella para seleccionar el siguiente error después del cursor. Cuando llegue al último, un clic vuelve al primero.

JeffWrite cuenta mientras trabaja, poco a poco, y vuelve a revisar solo los párrafos que cambia. En un documento largo, el recuento muestra un signo más, por ejemplo **120+ errores de revisión**, hasta que se hayan revisado todos los párrafos. Una palabra que la comprobación ortográfica y la gramatical marcan a la vez se cuenta dos veces. Los errores de gramática se cuentan en los párrafos que la comprobación gramatical ya ha revisado, así que ese número puede aumentar al desplazarse. El recuento se oculta cuando la comprobación ortográfica y la gramatical están desactivadas.

## Sus propias palabras

Haga clic en **Archivo > Opciones > Revisión > Diccionario personalizado** para ver y cambiar sus palabras. El cuadro de diálogo tiene dos listas, cada una con un cuadro **Buscar** para reducirla:

- **Palabras agregadas** son las palabras que enseñó a JeffWrite. Se aceptan en todos los idiomas.
- **Palabras excluidas** son palabras que quiere que se marquen como mal escritas aunque un diccionario las acepte, por ejemplo una palabra que siempre escribe mal. Se marcan dondequiera que aparezcan, con cualquier combinación de mayúsculas, y nunca se ofrecen como sugerencia. Una palabra excluida es solo esa forma exacta, no sus otras formas: excluir «casa» no marca «casas». Agregar una palabra al diccionario, desde el panel, el menú contextual o el cuadro de diálogo, la quita de la lista de excluidas. **Ignorar todo** sigue funcionando con una palabra excluida hasta que cierre el programa.

Escriba una palabra y haga clic en **Agregar**, o seleccione palabras y haga clic en **Quitar**. Una palabra solo puede estar en una lista a la vez: agregarla a una quita de la otra. **Importar** toma las palabras de un fichero de texto sin formato con una palabra por línea, y **Exportar** escribe la lista como un fichero del mismo tipo (UTF-8). Una línea con un espacio o un carácter de control, o con más de 100 letras, se omite, y el cuadro de diálogo le indica cuántas se han omitido. Una lista puede tener hasta 1 MB de texto. Nada cambia hasta que haga clic en **Aceptar**. Las listas son suyas, son las mismas en todas las aplicaciones de JeffOffice y nunca se guardan en un documento.

## Cuando la Autocorrección cambia una palabra

Cuando la Autocorrección cambia una palabra mientras escribe, aparece un pequeño botón **Opciones de autocorrección** debajo del cursor. Tiene tres opciones. Un lector de pantalla indica que hay opciones cuando aparece el botón, y la descripción del botón explica cómo abrirlas. También puede pulsar **Alt+Mayús+F10** para abrirlas. Corresponde al comando **Menú de opciones de autocorrección**, que está en la página Teclado de las opciones junto a los demás comandos, y allí puede asignarle otra tecla.

- **Deshacer corrección automática** devuelve la palabra tal como la escribió. Es un solo paso, así que **Rehacer** vuelve a hacer el cambio.
- Una opción que detiene el cambio. Depende de lo que cambió la palabra: **Dejar de reemplazar** quita la entrada de su lista de reemplazos; **Mantener como se escribió** agrega una palabra con dos mayúsculas («THursday») a las palabras que conservan sus mayúsculas; y **Dejar de poner en mayúscula** desactiva la mayúscula de los nombres de los días, de la primera letra de las oraciones o de la primera letra de las celdas de tabla en todo el documento, no solo en esta palabra, y lo indica. Es el mismo interruptor que en el cuadro de diálogo Autocorrección. También devuelve la palabra tal como la escribió.
- **Opciones de autocorrección...** abre el cuadro de diálogo Autocorrección.

La pestaña **Símbolos matemáticos** del cuadro de diálogo Autocorrección tiene un interruptor, **Reemplazar los nombres de los símbolos matemáticos mientras escribe**, que está desactivado hasta que lo active. Con él activado, una barra invertida seguida de un nombre, como \alpha, se convierte en el símbolo (α) cuando escribe un espacio o un signo después. El nombre debe empezar una palabra, así que una ruta de carpeta como C:\temp\pi se conserva tal como la escribió. La pestaña muestra todos los nombres. Cuando se ha reemplazado un nombre, la opción del botón es **Dejar de reemplazar nombres matemáticos**, que desactiva el interruptor. JeffDeck y JeffPub también los cambian; JeffCrunch no cambia nada dentro de una celda.

El botón desaparece cuando escribe, edita o mueve el cursor.

## Activar o desactivar las líneas rojas

Haga clic en **Revisar > Comprobar ortografía mientras escribe**. La misma opción está en **Archivo > Opciones > Revisión**, y es la única opción que comparten todas las aplicaciones de JeffOffice. Allí también puede elegir omitir las palabras en MAYÚSCULAS y las palabras con números, como 3rd. Ambas se omiten al principio.

## Comprobar gramática mientras escribe

JeffWrite también busca errores claros de gramática y estilo en el texto. Un problema recibe una línea azul punteada por debajo. La ortografía conserva su línea roja ondulada, y una palabra mal escrita nunca se marca como problema de gramática. Estos son los errores que busca:

- una palabra repetida («the the»)
- «a» y «an» delante de un sonido incorrecto («an test», «a hour»)
- una oración que empieza con minúscula después de un punto
- «its» y «it's», «their» y «there», «your» y «you're», «then» y «than», y «lose» y «loose», cuando las palabras de alrededor dejan claro el error
- «could of» en lugar de «could have»
- dos espacios seguidos dentro de una oración
- un espacio antes de una coma, un punto u otro signo que pertenece a la palabra anterior
- ningún espacio después del final de una oración
- «alot», y «everyday» en lugar de «every day» («almost everyday»)
- verbos que no concuerdan con el sujeto en casos claros («she don't», «they was», «it have», «everyone are», «there is many reasons»)
- la forma incorrecta de un verbo («did went», «has went», «can't hardly», «don't have no»)
- palabras confundidas («who's car», «an affect on», «less people», «irregardless», «can't breath», «weather» en lugar de «whether», «chose» en lugar de «choose», «advise» en lugar de «advice»)
- «between you and I», «Me and him went», «more better» y «these thing»
- una coma después del año de una fecha («On June 5, 2020, we left»), una coma después de la ciudad y el estado («in Austin, Texas, the heat»), y un guion en un compuesto delante de un sustantivo («a well-known author»)
- una «i» minúscula en lugar de «I», y los días y los meses en minúscula

Prefiere pasar por alto un error antes que marcar un texto correcto, así que una oración sin línea azul no garantiza que sea correcta.

Haga clic con el botón derecho en el problema. El menú indica qué está mal y después muestra una o más sugerencias. Haga clic en una sugerencia para ponerla en el texto, y pulse Ctrl+Z para deshacerla. **Ignorar este problema** quita la línea solo en este lugar, hasta que cierre JeffWrite. Las mismas palabras en otro lugar conservan su línea, y editar otra parte del párrafo no la vuelve a mostrar. **Ignorar todo** quita la línea de todos los problemas con este mensaje en estas palabras.

La revisión se hace en su propio equipo, sin conexión a internet, en segundo plano: escribir nunca se interrumpe, y un párrafo se revisa poco después de dejar de escribir en él. Cada párrafo se revisa hasta sus primeros 100 000 caracteres. Un documento que requiere más revisión que el límite deja de revisarse, y la barra de estado lo indica una vez; la ortografía sigue funcionando.

**Qué idiomas se revisan.** Inglés, francés, alemán y español, cada uno con reglas propias. El francés revisa el espacio sin separación antes de ; : ! ? y dentro de « », el apóstrofo en palabras como «l'ami» y «qu'il», y confusiones como «a» y «à». El alemán revisa «das» y «dass», «seid» y «seit», «wider» y «wieder», la coma antes de «dass» y «weil», y las mayúsculas en sustantivos comunes. El español revisa la falta de ¿ o ¡, las tildes en las palabras interrogativas, «a ver» y «haber», y «del» y «al». Un párrafo se revisa en el idioma en el que está marcado, y el texto sin idioma se revisa como inglés. En un párrafo con más de un idioma se usa el primer idioma revisado y se omite el resto. El texto que un fichero marca como «no revisar» también se omite. La gramática se revisa en el texto del documento, pero todavía no en las tablas, los encabezados, los pies de página ni las notas.

Para desactivarla, haga clic en **Revisar > Comprobar gramática mientras escribe**. La misma opción está en **Archivo > Opciones > Revisión**. Es una sola opción para todas las aplicaciones de JeffOffice, y es la misma en todas las ventanas. El panel Ortografía (F7) recorre los problemas de gramática después de los de ortografía, cuando esta opción está activada.

**Elegir qué busca.** Haga clic en **Archivo > Opciones > Revisión > Configuración de gramática** para elegir los tipos de problemas: **Gramática** (verbos que no concuerdan, la forma incorrecta de un verbo, «a» y «an»), **Puntuación** (espacios, comas y guiones sobrantes o que faltan), **Mayúsculas**, **Palabras confundidas** (palabras que suenan igual), **Repetición** y **Estilo**. **Estilo** está desactivado hasta que lo active. Sugiere mejoras y no señala errores: oraciones de más de 40 palabras, la voz pasiva, tres oraciones seguidas que empiezan con la misma palabra y frases innecesariamente largas («para» en lugar de «en orden a», «debido a que» en lugar de «debido al hecho de que»). La opción se conserva al hacer clic en **Aceptar** en Opciones. Se aplica de inmediato a las líneas azules y al panel Ortografía (F7) en las ventanas abiertas de esta aplicación, y a las demás aplicaciones de JeffOffice la próxima vez que se inicien.

## Idiomas

JeffWrite revisa cada fragmento de texto en su propio idioma. Un fichero puede indicar en qué idioma está su texto, hasta una sola palabra, y JeffWrite lo lee. El texto que no lo indica se revisa en el idioma del documento. El fichero da ese idioma para todo el documento, y un documento nuevo usa el inglés de Estados Unidos.

El texto en un idioma para el que JeffWrite no tiene diccionario no se revisa. Tampoco se revisa el texto que un fichero marca como «no revisar la ortografía». Todavía no puede cambiar el idioma del texto en JeffWrite, pero se conserva el que indique el fichero.

Los 31 diccionarios son: inglés (estadounidense, británico, canadiense y australiano), español (21 variantes regionales, incluidos México y España), francés, alemán, italiano, neerlandés y portugués (Brasil y Portugal).

## Lo que todavía no se revisa

Las líneas rojas se muestran en el texto, en las celdas de las tablas y en un encabezado, pie de página o nota al pie mientras lo edita. Todavía no se muestran en los cuadros de texto, pero el panel Ortografía sí los recorre.

## Temas relacionados

- [Bienvenido a JeffWrite](start)
- [Imprimir](print)
