# Formatos de número e idioma
<!-- keywords: formato de número, números, decimal, separador decimal, coma decimal, posiciones decimales, decimales, miles, separador de miles, agrupación, coma, punto, idioma, configuración regional, moneda, divisa, porcentaje, fecha, hora, científica, fracción, texto, contabilidad, general, personalizado, código, muestra, Formato de celdas, Ctrl+1, Ctrl+Mayús+4, Ctrl+Mayús+5, aumentar decimales, disminuir decimales, estilo millares, separador de fórmulas, punto y coma -->

Una celda contiene un número, y su **formato de número** indica cómo se escribe ese número en la pantalla. El formato cambia lo que se ve, nunca lo que contiene la celda. Ejemplo: la celda contiene 0,256. Con el formato Porcentaje muestra **25,6 %**, con Moneda muestra **0,26 €** y con General muestra **0,256**. Si hace clic en la celda, la barra de fórmulas sigue mostrando 0,256.

## Los números se escriben como los escribe su equipo

JeffCrunch usa los signos del idioma configurado en su equipo. En inglés (Estados Unidos), mil doscientos treinta y cuatro y medio se escribe **1,234.5**. En alemán es **1.234,5** y en francés, **1 234,5**. El libro no cambia: guarda una forma neutra, de modo que el mismo archivo muestra 1,234.5 a una persona y 1.234,5 a otra.

Esto se aplica en todos los lugares donde se muestra o se escribe un número:

- Una celda, la copia de una celda y el ancho que **Autoajustar** da a una columna siguen los signos de su idioma.
- La **barra de fórmulas** y una celda que modifica con **F2** muestran el número tal como lo escribe. Si su idioma escribe los decimales con coma, escriba **2,5**, no 2.5.
- En un idioma que escribe los decimales con coma, una fórmula lleva **punto y coma** entre sus argumentos: **=SUM(A1;A2)**. Los nombres de las funciones se mantienen en inglés.
- Las fechas y horas siguen el código del formato: un formato que indica "mes, día, año" las muestra en ese orden, y un formato pensado para su idioma (los predefinidos de la lista) usa el orden y los nombres de su idioma.

El texto que el formato pone entre comillas, como "unidades", se muestra tal cual.

## Elegir un formato

1. Seleccione las celdas.
2. Use el grupo **Número** de la pestaña **Inicio**, o presione **Ctrl+1** (en un Mac, **Comando+1**) para abrir **Formato de celdas**, que se abre en la pestaña **Número**.

El cuadro del grupo **Número** enumera los formatos habituales: General, Número, Moneda, Contabilidad, Fecha corta, Fecha larga, Hora, Porcentaje, Fracción, Científica, Texto y Personalizado. **Más formatos de número**, al final, abre la ventana Formato de celdas. Los botones situados debajo del cuadro son:

| Botón | Qué hace |
| --- | --- |
| Moneda | Muestra el número como importe monetario, con el símbolo de moneda de su idioma (**Ctrl+Mayús+4**). La flecha situada junto a él ofrece General y **Más formatos de número**. |
| Estilo porcentual | Multiplica por 100 y agrega el signo de porcentaje, sin decimales (**Ctrl+Mayús+5**). Haga clic de nuevo para quitarlo. |
| Estilo millares | Agrupa los dígitos por miles, muestra dos decimales y alinea las comas decimales en la columna, sin símbolo. |
| Aumentar decimales, Disminuir decimales | Agrega o quita una posición decimal. |
| General (en el cuadro y en la flecha situada junto a Moneda) | Quita el formato: el número se muestra tal cual (**Ctrl+Mayús+~**). |

## La pestaña Número de Formato de celdas

Elija una **Categoría** a la izquierda; sus opciones aparecen a la derecha, y la **Muestra** indica el aspecto que tendrá la primera celda seleccionada, en su idioma.

- **Número**: posiciones decimales, si se agrupan los dígitos por miles y cómo se muestran los números negativos (con signo menos, en rojo o entre paréntesis).
- **Moneda** y **Contabilidad**: posiciones decimales y un símbolo. Contabilidad alinea los símbolos y las comas decimales en la columna.
- **Fecha** y **Hora**: una lista de formatos predefinidos, con el orden y los nombres de su idioma.
- **Porcentaje**, **Científica**: posiciones decimales.
- **Fracción**: formatos predefinidos como "Hasta un dígito" (1/4), "Hasta dos dígitos" (21/25), "Como cuartos" y "Como dieciseisavos".
- **Especial**: formatos predefinidos para un código postal, un código postal + 4, un número de teléfono y un número de la seguridad social.
- **Texto**: la celda guarda lo que escriba como texto, de modo que un número como 00123 conserva sus ceros.
- **Personalizado**: escriba un código propio o parta de uno de la lista (los códigos habituales y los que ya usa el libro).

El **código** es el texto breve que indica a JeffCrunch cómo escribir un número. Ejemplo: **#,##0.00** significa "agrupar los dígitos por miles y mostrar siempre dos decimales", de modo que 1234,5 se muestra como 1.234,50. Cada **0** es un dígito que siempre se muestra, cada **#** un dígito que solo se muestra si es necesario, y **%** multiplica por 100. Un código puede tener hasta cuatro secciones separadas por punto y coma: para números positivos, números negativos, cero y texto.

## Qué se conserva

El código se guarda en el libro en su forma neutra, de modo que el libro se lee correctamente en cualquier equipo. Aplicar formato es un solo paso que se puede deshacer, y un libro que ya usaba un formato lo conserva exactamente: un formato que está en la lista se vuelve a encontrar y ninguno se crea dos veces.
