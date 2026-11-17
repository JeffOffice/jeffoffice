# Funções de texto
<!-- keywords: funções, ASC, BAHTTEXT, CHAR, CLEAN, CODE, CONCAT, CONCATENATE, DBCS, DOLLAR, ENCODEURL, EXACT, FILTERXML, FIND, FINDB, FIXED, LEFT, LEFTB, LEN, LENB, LOWER, MID, MIDB, NUMBERVALUE, PHONETIC, PROPER, REPLACE, REPLACEB, REPT, RIGHT, RIGHTB, SEARCH, SEARCHB, SUBSTITUTE, T, TEXT, TEXTJOIN, TRIM, UNICHAR, UNICODE, UPPER, VALUE, texto, função de texto, fórmulas -->

Estas são as funções de texto que o JeffCrunch consegue calcular. Cada uma mostra a forma como é escrita, o que faz e o que é cada um dos seus argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode acrescentar tantos outros do último tipo quantos precisar. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, aparece por baixo da célula uma pequena sugestão com os respetivos argumentos, e se clicar no nome da função nessa sugestão, abre-se aqui a respetiva entrada.

## ASC

`ASC(texto)`

Altera as letras e os algarismos de largura total do texto para largura normal.

- `texto`: O texto a processar, ou uma célula que o contenha.

## BAHTTEXT

`BAHTTEXT(número)`

Escreve um número em palavras tailandesas, com a palavra para o baht.

- `número`: O número a processar.

## CHAR

`CHAR(número)`

Devolve o carácter que corresponde a um número de código.

- `número`: O número a processar.

## CLEAN

`CLEAN(texto)`

Retira de um texto os carateres que não são impressos.

- `texto`: O texto a processar, ou uma célula que o contenha.

## CODE

`CODE(texto)`

Devolve o número de código do primeiro carácter de um texto.

- `texto`: O texto a processar, ou uma célula que o contenha.

## CONCAT

`CONCAT(texto1, ...)`

Une textos e os textos de blocos de células num só.

- `texto1`: O primeiro texto, ou bloco de células com texto.
- `...`: Quantos mais precisar, do mesmo tipo que o argumento anterior.

## CONCATENATE

`CONCATENATE(texto1, ...)`

Une textos num só.

- `texto1`: O primeiro texto, ou bloco de células com texto.
- `...`: Quantos mais precisar, do mesmo tipo que o argumento anterior.

## DBCS

`DBCS(texto)`

Altera as letras e os algarismos de largura normal do texto para largura total.

- `texto`: O texto a processar, ou uma célula que o contenha.

## DOLLAR

`DOLLAR(número, [casas_decimais])`

Converte um número em texto, como um valor monetário.

- `número`: O número a processar.
- `casas_decimais` (opcional): Quantos algarismos manter depois da vírgula decimal.

## ENCODEURL

`ENCODEURL(texto)`

Altera o texto para que possa ser usado num endereço Web.

- `texto`: O texto a processar, ou uma célula que o contenha.

## EXACT

`EXACT(texto1, texto2)`

Verdadeiro quando dois textos são exatamente iguais, incluindo as maiúsculas.

- `texto1`: O primeiro texto, ou bloco de células com texto.
- `texto2`: O segundo texto.

## FILTERXML

`FILTERXML(xml, xpath)`

Extrai valores de um texto que contém XML, usando uma expressão XPath.

- `xml`: O texto que contém o XML.
- `xpath`: O XPath que indica as partes a devolver, como //item/preço.

## FIND

`FIND(texto_procurado, texto_onde_procurar, [núm_inicial])`

Devolve a posição onde um texto começa dentro de outro, distinguindo maiúsculas de minúsculas.

- `texto_procurado`: O texto a procurar.
- `texto_onde_procurar`: O texto onde procurar.
- `núm_inicial` (opcional): A posição do texto onde começar, contando o primeiro carácter como 1.

## FINDB

`FINDB(texto_procurado, texto_onde_procurar, [núm_inicial])`

Devolve a posição onde um texto aparece pela primeira vez noutro texto, distinguindo maiúsculas de minúsculas.

- `texto_procurado`: O texto a procurar.
- `texto_onde_procurar`: O texto onde procurar.
- `núm_inicial` (opcional): A posição do texto onde começar, contando o primeiro carácter como 1.

## FIXED

`FIXED(número, [casas_decimais], [sem_separadores])`

Converte um número em texto, com um número fixo de casas decimais.

- `número`: O número a processar.
- `casas_decimais` (opcional): Quantos algarismos manter depois da vírgula decimal.
- `sem_separadores` (opcional): Verdadeiro para não incluir os separadores que agrupam os milhares.

## LEFT

`LEFT(texto, [núm_carateres])`

Devolve os primeiros carateres de um texto.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `núm_carateres` (opcional): Quantos carateres tirar (1 se for omitido).

## LEFTB

`LEFTB(texto, [núm_carateres])`

Devolve os primeiros carateres do texto.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `núm_carateres` (opcional): Quantos carateres tirar (1 se for omitido).

## LEN

`LEN(texto)`

Conta os carateres de um texto.

- `texto`: O texto a processar, ou uma célula que o contenha.

## LENB

`LENB(texto)`

Devolve o número de carateres do texto.

- `texto`: O texto a processar, ou uma célula que o contenha.

## LOWER

`LOWER(texto)`

Converte um texto em minúsculas.

- `texto`: O texto a processar, ou uma célula que o contenha.

## MID

`MID(texto, núm_inicial, núm_carateres)`

Devolve carateres do meio de um texto.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `núm_inicial`: A posição do texto onde começar, contando o primeiro carácter como 1.
- `núm_carateres`: Quantos carateres tirar (1 se for omitido).

## MIDB

`MIDB(texto, núm_inicial, núm_carateres)`

Devolve carateres do meio do texto.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `núm_inicial`: A posição do texto onde começar, contando o primeiro carácter como 1.
- `núm_carateres`: Quantos carateres tirar (1 se for omitido).

## NUMBERVALUE

`NUMBERVALUE(texto, [separador_decimal], [separador_de_grupo])`

Lê um texto como número, com os separadores que indicar.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `separador_decimal` (opcional): O carácter que marca as casas decimais no texto (o ponto, se for omitido).
- `separador_de_grupo` (opcional): O carácter que agrupa os algarismos no texto (a vírgula, se for omitida).

## PHONETIC

`PHONETIC(referência)`

Devolve o texto de uma célula, para as leituras de texto em japonês.

- `referência`: Uma célula ou bloco de células.

## PROPER

`PROPER(texto)`

Coloca em maiúscula a primeira letra de cada palavra de um texto.

- `texto`: O texto a processar, ou uma célula que o contenha.

## REPLACE

`REPLACE(texto_antigo, núm_inicial, núm_carateres, texto_novo)`

Coloca texto novo no lugar de alguns carateres de um texto.

- `texto_antigo`: O texto a alterar.
- `núm_inicial`: A posição do texto onde começar, contando o primeiro carácter como 1.
- `núm_carateres`: Quantos carateres tirar (1 se for omitido).
- `texto_novo`: O texto a inserir.

## REPLACEB

`REPLACEB(texto_antigo, núm_inicial, núm_carateres, texto_novo)`

Substitui parte de um texto por outro texto.

- `texto_antigo`: O texto a alterar.
- `núm_inicial`: A posição do texto onde começar, contando o primeiro carácter como 1.
- `núm_carateres`: Quantos carateres tirar (1 se for omitido).
- `texto_novo`: O texto a inserir.

## REPT

`REPT(texto, núm_vezes)`

Repete um texto um determinado número de vezes.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `núm_vezes`: Quantas vezes repetir o texto.

## RIGHT

`RIGHT(texto, [núm_carateres])`

Devolve os últimos carateres de um texto.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `núm_carateres` (opcional): Quantos carateres tirar (1 se for omitido).

## RIGHTB

`RIGHTB(texto, [núm_carateres])`

Devolve os últimos carateres do texto.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `núm_carateres` (opcional): Quantos carateres tirar (1 se for omitido).

## SEARCH

`SEARCH(texto_procurado, texto_onde_procurar, [núm_inicial])`

Devolve a posição onde um texto começa dentro de outro, ignorando as maiúsculas e permitindo ? e * no texto procurado.

- `texto_procurado`: O texto a procurar.
- `texto_onde_procurar`: O texto onde procurar.
- `núm_inicial` (opcional): A posição do texto onde começar, contando o primeiro carácter como 1.

## SEARCHB

`SEARCHB(texto_procurado, texto_onde_procurar, [núm_inicial])`

Devolve a posição onde um texto aparece pela primeira vez noutro texto, ignorando as maiúsculas.

- `texto_procurado`: O texto a procurar.
- `texto_onde_procurar`: O texto onde procurar.
- `núm_inicial` (opcional): A posição do texto onde começar, contando o primeiro carácter como 1.

## SUBSTITUTE

`SUBSTITUTE(texto, texto_antigo, texto_novo, [núm_ocorrência])`

Troca um trecho de texto por outro sempre que ele aparece.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `texto_antigo`: O texto a procurar.
- `texto_novo`: O texto a inserir.
- `núm_ocorrência` (opcional): Qual a ocorrência a substituir (todas, se for omitido).

## T

`T(valor)`

Devolve o próprio texto, e nada para o que não seja texto.

- `valor`: O valor a processar, ou uma célula que o contenha.

## TEXT

`TEXT(valor, texto_formato)`

Converte um número em texto, num formato à sua escolha.

- `valor`: O valor a processar, ou uma célula que o contenha.
- `texto_formato`: O formato de número em que mostrar o valor, como "0,00" ou "dd/mm/aaaa".

## TEXTJOIN

`TEXTJOIN(delimitador, ignorar_vazios, texto1, ...)`

Une textos com um separador entre eles.

- `delimitador`: O texto a colocar entre as partes.
- `ignorar_vazios`: Verdadeiro para omitir as partes vazias; falso para as manter.
- `texto1`: O primeiro texto, ou bloco de células com texto.
- `...`: Quantos mais precisar, do mesmo tipo que o argumento anterior.

## TRIM

`TRIM(texto)`

Retira os espaços extra de um texto, deixando apenas um espaço entre as palavras.

- `texto`: O texto a processar, ou uma célula que o contenha.

## UNICHAR

`UNICHAR(número)`

Devolve o carácter que corresponde a um número Unicode.

- `número`: O número a processar.

## UNICODE

`UNICODE(texto)`

Devolve o número Unicode do primeiro carácter do texto.

- `texto`: O texto a processar, ou uma célula que o contenha.

## UPPER

`UPPER(texto)`

Converte um texto em maiúsculas.

- `texto`: O texto a processar, ou uma célula que o contenha.

## VALUE

`VALUE(texto)`

Converte um texto que parece um número num número.

- `texto`: O texto a processar, ou uma célula que o contenha.
