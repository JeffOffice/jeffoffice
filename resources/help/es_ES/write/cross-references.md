# Referencias cruzadas
<!-- keywords: referencia cruzada, referencias cruzadas, remisión, REF, PAGEREF, NOTEREF, ver página, ver figura, arriba, abajo, número de título, vínculo, hipervínculo -->

Una **referencia cruzada** es un texto del documento que remite a otro lugar, como "consulte la Figura 2 en la página 7". Es un campo, de modo que cuando la figura o la página cambian, se actualiza el campo y el texto cambia con ellas.

## Insertar una referencia cruzada

1. Haga clic donde debe ir el texto.
2. Haga clic en **Referencias > Referencia cruzada**.
3. Elija el **Tipo de referencia**: un título, un marcador, una nota al pie, una nota al final, un elemento numerado o un título de ilustración como Figura o Tabla.
4. Elija qué desea en **Insertar referencia a**: el texto, el número, el número de página o "arriba" y "abajo".
5. Elija el elemento en la lista, deje activada la casilla **Insertar como hipervínculo** si quiere poder ir hasta él y haga clic en **Insertar**.

Por ejemplo, para escribir "Consulte el título Resultados en la página 4": escriba `Consulte el título `, inserte una referencia al título con **Texto del título**, escriba ` en la página ` e inserte una segunda referencia al mismo título con **Número de página**.

## Seguir una referencia

Mantenga pulsada la tecla **Ctrl** y haga clic en una referencia que sea un hipervínculo. El cursor se desplaza al lugar al que remite.

## Mantener el texto actualizado

Después de agregar páginas, cambiar títulos o mover figuras, seleccione el texto y pulse **F9**. Cada referencia vuelve a calcular su texto. Si se ha eliminado el lugar al que remite una referencia, el campo muestra `Error: falta el marcador al que se hace referencia.` y puede elegir un nuevo destino.

## Qué ocurre internamente

JeffWrite marca el destino con un marcador oculto (cuyo nombre empieza por `_Ref`) y crea un campo que lo nombra: `REF` para textos y números, `PAGEREF` para números de página y `NOTEREF` para notas. Otro programa abre el fichero con las referencias en funcionamiento.
