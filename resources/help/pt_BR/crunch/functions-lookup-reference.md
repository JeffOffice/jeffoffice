# Funções de pesquisa e referência
<!-- keywords: funções, ADDRESS, AREAS, COLUMN, COLUMNS, FILTER, FORMULATEXT, GETPIVOTDATA, HLOOKUP, HYPERLINK, INDEX, INDIRECT, LOOKUP, MATCH, OFFSET, ROW, ROWS, SORT, SORTBY, TRANSPOSE, UNIQUE, VLOOKUP, XLOOKUP, XMATCH -->

Estas são as funções de pesquisa e referência que o JeffCrunch consegue calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros do último tipo forem necessários. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra seus argumentos, e um clique no nome da função na dica abre o item correspondente aqui.

## ADDRESS

`ADDRESS(row_num, column_num, [abs_num], [a1], [sheet_text])`

Escreve o endereço de uma célula como texto, a partir de seus números de linha e coluna.

- `row_num`: O número da linha no intervalo, contando a primeira linha como 1.
- `column_num`: O número da coluna no intervalo, contando a primeira coluna como 1.
- `abs_num` (opcional): Quanto do endereço fica fixo: 1 tudo, 2 somente a linha, 3 somente a coluna, 4 nada.
- `a1` (opcional): Verdadeiro ou omitido para ler o endereço como letra de coluna e número de linha; falso para ler como números de linha e coluna.
- `sheet_text` (opcional): O nome da planilha a colocar antes do endereço.

## AREAS

`AREAS(reference)`

Informa o número de intervalos separados de células em uma referência.

- `reference`: Uma célula ou intervalo de células.

## COLUMN

`COLUMN([reference])`

Informa o número da coluna de uma célula.

- `reference` (opcional): Uma célula ou intervalo de células.

## COLUMNS

`COLUMNS(array)`

Conta as colunas de um intervalo de células.

- `array`: Um intervalo de células ou uma lista de valores escrita entre chaves.

## FILTER

`FILTER(array, include, [if_empty])`

Informa as linhas ou colunas de um intervalo que passam em um teste.

- `array`: Um intervalo de células ou uma lista de valores escrita entre chaves.
- `include`: Uma coluna ou linha de valores verdadeiro e falso, um para cada linha ou coluna do intervalo: verdadeiro mantém a linha ou coluna.
- `if_empty` (opcional): O que retornar quando nenhuma linha ou coluna passar no teste (o erro #CALC! se omitido).

## FORMULATEXT

`FORMULATEXT(reference)`

Informa a fórmula de uma célula como texto.

- `reference`: Uma célula ou intervalo de células.

## GETPIVOTDATA

`GETPIVOTDATA(data_field, pivot_table, [field1], [item1], ...)`

Informa o valor que uma tabela dinâmica mostra para um valor e para os itens de suas linhas e colunas. Uma célula da tabela é o segundo argumento; os campos indicados devem ser os primeiros das linhas ou das colunas.

- `data_field`: O nome do valor a obter, entre aspas, como a tabela dinâmica o mostra ("Soma de Amount").
- `pivot_table`: Qualquer célula da tabela dinâmica.
- `field1` (opcional): O nome de um campo das linhas ou das colunas, entre aspas (pode ser omitido junto com o item).
- `item1` (opcional): O item desse campo para o qual se deseja obter o valor, entre aspas.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## HLOOKUP

`HLOOKUP(lookup_value, table_array, row_index_num, [range_lookup])`

Procura um valor na primeira linha de uma tabela e informa um valor da mesma coluna.

- `lookup_value`: O valor a procurar.
- `table_array`: O intervalo de células em que procurar; o valor procurado deve estar em sua primeira coluna.
- `row_index_num`: O número da linha da tabela da qual obter um valor, contando a primeira linha como 1.
- `range_lookup` (opcional): Falso encontra uma correspondência exata; verdadeiro ou omitido encontra o valor mais próximo abaixo dele (a primeira coluna deve estar classificada em ordem crescente).

## HYPERLINK

`HYPERLINK(link_location, [friendly_name])`

Cria uma célula que abre um link quando é clicada.

- `link_location`: Para onde o link leva: um endereço da web, um arquivo ou um local nesta pasta de trabalho.
- `friendly_name` (opcional): O texto a mostrar na célula (o próprio link, se omitido).

## INDEX

`INDEX(array, row_num, [column_num], [area_num])`

Informa o valor em uma linha e coluna de um intervalo de células.

- `array`: Um intervalo de células ou uma lista de valores escrita entre chaves.
- `row_num`: O número da linha no intervalo, contando a primeira linha como 1.
- `column_num` (opcional): O número da coluna no intervalo, contando a primeira coluna como 1.
- `area_num` (opcional): Qual intervalo usar quando o primeiro argumento indica vários (1 para o primeiro).

## INDIRECT

`INDIRECT(ref_text, [a1])`

Informa a célula para a qual aponta um endereço escrito como texto.

- `ref_text`: Um endereço escrito como texto, como "B5".
- `a1` (opcional): Verdadeiro ou omitido para ler o endereço como letra de coluna e número de linha; falso para ler como números de linha e coluna.

## LOOKUP

`LOOKUP(lookup_value, lookup_vector, [result_vector])`

Procura um valor em uma linha ou coluna e informa o valor na mesma posição de outra.

- `lookup_value`: O valor a procurar.
- `lookup_vector`: A única linha ou a única coluna de células em que procurar.
- `result_vector` (opcional): A única linha ou a única coluna de células da qual obter o resultado.

## MATCH

`MATCH(lookup_value, lookup_array, [match_type])`

Informa a posição de um valor em uma lista.

- `lookup_value`: O valor a procurar.
- `lookup_array`: O intervalo de células, uma linha ou uma coluna, em que procurar.
- `match_type` (opcional): 1 ou omitido encontra o maior valor que não seja superior a ele (a lista deve estar classificada em ordem crescente); 0 encontra uma correspondência exata; -1 encontra o menor valor que não seja inferior a ele (a lista deve estar classificada em ordem decrescente).

## OFFSET

`OFFSET(reference, rows, cols, [height], [width])`

Informa as células a um determinado número de linhas e colunas de uma célula inicial.

- `reference`: Uma célula ou intervalo de células.
- `rows`: Quantas linhas mover: positivo para baixo, negativo para cima.
- `cols`: Quantas colunas mover: positivo para a direita, negativo para a esquerda.
- `height` (opcional): Quantas linhas de altura terá o resultado.
- `width` (opcional): Quantas colunas de largura terá o resultado.

## ROW

`ROW([reference])`

Informa o número da linha de uma célula.

- `reference` (opcional): Uma célula ou intervalo de células.

## ROWS

`ROWS(array)`

Conta as linhas de um intervalo de células.

- `array`: Um intervalo de células ou uma lista de valores escrita entre chaves.

## SORT

`SORT(array, [sort_index], [sort_order], [by_col])`

Informa um intervalo com suas linhas ou colunas colocadas em ordem.

- `array`: Um intervalo de células ou uma lista de valores escrita entre chaves.
- `sort_index` (opcional): O número da coluna (ou linha) pela qual classificar (1, se omitido).
- `sort_order` (opcional): 1 ou omitido para crescente; -1 para decrescente.
- `by_col` (opcional): Verdadeiro para trabalhar com colunas; falso ou omitido para trabalhar com linhas.

## SORTBY

`SORTBY(array, by_array1, [sort_order1], ...)`

Informa um intervalo colocado na ordem de um ou mais outros intervalos.

- `array`: Um intervalo de células ou uma lista de valores escrita entre chaves.
- `by_array1`: O intervalo de células pelo qual ordenar o primeiro intervalo; deve ter o mesmo tamanho do primeiro.
- `sort_order1` (opcional): 1 ou omitido para crescente; -1 para decrescente.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## TRANSPOSE

`TRANSPOSE(array)`

Transforma linhas em colunas e colunas em linhas.

- `array`: Um intervalo de células ou uma lista de valores escrita entre chaves.

## UNIQUE

`UNIQUE(array, [by_col], [exactly_once])`

Informa as linhas ou colunas diferentes de um intervalo.

- `array`: Um intervalo de células ou uma lista de valores escrita entre chaves.
- `by_col` (opcional): Verdadeiro para trabalhar com colunas; falso ou omitido para trabalhar com linhas.
- `exactly_once` (opcional): Verdadeiro para informar somente os itens que aparecem uma única vez; falso ou omitido para informar cada item diferente.

## VLOOKUP

`VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])`

Procura um valor na primeira coluna de uma tabela e informa um valor da mesma linha.

- `lookup_value`: O valor a procurar.
- `table_array`: O intervalo de células em que procurar; o valor procurado deve estar em sua primeira coluna.
- `col_index_num`: O número da coluna da tabela da qual obter um valor, contando a primeira coluna como 1.
- `range_lookup` (opcional): Falso encontra uma correspondência exata; verdadeiro ou omitido encontra o valor mais próximo abaixo dele (a primeira coluna deve estar classificada em ordem crescente).

## XLOOKUP

`XLOOKUP(lookup_value, lookup_array, return_array, [if_not_found], [match_mode], [search_mode])`

Procura um valor em uma lista e informa o item correspondente de outra lista.

- `lookup_value`: O valor a procurar.
- `lookup_array`: O intervalo de células, uma linha ou uma coluna, em que procurar.
- `return_array`: O intervalo de células do qual obter o resultado da correspondência.
- `if_not_found` (opcional): O que retornar quando não houver correspondência (o erro #N/D se omitido).
- `match_mode` (opcional): 0 ou omitido para correspondência exata; -1 para o próximo menor; 1 para o próximo maior; 2 para usar curingas.
- `search_mode` (opcional): 1 ou omitido para pesquisar do primeiro; -1 do último; 2 ou -2 para uma pesquisa binária em uma lista classificada.

## XMATCH

`XMATCH(lookup_value, lookup_array, [match_mode], [search_mode])`

Informa a posição de um valor em uma lista, com opções de como correspondê-lo e em que sentido procurar.

- `lookup_value`: O valor a procurar.
- `lookup_array`: O intervalo de células, uma linha ou uma coluna, em que procurar.
- `match_mode` (opcional): 0 ou omitido para correspondência exata; -1 para o próximo menor; 1 para o próximo maior; 2 para usar curingas.
- `search_mode` (opcional): 1 ou omitido para pesquisar do primeiro; -1 do último; 2 ou -2 para uma pesquisa binária em uma lista classificada.
