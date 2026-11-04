# Tabelas dinâmicas
<!-- keywords: tabela dinâmica, detalhar, subir nível, inserir tabela dinâmica, criar, fazer, selecionar, mover tabela dinâmica, getpivotdata, gerar getpivotdata, atualizar, atualizar tudo, alterar fonte de dados, lista de campos, campos, linhas, colunas, valores, filtros, filtro de relatório, total geral, subtotal, layout, compacto, estrutura de tópicos, tabular, limpar, configurações do campo de valor, soma, contagem, média, mostrar valores como, porcentagem do total, diferença, total acumulado, formato de número, configurações do campo, classificar, filtrar, primeiros, agrupar, desagrupar, datas, meses, anos, expandir, recolher, opções, estilo, linhas alternadas, linhas em branco, repetir rótulos, atualizar ao abrir o arquivo, gráfico dinâmico, segmentação de dados, linha do tempo, mostrar detalhes, detalhar, linhas por trás de um valor -->

Uma tabela dinâmica é um pequeno relatório feito a partir de uma lista longa. A lista tem uma linha para cada venda, por exemplo; o relatório soma as vendas por região e mês. Você não escreve nenhuma fórmula. Você diz quais colunas da lista ficam na vertical, quais ficam no topo e qual delas é somada, e o relatório se preenche sozinho.

## Um exemplo

Aqui está uma lista nas células A1 a C7. Cada linha é uma venda.

| Região | Mês | Valor |
|---|---|---|
| Leste | Jan | 10 |
| Leste | Jan | 5 |
| Leste | Fev | 20 |
| Oeste | Jan | 7 |
| Oeste | Fev | 3 |
| Oeste | Fev | 4 |

Uma tabela dinâmica com **Região** na vertical, **Mês** no topo e a soma de **Valor** no meio fica assim.

| Soma de Valor | Mês | | |
|---|---|---|---|
| Região | Jan | Fev | Total Geral |
| Leste | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total Geral | 22 | 27 | 49 |

Leste em janeiro é 15 porque as duas linhas de Leste de janeiro, 10 e 5, são somadas. A coluna Total Geral soma cada linha, a linha Total Geral soma cada coluna, e a célula do canto soma tudo.

As palavras usadas aqui:

- **Campo** é uma coluna da lista. Região, Mês e Valor são os campos.
- **Linhas**, **Colunas** e **Valores** são os três lugares em que um campo pode ficar. Um campo em Linhas gera uma linha para cada um de seus itens diferentes; um campo em Colunas gera uma coluna para cada um; um campo em Valores é o que é somado. Um quarto lugar, **Filtros**, guarda um campo que deixa de fora as linhas que você não escolhe.
- Um **cache** (o cache da tabela dinâmica) é uma cópia da sua lista que a tabela dinâmica guarda dentro do arquivo, para mostrar os totais sem consultar as células. O relatório é calculado a partir do cache, e não a partir das células, por isso não muda quando as células mudam. **Atualizar** copia as células para o cache novamente.

## Criar uma tabela dinâmica

1. Clique em uma célula da lista ou selecione as células que quer usar. A lista precisa de uma linha com nomes acima dos dados.
2. Escolha **Inserir** e depois **Tabela Dinâmica**.
3. A caixa mostra as células em que estão os dados. Altere-as se não estiverem corretas. Você também pode digitar o nome de uma tabela.
4. Escolha onde a tabela dinâmica vai ficar: **Nova planilha** (o padrão), ou uma célula desta planilha ou de outra. Escolha um lugar onde não haja nada, porque a tabela vazia ocupa cerca de dezoito linhas e três colunas.
5. Escolha **OK**. A tabela começa vazia, e o painel **Campos da Tabela Dinâmica** se abre. Marque os campos que quiser: um campo de números é somado, e qualquer outro vira uma linha. Depois, mova os campos para as caixas como explicam as próximas seções.

Uma nova planilha é um passo próprio do Desfazer, e a tabela também. Ao salvar como .xlsx, a tabela é gravada do jeito que o programa original grava uma, por isso abre lá como tabela dinâmica. Uma cópia salva como .xls ou .ods mantém os números que a tabela mostra em suas células, mas não a própria tabela, e o salvamento avisa você disso.

## GETPIVOTDATA: ler um número de uma tabela dinâmica

`GETPIVOTDATA` informa o número que uma tabela dinâmica mostra para os valores e as linhas e colunas que você indicar. Para o exemplo acima:

`=GETPIVOTDATA("Soma de Valor"; $E$3; "Região"; "Leste"; "Mês"; "Jan")` resulta em 15.

- O primeiro argumento é o nome do valor, como a tabela o mostra (por exemplo, Soma de Valor). O nome do campo que é somado (Valor) também funciona.
- O segundo argumento é qualquer célula da tabela dinâmica.
- Depois vêm pares de um campo e um de seus itens. Se indicar apenas Região e Leste, você obtém o total de Leste, 35. Se não indicar nada, obtém o total geral, 49. Os campos indicados precisam ser os primeiros campos das linhas, ou os primeiros campos das colunas, na ordem.
- Se a tabela não mostrar o número, a célula exibe `#REF!`.

Quando você clica em um valor de uma tabela dinâmica enquanto digita uma fórmula, o JeffCrunch escreve essa fórmula para você, para que ela continue lendo o número certo depois que a tabela for reorganizada. Para desativar isso, escolha **Opções** na guia **Tabela Dinâmica** e desmarque a caixa de seleção sobre GETPIVOTDATA.

## O que o JeffCrunch faz com uma tabela dinâmica

Uma tabela dinâmica em uma pasta de trabalho é mostrada como foi salva. Se você não alterar nada nela, o arquivo é salvo com a tabela dinâmica exatamente como estava. Quando você a atualiza ou altera, o JeffCrunch calcula o relatório de novo, coloca os novos números nas células e grava a tabela dinâmica de volta, para que o programa que a criou a abra sem reclamar.

O JeffCrunch calcula tabelas dinâmicas que somam, contam, calculam médias e assim por diante, que agrupam datas e números, e que mostram valores como porcentagem, diferença ou total acumulado. Uma tabela dinâmica que tenha campo ou item calculado, que mostre um valor como classificação ou como porcentagem de um item pai, que tenha grupos de itens criados manualmente ou que use um modelo de dados é mostrada e mantida como foi salva, e os comandos avisam que não podem alterá-la.

Clique em qualquer célula de uma tabela dinâmica. A guia **Tabela Dinâmica** aparece.

## Atualizar

**Atualizar** lê as células em que a tabela dinâmica se baseia e calcula o relatório de novo. Novos itens vêm depois dos que já estão lá; itens que não existem mais são removidos. A largura das colunas e o layout que você escolheu permanecem. **Atualizar Tudo**, na guia **Dados** e na guia **Tabela Dinâmica**, faz o mesmo para todas as tabelas dinâmicas da pasta de trabalho.

Se o relatório fosse crescer sobre células que têm algum conteúdo, o JeffCrunch não atualiza. Ele informa quais células estão no caminho. Limpe-as ou mova a tabela dinâmica e atualize de novo.

**Desfazer** desfaz uma atualização em um único passo.

## Alterar Fonte de Dados

**Alterar Fonte de Dados** pede as células em que a lista está, incluindo a primeira linha de nomes. Digite-as como `Plan1!A1:D100`, ou digite o nome de uma tabela ou de um nome que você criou. O JeffCrunch lê essas células e calcula o relatório de novo. Se um campo da lista tiver o mesmo nome nas novas células, a posição dele no relatório se mantém.

## Limpar

**Limpar** tem duas opções. **Limpar Filtros** remove todos os filtros e mostra todos os itens de novo. **Limpar Tudo** tira todos os campos do relatório e o deixa vazio, pronto para ser montado de novo.

## Detalhar e subir nível

Quando um relatório tem mais de um campo nas linhas, um item do primeiro campo pode ser aberto ou fechado. Clique na célula com o nome do item e escolha **Subir Nível** para fechá-lo: as linhas do campo seguinte abaixo dele são ocultadas, e só o total permanece. **Detalhar** o abre de novo. Um item do último campo não tem nada abaixo. Cada ação é um **Desfazer**.

## Selecionar e mover

**Selecionar** na guia **Tabela Dinâmica** seleciona parte do relatório: **Tabela Dinâmica Inteira**, **Valores** (os números) ou **Rótulos** (os nomes na vertical; se o relatório não tiver nomes na vertical, os nomes no topo). **Mover Tabela Dinâmica…** pede uma célula da mesma planilha e desenha o relatório de novo com a primeira célula ali. As células que ele deixa são esvaziadas. Fórmulas que apontam para as células do relatório as acompanham, como acontece ao recortar e colar. Se o novo lugar tiver algum conteúdo, o JeffCrunch não move o relatório e informa isso. Um **Desfazer** o devolve ao lugar anterior.

## O painel de campos

**Lista de Campos** na guia **Tabela Dinâmica** abre o painel **Campos da Tabela Dinâmica**. Ele lista os campos da lista com uma caixa ao lado de cada um e, abaixo, os quatro lugares: **Filtros**, **Colunas**, **Linhas** e **Valores**.

- Marque um campo para adicioná-lo. Um campo que tem apenas números vai para **Valores** e é somado; qualquer outro vai para **Linhas**.
- Desmarque um campo para removê-lo de todos os lugares.
- Clique em um campo em um dos quatro lugares e escolha **Mover Para** para colocá-lo em outro. As opções **Mover para Cima** e **Mover para Baixo** mudam a ordem dele em seu lugar, e **Remover Campo** o retira.
- Com mais de um valor, uma linha chamada **Valores** aparece em **Colunas**. Mova-a para **Linhas** para colocar os valores na vertical.
- **Configurações do Campo de Valor** (um clique duplo em um valor, ou o botão) define o nome que o relatório mostra para o valor e como ele é totalizado: **Soma**, **Contagem**, **Contagem de Números**, **Média**, **Máx**, **Mín**, **Produto**, **DesvPad** e **DesvPadp** (o desvio padrão de uma amostra, ou de todos os valores), e **Var** e **Varp** (a variância de uma amostra, ou de todos os valores). Também define como o valor é mostrado e seu formato de número. Veja a próxima seção.
- **Configurações do Campo** (um clique duplo em um campo em Filtros, Colunas ou Linhas, ou o botão) define um campo. Veja "Configurações do Campo".

Cada alteração é um passo do **Desfazer**.

## Mostrar valores como, e o formato de número de um valor

Em **Configurações do Campo de Valor**, **Mostrar valores como** altera o que uma célula do relatório mostra.

- **% do Total Geral**: a célula dividida pelo total de todo o relatório. No exemplo, Leste em janeiro é 15 de 49, o que dá 30,6%.
- **% do Total da Coluna** e **% do Total da Linha**: a célula dividida pelo total de sua coluna ou de sua linha.
- **Índice**: como a célula se compara com o que sua linha e sua coluna dariam se os dados estivessem distribuídos igualmente. Um número acima de 1 significa mais do que a parcela igual.
- **Diferença De**, **% De** e **% da Diferença De**: a célula comparada com outro item de um campo. Escolha o **Campo base** (um campo em Linhas ou Colunas) e o **Item base** (um de seus itens, ou o anterior ou o seguinte). Por exemplo, com Mês como campo base e Jan como item base, fevereiro mostra quanto ele é maior ou menor que janeiro. O próprio item base fica vazio em uma diferença, e vale 100% em **% De**.
- **Total Acumulado Em**: cada célula soma as células anteriores ao longo do campo base.

**Formato de número** aceita um código de formato, como `0,00`, `#.##0` ou `0,0%`. Deixe em branco para manter o formato que as células já têm. Um valor mostrado como porcentagem recebe `0,00%`, a menos que você escolha um formato. As células do relatório assumem o formato na hora. **Desfazer** o desfaz junto com os números.

## Configurações do Campo

Selecione um campo na caixa **Filtros**, **Colunas** ou **Linhas** e escolha **Configurações do Campo** na guia **Tabela Dinâmica** (ou clique duas vezes nele).

- **Nome personalizado** altera o nome que o relatório mostra para o campo.
- **Subtotais**: **Automático** mostra um total para cada grupo do campo; **Nenhum** não mostra.
- **Mostrar rótulos de itens em**: formato compacto, de estrutura de tópicos ou tabular, como no Layout do Relatório.
- **Inserir uma linha em branco após cada item** coloca uma linha vazia após cada grupo.
- **Repetir rótulos de itens** mostra o nome de um item externo em cada linha de seu grupo (formatos de estrutura de tópicos e tabular).
- **Classificar**: manter a ordem em que os itens estão (manual), ou classificar de A a Z ou de Z a A pelos rótulos. **Do menor para o maior por um valor** e **Do maior para o menor por um valor** classificam os itens pelos números de um campo de valor. O relatório mantém a classificação quando os dados mudam.
- **Filtrar** deixa itens de fora pelos rótulos (começa com, termina com, contém, é igual a) ou por um valor (é maior que, é menor que, está entre e assim por diante), ou mantém apenas os primeiros ou últimos itens por um valor. O filtro é calculado quando você escolhe OK. Ele oculta os itens que deixa de fora; as caixas de seleção abaixo mostram quais são, e você pode marcar um item para que ele apareça de novo.
- **Mostrar estes itens** é uma lista dos itens do campo, com uma caixa de seleção para cada um. Desmarque uma caixa para ocultar o item, e os totais o deixam de fora.

## Agrupar datas e números

Selecione um campo de datas ou de números em **Linhas** ou **Colunas** e escolha **Agrupar Campo** na guia **Tabela Dinâmica**. Os grupos são criados a partir dos dados, por isso a coluna precisa ter apenas números ou apenas datas, sem texto e sem células vazias.

- **Datas**: marque como agrupar: **Segundos**, **Minutos**, **Horas**, **Dias**, **Meses**, **Trimestres** ou **Anos**. A parte mais fina que você marcar permanece no campo. Para cada parte mais grossa, o JeffCrunch adiciona um campo com o nome dela (por exemplo, **Anos**) e o coloca antes do campo de datas, no mesmo lugar. **Iniciar em** e **Terminar em** são a primeira e a última data; deixe-as como estão para usar a menor e a maior data dos dados. Datas anteriores ao início ou posteriores ao fim vão para dois grupos extras, que começam com `<` e `>`.
- **Números**: **Iniciar em**, **Terminar em** e **Por** criam grupos como 1-10, 11-20 e assim por diante.

**Desagrupar** remove os grupos e os campos que foram adicionados para eles. Atualizar cria os grupos de novo a partir dos novos dados. Quando duas tabelas dinâmicas são feitas a partir dos mesmos dados, não é possível criar grupos, porque um grupo é uma parte dos dados que elas compartilham.

## Mostrar as linhas por trás de um valor

Clique duas vezes em um valor de uma tabela dinâmica ou escolha **Mostrar Detalhes** na guia **Tabela Dinâmica**. O JeffCrunch cria uma nova planilha, chamada Detalhes 1, 2 e assim por diante, com as linhas da lista que formam o número. Os nomes dos campos ficam na primeira linha. F2 ainda edita a célula, se ela puder ser editada. **Desfazer** remove as linhas, e outro **Desfazer** remove a planilha.

## Expandir e recolher

Um campo cujos itens têm outros abaixo deles pode mostrar ou ocultar as linhas sob os seus itens. **Expandir Campo Inteiro** e **Recolher Campo Inteiro** fazem isso para todos os itens do campo selecionado no painel.

## Opções da Tabela Dinâmica

**Opções** na guia **Tabela Dinâmica** abre uma caixa com as opções da tabela.

- **Nome** é o nome da tabela dinâmica. Nenhuma outra tabela dinâmica da pasta de trabalho pode ter esse nome.
- **Para valores de erro, mostrar** e **Para células vazias, mostrar** colocam um texto próprio nessas células.
- **Mesclar e centralizar células com rótulos**, **Ajustar largura das colunas ao atualizar a tabela** e **Manter a formatação das células ao atualizar a tabela** são salvas com a tabela.
- **Mostrar totais gerais para linhas** e **para colunas** são as mesmas opções de Totais Gerais.
- **Atualizar os dados ao abrir o arquivo** lê as células de novo sempre que a pasta de trabalho é aberta. É um passo do Desfazer, como qualquer atualização.

## Estilo: cabeçalhos, faixas e galeria

As **Opções de Estilo** na guia **Tabela Dinâmica** ativam ou desativam a linha de cabeçalho, a primeira coluna (cabeçalhos de linha) e as faixas de linhas e de colunas. **Estilos** tem uma galeria de aparências, e **Limpar Estilo** remove a aparência. Essas aparências são desenhadas do mesmo modo que as aparências das tabelas. **Linhas em Branco** e **Repetir Rótulos** definem a linha em branco e os rótulos repetidos para todos os campos das linhas de uma só vez.

## Estrutura: subtotais, totais gerais e layout

- **Subtotais** mostra ou oculta o total de cada grupo, no início ou no fim do grupo. Quando há mais de um campo em Linhas, o total de cada campo externo é um subtotal.
- **Totais Gerais** tem quatro opções: desativado para linhas e colunas, ativado para os dois, ativado apenas para linhas (uma coluna de totais à direita, um para cada linha) e ativado apenas para colunas (uma linha de totais embaixo).
- **Layout do Relatório** é a forma dos rótulos. No formato **compacto**, todos os campos de Linhas dividem uma só coluna, e um novo grupo aparece um pouco mais à direita. No formato de **estrutura de tópicos**, cada campo tem sua própria coluna, e cada grupo tem uma linha própria acima de seus itens. No formato **tabular**, cada campo tem sua própria coluna, e o nome do grupo fica na primeira linha dele, com o subtotal abaixo do grupo.

## A mesma lista, com um filtro

Mova **Mês** de **Colunas** para **Filtros**. O relatório perde suas colunas, e uma linha acima dele mostra **Mês** e **(Tudo)**. Os números agora são o total de cada região em todos os meses.

## Gráfico dinâmico

Clique em uma célula de uma tabela dinâmica e escolha **Inserir** e depois **Gráfico Dinâmico**. O JeffCrunch cria um gráfico de colunas com os valores da tabela. Ele deixa de fora os totais gerais. O gráfico é marcado no arquivo como gráfico da tabela dinâmica. Ele lê as células da tabela, por isso seus números mudam quando você atualiza a tabela ou altera o layout dela. Se a tabela ganhar mais linhas ou colunas, crie o gráfico de novo, porque ele mantém as células a partir das quais foi feito. Um gráfico dinâmico vindo de um arquivo é mostrado como foi salvo.

## Segmentações de dados e linhas do tempo

Uma segmentação de dados é um conjunto de botões que filtra uma tabela dinâmica, e uma linha do tempo é um controle deslizante que faz o mesmo para datas.

Clique em uma célula de uma tabela dinâmica e depois em **Segmentações** na guia Tabela Dinâmica. O painel Segmentações se abre e lista as segmentações de dados e as linhas do tempo da pasta de trabalho. Uma segmentação tem um botão para cada item de seu campo. Os botões em azul são os itens que a tabela mostra.

Clique em um botão para mostrar apenas aquele item. Clique nele de novo para mostrar todos os itens. Mantenha **Ctrl** pressionada e clique para acrescentar um item aos que estão mostrados, ou para retirar um. Cada clique é um passo do Desfazer, e o Desfazer devolve a tabela ao estado anterior. Ao salvar, o arquivo guarda a escolha da segmentação.

**Conexões do Relatório** informa quais tabelas dinâmicas uma segmentação filtra. Uma linha do tempo mostra o período de datas que ela escolheu. O JeffCrunch mostra as segmentações neste painel, e não sobre as células, e ainda não altera uma linha do tempo. Ambas são mantidas no arquivo como foram salvas.

## Limites

Uma tabela dinâmica pode ter até 2.000 campos, uma lista de até cinco milhões de linhas e um relatório de até quatro milhões de células. Um arquivo que ultrapasse esses limites é mostrado como foi salvo e não pode ser alterado. Ler de novo cem mil linhas de vinte colunas e calcular o relatório levou cerca de meio segundo no computador em que isso foi medido; um computador mais lento leva mais tempo.
