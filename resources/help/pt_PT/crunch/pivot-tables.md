# Tabelas dinâmicas
<!-- keywords: tabela dinâmica, inserir tabela dinâmica, criar, fazer, getpivotdata, obterdadosdin, gerar getpivotdata, atualizar, atualizar tudo, alterar origem de dados, lista de campos, campos, linhas, colunas, valores, filtros, filtro de relatório, total geral, subtotal, esquema, compacto, destaques, tabela, limpar, definições do campo de valor, soma, contar, média -->

Uma tabela dinâmica é um pequeno relatório criado a partir de uma lista extensa. A lista tem, por exemplo, uma linha para cada venda; o relatório soma as vendas por região e por mês. Não precisa de escrever fórmulas. Indica quais as colunas da lista que ficam na lateral, quais ficam no topo e qual é somada, e o relatório preenche-se sozinho.

## Um exemplo

Eis uma lista nas células A1 a C7. Cada linha é uma venda.

| Região | Mês | Montante |
|---|---|---|
| Este | jan | 10 |
| Este | jan | 5 |
| Este | fev | 20 |
| Oeste | jan | 7 |
| Oeste | fev | 3 |
| Oeste | fev | 4 |

Uma tabela dinâmica com **Região** na lateral, **Mês** no topo e a soma de **Montante** no meio tem este aspeto.

| Soma de Montante | Mês | | |
|---|---|---|---|
| Região | jan | fev | Total Geral |
| Este | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total Geral | 22 | 27 | 49 |

Este em janeiro é 15 porque as duas linhas de Este em janeiro, 10 e 5, são somadas. A coluna Total Geral soma cada linha, a linha Total Geral soma cada coluna e a célula do canto soma tudo.

Os termos utilizados aqui:

- **Campo** é uma coluna da lista. Região, Mês e Montante são os campos.
- **Linhas**, **Colunas** e **Valores** são os três locais onde um campo pode ser colocado. Um campo em Linhas dá uma linha para cada uma das suas entradas diferentes; um campo em Colunas dá uma coluna para cada uma; um campo em Valores é o que é somado. Um quarto local, **Filtros**, contém um campo que exclui as linhas que não escolher.
- Uma **cache** (a cache da tabela dinâmica) é uma cópia da lista que a tabela dinâmica guarda dentro do ficheiro, para poder mostrar totais sem consultar as células. O relatório é calculado a partir da cache e não das células, pelo que não muda quando as células mudam. **Atualizar** copia novamente as células para a cache.

## Criar uma tabela dinâmica

1. Clique numa célula da lista ou selecione as células que pretende utilizar. A lista precisa de uma linha de nomes por cima dos dados.
2. Escolha **Inserir** e, em seguida, **Tabela Dinâmica**.
3. A caixa mostra as células onde estão os dados. Altere-as se não estiverem corretas. Também pode escrever o nome de uma tabela.
4. Escolha onde colocar a tabela dinâmica: **Uma nova folha** (a predefinição) ou uma célula nesta folha ou noutra. Escolha um local onde não exista mais nada, porque a tabela vazia ocupa cerca de dezoito linhas e três colunas.
5. Escolha **OK**. A tabela começa vazia e abre-se o painel **Campos da Tabela Dinâmica**. Selecione os campos pretendidos: um campo de números é somado e qualquer outro campo passa a ser uma linha. Em seguida, mova os campos para as caixas, conforme indicado nas secções seguintes.

Uma nova folha é um passo próprio de Anular, tal como a tabela. Quando guarda como .xlsx, a tabela é escrita tal como o programa original a escreve, pelo que abre aí como tabela dinâmica. Uma cópia guardada como .xls ou .ods mantém os números que a tabela mostra nas células, mas não a tabela em si, e a gravação informa-o disso.

## GETPIVOTDATA: ler um número de uma tabela dinâmica

`GETPIVOTDATA` devolve o número que uma tabela dinâmica mostra para os valores e as linhas e colunas que indicar. Para o exemplo acima:

`=GETPIVOTDATA("Soma de Montante", $E$3, "Região", "Este", "Mês", "jan")` devolve 15.

- O primeiro argumento é o nome do valor, tal como a tabela o mostra (por exemplo, Soma de Montante). O nome do campo que é somado (Montante) também funciona.
- O segundo argumento é qualquer célula da tabela dinâmica.
- Seguem-se pares de um campo e de um dos seus itens. Indique apenas Região, Este, e obtém o total de Este, 35. Não indique nada e obtém o total geral, 49. Os campos que indicar têm de ser os primeiros campos das linhas, ou os primeiros campos das colunas, pela ordem.
- Se a tabela não mostrar o número, a célula mostra `#REF!`.

Quando clica num valor de uma tabela dinâmica enquanto escreve uma fórmula, o JeffCrunch escreve esta fórmula por si, para que continue a ler o número correto depois de a tabela ser reorganizada. Para desativar esta funcionalidade, escolha **Opções** no separador **Tabela Dinâmica** e desmarque a caixa de verificação relativa a GETPIVOTDATA.

## O que o JeffCrunch faz com uma tabela dinâmica

Uma tabela dinâmica num livro é mostrada tal como foi guardada. Se não alterar nada nela, o ficheiro é guardado com a tabela dinâmica exatamente como estava. Quando a atualiza ou altera, o JeffCrunch volta a calcular o relatório, coloca os novos números nas células e volta a escrever a tabela dinâmica para que o programa que a criou a abra sem problemas.

O JeffCrunch calcula tabelas dinâmicas que somam, contam, calculam médias, etc. Uma tabela dinâmica que agrupa datas ou números, tem um campo ou item calculado, mostra valores como percentagem, diferença ou classificação, ou utiliza um modelo de dados, é mostrada e mantida tal como foi guardada, e os comandos informam-no de que não a podem alterar.

Clique em qualquer célula de uma tabela dinâmica. É apresentado o separador **Tabela Dinâmica**.

## Atualizar

**Atualizar** lê as células a partir das quais a tabela dinâmica é criada e volta a calcular o relatório. As novas entradas surgem depois das já existentes; as entradas que deixaram de existir são removidas. As larguras das colunas e o esquema escolhido mantêm-se. **Atualizar Tudo**, no separador **Dados** e no separador **Tabela Dinâmica**, faz o mesmo para todas as tabelas dinâmicas do livro.

Se o relatório for ocupar células que tenham conteúdo, o JeffCrunch não atualiza. Indica quais as células que estão no caminho. Limpe-as ou mova a tabela dinâmica e atualize novamente.

**Anular** desfaz uma atualização num só passo.

## Alterar Origem de Dados

**Alterar Origem de Dados** pede as células onde está a lista, incluindo a primeira linha de nomes. Escreva-as como `Folha1!A1:D100` ou escreva o nome de uma tabela ou de um nome que tenha criado. O JeffCrunch lê-as e volta a calcular o relatório. Se um campo da lista tiver o mesmo nome nas novas células, mantém a sua posição no relatório.

## Limpar

**Limpar** tem duas opções. **Limpar Filtros** remove todos os filtros e volta a mostrar todas as entradas. **Limpar Tudo** retira todos os campos do relatório e deixa-o vazio, pronto para ser criado de novo.

## O painel de campos

**Lista de Campos** no separador **Tabela Dinâmica** abre o painel **Campos da Tabela Dinâmica**. Este lista os campos da lista com uma caixa ao lado de cada um e, por baixo, os quatro locais: **Filtros**, **Colunas**, **Linhas** e **Valores**.

- Selecione um campo para o adicionar. Um campo que contenha apenas números vai para **Valores** e é somado; qualquer outro vai para **Linhas**.
- Desmarque um campo para o retirar de todo o lado.
- Clique num campo num dos quatro locais e escolha **Mover Para** para o colocar noutro. Os comandos **Mover para Cima** e **Mover para Baixo** alteram a sua ordem no respetivo local e **Remover Campo** retira-o.
- Com mais do que um valor, surge uma linha chamada **Valores** em **Colunas**. Mova-a para **Linhas** para colocar os valores na lateral.
- **Definições do Campo de Valor** (um duplo clique num valor ou o botão) define o nome que o relatório mostra para o valor e a forma como é totalizado: **Soma**, **Contagem**, **Contar Números**, **Média**, **Máximo**, **Mínimo**, **Produto**, **DesvPad** e **DesvPadP** (o desvio padrão de uma amostra ou de todos os valores) e **Var** e **VarP** (a variância dos mesmos).

Cada alteração é um passo de **Anular**.

## Estrutura: subtotais, totais gerais e esquema

- **Subtotais** mostra ou oculta o total de cada grupo, no início ou no fim do grupo. Quando existe mais do que um campo em Linhas, o total de cada campo exterior é um subtotal.
- **Totais Gerais** tem quatro opções: desativado para linhas e colunas, ativado para ambos, ativado apenas para linhas (uma coluna de totais à direita, um para cada linha) e ativado apenas para colunas (uma linha de totais em baixo).
- **Esquema do Relatório** é a disposição das etiquetas. No formato **compacto**, todos os campos de Linhas partilham uma coluna e um novo grupo é mostrado um pouco para a direita. No formato de **destaques**, cada campo tem a sua própria coluna e cada grupo tem uma linha própria acima dos seus itens. No formato de **tabela**, cada campo tem a sua própria coluna e o nome do grupo está na primeira linha, com o subtotal por baixo do grupo.

## A mesma lista, um filtro

Mova **Mês** de **Colunas** para **Filtros**. O relatório perde as colunas e uma linha por cima mostra **Mês** e **(Tudo)**. Os números passam a ser o total de cada região em todos os meses.

## Limites

Uma tabela dinâmica pode ter até 2000 campos, uma lista de até cinco milhões de linhas e um relatório de até quatro milhões de células. Um ficheiro que ultrapasse estes limites é mostrado tal como foi guardado e não pode ser alterado. Ler novamente cem mil linhas de vinte colunas e calcular o relatório demorou cerca de meio segundo no computador em que foi medido; um computador mais lento demora mais tempo.
