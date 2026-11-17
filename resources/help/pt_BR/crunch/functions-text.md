# Funções de texto
<!-- keywords: functions, ASC, BAHTTEXT, CHAR, CLEAN, CODE, CONCAT, CONCATENATE, DBCS, DOLLAR, ENCODEURL, EXACT, FILTERXML, FIND, FINDB, FIXED, LEFT, LEFTB, LEN, LENB, LOWER, MID, MIDB, NUMBERVALUE, PHONETIC, PROPER, REPLACE, REPLACEB, REPT, RIGHT, RIGHTB, SEARCH, SEARCHB, SUBSTITUTE, T, TEXT, TEXTJOIN, TRIM, UNICHAR, UNICODE, UPPER, VALUE -->

Estas são as funções de texto que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros do último tipo forem necessários. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra os seus argumentos, e um clique no nome da função na dica abre a sua entrada aqui.

## ASC

`ASC(texto)`

Altera letras e dígitos de largura total em um texto para a largura normal.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## BAHTTEXT

`BAHTTEXT(número)`

Escreve um número em palavras tailandesas, com a palavra para baht.

- `número`: O número a ser processado.

## CARACT

`CARACT(número)`

Retorna o caractere que corresponde a um número de código.

- `número`: O número a ser processado.

## LIMPAR

`LIMPAR(texto)`

Remove de um texto os caracteres que não são impressos.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## CÓDIGO

`CÓDIGO(texto)`

Retorna o número de código do primeiro caractere de um texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## CONCAT

`CONCAT(texto1, ...)`

Une textos e os textos de intervalos de células em um só.

- `texto1`: O primeiro texto, ou intervalo de células com texto.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## CONCATENAR

`CONCATENAR(texto1, ...)`

Une textos em um só.

- `texto1`: O primeiro texto, ou intervalo de células com texto.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## DBCS

`DBCS(texto)`

Altera letras e dígitos de largura normal em um texto para a largura total.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## DÓLAR

`DÓLAR(número, [decimais])`

Transforma um número em texto como valor monetário.

- `número`: O número a ser processado.
- `decimais` (opcional): Quantos dígitos manter após o ponto decimal.

## CODIFURLWEB

`CODIFURLWEB(texto)`

Altera um texto para que possa ser usado em um endereço da web.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## EXATO

`EXATO(texto1, texto2)`

Verdadeiro quando dois textos são exatamente iguais, incluindo maiúsculas e minúsculas.

- `texto1`: O primeiro texto, ou intervalo de células com texto.
- `texto2`: O segundo texto.

## FILTERXML

`FILTERXML(xml, xpath)`

Extrai valores de um texto que contém XML, usando uma expressão XPath.

- `xml`: O texto que contém o XML.
- `xpath`: O XPath que indica quais partes retornar, como //item/price.

## LOCALIZAR

`LOCALIZAR(texto_procurado, texto_original, [núm_inicial])`

Retorna a posição em que um texto começa dentro de outro, diferenciando maiúsculas de minúsculas.

- `texto_procurado`: O texto a localizar.
- `texto_original`: O texto em que procurar.
- `núm_inicial` (opcional): A posição do texto em que começar a busca, contando o primeiro caractere como 1.

## LOCALIZARB

`LOCALIZARB(texto_procurado, texto_original, [núm_inicial])`

Retorna a posição em que um texto aparece pela primeira vez em outro texto, diferenciando maiúsculas.

- `texto_procurado`: O texto a localizar.
- `texto_original`: O texto em que procurar.
- `núm_inicial` (opcional): A posição do texto em que começar a busca, contando o primeiro caractere como 1.

## FIXO

`FIXO(número, [decimais], [sem_separador])`

Transforma um número em texto com uma quantidade fixa de casas decimais.

- `número`: O número a ser processado.
- `decimais` (opcional): Quantos dígitos manter após o ponto decimal.
- `no_commas` (opcional): Verdadeiro para omitir as vírgulas que agrupam os milhares.

## ESQUERDA

`ESQUERDA(texto, [núm_caract])`

Retorna os primeiros caracteres de um texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `núm_caract` (opcional): Quantos caracteres pegar (1, se omitido).

## ESQUERDAB

`ESQUERDAB(texto, [núm_caract])`

Retorna os primeiros caracteres do texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `núm_caract` (opcional): Quantos caracteres pegar (1, se omitido).

## NÚM.CARACT

`NÚM.CARACT(texto)`

Conta os caracteres de um texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## NÚM.CARACTB

`NÚM.CARACTB(texto)`

Retorna o número de caracteres do texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## MINÚSCULA

`MINÚSCULA(texto)`

Transforma um texto em letras minúsculas.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## EXT.TEXTO

`EXT.TEXTO(texto, núm_inicial, núm_caract)`

Retorna caracteres do meio de um texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `núm_inicial`: A posição do texto em que começar, contando o primeiro caractere como 1.
- `núm_caract`: Quantos caracteres pegar (1, se omitido).

## EXT.TEXTOB

`EXT.TEXTOB(texto, núm_inicial, núm_caract)`

Retorna caracteres do meio do texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `núm_inicial`: A posição do texto em que começar, contando o primeiro caractere como 1.
- `núm_caract`: Quantos caracteres pegar (1, se omitido).

## NUMBERVALUE

`NUMBERVALUE(texto, [separador_decimal], [separador_grupo])`

Lê um texto como número, com os separadores que você indicar.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `separador_decimal` (opcional): O caractere que marca as casas decimais no texto (o ponto, se omitido).
- `separador_grupo` (opcional): O caractere que agrupa os dígitos no texto (a vírgula, se omitido).

## FONÉTICA

`FONÉTICA(referência)`

Retorna o texto de uma célula, para as leituras de textos em japonês.

- `referência`: Uma célula ou intervalo de células.

## PRI.MAIÚSCULA

`PRI.MAIÚSCULA(texto)`

Coloca em maiúscula a primeira letra de cada palavra de um texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## SUBSTITUIR

`SUBSTITUIR(texto_antigo, núm_inicial, núm_caract, novo_texto)`

Coloca um novo texto no lugar de alguns caracteres de um texto.

- `texto_antigo`: O texto a alterar.
- `núm_inicial`: A posição do texto em que começar, contando o primeiro caractere como 1.
- `núm_caract`: Quantos caracteres pegar (1, se omitido).
- `novo_texto`: O texto a inserir.

## SUBSTITUIRB

`SUBSTITUIRB(texto_antigo, núm_inicial, núm_caract, novo_texto)`

Substitui parte do texto por outro texto.

- `texto_antigo`: O texto a alterar.
- `núm_inicial`: A posição do texto em que começar, contando o primeiro caractere como 1.
- `núm_caract`: Quantos caracteres pegar (1, se omitido).
- `novo_texto`: O texto a inserir.

## REPT

`REPT(texto, núm_vezes)`

Repete um texto um determinado número de vezes.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `núm_vezes`: Quantas vezes repetir o texto.

## DIREITA

`DIREITA(texto, [núm_caract])`

Retorna os últimos caracteres de um texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `núm_caract` (opcional): Quantos caracteres pegar (1, se omitido).

## DIREITAB

`DIREITAB(texto, [núm_caract])`

Retorna os últimos caracteres do texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `núm_caract` (opcional): Quantos caracteres pegar (1, se omitido).

## PROCURAR

`PROCURAR(texto_procurado, texto_original, [núm_inicial])`

Retorna a posição em que um texto começa dentro de outro, ignorando maiúsculas e minúsculas, e aceitando ? e * no que está sendo procurado.

- `texto_procurado`: O texto a localizar.
- `texto_original`: O texto em que procurar.
- `núm_inicial` (opcional): A posição do texto em que começar a busca, contando o primeiro caractere como 1.

## PROCURARB

`PROCURARB(texto_procurado, texto_original, [núm_inicial])`

Retorna a posição em que um texto aparece pela primeira vez em outro texto, ignorando maiúsculas e minúsculas.

- `texto_procurado`: O texto a localizar.
- `texto_original`: O texto em que procurar.
- `núm_inicial` (opcional): A posição do texto em que começar a busca, contando o primeiro caractere como 1.

## SUBSTITUIR

`SUBSTITUIR(texto, texto_antigo, novo_texto, [núm_ocorrência])`

Troca um trecho de texto por outro sempre que ele aparecer.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `texto_antigo`: O texto a localizar.
- `novo_texto`: O texto a inserir.
- `núm_ocorrência` (opcional): Qual ocorrência substituir (todas, se omitido).

## T

`T(valor)`

Retorna o próprio texto, e nada para o que não for texto.

- `valor`: O valor a ser processado, ou uma célula que o contenha.

## TEXTO

`TEXTO(valor, formato_texto)`

Transforma um número em texto em um formato que você escolher.

- `valor`: O valor a ser processado, ou uma célula que o contenha.
- `formato_texto`: O formato de número em que o valor será exibido, como "0,00" ou "dd/mm/aaaa".

## TEXTOJUNTO

`TEXTOJUNTO(delimitador, ignorar_vazio, texto1, ...)`

Une textos com um separador entre eles.

- `delimitador`: O texto a inserir entre as partes.
- `ignorar_vazio`: Verdadeiro para omitir as partes vazias; falso para mantê-las.
- `texto1`: O primeiro texto, ou intervalo de células com texto.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## ARRUMAR

`ARRUMAR(texto)`

Remove espaços extras de um texto, deixando um único espaço entre as palavras.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## UNICHAR

`UNICHAR(número)`

Retorna o caractere que corresponde a um número Unicode.

- `número`: O número a ser processado.

## UNICODE

`UNICODE(texto)`

Retorna o número Unicode do primeiro caractere do texto.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## MAIÚSCULA

`MAIÚSCULA(texto)`

Transforma um texto em letras maiúsculas.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## VALOR

`VALOR(texto)`

Transforma um texto que parece um número em número.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
