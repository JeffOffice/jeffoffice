# Formatos de número e idioma

<!-- keywords: formato de número, números, decimal, separador decimal, posiciones decimales, miles, separador de miles, agrupación, coma, punto, idioma, configuración regional, moneda, porcentaje, fecha, hora, científico, fracción, texto, contabilidad, general, personalizado, código, muestra, Formato de celdas, Ctrl+1, Ctrl+Mayús+4, Ctrl+Mayús+5, aumentar decimales, disminuir decimales, estilo millares, separador de fórmulas, punto y coma -->

Una celda contiene un número, y su **formato de número** indica cómo se escribe ese número en la pantalla. El formato cambia lo que ve, nunca lo que contiene la celda. Por ejemplo: la celda contiene 0,256. Con el formato Porcentaje muestra **25,6%**; con Moneda, **0,26 €**; y con General, **0,256**. Si hace clic en la celda, la barra de fórmulas sigue mostrando 0,256.

## Los números se escriben como los escribe su equipo

JeffCrunch usa los signos del idioma que tenga configurado su equipo. En inglés (Estados Unidos), mil doscientos treinta y cuatro y medio es **1,234.5**. En alemán es **1.234,5** y en francés, **1 234,5**. El libro no cambia: conserva una forma neutra, de modo que el mismo archivo muestra 1,234.5 a una persona y 1.234,5 a otra.

Esto se aplica a todo lugar donde se muestra o se escribe un número:

- Una celda, la copia de una celda y el ancho que **Autoajustar ancho de columna** da a una columna siguen los signos de su idioma.
- La **barra de fórmulas** y una celda que edita con **F2** muestran un número tal como lo escribe. Si su idioma escribe los decimales con coma, escriba **2,5** y no 2.5.
- En un idioma que escribe los decimales con coma, una fórmula usa **punto y coma** entre sus argumentos: **=SUMA(A1;A2)**. Los nombres de las funciones están en inglés.
- Las fechas y las horas siguen el código del formato: un formato que dice "mes, día, año" las muestra en ese orden, y un formato pensado para su idioma (los predefinidos de la lista) usa el orden y los nombres de su idioma.

El texto que el formato pone entre comillas, como "unidades", se muestra tal cual.

## Elegir un formato

1. Seleccione las celdas.
2. Use el grupo **Número** de la pestaña **Inicio**, o pulse **Ctrl+1** (en un Mac, **Comando+1**) para abrir **Formato de celdas**, que se abre en la pestaña **Número**.

El cuadro del grupo **Número** muestra los formatos habituales: General, Número, Moneda, Contabilidad, Fecha corta, Fecha larga, Hora, Porcentaje, Fracción, Científico, Texto y Personalizado. **Más formatos de número**, al final, abre la ventana Formato de celdas. Los botones situados debajo del cuadro son:

| Botón | Qué hace |
| --- | --- |
| Moneda | Muestra el número como dinero, con el símbolo monetario de su idioma (**Ctrl+Mayús+4**). La flecha que está junto a él ofrece **Formato de número contable** con una moneda para elegir (dólar estadounidense, libra esterlina, euro, yen japonés, yuan chino o franco suizo), General y **Más formatos de número**. |
| Estilo porcentual | Multiplica por 100 y añade el signo de porcentaje, sin decimales (**Ctrl+Mayús+5**). Haga clic de nuevo para quitarlo. |
| Estilo millares | Agrupa los dígitos por miles, muestra dos decimales y alinea los puntos decimales en una columna, sin símbolo. |
| Aumentar decimales, Disminuir decimales | Añade o quita una posición decimal. |
| General (en el cuadro, y en la flecha junto a Moneda) | Quita el formato: el número se muestra tal como es (**Ctrl+Mayús+~**). |

## La pestaña Número de Formato de celdas

Elija una **Categoría** a la izquierda; a la derecha aparecen sus opciones, y la **Muestra** indica cómo se verá la primera celda seleccionada, en su idioma.

- **Número**: posiciones decimales, si los dígitos se agrupan por miles y cómo se ven los números negativos (con signo menos, en rojo o entre paréntesis).
- **Moneda** y **Contabilidad**: posiciones decimales y un símbolo. Contabilidad alinea los símbolos y los puntos decimales en una columna.
- **Fecha** y **Hora**: una lista de formatos predefinidos, con el orden y los nombres de su idioma.
- **Porcentaje**, **Científico**: posiciones decimales.
- **Fracción**: formatos predefinidos como "hasta un dígito" (1/4), "hasta dos dígitos" (21/25), "como cuartos" y "como dieciseisavos".
- **Especial**: formatos predefinidos para un código postal, un código postal más 4, un número de teléfono y un número de la seguridad social.
- **Texto**: la celda contiene lo que escribe como texto, de modo que un número como 00123 conserva sus ceros.
- **Personalizado**: escriba un código propio o parta de uno de la lista (los códigos habituales y los que ya usa el libro).

El **código** es el texto breve que indica a JeffCrunch cómo escribir un número. Por ejemplo, **#.##0,00** significa "agrupar los dígitos por miles y mostrar siempre dos decimales", así que 1234,5 se muestra como 1.234,50. Cada **0** es un dígito que siempre se muestra; cada **#**, un dígito que solo se muestra si hace falta, y **%** multiplica por 100. Un código puede tener hasta cuatro secciones separadas por punto y coma, para números positivos, números negativos, cero y texto.

## Qué se conserva

El código se guarda en el libro en su forma neutra, de modo que el libro se lee bien en cualquier equipo. El formato es un paso que puede deshacer, y un libro que ya usaba un formato lo conserva exactamente: un formato que está en la lista se vuelve a encontrar, y no se crea dos veces.
