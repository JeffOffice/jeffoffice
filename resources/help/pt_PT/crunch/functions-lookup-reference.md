# Funções de pesquisa e referência
<!-- keywords: functions, ADDRESS, AREAS, COLUMN, COLUMNS, FILTER, FORMULATEXT, GETPIVOTDATA, HLOOKUP, HYPERLINK, INDEX, INDIRECT, LOOKUP, MATCH, OFFSET, ROW, ROWS, SORT, SORTBY, TRANSPOSE, UNIQUE, VLOOKUP, XLOOKUP, XMATCH -->

Estas são as funções de pesquisa e referência que o JeffCrunch consegue calcular. Cada uma mostra como se escreve, o que faz e o que é cada um dos seus argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos mais do último quantos precisar. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, aparece uma pequena sugestão por baixo da célula com os respetivos argumentos, e se clicar no nome da função nessa sugestão, abre-se a respetiva entrada aqui.

## ADDRESS

`ADDRESS(row_num, column_num, [abs_num], [a1], [sheet_text])`

Escreve o endereço de uma célula como texto, a partir dos respetivos números de linha e de coluna.

- `row_num`: O número da linha no bloco, contando a primeira linha como 1.
- `column_num`: O número da coluna no bloco, contando a primeira coluna como 1.
- `abs_num` (opcional): Quanto do endereço fica fixo: 1 tudo, 2 apenas a linha, 3 apenas a coluna, 4 nada.
- `a1` (opcional): Verdadeiro ou omitido para ler o endereço como uma letra de coluna e um número de linha; falso para o ler como números de linha e de coluna.
- `sheet_text` (opcional): O nome da folha a colocar antes do endereço.

## AREAS

`AREAS(reference)`

Indica o número de blocos separados de células numa referência.

- `reference`: Uma célula ou um bloco de células.

## COLUMN

`COLUMN([reference])`

Devolve o número da coluna de uma célula.

- `reference` (opcional): Uma célula ou um bloco de células.

## COLUMNS

`COLUMNS(array)`

Conta as colunas de um bloco de células.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.

## FILTER

`FILTER(array, include, [if_empty])`

Devolve as linhas ou colunas de um bloco que cumprem um teste.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `include`: Uma coluna ou linha de valores verdadeiro e falso, um para cada linha ou coluna do bloco: verdadeiro mantém-na.
- `if_empty` (opcional): O que devolver quando nenhuma linha ou coluna cumpre o teste (o erro #CALC! se for omitido).

## FORMULATEXT

`FORMULATEXT(reference)`

Devolve a fórmula de uma célula como texto.

- `reference`: Uma célula ou um bloco de células.

## GETPIVOTDATA

`GETPIVOTDATA(data_field, pivot_table, [field1], [item1], ...)`

Devolve o valor que uma tabela dinâmica mostra para um valor e para os itens das suas linhas e colunas. Uma célula da tabela é o segundo argumento; os campos indicados têm de ser os primeiros das linhas ou das colunas.

- `data_field`: O nome do valor a obter, entre aspas, tal como a tabela dinâmica o mostra ("Soma de Montante").
- `pivot_table`: Qualquer célula da tabela dinâmica.
- `field1` (opcional): O nome de um campo das linhas ou das colunas, entre aspas (pode ser omitido juntamente com o item).
- `item1` (opcional): O item desse campo para o qual obter o valor, entre aspas.
- `...`: Tantos mais quantos precisar, do mesmo tipo do argumento anterior.

## HLOOKUP

`HLOOKUP(lookup_value, table_array, row_index_num, [range_lookup])`

Procura um valor na primeira linha de uma tabela e devolve um valor da mesma coluna.

- `lookup_value`: O valor a procurar.
- `table_array`: O bloco de células onde procurar; o valor procurado tem de estar na respetiva primeira coluna.
- `row_index_num`: O número da linha da tabela da qual devolver um valor, contando a primeira linha como 1.
- `range_lookup` (opcional): Falso encontra uma correspondência exata; verdadeiro ou omitido encontra o valor mais próximo abaixo dele (a primeira coluna tem de estar ordenada crescentemente).

## HYPERLINK

`HYPERLINK(link_location, [friendly_name])`

Cria uma célula que abre uma ligação quando é clicada.

- `link_location`: Para onde vai a ligação: um endereço Web, um ficheiro ou um local deste livro.
- `friendly_name` (opcional): O texto a mostrar na célula (a própria ligação, se for omitido).

## INDEX

`INDEX(array, row_num, [column_num], [area_num])`

Devolve o valor numa linha e coluna de um bloco de células.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `row_num`: O número da linha no bloco, contando a primeira linha como 1.
- `column_num` (opcional): O número da coluna no bloco, contando a primeira coluna como 1.
- `area_num` (opcional): Qual o bloco a utilizar quando o primeiro argumento indica vários (1 para o primeiro).

## INDIRECT

`INDIRECT(ref_text, [a1])`

Devolve a célula para a qual aponta um endereço escrito como texto.

- `ref_text`: Um endereço escrito como texto, como "B5".
- `a1` (opcional): Verdadeiro ou omitido para ler o endereço como uma letra de coluna e um número de linha; falso para o ler como números de linha e de coluna.

## LOOKUP

`LOOKUP(lookup_value, lookup_vector, [result_vector])`

Procura um valor numa linha ou coluna e devolve o valor na mesma posição de outra.

- `lookup_value`: O valor a procurar.
- `lookup_vector`: A única linha ou a única coluna de células onde procurar.
- `result_vector` (opcional): A única linha ou a única coluna de células da qual devolver o resultado.

## MATCH

`MATCH(lookup_value, lookup_array, [match_type])`

Devolve a posição de um valor numa lista.

- `lookup_value`: O valor a procurar.
- `lookup_array`: O bloco de células, uma linha ou uma coluna, onde procurar.
- `match_type` (opcional): 1 ou omitido encontra o maior valor que não seja superior a ele (a lista tem de estar ordenada crescentemente); 0 encontra uma correspondência exata; -1 encontra o menor valor que não seja inferior a ele (a lista tem de estar ordenada decrescentemente).

## OFFSET

`OFFSET(reference, rows, cols, [height], [width])`

Devolve as células que estão a um determinado número de linhas e colunas de uma célula inicial.

- `reference`: Uma célula ou um bloco de células.
- `rows`: Quantas linhas deslocar: positivo para baixo, negativo para cima.
- `cols`: Quantas colunas deslocar: positivo para a direita, negativo para a esquerda.
- `height` (opcional): Quantas linhas de altura tem o resultado.
- `width` (opcional): Quantas colunas de largura tem o resultado.

## ROW

`ROW([reference])`

Devolve o número da linha de uma célula.

- `reference` (opcional): Uma célula ou um bloco de células.

## ROWS

`ROWS(array)`

Conta as linhas de um bloco de células.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.

## SORT

`SORT(array, [sort_index], [sort_order], [by_col])`

Devolve um bloco com as suas linhas ou colunas ordenadas.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `sort_index` (opcional): O número da coluna (ou linha) pela qual ordenar (1 se for omitido).
- `sort_order` (opcional): 1 ou omitido para crescente; -1 para decrescente.
- `by_col` (opcional): Verdadeiro para trabalhar com colunas; falso ou omitido para trabalhar com linhas.

## SORTBY

`SORTBY(array, by_array1, [sort_order1], ...)`

Devolve um bloco ordenado de acordo com um ou mais outros blocos.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `by_array1`: O bloco de células segundo o qual ordenar o primeiro bloco; com o mesmo comprimento do primeiro bloco.
- `sort_order1` (opcional): 1 ou omitido para crescente; -1 para decrescente.
- `...`: Tantos mais quantos precisar, do mesmo tipo do argumento anterior.

## TRANSPOSE

`TRANSPOSE(array)`

Transforma linhas em colunas e colunas em linhas.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.

## UNIQUE

`UNIQUE(array, [by_col], [exactly_once])`

Devolve as linhas ou colunas diferentes de um bloco.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `by_col` (opcional): Verdadeiro para trabalhar com colunas; falso ou omitido para trabalhar com linhas.
- `exactly_once` (opcional): Verdadeiro para devolver apenas os itens que aparecem uma só vez; falso ou omitido para devolver cada item diferente.

## VLOOKUP

`VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])`

Procura um valor na primeira coluna de uma tabela e devolve um valor da mesma linha.

- `lookup_value`: O valor a procurar.
- `table_array`: O bloco de células onde procurar; o valor procurado tem de estar na respetiva primeira coluna.
- `col_index_num`: O número da coluna da tabela da qual devolver um valor, contando a primeira coluna como 1.
- `range_lookup` (opcional): Falso encontra uma correspondência exata; verdadeiro ou omitido encontra o valor mais próximo abaixo dele (a primeira coluna tem de estar ordenada crescentemente).

## XLOOKUP

`XLOOKUP(lookup_value, lookup_array, return_array, [if_not_found], [match_mode], [search_mode])`

Procura um valor numa lista e devolve o item correspondente de outra lista.

- `lookup_value`: O valor a procurar.
- `lookup_array`: O bloco de células, uma linha ou uma coluna, onde procurar.
- `return_array`: O bloco de células do qual devolver a correspondência.
- `if_not_found` (opcional): O que devolver quando não há correspondência (o erro #N/D se for omitido).
- `match_mode` (opcional): 0 ou omitido para uma correspondência exata; -1 para o valor imediatamente inferior; 1 para o valor imediatamente superior; 2 para utilizar caracteres universais.
- `search_mode` (opcional): 1 ou omitido para pesquisar do primeiro; -1 do último; 2 ou -2 para uma pesquisa binária numa lista ordenada.

## XMATCH

`XMATCH(lookup_value, lookup_array, [match_mode], [search_mode])`

Devolve a posição de um valor numa lista, com opções para o modo de correspondência e o sentido da pesquisa.

- `lookup_value`: O valor a procurar.
- `lookup_array`: O bloco de células, uma linha ou uma coluna, onde procurar.
- `match_mode` (opcional): 0 ou omitido para uma correspondência exata; -1 para o valor imediatamente inferior; 1 para o valor imediatamente superior; 2 para utilizar caracteres universais.
- `search_mode` (opcional): 1 ou omitido para pesquisar do primeiro; -1 do último; 2 ou -2 para uma pesquisa binária numa lista ordenada.
