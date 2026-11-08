# Tabelas dinâmicas
<!-- keywords: listar fórmulas, campo calculado, item calculado, ordem de cálculo, campo de fórmula, páginas de filtro do relatório, tabela dinâmica, detalhar, subir nível, inserir tabela dinâmica, criar, fazer, selecionar, mover tabela dinâmica, getpivotdata, gerar getpivotdata, atualizar, atualizar tudo, alterar fonte de dados, lista de campos, campos, linhas, colunas, valores, filtros, filtro do relatório, total geral, subtotal, layout, compacto, estrutura de tópicos, tabular, limpar, configurações do campo de valor, soma, contagem, média, mostrar valores como, porcentagem do total, diferença, total acumulado, formato de número, configurações do campo, classificar, filtrar, primeiros, agrupar, desagrupar, datas, meses, anos, expandir, recolher, opções, estilo, linhas alternadas, linhas em branco, repetir rótulos, atualizar ao abrir o arquivo, gráfico dinâmico, segmentação de dados, linha do tempo, mostrar detalhes, detalhar, linhas por trás de um valor -->

Uma tabela dinâmica é um pequeno relatório feito a partir de uma lista longa. A lista tem uma linha para cada venda, por exemplo; o relatório soma as vendas por região e por mês. Você não escreve nenhuma fórmula. Você diz quais colunas da lista vão na lateral, quais vão no topo e qual delas é somada, e o relatório se preenche sozinho.

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

Uma tabela dinâmica com **Região** na lateral, **Mês** no topo e a soma de **Valor** no meio fica assim.

| Soma de Valor | Mês | | |
|---|---|---|---|
| Região | Jan | Fev | Total Geral |
| Leste | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total Geral | 22 | 27 | 49 |

Leste em janeiro é 15 porque as duas linhas de Leste de janeiro, 10 e 5, são somadas. A coluna Total Geral soma cada linha, a linha Total Geral soma cada coluna, e a célula do canto soma tudo.

As palavras usadas aqui:

- **Campo** é uma coluna da lista. Região, Mês e Valor são campos.
- **Linhas**, **Colunas** e **Valores** são os três lugares em que um campo pode ficar. Um campo em Linhas gera uma linha para cada um de seus itens diferentes; um campo em Colunas gera uma coluna para cada um; um campo em Valores é o que é somado. Um quarto lugar, **Filtros**, guarda um campo que deixa de fora as linhas que você não escolhe.
- Um **cache** (o cache da tabela dinâmica) é uma cópia da sua lista que a tabela guarda dentro do arquivo, para mostrar totais sem consultar as células. O relatório é calculado a partir do cache, e não a partir das células, por isso não muda quando as células mudam. **Atualizar** copia as células para o cache de novo.

## Criar uma tabela dinâmica

1. Clique em uma célula da lista, ou selecione as células que deseja usar. A lista precisa de uma linha com os nomes acima dos dados.
2. Escolha **Inserir** e depois **Tabela Dinâmica**.
3. A caixa mostra as células em que estão os dados. Altere-as se não estiverem corretas. Você também pode digitar o nome de uma tabela.
4. Escolha onde a tabela dinâmica vai ficar: **Nova planilha** (o padrão), ou uma célula desta planilha ou de outra. Escolha um lugar em que não haja nada, porque a tabela vazia ocupa cerca de dezoito linhas e três colunas.
5. Escolha **OK**. A tabela começa vazia, e o painel **Campos da Tabela Dinâmica** se abre. Marque os campos que deseja: um campo de números é somado, e qualquer outro vira uma linha. Depois mova os campos para as caixas, como explicam as próximas seções.

Uma nova planilha é uma etapa própria do Desfazer, e a tabela também. Ao salvar como .xlsx, a tabela é gravada da forma como o programa original grava uma, de modo que abre ali como tabela dinâmica. Uma cópia salva como .xls ou .ods mantém nas células os números que a tabela mostra, mas não a tabela em si, e o salvamento avisa você disso.

## GETPIVOTDATA: ler um número de uma tabela dinâmica

`GETPIVOTDATA` informa o número que uma tabela dinâmica mostra para os valores, linhas e colunas que você indicar. Para o exemplo acima:

`=GETPIVOTDATA("Soma de Valor"; $E$3; "Região"; "Leste"; "Mês"; "Jan")` resulta em 15.

- O primeiro argumento é o nome do valor, como a tabela o mostra (por exemplo, Soma de Valor). O nome do campo que é somado (Valor) também funciona.
- O segundo argumento é qualquer célula da tabela dinâmica.
- Depois vêm pares de um campo e um de seus itens. Indique apenas Região e Leste, e você obtém o total de Leste, 35. Não indique nada, e você obtém o total geral, 49. Os campos que você indicar devem ser os primeiros campos das linhas, ou os primeiros campos das colunas, na ordem.
- Se a tabela não mostrar o número, a célula exibe `#REF!`.

Quando você clica em um valor de uma tabela dinâmica enquanto digita uma fórmula, o JeffCrunch escreve essa fórmula para você, para que ela continue lendo o número certo depois que a tabela for reorganizada. Para desativar isso, escolha **Opções** na guia **Tabela Dinâmica** e desmarque a caixa de seleção sobre o GETPIVOTDATA.

## O que o JeffCrunch faz com uma tabela dinâmica

Uma tabela dinâmica em uma pasta de trabalho é mostrada como foi salva. Se você não alterar nada nela, o arquivo é salvo com a tabela dinâmica exatamente como estava. Quando você a atualiza ou altera, o JeffCrunch calcula o relatório de novo, coloca os novos números nas células e regrava a tabela dinâmica, para que o programa que a criou a abra sem reclamar.

O JeffCrunch calcula tabelas dinâmicas que somam, contam, calculam médias e assim por diante, que agrupam datas e números, e que mostram valores como porcentagem, diferença ou total acumulado. Uma tabela dinâmica que mostra um valor como classificação ou como porcentagem de um item pai, que tem grupos de itens criados manualmente, ou que usa um modelo de dados é mostrada e mantida como foi salva, e os comandos avisam que não podem alterá-la.

Clique em qualquer célula de uma tabela dinâmica. A guia **Tabela Dinâmica** aparece.

## Atualizar

**Atualizar** lê as células de que a tabela dinâmica é feita e calcula o relatório de novo. Novas entradas aparecem depois das que já existem; entradas que deixaram de existir são retiradas. As larguras das colunas e o layout que você escolheu permanecem. **Atualizar Tudo**, na guia **Dados** e na guia **Tabela Dinâmica**, faz o mesmo para todas as tabelas dinâmicas da pasta de trabalho.

Se o relatório for crescer sobre células que contêm algo, o JeffCrunch não atualiza. Ele informa quais células estão no caminho. Limpe-as ou mova a tabela dinâmica e atualize de novo.

**Desfazer** desfaz uma atualização em uma única etapa.

## Alterar Fonte de Dados

**Alterar Fonte de Dados** pede as células em que está a lista, incluindo a primeira linha com os nomes. Digite-as como `Plan1!A1:D100`, ou digite o nome de uma tabela ou de um nome que você tenha criado. O JeffCrunch lê essas células e calcula o relatório de novo. Se um campo da lista tiver o mesmo nome nas novas células, a posição dele no relatório é mantida.

## Limpar

**Limpar** tem duas opções. **Limpar Filtros** remove todos os filtros e mostra todas as entradas novamente. **Limpar Tudo** tira todos os campos do relatório e o deixa vazio, pronto para ser montado de novo.

## Detalhar e subir nível

## Páginas de Filtro do Relatório

Um filtro fica acima de uma tabela dinâmica e permite ver um item de cada vez, como um mês. **Mostrar Páginas de Filtro do Relatório**, na guia **Tabela Dinâmica**, cria uma página para cada item de uma só vez. Cada página é uma nova planilha, com o nome do item, e com sua própria cópia da tabela. O filtro de cada cópia é definido para esse item. Se a tabela tiver mais de um filtro, você escolhe qual usar. Um único **Desfazer** remove todas as páginas.

Quando um relatório tem mais de um campo nas linhas, um item do primeiro campo pode ser aberto ou fechado. Clique na célula com o nome do item e escolha **Subir Nível** para fechá-lo: as linhas do próximo campo abaixo dele ficam ocultas e só o seu total permanece. **Detalhar** o abre de novo. Um item do último campo não tem nada abaixo dele. Cada ação é um **Desfazer**.

## Selecionar e mover

**Selecionar**, na guia **Tabela Dinâmica**, seleciona parte do relatório: **Tabela Dinâmica Inteira**, **Valores** (os números) ou **Rótulos** (os nomes na lateral; se o relatório não tiver nomes na lateral, os nomes no topo). **Mover Tabela Dinâmica…** pede uma célula da mesma planilha e desenha o relatório de novo com a primeira célula ali. As células que ele deixa são esvaziadas. As fórmulas que apontam para as células do relatório acompanham essas células, como acontece ao recortar e colar. Se o novo lugar tiver algo, o JeffCrunch não move o relatório e informa isso. Um **Desfazer** o devolve ao lugar anterior.

## O painel de campos

**Lista de Campos**, na guia **Tabela Dinâmica**, abre o painel **Campos da Tabela Dinâmica**. Ele lista os campos da lista, com uma caixa ao lado de cada um, e, abaixo, os quatro lugares: **Filtros**, **Colunas**, **Linhas** e **Valores**.

- Marque um campo para adicioná-lo. Um campo que tem apenas números vai para **Valores** e é somado; qualquer outro vai para **Linhas**.
- Desmarque um campo para retirá-lo de todos os lugares.
- Clique em um campo em um dos quatro lugares e escolha **Mover Para** para colocá-lo em outro. As palavras **Mover para Cima** e **Mover para Baixo** mudam a ordem dele no seu lugar, e **Remover Campo** o retira.
- Com mais de um valor, aparece uma linha chamada **Valores** em **Colunas**. Mova-a para **Linhas** para colocar os valores na lateral.
- **Configurações do Campo de Valor** (um clique duplo sobre um valor, ou o botão) define o nome que o relatório mostra para o valor e como ele é totalizado: **Soma**, **Contagem**, **Contagem de Números**, **Média**, **Máx**, **Mín**, **Produto**, **DesvPad** e **DesvPadP** (o desvio padrão de uma amostra, ou de todos os valores), e **Var** e **VarP** (a variância desses mesmos conjuntos). Também define como o valor é mostrado e o seu formato de número. Veja a próxima seção.
- **Configurações do Campo** (um clique duplo sobre um campo em Filtros, Colunas ou Linhas, ou o botão) define um só campo. Veja "Configurações do Campo".

Cada alteração é uma etapa do **Desfazer**.

## Mostrar Valores Como, e o formato de número de um valor

Em **Configurações do Campo de Valor**, **Mostrar valores como** altera o que uma célula do relatório mostra.

- **% do Total Geral**: a célula dividida pelo total de todo o relatório. No exemplo, Leste em janeiro é 15 de 49, o que dá 30,6%.
- **% do Total da Coluna** e **% do Total da Linha**: a célula dividida pelo total de sua coluna ou de sua linha.
- **Índice**: como a célula se compara com o que a sua linha e a sua coluna dariam se os dados fossem distribuídos igualmente. Um número acima de 1 significa mais do que a parcela igual.
- **Diferença De**, **% De** e **% da Diferença De**: a célula comparada com outro item de um campo. Escolha o **Campo base** (um campo em Linhas ou Colunas) e o **Item base** (um de seus itens, ou o anterior ou o seguinte). Por exemplo, com Mês como campo base e Jan como item base, fevereiro mostra quanto ele é maior ou menor que janeiro. O próprio item base fica vazio para uma diferença, e é 100% para **% De**.
- **Total Acumulado Em**: cada célula soma as células anteriores ao longo do campo base.
- **Classificar do Menor para o Maior** e **Classificar do Maior para o Menor**: cada célula mostra a sua posição, 1, 2, 3 e assim por diante, entre as células do campo base na mesma linha ou coluna. A posição 1 é o menor número, ou o maior.
- **% do Total da Linha Pai** e **% do Total da Coluna Pai**: cada célula mostra que parte do seu pai ela representa. O pai de um item é o item um nível acima, como o ano que contém um mês. Um item sem pai é mostrado como parte do total geral.

**Formato de número** aceita um código de formato, como `0,00`, `#.##0` ou `0,0%`. Deixe-o vazio para manter o formato que as células já têm. Um valor mostrado como porcentagem recebe `0,00%`, a menos que você escolha um formato. As células do relatório assumem o formato na hora. **Desfazer** o desfaz junto com os números.

## Configurações do Campo

Selecione um campo na caixa **Filtros**, **Colunas** ou **Linhas** e escolha **Configurações do Campo** na guia **Tabela Dinâmica** (ou dê um clique duplo sobre ele).

- **Nome personalizado** altera o nome que o relatório mostra para o campo.
- **Subtotais**: **Automático** mostra um total para cada grupo do campo, **Nenhum** não mostra.
- **Mostrar rótulos de itens em**: formato compacto, de estrutura de tópicos ou tabular, como em Layout do Relatório.
- **Inserir uma linha em branco após cada item** coloca uma linha vazia depois de cada grupo.
- **Repetir rótulos de itens** mostra o nome de um item externo em cada linha do seu grupo (formatos de estrutura de tópicos e tabular).
- **Classificar**: manter a ordem em que os itens estão (manual), ou classificar de A a Z ou de Z a A pelos rótulos. **Do menor para o maior por um valor** e **Do maior para o menor por um valor** classificam os itens pelos números de um campo de valor. O relatório mantém a classificação quando os dados mudam.
- **Filtrar** deixa itens de fora por seus rótulos (começa com, termina com, contém, é igual a) ou por um valor (maior que, menor que, entre, e assim por diante), ou mantém apenas os primeiros ou os últimos itens por um valor. O filtro é calculado quando você escolhe OK. Ele oculta os itens que deixa de fora; as caixas de seleção abaixo mostram quais são, e você pode marcar um item para mostrá-lo de novo.
- **Mostrar estes itens** é uma lista dos itens do campo, com uma caixa de seleção para cada um. Desmarque uma caixa para ocultar o item, e os totais o deixam de fora.

## Agrupar datas e números

Selecione um campo de datas ou de números em **Linhas** ou **Colunas** e escolha **Agrupar Campo** na guia **Tabela Dinâmica**. Os grupos são criados a partir dos dados, por isso a coluna deve ter apenas números ou datas, sem texto e sem células vazias.

- **Datas**: marque por que agrupar: **Segundos**, **Minutos**, **Horas**, **Dias**, **Meses**, **Trimestres** ou **Anos**. A parte mais detalhada que você marcar permanece no campo. Para cada parte mais ampla, o JeffCrunch acrescenta um campo, com o nome dela (por exemplo, **Anos**), e o coloca antes do campo de data, no mesmo lugar. **Iniciar em** e **Terminar em** são a primeira e a última data; deixe-as como estão para usar a menor e a maior data dos dados. As datas anteriores ao início ou posteriores ao fim ficam em dois grupos extras, que começam com `<` e `>`.
- **Números**: **Iniciar em**, **Terminar em** e **Em intervalos de** criam grupos como 1-10, 11-20, e assim por diante.

**Desagrupar** retira os grupos e os campos que foram acrescentados para eles. Atualizar refaz os grupos com os novos dados. Quando duas tabelas dinâmicas são feitas dos mesmos dados, não é possível criar grupos, porque um grupo é uma parte dos dados que elas compartilham.

## Campos calculados

Um campo calculado é um campo que você cria a partir de uma fórmula. Ele usa outros campos da lista. Por exemplo, uma bonificação de um décimo do valor é `=Valor*0,1`.

Escolha **Campo Calculado** na guia **Tabela Dinâmica**. Digite um nome e uma fórmula. Escreva o nome de um campo na fórmula, ou clique duas vezes nele na lista **Campos** para colocá-lo ali. Coloque um nome que tenha espaços entre aspas simples, como em `='Preço unitário'*Quantidade`. Você pode usar números, `+ - * / ^ %`, comparações e algumas funções, como `SE`, `ARRED`, `SOMA`, `MÍN`, `MÁX`, `MÉDIA` e `ABS`.

Escolha **Adicionar**. O novo campo é colocado na caixa **Valores**. Ele também aparece na lista de campos, onde você pode marcá-lo ou desmarcá-lo como qualquer outro. Ele não pode ir para as linhas, as colunas ou os filtros.

Um campo calculado é calculado em cada célula do relatório. O nome de um campo representa o total desse campo na célula, por isso `=Valor*Valor` eleva ao quadrado o total da célula. Ele não eleva ao quadrado cada linha e depois soma. Um total geral usa os totais de toda a lista.

**Listar Fórmulas** (ao lado de **Campo Calculado**) cria uma nova planilha que informa o nome de cada campo calculado e sua fórmula, e cada item calculado com a sua fórmula, o seu campo e a sua ordem de cálculo. **Desfazer** retira as células, e outro **Desfazer** retira a planilha.

Para alterar um campo, escolha o nome dele na caixa **Nome**. Sua fórmula aparece. Altere-a e escolha **Modificar**. **Excluir** retira o campo e o seu valor. Cada ação é uma etapa do **Desfazer**. Um campo calculado é salvo com o arquivo e é mantido quando o relatório é atualizado. Duas tabelas dinâmicas feitas dos mesmos dados não podem ter um, porque um campo calculado é parte dos dados que elas compartilham.

## Itens calculados

Um item calculado é um item que você cria a partir de outros itens do mesmo campo. Por exemplo, o campo Região tem os itens Leste e Oeste. Um novo item chamado Ambos pode ser `=Leste+Oeste`.

Clique em uma célula da tabela dinâmica e escolha **Item Calculado** na guia **Tabela Dinâmica**. Escolha o **Campo**. Ele precisa estar nas linhas ou nas colunas. Digite um **Nome** e uma **Fórmula**. Escreva o nome de um item na fórmula, ou clique duas vezes nele na lista **Itens** para colocá-lo ali. Coloque um nome que tenha espaços entre aspas simples, como em `='Leste grande'+Oeste`. Você pode usar números, `+ - * / ^ %` e parênteses.

O novo item ganha uma linha no relatório. Em cada coluna, ele mostra a fórmula calculada a partir das linhas dos itens que ela nomeia. O novo item entra nos totais. Um total de somas, como um total geral ou um subtotal, soma a linha do novo item às linhas dos outros itens. Assim, com Leste em 35, Oeste em 14 e Ambos (Leste mais Oeste) em 49, o total geral é 98. Se um item nomeado pela fórmula estiver oculto, a célula mostra **#REF!**. Se a fórmula nomear um item que não existe, a célula mostra **#NOME?**.

Para alterar um item, escolha o nome dele na caixa **Nome**. Sua fórmula aparece. Altere-a e escolha **Modificar**. **Excluir** retira o item e a sua linha. Cada ação é uma etapa do **Desfazer**. Um item calculado é salvo com o arquivo e é mantido quando o relatório é atualizado. Duas tabelas dinâmicas feitas dos mesmos dados não podem ter um, porque um item calculado é parte dos dados que elas compartilham.

### Ordem de cálculo

Às vezes, uma célula tem dois itens calculados. Um está na sua linha e o outro está na sua coluna. A ordem em que eles são calculados pode mudar o resultado. Suponha que o item da linha seja `=Leste-Oeste` e o item da coluna seja `=Jan+1`. Calculada em uma ordem, a célula é (Leste - Oeste) + 1. Na outra ordem, é (Leste + 1) - (Oeste + 1), que é outro número.

**Ordem de Cálculo**, na guia **Tabela Dinâmica**, lista os itens calculados na ordem em que são calculados. O item mais abaixo na lista é calculado por último. Escolha um item e clique em **Mover para Cima** ou **Mover para Baixo**. A ordem é salva com o arquivo como a ordem dos itens, e outros programas a leem da mesma forma. **Listar Fórmulas** mostra o lugar de cada item nela.

## Mostrar as linhas por trás de um valor

Clique duas vezes em um valor de uma tabela dinâmica, ou escolha **Mostrar Detalhes** na guia **Tabela Dinâmica**. O JeffCrunch cria uma nova planilha, chamada Detalhe 1, 2, e assim por diante, com as linhas da lista que formam o número. Os nomes dos campos ficam na primeira linha. F2 ainda edita a célula, se ela puder ser editada. **Desfazer** retira as linhas, e outro **Desfazer** retira a planilha.

## Expandir e recolher

Um campo cujos itens têm outros abaixo deles pode mostrar ou ocultar as linhas abaixo de seus itens. **Expandir Campo Inteiro** e **Recolher Campo Inteiro** fazem isso para todos os itens do campo que você selecionar no painel.

## Opções da Tabela Dinâmica

**Opções**, na guia **Tabela Dinâmica**, abre uma caixa com as opções da tabela.

- **Nome** é o nome da tabela dinâmica. Nenhuma outra tabela dinâmica da pasta de trabalho pode tê-lo.
- **Para valores de erro, mostrar** e **Para células vazias, mostrar** colocam um texto de sua escolha nessas células.
- **Mesclar e centralizar células com rótulos**, **Ajustar a largura das colunas quando a tabela for atualizada** e **Manter a formatação das células quando a tabela for atualizada** são salvos com a tabela.
- **Mostrar totais gerais para linhas** e **para colunas** são o mesmo que as opções de Totais Gerais.
- **Atualizar os dados quando o arquivo for aberto** lê as células de novo sempre que a pasta de trabalho é aberta. É uma etapa do Desfazer, como qualquer atualização.

## Estilo: cabeçalhos, faixas e galeria

As **Opções de Estilo**, na guia **Tabela Dinâmica**, ativam ou desativam a linha de cabeçalho, a primeira coluna (cabeçalhos de linha) e as faixas de linhas e de colunas. **Estilos** tem uma galeria de aparências, e **Limpar Estilo** remove a aparência. Essas aparências são desenhadas do mesmo modo que as aparências das tabelas. **Linhas em Branco** e **Repetir Rótulos** definem a linha em branco e os rótulos repetidos para todos os campos das linhas de uma só vez.

## Estrutura: subtotais, totais gerais e layout

- **Subtotais** mostra ou oculta o total de cada grupo, no início ou no fim do grupo. Quando há mais de um campo em Linhas, o total de cada campo externo é um subtotal.
- **Totais Gerais** tem quatro opções: desativado para linhas e colunas, ativado para os dois, ativado só para linhas (uma coluna de totais à direita, um para cada linha), e ativado só para colunas (uma linha de totais embaixo).
- **Layout do Relatório** é o formato dos rótulos. No formato **compacto**, todos os campos de Linhas dividem uma só coluna, e um novo grupo aparece um pouco à direita. No formato de **estrutura de tópicos**, cada campo tem a sua própria coluna, e cada grupo tem uma linha própria acima de seus itens. No formato **tabular**, cada campo tem a sua própria coluna, o nome do grupo fica na primeira linha dele, e o subtotal aparece abaixo do grupo.

## A mesma lista, um filtro

Mova **Mês** de **Colunas** para **Filtros**. O relatório perde as suas colunas, e uma linha acima dele mostra **Mês** e **(Todos)**. Os números passam a ser o total de cada região em todos os meses.

## Gráfico dinâmico

Clique em uma célula de uma tabela dinâmica e escolha **Inserir** e depois **Gráfico Dinâmico**. O JeffCrunch cria um gráfico de colunas com os valores da tabela. Ele deixa de fora os totais gerais. O gráfico fica marcado no arquivo como o gráfico da tabela dinâmica. Ele lê as células da tabela, por isso os seus números mudam quando você atualiza a tabela ou altera o layout dela. Se a tabela ganhar mais linhas ou colunas, crie o gráfico de novo, porque ele mantém as células a partir das quais foi feito. Um gráfico dinâmico que vem de um arquivo é mostrado como foi salvo.

## Segmentações de dados e linhas do tempo

Uma segmentação de dados é um conjunto de botões que filtra uma tabela dinâmica, e uma linha do tempo é um controle deslizante que faz o mesmo com datas.

Clique em uma célula de uma tabela dinâmica e depois clique em **Segmentações de Dados** na guia Tabela Dinâmica. O painel Segmentações de Dados se abre e lista as segmentações e as linhas do tempo da pasta de trabalho. Uma segmentação tem um botão para cada item do seu campo. Os botões em azul são os itens que a tabela mostra.

Clique em um botão para mostrar apenas esse item. Clique nele de novo para mostrar todos os itens. Mantenha **Ctrl** pressionada e clique para acrescentar um item aos que são mostrados, ou para tirar um deles. Cada clique é uma etapa do Desfazer, e o Desfazer devolve a tabela ao estado anterior. Ao salvar, o arquivo guarda a escolha da segmentação.

**Conexões do Relatório** informa quais tabelas dinâmicas uma segmentação filtra. Uma linha do tempo mostra o período de datas que ela escolheu. O JeffCrunch mostra as segmentações neste painel, não sobre as células, e ainda não altera uma linha do tempo. Ambas são mantidas no arquivo como foram salvas.

## Limites

Uma tabela dinâmica pode ter até 2.000 campos, uma lista de até cinco milhões de linhas e um relatório de até quatro milhões de células. Um arquivo que ultrapasse esses limites é mostrado como foi salvo e não pode ser alterado. Ler de novo cem mil linhas de vinte colunas e calcular o relatório levou cerca de meio segundo no computador em que isto foi medido; um computador mais lento leva mais tempo.
