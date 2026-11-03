# Tabelas dinâmicas
<!-- keywords: tabela dinâmica, inserir tabela dinâmica, criar, fazer, getpivotdata, gerar getpivotdata, atualizar, atualizar tudo, alterar fonte de dados, lista de campos, campos, linhas, colunas, valores, filtros, filtro de relatório, total geral, subtotal, layout, compacto, estrutura de tópicos, tabular, limpar, configurações do campo de valor, soma, contagem, média -->

Uma tabela dinâmica é um pequeno relatório feito a partir de uma lista longa. A lista tem uma linha para cada venda, por exemplo, e o relatório soma as vendas por região e por mês. Você não escreve nenhuma fórmula. Basta indicar quais colunas da lista ficam na lateral, quais ficam na parte superior e qual delas é somada, e o relatório se preenche sozinho.

## Um exemplo

Esta é uma lista nas células A1 a C7. Cada linha é uma venda.

| Região | Mês | Valor |
|---|---|---|
| Leste | Jan | 10 |
| Leste | Jan | 5 |
| Leste | Fev | 20 |
| Oeste | Jan | 7 |
| Oeste | Fev | 3 |
| Oeste | Fev | 4 |

Uma tabela dinâmica com **Região** na lateral, **Mês** na parte superior e a soma de **Valor** no meio fica assim.

| Soma de Valor | Mês | | |
|---|---|---|---|
| Região | Jan | Fev | Total Geral |
| Leste | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total Geral | 22 | 27 | 49 |

Leste em janeiro é 15 porque as duas linhas de Leste em janeiro, 10 e 5, são somadas. A coluna Total Geral soma cada linha, a linha Total Geral soma cada coluna e a célula do canto soma tudo.

Os termos usados aqui:

- **Campo** é uma coluna da lista. Região, Mês e Valor são os campos.
- **Linhas**, **Colunas** e **Valores** são os três lugares onde um campo pode ficar. Um campo em Linhas gera uma linha para cada uma de suas entradas diferentes; um campo em Colunas gera uma coluna para cada uma; um campo em Valores é o que é somado. Um quarto lugar, **Filtros**, contém um campo que exclui as linhas que você não escolher.
- O **cache** (o cache da tabela dinâmica) é uma cópia da sua lista que a tabela dinâmica guarda dentro do arquivo, para poder mostrar os totais sem consultar as células. O relatório é calculado a partir do cache, e não das células, por isso ele não muda quando as células mudam. **Atualizar** copia as células para o cache novamente.

## Criar uma tabela dinâmica

1. Clique em uma célula da lista ou selecione as células que deseja usar. A lista precisa ter uma linha de nomes acima dos dados.
2. Escolha **Inserir** e depois **Tabela Dinâmica**.
3. A caixa mostra as células em que os dados estão. Altere-as se não estiverem corretas. Você também pode digitar o nome de uma tabela.
4. Escolha onde colocar a tabela dinâmica: **Uma nova planilha** (o padrão) ou uma célula desta planilha ou de outra. Escolha um local vazio, pois a tabela vazia ocupa cerca de dezoito linhas e três colunas.
5. Escolha **OK**. A tabela começa vazia e o painel **Campos da tabela dinâmica** é aberto. Marque os campos desejados: um campo de números é somado e qualquer outro campo vira uma linha. Depois, mova os campos para as caixas conforme explicado nas próximas seções.

A nova planilha é uma etapa própria de Desfazer, e a tabela também. Quando você salva como .xlsx, a tabela é gravada da forma como o programa original grava uma, por isso ela é aberta lá como tabela dinâmica. Uma cópia salva como .xls ou .ods mantém os números que a tabela mostra nas células, mas não a própria tabela, e o salvamento avisa sobre isso.

## GETPIVOTDATA: ler um número de uma tabela dinâmica

`GETPIVOTDATA` retorna o número que uma tabela dinâmica mostra para os valores e as linhas e colunas que você indicar. Para o exemplo acima:

`=GETPIVOTDATA("Soma de Valor", $E$3, "Região", "Leste", "Mês", "Jan")` retorna 15.

- O primeiro argumento é o nome do valor, como a tabela o mostra (por exemplo, Soma de Valor). O nome do campo somado (Valor) também funciona.
- O segundo argumento é qualquer célula da tabela dinâmica.
- Em seguida vêm pares formados por um campo e um de seus itens. Indique apenas Região, Leste, e você obtém o total de Leste, 35. Não indique nada, e você obtém o total geral, 49. Os campos indicados devem ser os primeiros campos das linhas, ou os primeiros campos das colunas, na ordem.
- Se a tabela não mostrar o número, a célula mostra `#REF!`.

Quando você clica em um valor de uma tabela dinâmica enquanto digita uma fórmula, o JeffCrunch escreve essa fórmula para você, para que ela continue lendo o número correto depois que a tabela for reorganizada. Para desativar isso, escolha **Opções** na guia **Tabela Dinâmica** e desmarque a caixa de seleção referente a GETPIVOTDATA.

## O que o JeffCrunch faz com uma tabela dinâmica

Uma tabela dinâmica em uma pasta de trabalho é mostrada como foi salva. Se você não alterar nada nela, o arquivo é salvo com a tabela dinâmica exatamente como estava. Quando você a atualiza ou altera, o JeffCrunch calcula o relatório novamente, coloca os novos números nas células e grava a tabela dinâmica de volta de modo que o programa que a criou a abra sem problemas.

O JeffCrunch calcula tabelas dinâmicas que somam, contam, calculam médias e assim por diante. Uma tabela dinâmica que agrupa datas ou números, tem um campo ou item calculado, mostra valores como porcentagem, diferença ou classificação, ou usa um modelo de dados é mostrada e mantida como foi salva, e os comandos avisam que não podem alterá-la.

Clique em qualquer célula de uma tabela dinâmica. A guia **Tabela Dinâmica** aparece.

## Atualizar

**Atualizar** lê as células a partir das quais a tabela dinâmica é feita e calcula o relatório novamente. As novas entradas vêm depois das que já existem; as entradas que não existem mais são removidas. As larguras das colunas e o layout escolhido são mantidos. **Atualizar Tudo**, na guia **Dados** e na guia **Tabela Dinâmica**, faz o mesmo para todas as tabelas dinâmicas da pasta de trabalho.

Se o relatório for crescer sobre células que têm conteúdo, o JeffCrunch não o atualiza. Ele informa quais células estão no caminho. Limpe-as ou mova a tabela dinâmica e atualize novamente.

**Desfazer** desfaz uma atualização em uma única etapa.

## Alterar fonte de dados

**Alterar fonte de dados** pede as células em que a lista está, incluindo a primeira linha de nomes. Digite-as no formato `Planilha1!A1:D100`, ou digite o nome de uma tabela ou de um nome que você criou. O JeffCrunch as lê e calcula o relatório novamente. Se um campo da lista tiver o mesmo nome nas novas células, a posição dele no relatório é mantida.

## Limpar

**Limpar** tem duas opções. **Limpar Filtros** remove todos os filtros e mostra todas as entradas novamente. **Limpar Tudo** retira todos os campos do relatório e o deixa vazio, pronto para ser montado de novo.

## O painel de campos

**Lista de campos**, na guia **Tabela Dinâmica**, abre o painel **Campos da tabela dinâmica**. Ele lista os campos da lista com uma caixa ao lado de cada um e, abaixo, os quatro lugares: **Filtros**, **Colunas**, **Linhas** e **Valores**.

- Marque um campo para adicioná-lo. Um campo que contém apenas números vai para **Valores** e é somado; qualquer outro vai para **Linhas**.
- Desmarque um campo para retirá-lo de todos os lugares.
- Clique em um campo em um dos quatro lugares e escolha **Mover para** para colocá-lo em outro. **Mover para Cima** e **Mover para Baixo** alteram a ordem dele no lugar em que está, e **Remover campo** o retira.
- Com mais de um valor, uma linha chamada **Valores** aparece em **Colunas**. Mova-a para **Linhas** para colocar os valores na lateral.
- **Configurações do campo de valor** (clique duas vezes em um valor ou use o botão) define o nome que o relatório mostra para o valor e como ele é totalizado: **Soma**, **Contagem**, **Contar números**, **Média**, **Máx.**, **Mín.**, **Produto**, **DesvPad** e **DesvPadp** (o desvio padrão de uma amostra ou de todos os valores) e **Var** e **Varp** (a variância dos mesmos).

Cada alteração é uma etapa de **Desfazer**.

## Design: subtotais, totais gerais e layout

- **Subtotais** mostra ou oculta o total de cada grupo, no início ou no final do grupo. Quando há mais de um campo em Linhas, o total de cada campo externo é um subtotal.
- **Totais Gerais** tem quatro opções: desabilitado para linhas e colunas, habilitado para ambos, habilitado somente para linhas (uma coluna de totais à direita, um para cada linha) e habilitado somente para colunas (uma linha de totais na parte inferior).
- **Layout do Relatório** é a disposição dos rótulos. No formato **compacto**, todos os campos de Linhas compartilham uma coluna, e cada novo grupo aparece um pouco mais à direita. No formato de **estrutura de tópicos**, cada campo tem sua própria coluna, e cada grupo tem uma linha própria acima de seus itens. No formato **tabular**, cada campo tem sua própria coluna e o nome do grupo fica na primeira linha dele, com o subtotal abaixo do grupo.

## A mesma lista, com um filtro

Mova **Mês** de **Colunas** para **Filtros**. O relatório perde as colunas, e uma linha acima dele mostra **Mês** e **(Tudo)**. Os números passam a ser o total de cada região em todos os meses.

## Limites

Uma tabela dinâmica pode ter até 2.000 campos, uma lista de até cinco milhões de linhas e um relatório de até quatro milhões de células. Um arquivo que ultrapasse esses limites é mostrado como foi salvo e não pode ser alterado. Ler novamente cem mil linhas de vinte colunas e calcular o relatório levou cerca de meio segundo no computador em que isso foi medido; um computador mais lento leva mais tempo.
