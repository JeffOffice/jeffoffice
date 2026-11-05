# Tabelas dinâmicas
<!-- keywords: relatório de filtros de página, tabela dinâmica, detalhar, resumir, inserir tabela dinâmica, criar, fazer, selecionar, mover tabela dinâmica, getpivotdata, gerar getpivotdata, atualizar, atualizar tudo, alterar origem de dados, lista de campos, campos, linhas, colunas, valores, filtros, filtro de relatório, total geral, subtotal, esquema, compacto, destaques, tabular, limpar, definições do campo de valor, soma, contar, média, mostrar valores como, percentagem do total, diferença, total acumulado, formato de número, definições do campo, ordenar, filtrar, superiores, agrupar, desagrupar, datas, meses, anos, expandir, recolher, opções, estilo, linhas alternadas, linhas em branco, repetir etiquetas, atualizar ao abrir o ficheiro, gráfico dinâmico, segmentação de dados, linha cronológica, mostrar detalhes, detalhar, linhas por trás de um valor -->

Uma tabela dinâmica é um pequeno relatório feito a partir de uma lista longa. A lista tem, por exemplo, uma linha por cada venda; o relatório soma as vendas por região e por mês. Não escreve nenhuma fórmula. Indica quais as colunas da lista que vão na vertical, quais vão na horizontal e qual é somada, e o relatório preenche-se sozinho.

## Um exemplo

Eis uma lista nas células A1 a C7. Cada linha é uma venda.

| Região | Mês | Montante |
|---|---|---|
| Este | Jan | 10 |
| Este | Jan | 5 |
| Este | Fev | 20 |
| Oeste | Jan | 7 |
| Oeste | Fev | 3 |
| Oeste | Fev | 4 |

Uma tabela dinâmica com **Região** na vertical, **Mês** na horizontal e a soma de **Montante** no centro tem este aspeto.

| Soma de Montante | Mês | | |
|---|---|---|---|
| Região | Jan | Fev | Total Geral |
| Este | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total Geral | 22 | 27 | 49 |

O Este em janeiro é 15, porque as duas linhas do Este de janeiro, 10 e 5, são somadas. A coluna Total Geral soma cada linha, a linha Total Geral soma cada coluna, e a célula do canto soma tudo.

Os termos usados aqui:

- **Campo** é uma coluna da lista. Região, Mês e Montante são campos.
- **Linhas**, **Colunas** e **Valores** são os três sítios onde um campo pode ficar. Um campo em Linhas dá uma linha por cada um dos seus valores diferentes; um campo em Colunas dá uma coluna por cada um; um campo em Valores é o que é somado. Um quarto sítio, **Filtros**, contém um campo que deixa de fora as linhas que não escolhe.
- Uma **cache** (a cache da tabela dinâmica) é uma cópia da sua lista que a tabela dinâmica guarda dentro do ficheiro, para poder mostrar totais sem consultar as células. O relatório é calculado a partir da cache, e não a partir das células, por isso não muda quando as células mudam. **Atualizar** copia outra vez as células para a cache.

## Criar uma tabela dinâmica

1. Clique numa célula da lista ou selecione as células que quer usar. A lista precisa de uma linha com os nomes por cima dos dados.
2. Escolha **Inserir** e, a seguir, **Tabela Dinâmica**.
3. A caixa mostra as células onde estão os dados. Altere-as se não estiverem certas. Também pode escrever o nome de uma tabela.
4. Escolha onde fica a tabela dinâmica: **Uma nova folha** (a predefinição) ou uma célula desta folha ou de outra. Escolha um sítio onde não haja mais nada, porque a tabela vazia ocupa cerca de dezoito linhas e três colunas.
5. Escolha **OK**. A tabela começa vazia e abre-se o painel **Campos da Tabela Dinâmica**. Marque os campos que quer: um campo de números é somado, e qualquer outro torna-se uma linha. Depois, mova os campos para as caixas, como indicam as secções seguintes.

Uma nova folha é um passo próprio de **Anular**, tal como a tabela. Ao guardar como .xlsx, a tabela é escrita da forma como o programa original a escreve, por isso abre-se lá como tabela dinâmica. Uma cópia guardada como .xls ou .ods mantém nas células os números que a tabela mostra, mas não a própria tabela, e ao guardar é indicado que assim é.

## GETPIVOTDATA: ler um número de uma tabela dinâmica

`GETPIVOTDATA` devolve o número que uma tabela dinâmica mostra para os valores e as linhas e colunas indicados. Para o exemplo acima:

`=GETPIVOTDATA("Soma de Montante"; $E$3; "Região"; "Este"; "Mês"; "Jan")` devolve 15.

- O primeiro argumento é o nome do valor, tal como a tabela o mostra (por exemplo, Soma de Montante). O nome do campo que é somado (Montante) também funciona.
- O segundo argumento é qualquer célula da tabela dinâmica.
- A seguir vêm pares de um campo e de um dos seus itens. Se indicar apenas Região e Este, obtém o total do Este, 35. Se não indicar nada, obtém o total geral, 49. Os campos que indicar têm de ser os primeiros campos das linhas, ou os primeiros campos das colunas, pela mesma ordem.
- Se a tabela não mostrar o número, a célula mostra `#REF!`.

Quando clica num valor de uma tabela dinâmica enquanto escreve uma fórmula, o JeffCrunch escreve esta fórmula por si, para que continue a ler o número certo depois de a tabela ser reorganizada. Para desativar isto, escolha **Opções** no separador Tabela Dinâmica e desmarque a caixa de verificação sobre GETPIVOTDATA.

## O que o JeffCrunch faz com uma tabela dinâmica

Uma tabela dinâmica num livro é mostrada tal como foi guardada. Se não alterar nada nela, o ficheiro é guardado com a tabela dinâmica exatamente como estava. Quando a atualiza ou altera, o JeffCrunch volta a calcular o relatório, coloca os novos números nas células e escreve a tabela dinâmica de volta, para que o programa que a criou a abra sem problemas.

O JeffCrunch calcula tabelas dinâmicas que somam, contam, calculam médias e outras funções, que agrupam datas e números, e que mostram valores como percentagem, diferença ou total acumulado. Uma tabela dinâmica que tenha um campo ou um item calculado, que mostre um valor como classificação ou como percentagem de um item-pai, que tenha grupos de itens criados manualmente, ou que use um modelo de dados, é mostrada e mantida como foi guardada, e os comandos indicam que não a podem alterar.

Clique em qualquer célula de uma tabela dinâmica. Aparece o separador **Tabela Dinâmica**.

## Atualizar

**Atualizar** lê as células de que a tabela dinâmica é feita e volta a calcular o relatório. As entradas novas são acrescentadas às que já existem; as entradas que já não existem são retiradas. As larguras das colunas e o esquema que escolheu mantêm-se. **Atualizar Tudo**, no separador **Dados** e no separador **Tabela Dinâmica**, faz o mesmo para todas as tabelas dinâmicas do livro.

Se o relatório fosse crescer por cima de células que têm conteúdo, o JeffCrunch não atualiza. Indica-lhe quais são as células que estão no caminho. Limpe-as ou mova a tabela dinâmica e volte a atualizar.

**Anular** desfaz uma atualização num único passo.

## Alterar Origem de Dados

**Alterar Origem de Dados** pede as células onde está a lista, incluindo a primeira linha com os nomes. Escreva-as como `Folha1!A1:D100`, ou escreva o nome de uma tabela ou de um nome que tenha criado. O JeffCrunch lê-as e volta a calcular o relatório. Se um campo da lista tiver o mesmo nome nas novas células, o seu lugar no relatório mantém-se.

## Limpar

**Limpar** tem duas opções. **Limpar Filtros** remove todos os filtros e volta a mostrar todas as entradas. **Limpar Tudo** retira todos os campos do relatório e deixa-o vazio, pronto para ser construído de novo.

## Detalhar e resumir

## Páginas de filtro do relatório

Um filtro fica acima de uma tabela dinâmica e permite ver um item de cada vez, como um mês. **Mostrar Páginas de Filtro do Relatório**, no separador **Tabela Dinâmica**, cria uma página para cada item de uma só vez. Cada página é uma nova folha, com o nome do item, e com a sua própria cópia da tabela. O filtro de cada cópia fica definido para esse item. Se a tabela tiver mais do que um filtro, escolhe qual usar. Um único **Anular** remove todas as páginas.

Quando um relatório tem mais do que um campo nas linhas, um item do primeiro campo pode ser aberto ou fechado. Clique na célula com o nome do item e escolha **Resumir** para o fechar: as linhas do campo seguinte, abaixo dele, ficam ocultas e só o seu total se mantém. **Detalhar** volta a abri-lo. Um item do último campo não tem nada por baixo. Cada uma destas ações é um **Anular**.

## Selecionar e mover

**Selecionar**, no separador **Tabela Dinâmica**, seleciona parte do relatório: **Tabela Dinâmica Inteira**, **Valores** (os números) ou **Etiquetas** (os nomes na vertical; se o relatório não tiver nomes na vertical, os nomes na horizontal). **Mover Tabela Dinâmica…** pede uma célula da mesma folha e volta a desenhar o relatório com a primeira célula nesse local. As células que deixa para trás ficam vazias. As fórmulas que apontam para as células do relatório seguem-nas, tal como acontece quando recorta e cola. Se o novo local tiver conteúdo, o JeffCrunch não move o relatório e indica-lho. Um único **Anular** move-o de volta.

## O painel de campos

**Lista de Campos**, no separador **Tabela Dinâmica**, abre o painel **Campos da Tabela Dinâmica**. Mostra os campos da lista, com uma caixa ao lado de cada um, e, por baixo, os quatro sítios: **Filtros**, **Colunas**, **Linhas** e **Valores**.

- Marque um campo para o adicionar. Um campo que tenha apenas números vai para **Valores** e é somado; qualquer outro vai para **Linhas**.
- Desmarque um campo para o retirar de todos os sítios.
- Clique num campo num dos quatro sítios e escolha **Mover Para** para o pôr noutro sítio. As palavras **Mover para Cima** e **Mover para Baixo** alteram a sua ordem dentro do sítio, e **Remover Campo** retira-o.
- Com mais do que um valor, aparece uma linha chamada **Valores** em **Colunas**. Mova-a para **Linhas** para pôr os valores na vertical.
- **Definições do Campo de Valor** (um duplo clique num valor ou o botão) define o nome que o relatório mostra para o valor e a forma como é totalizado: **Soma**, **Contar**, **Contar Números**, **Média**, **Máx**, **Mín**, **Produto**, **DesvPad** e **DesvPadp** (o desvio-padrão de uma amostra, ou de todos os valores), e **Var** e **Varp** (a variância das mesmas). Define também a forma como o valor é mostrado e o seu formato de número. Veja a secção seguinte.
- **Definições do Campo** (um duplo clique num campo dos Filtros, das Colunas ou das Linhas, ou o botão) define um único campo. Veja "Definições do Campo".

Cada alteração é um passo de **Anular**.

## Mostrar valores como, e o formato de número de um valor

Em **Definições do Campo de Valor**, **Mostrar valores como** altera o que uma célula do relatório mostra.

- **% do Total Geral**: a célula dividida pelo total de todo o relatório. No exemplo, o Este em janeiro é 15 de 49, o que dá 30,6%.
- **% do Total da Coluna** e **% do Total da Linha**: a célula dividida pelo total da sua coluna ou da sua linha.
- **Índice**: a comparação da célula com o que a sua linha e a sua coluna dariam se os dados estivessem distribuídos por igual. Um número acima de 1 significa mais do que a parte igual.
- **Diferença De**, **% De** e **% Diferença De**: a célula comparada com outro item de um campo. Escolha o **Campo base** (um campo nas Linhas ou nas Colunas) e o **Item base** (um dos seus itens, ou o anterior ou o seguinte). Por exemplo, com Mês como campo base e Jan como item base, fevereiro mostra quanto é mais ou menos do que janeiro. O próprio item base fica vazio numa diferença, e fica com 100% em **% De**.
- **Total Acumulado Em**: cada célula soma as células que estão antes dela ao longo do campo base.
- **Classificar do Menor para o Maior** e **Classificar do Maior para o Menor**: cada célula mostra a sua posição, 1, 2, 3 e assim por diante, entre as células do campo base na mesma linha ou coluna. A posição 1 é o menor número, ou o maior.
- **% do Total da Linha-Pai** e **% do Total da Coluna-Pai**: cada célula mostra que parte do seu pai representa. O pai de um item é o item um nível acima, como o ano que contém um mês. Um item sem pai é mostrado como parte do total geral.

**Formato de número** aceita um código de formato, como `0,00`, `#.##0` ou `0,0%`. Deixe-o vazio para manter o formato que as células têm. Um valor mostrado como percentagem fica com `0,00%`, a não ser que escolha outro formato. As células do relatório adotam o formato de imediato. **Anular** desfaz-o juntamente com os números.

## Definições do Campo

Selecione um campo na caixa **Filtros**, **Colunas** ou **Linhas** e escolha **Definições do Campo** no separador **Tabela Dinâmica** (ou faça duplo clique nele).

- **Nome personalizado** altera o nome que o relatório mostra para o campo.
- **Subtotais**: **Automático** mostra um total para cada grupo do campo, **Nenhum** não mostra.
- **Mostrar etiquetas de itens em**: forma compacta, de destaques ou tabular, como em Esquema do Relatório.
- **Inserir uma linha em branco após cada item** coloca uma linha vazia depois de cada grupo.
- **Repetir etiquetas de itens** mostra o nome de um item exterior em todas as linhas do seu grupo (formas de destaques e tabular).
- **Ordenar**: manter a ordem em que os itens estão (manual), ou ordenar de A a Z ou de Z a A pelas etiquetas. **Do menor para o maior por um valor** e **Do maior para o menor por um valor** ordenam os itens pelos números de um campo de valor. O relatório mantém a ordenação quando os dados mudam.
- **Filtro** deixa de fora itens pelas suas etiquetas (começa por, termina em, contém, é igual a) ou por um valor (é maior do que, é menor do que, está entre, e assim por diante), ou mantém apenas os primeiros ou os últimos itens por um valor. O filtro é calculado quando escolhe OK. Oculta os itens que deixa de fora; as caixas de verificação abaixo mostram quais são, e pode marcar um item para o voltar a mostrar.
- **Mostrar estes itens** é uma lista dos itens do campo, com uma caixa de verificação em cada um. Desmarque uma caixa para ocultar o item, e os totais deixam-no de fora.

## Agrupar datas e números

Selecione um campo de datas ou de números nas **Linhas** ou nas **Colunas** e escolha **Agrupar Campo** no separador **Tabela Dinâmica**. Os grupos são criados a partir dos dados, por isso a coluna tem de conter apenas números ou apenas datas, sem texto e sem células vazias.

- **Datas**: marque o que agrupar: **Segundos**, **Minutos**, **Horas**, **Dias**, **Meses**, **Trimestres** ou **Anos**. A parte mais detalhada que marcar fica no campo. Para cada parte mais grosseira, o JeffCrunch acrescenta um campo com o seu nome (por exemplo, **Anos**) e coloca-o antes do campo de datas, no mesmo sítio. **Começar em** e **Terminar em** são a primeira e a última data; deixe-as como estão para usar a data mais antiga e a mais recente dos dados. As datas anteriores ao início ou posteriores ao fim ficam em dois grupos extra, que começam por `<` e `>`.
- **Números**: **Começar em**, **Terminar em** e **Por** criam grupos como 1-10, 11-20, e assim por diante.

**Desagrupar** retira os grupos e os campos que foram acrescentados para eles. Atualizar volta a criar os grupos a partir dos novos dados. Quando duas tabelas dinâmicas são feitas a partir dos mesmos dados, não é possível criar grupos, porque um grupo é uma parte dos dados que partilham.

## Mostrar as linhas por trás de um valor

Faça duplo clique num valor de uma tabela dinâmica ou escolha **Mostrar Detalhes** no separador **Tabela Dinâmica**. O JeffCrunch cria uma nova folha, com o nome Detalhe 1, 2, e assim por diante, com as linhas da lista que formam o número. Os nomes dos campos estão na primeira linha. F2 continua a editar a célula, se a célula puder ser editada. **Anular** retira as linhas, e outro **Anular** retira a folha.

## Expandir e recolher

Um campo cujos itens têm outros por baixo deles pode mostrar ou ocultar as linhas abaixo dos seus itens. **Expandir Campo Inteiro** e **Recolher Campo Inteiro** fazem-no para todos os itens do campo que selecionar no painel.

## Opções da Tabela Dinâmica

**Opções**, no separador **Tabela Dinâmica**, abre uma caixa com as opções da tabela.

- **Nome** é o nome da tabela dinâmica. Nenhuma outra tabela dinâmica do livro o pode ter.
- **Para valores de erro, mostrar** e **Para células vazias, mostrar** colocam nessas células um texto à sua escolha.
- **Unir e centrar células com etiquetas**, **Ajustar larguras das colunas ao atualizar a tabela** e **Manter a formatação das células ao atualizar a tabela** são guardadas com a tabela.
- **Mostrar totais gerais para linhas** e **para colunas** são o mesmo que as opções de Totais Gerais.
- **Atualizar os dados ao abrir o ficheiro** lê as células outra vez sempre que o livro é aberto. É um passo de Anular, como qualquer atualização.

## Estilo: cabeçalhos, faixas e galeria

As **Opções de Estilo**, no separador **Tabela Dinâmica**, ativam ou desativam a linha de cabeçalho, a primeira coluna (cabeçalhos de linha) e as faixas de linhas e de colunas. **Estilos** tem uma galeria de aspetos, e **Limpar Estilo** retira o aspeto. Estes aspetos são desenhados da mesma forma que os das tabelas. **Linhas em Branco** e **Repetir Etiquetas** definem a linha em branco e as etiquetas repetidas para todos os campos das linhas de uma só vez.

## Estrutura: subtotais, totais gerais e esquema

- **Subtotais** mostra ou oculta o total de cada grupo, no início ou no fim do grupo. Quando há mais do que um campo nas Linhas, o total de cada campo exterior é um subtotal.
- **Totais Gerais** tem quatro opções: desativado para linhas e colunas, ativado para ambos, ativado apenas para linhas (uma coluna de totais à direita, um para cada linha) e ativado apenas para colunas (uma linha de totais em baixo).
- **Esquema do Relatório** é a forma das etiquetas. Na forma **compacta**, todos os campos das Linhas partilham uma coluna, e um novo grupo é mostrado um pouco mais à direita. Na forma de **destaques**, cada campo tem a sua própria coluna, e cada grupo tem uma linha própria por cima dos seus itens. Na forma **tabular**, cada campo tem a sua própria coluna e o nome do grupo fica na sua primeira linha, com o subtotal por baixo do grupo.

## A mesma lista, com um filtro

Mova **Mês** das **Colunas** para os **Filtros**. O relatório perde as colunas, e uma linha por cima dele mostra **Mês** e **(Tudo)**. Os números passam a ser o total de cada região em todos os meses.

## Gráfico dinâmico

Clique numa célula de uma tabela dinâmica e escolha **Inserir** e, a seguir, **Gráfico Dinâmico**. O JeffCrunch cria um gráfico de colunas com os valores da tabela. Deixa de fora os totais gerais. O gráfico fica marcado no ficheiro como o gráfico da tabela dinâmica. Lê as células da tabela, por isso os seus números mudam quando atualiza a tabela ou altera o seu esquema. Se a tabela ganhar mais linhas ou colunas, crie o gráfico outra vez, porque o gráfico mantém as células a partir das quais foi feito. Um gráfico dinâmico que vem de um ficheiro é mostrado tal como foi guardado.

## Segmentações e linhas cronológicas

Uma segmentação é um conjunto de botões que filtra uma tabela dinâmica, e uma linha cronológica é um cursor que faz o mesmo para as datas.

Clique numa célula de uma tabela dinâmica e, a seguir, clique em **Segmentações** no separador Tabela Dinâmica. Abre-se o painel Segmentações, que lista as segmentações e as linhas cronológicas do livro. Uma segmentação tem um botão para cada item do seu campo. Os botões que estão a azul são os itens que a tabela mostra.

Clique num botão para mostrar apenas esse item. Clique outra vez para mostrar todos os itens. Mantenha **Ctrl** premido e clique para acrescentar um item aos que estão mostrados, ou para retirar um. Cada clique é um passo de Anular, e Anular repõe a tabela. Ao guardar, o ficheiro mantém a escolha da segmentação.

**Ligações de Relatório** indica quais as tabelas dinâmicas que uma segmentação filtra. Uma linha cronológica mostra o intervalo de datas que escolheu. O JeffCrunch mostra as segmentações neste painel, e não por cima das células, e ainda não altera uma linha cronológica. Ambas são mantidas no ficheiro tal como foram guardadas.

## Limites

Uma tabela dinâmica pode ter até 2000 campos, uma lista de até cinco milhões de linhas e um relatório de até quatro milhões de células. Um ficheiro que ultrapasse estes valores é mostrado tal como foi guardado e não pode ser alterado. Ler de novo cem mil linhas com vinte colunas e calcular o relatório demorou cerca de meio segundo no computador onde foi medido; um computador mais lento demora mais tempo.
