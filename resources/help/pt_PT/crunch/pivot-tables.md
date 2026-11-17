# Tabelas dinâmicas
<!-- keywords: outro ficheiro, outro livro, origem externa, listar fórmulas, campo calculado, item calculado, ordem de cálculo, campo de fórmula, páginas de filtro de relatório, tabela dinâmica, detalhar, resumir, inserir tabela dinâmica, criar, fazer, selecionar, mover tabela dinâmica, getpivotdata, gerar getpivotdata, atualizar, atualizar tudo, alterar origem de dados, lista de campos, campos, linhas, colunas, valores, filtros, filtro de relatório, total geral, subtotal, esquema, compacto, destaques, tabular, limpar, definições do campo de valor, soma, contar, média, mostrar valores como, percentagem do total, diferença, total acumulado, formato de número, definições do campo, ordenar, filtrar, superiores, agrupar, desagrupar, datas, meses, anos, expandir, recolher, opções, estilo, linhas alternadas, linhas em branco, repetir etiquetas, atualizar ao abrir o ficheiro, gráfico dinâmico, segmentação de dados, linha do tempo, mostrar detalhes, detalhar, linhas por trás de um valor -->

Uma tabela dinâmica é um pequeno relatório feito a partir de uma lista longa. A lista tem uma linha por cada venda, por exemplo; o relatório soma as vendas por região e por mês. Não escreve nenhuma fórmula. Indica quais as colunas da lista que vão para o lado, quais vão para o topo, e qual é somada, e o relatório preenche-se sozinho.

## Um exemplo

Aqui está uma lista nas células A1 a C7. Cada linha é uma venda.

| Região | Mês | Valor |
|---|---|---|
| Este | Jan | 10 |
| Este | Jan | 5 |
| Este | Fev | 20 |
| Oeste | Jan | 7 |
| Oeste | Fev | 3 |
| Oeste | Fev | 4 |

Uma tabela dinâmica com **Região** do lado, **Mês** no topo e a soma de **Valor** no centro tem este aspeto.

| Soma de Valor | Mês | | |
|---|---|---|---|
| Região | Jan | Fev | Total Geral |
| Este | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total Geral | 22 | 27 | 49 |

Este em janeiro é 15 porque se somam as duas linhas de Este de janeiro, 10 e 5. A coluna Total Geral soma cada linha, a linha Total Geral soma cada coluna, e a célula do canto soma tudo.

As palavras usadas aqui:

- **Campo** é uma coluna da lista. Região, Mês e Valor são campos.
- **Linhas**, **Colunas** e **Valores** são os três sítios onde um campo pode ficar. Um campo em Linhas dá uma linha para cada uma das suas entradas diferentes; um campo em Colunas dá uma coluna para cada uma; um campo em Valores é o que é somado. Um quarto sítio, **Filtros**, guarda um campo que deixa de fora as linhas que não escolhe.
- Uma **cache** (a cache da tabela dinâmica) é uma cópia da sua lista que a tabela dinâmica guarda dentro do ficheiro, para poder mostrar totais sem olhar para as células. O relatório é calculado a partir da cache, e não a partir das células, por isso não muda quando as células mudam. **Atualizar** copia outra vez as células para a cache.

## Criar uma tabela dinâmica

1. Clique numa célula da lista, ou selecione as células que quer usar. A lista precisa de uma linha com nomes por cima dos dados.
2. Escolha **Inserir** e, depois, **Tabela Dinâmica**.
3. A caixa mostra as células onde estão os dados. Altere-as se não estiverem certas. Também pode escrever o nome de uma tabela. Para usar uma lista que está noutro ficheiro, escolha **Um intervalo noutro ficheiro**, selecione o ficheiro com **Procurar** e escreva o intervalo, ou o nome de uma tabela ou de um nome que tenha criado nesse ficheiro, como `Folha1!A1:D100`. O JeffCrunch lê livros (`.xlsx`, `.xlsm`, `.xls` e `.ods`) e ficheiros `.csv`.
4. Escolha onde fica a tabela dinâmica: **Uma nova folha** (a predefinição), ou uma célula desta folha ou de outra. Escolha um local onde não haja mais nada, porque a tabela vazia ocupa cerca de dezoito linhas e três colunas.
5. Escolha **OK**. A tabela começa vazia, e abre-se o painel **Campos da Tabela Dinâmica**. Marque os campos que quer: um campo de números é somado, e qualquer outro torna-se uma linha. Depois, mova os campos para as caixas, como indicam as secções seguintes.

Quando os dados estão noutro ficheiro, a tabela dinâmica guarda uma cópia deles, pelo que a tabela funciona e grava-se sem esse ficheiro. Veja Dados noutro ficheiro, mais abaixo.

Uma nova folha é um passo próprio de Anular, e o mesmo acontece com a tabela. Ao guardar como .xlsx, a tabela é gravada da forma como o programa original a grava, pelo que abre aí como tabela dinâmica. Uma cópia guardada como .xls ou .ods mantém nas células os números que a tabela mostra, mas não a própria tabela, e ao guardar é indicado isso.

## GETPIVOTDATA: ler um número de uma tabela dinâmica

`OBTER.DADOS.DINÂMICOS` devolve o número que uma tabela dinâmica mostra para os valores e as linhas e colunas indicados. Para o exemplo acima:

`=OBTER.DADOS.DINÂMICOS("Soma de Valor"; $E$3; "Região"; "Este"; "Mês"; "Jan")` devolve 15.

- O primeiro argumento é o nome do valor, tal como a tabela o mostra (por exemplo, Soma de Valor). O nome do campo que é somado (Valor) também funciona.
- O segundo argumento é qualquer célula da tabela dinâmica.
- Depois seguem pares de um campo e de um dos seus itens. Se indicar apenas Região e Este, obtém o total de Este, 35. Se não indicar nada, obtém o total geral, 49. Os campos que indicar têm de ser os primeiros campos das linhas, ou os primeiros campos das colunas, por ordem.
- Se a tabela não mostrar o número, a célula mostra `#REF!`.

Quando clica num valor de uma tabela dinâmica enquanto escreve uma fórmula, o JeffCrunch escreve esta fórmula por si, para que continue a ler o número certo depois de a tabela ser reorganizada. Para desativar isto, escolha **Opções** no separador **Tabela Dinâmica** e desmarque a caixa de verificação sobre GETPIVOTDATA.

## O que o JeffCrunch faz com uma tabela dinâmica

Uma tabela dinâmica num livro é mostrada tal como foi guardada. Se não alterar nada nela, o ficheiro é guardado com a tabela dinâmica exatamente como estava. Quando a atualiza ou a altera, o JeffCrunch volta a calcular o relatório, coloca os novos números nas células e grava a tabela dinâmica de forma a que o programa que a criou a abra sem problemas.

O JeffCrunch calcula tabelas dinâmicas que somam, contam, calculam médias e outras operações, que agrupam datas e números, e que mostram valores como percentagem, diferença ou total acumulado. Uma tabela dinâmica que mostra um valor como classificação ou como percentagem de um elemento-pai, que tem grupos de itens criados manualmente, ou que usa um modelo de dados, é mostrada e mantida como foi guardada, e os comandos indicam que não a podem alterar.

Clique em qualquer célula de uma tabela dinâmica. Aparece o separador **Tabela Dinâmica**.

## Atualizar

**Atualizar** lê as células de que a tabela dinâmica é feita e volta a calcular o relatório. Os novos registos ficam depois dos que já existem; os registos que já não existem são retirados. As larguras das colunas e o esquema que escolheu mantêm-se. **Atualizar Tudo**, no separador **Dados** e no separador **Tabela Dinâmica**, faz o mesmo para todas as tabelas dinâmicas do livro.

Se o relatório fosse crescer por cima de células que têm conteúdo, o JeffCrunch não atualiza. Indica quais as células que estão no caminho. Limpe-as ou mova a tabela dinâmica, e volte a atualizar.

**Anular** desfaz uma atualização num único passo.

## Alterar Origem de Dados

**Alterar Origem de Dados** pede as células onde está a lista, incluindo a sua primeira linha de nomes. Escreva-as como `Folha1!A1:D100`, ou escreva o nome de uma tabela ou de um nome que tenha criado. O JeffCrunch lê-as e volta a calcular o relatório. Se um campo da lista tiver o mesmo nome nas novas células, o seu lugar no relatório mantém-se.

Também pode apontar a tabela para um intervalo noutro ficheiro. Na caixa, escolha **Um intervalo noutro ficheiro**, selecione o ficheiro e escreva o intervalo nele. Escolher **Um intervalo neste livro** aponta a tabela de novo para células daqui. Cada uma destas alterações é um passo de Anular.

## Dados noutro ficheiro

Uma tabela dinâmica feita a partir de outro ficheiro guarda os dados de onde foi feita dentro deste livro, pelo que pode abrir, alterar e guardar o livro quando o outro ficheiro já não existe. **Atualizar** lê de novo o outro ficheiro. Se o ficheiro tiver sido movido, estiver em falta, for demasiado grande, não for um livro, ou já não tiver o intervalo, a tabela mantém os dados que tinha, e uma mensagem indica que não foi possível ler o ficheiro. Um ficheiro que está noutro computador da sua rede não é lido até o escolher você próprio em **Alterar Origem de Dados**. Uma atualização lê cada ficheiro uma só vez, e no total no máximo 32 ficheiros, 1 GB e 30 segundos; uma tabela que ultrapasse isto mantém os seus dados.

Se um livro indicar que a tabela deve ser lida de novo ao abrir, e os dados da tabela estiverem noutro ficheiro, o JeffCrunch pergunta primeiro, da mesma forma que o faz para as ligações a outros livros. Enquanto não disser que sim, nenhum ficheiro é lido.

Um livro guardado como .xlsx guarda onde está o ficheiro. Escolher o ficheiro na caixa guarda o seu endereço completo. Um livro que outro programa criou, cuja tabela aponta para um ficheiro através de um caminho curto como `Preços.xlsx`, mantém esse caminho, e o JeffCrunch procura o ficheiro na mesma pasta do livro. Quando usa **Guardar Como** para gravar o livro noutra pasta, um caminho desse tipo é reescrito para continuar a indicar o mesmo ficheiro. A atualização não segue quaisquer ligações que o outro ficheiro tenha para outros ficheiros.

## Limpar

**Limpar** tem duas opções. **Limpar Filtros** remove todos os filtros e volta a mostrar todos os registos. **Limpar Tudo** retira todos os campos do relatório e deixa-o vazio, pronto para ser construído de novo.

## Detalhar e resumir

## Páginas de Filtro de Relatório

Um filtro fica por cima de uma tabela dinâmica e permite ver um item de cada vez, como um mês. **Mostrar Páginas de Filtro de Relatório** no separador **Tabela Dinâmica** cria uma página para cada item de uma só vez. Cada página é uma nova folha, com o nome do item, e com a sua própria cópia da tabela. O filtro de cada cópia fica definido para esse item. Se a tabela tiver mais do que um filtro, escolhe qual usar. Um único **Anular** retira todas as páginas.

Quando um relatório tem mais do que um campo nas linhas, um item do primeiro campo pode ser aberto ou fechado. Clique na célula com o nome do item e escolha **Resumir** para o fechar: as linhas do campo seguinte, por baixo dele, ficam ocultas e só o seu total permanece. **Detalhar** volta a abri-lo. Um item do último campo não tem nada por baixo. Cada uma destas ações é um único **Anular**.

## Selecionar e mover

**Selecionar** no separador **Tabela Dinâmica** seleciona parte do relatório: **Toda a Tabela Dinâmica**, **Valores** (os números), ou **Etiquetas** (os nomes do lado; se o relatório não tiver nomes do lado, os nomes no topo). **Mover Tabela Dinâmica…** pede uma célula na mesma folha e volta a desenhar o relatório com a primeira célula aí. As células que deixa para trás ficam vazias. As fórmulas que apontam para as células do relatório seguem-nas, como acontece ao recortar e colar. Se o novo local tiver conteúdo, o JeffCrunch não move o relatório e indica-o. Um único **Anular** o devolve à posição anterior.

## O painel de campos

**Lista de Campos** no separador **Tabela Dinâmica** abre o painel **Campos da Tabela Dinâmica**. Lista os campos da lista, com uma caixa ao lado de cada um, e, por baixo, os quatro sítios: **Filtros**, **Colunas**, **Linhas** e **Valores**.

- Marque um campo para o adicionar. Um campo que tenha apenas números vai para **Valores** e é somado; qualquer outro vai para **Linhas**.
- Desmarque um campo para o retirar de todo o lado.
- Clique num campo de um dos quatro sítios e escolha **Mover para** para o colocar noutro. As palavras **Mover para Cima** e **Mover para Baixo** alteram a sua ordem no respetivo sítio, e **Remover Campo** retira-o.
- Com mais do que um valor, aparece uma linha chamada **Valores** em **Colunas**. Mova-a para **Linhas** para colocar os valores do lado.
- **Definições do Campo de Valor** (duplo clique num valor, ou o botão) define o nome que o relatório mostra para o valor e a forma como é totalizado: **Soma**, **Contar**, **Contar Números**, **Média**, **Máx**, **Mín**, **Produto**, **DesvPad** e **DesvPadp** (o desvio padrão de uma amostra, ou de todos os valores), e **Var** e **Varp** (a variância dos mesmos). Define também a forma como o valor é mostrado e o seu formato de número. Veja a secção seguinte.
- **Definições do Campo** (duplo clique num campo de Filtros, Colunas ou Linhas, ou o botão) define um só campo. Veja "Definições do Campo".

Cada alteração é um passo de **Anular**.

## Mostrar valores como, e o formato de número de um valor

Em **Definições do Campo de Valor**, **Mostrar valores como** altera o que uma célula do relatório mostra.

- **% do Total Geral**: a célula dividida pelo total de todo o relatório. No exemplo, Este em janeiro são 15 de 49, o que dá 30,6%.
- **% do Total da Coluna** e **% do Total da Linha**: a célula dividida pelo total da sua coluna, ou da sua linha.
- **Índice**: a comparação da célula com o que a sua linha e a sua coluna dariam se os dados estivessem distribuídos de forma uniforme. Um número acima de 1 significa mais do que a parte igual.
- **Diferença de**, **% de** e **% da Diferença de**: a célula comparada com outro item de um campo. Escolha o **Campo de base** (um campo de Linhas ou Colunas) e o **Item de base** (um dos seus itens, ou o anterior ou o seguinte). Por exemplo, com Mês como campo de base e Jan como item de base, fevereiro mostra quanto é mais ou menos do que janeiro. O próprio item de base fica vazio numa diferença, e é 100% em **% de**.
- **Total Acumulado em**: cada célula soma as células anteriores ao longo do campo de base.
- **Classificar do Menor para o Maior** e **Classificar do Maior para o Menor**: cada célula mostra o seu lugar, 1, 2, 3, e assim por diante, entre as células do campo de base na mesma linha ou coluna. O lugar 1 é o menor número, ou o maior.
- **% do Total da Linha Principal** e **% do Total da Coluna Principal**: cada célula mostra que parte do seu elemento-pai é. O pai de um item é o item um nível acima, como o ano que contém um mês. Um item sem pai é mostrado como parte do total geral.

**Formato de número** aceita um código de formato, como `0,00`, `#.##0` ou `0,0%`. Deixe-o vazio para manter o formato que as células têm. Um valor mostrado como percentagem recebe `0,00%`, a não ser que escolha outro formato. As células do relatório adotam o formato de imediato. **Anular** o desfaz juntamente com os números.

## Definições do Campo

Selecione um campo na caixa **Filtros**, **Colunas** ou **Linhas** e escolha **Definições do Campo** no separador **Tabela Dinâmica** (ou faça duplo clique nele).

- **Nome personalizado** altera o nome que o relatório mostra para o campo.
- **Subtotais**: **Automático** mostra um total para cada grupo do campo, **Nenhum** não mostra.
- **Mostrar etiquetas de itens em**: forma compacta, de destaques ou tabular, como em Esquema do Relatório.
- **Inserir uma linha em branco após cada item** coloca uma linha vazia depois de cada grupo.
- **Repetir etiquetas de itens** mostra o nome de um item exterior em todas as linhas do seu grupo (formas de destaques e tabular).
- **Ordenar**: manter a ordem em que os itens estão (manual), ou ordenar de A a Z ou de Z a A pelas etiquetas. **Do menor para o maior por um valor** e **Do maior para o menor por um valor** ordenam os itens pelos números de um campo de valor. O relatório mantém a ordenação quando os dados mudam.
- **Filtrar** deixa itens de fora pelas suas etiquetas (começa por, termina em, contém, é igual a) ou por um valor (maior do que, menor do que, entre, e assim por diante), ou mantém apenas os poucos itens superiores ou inferiores por um valor. O filtro é calculado quando escolhe OK. Oculta os itens que deixa de fora; as caixas de verificação abaixo mostram quais são, e pode marcar um item para o voltar a mostrar.
- **Mostrar estes itens** é uma lista dos itens do campo, com uma marcação em cada um. Desmarque um item para o ocultar, e os totais deixam-no de fora.

## Agrupar datas e números

Selecione um campo de datas ou de números em **Linhas** ou **Colunas** e escolha **Agrupar Campo** no separador **Tabela Dinâmica**. Os grupos são criados a partir dos dados, por isso a coluna tem de ter apenas números ou datas, sem texto e sem células vazias.

- **Datas**: marque o que agrupar: **Segundos**, **Minutos**, **Horas**, **Dias**, **Meses**, **Trimestres** ou **Anos**. A parte mais fina que marcar fica no campo. Para cada parte mais grossa, o JeffCrunch acrescenta um campo, com o nome dessa parte (por exemplo, **Anos**), e coloca-o antes do campo de datas no mesmo sítio. **Começar em** e **Terminar em** são a primeira e a última data; deixe-as como estão para usar as datas mais antigas e mais recentes dos dados. As datas anteriores ao início ou posteriores ao fim ficam em dois grupos extra, que começam por `<` e `>`.
- **Números**: **Começar em**, **Terminar em** e **Em intervalos de** criam grupos como 1-10, 11-20, e assim por diante.

**Desagrupar** retira os grupos e os campos que foram acrescentados para os criar. Atualizar volta a criar os grupos a partir dos novos dados. Quando duas tabelas dinâmicas são feitas a partir dos mesmos dados, não se podem criar grupos, porque um grupo é uma parte dos dados que partilham.

## Campos calculados

Um campo calculado é um campo que cria a partir de uma fórmula. Usa outros campos da lista. Por exemplo, uma comissão de um décimo do valor é `=Valor*0,1`.

Escolha **Campo Calculado** no separador **Tabela Dinâmica**. Escreva um nome e uma fórmula. Escreva o nome de um campo na fórmula, ou faça duplo clique nele na lista **Campos** para o colocar aí. Coloque um nome que tenha espaços entre aspas simples, como em `='Preço unitário'*Quantidade`. Pode usar números, `+ - * / ^ %`, comparações e algumas funções, como `SE`, `ARRED`, `SOMA`, `MÍN`, `MÁX`, `MÉDIA` e `ABS`.

Escolha **Adicionar**. O novo campo é colocado na caixa **Valores**. Também aparece na lista de campos, onde o pode marcar ou desmarcar como qualquer outro campo. Não pode ir para as linhas, as colunas ou os filtros.

Um campo calculado é calculado em cada célula do relatório. O nome de um campo representa o total desse campo na célula, por isso `=Valor*Valor` eleva ao quadrado o total da célula. Não eleva ao quadrado cada linha e depois soma. Um total geral usa os totais de toda a lista.

**Listar Fórmulas** (ao lado de **Campo Calculado**) cria uma nova folha que indica cada campo calculado e a sua fórmula, e cada item calculado com a sua fórmula, o seu campo e a sua ordem de cálculo. **Anular** retira as células, e outro **Anular** retira a folha.

Para alterar um campo, escolha o seu nome na caixa **Nome**. A sua fórmula aparece. Altere-a e escolha **Modificar**. **Eliminar** retira o campo e o seu valor. Cada uma destas ações é um passo de **Anular**. Um campo calculado é guardado com o ficheiro e mantém-se quando o relatório é atualizado. Duas tabelas dinâmicas feitas a partir dos mesmos dados não podem ter um, porque um campo calculado é uma parte dos dados que partilham.

## Itens calculados

Um item calculado é um item criado a partir de outros itens do mesmo campo. Por exemplo, o campo Região tem os itens Este e Oeste. Um novo item chamado Ambos pode ser `=Este+Oeste`.

Clique numa célula da tabela dinâmica e escolha **Item Calculado** no separador **Tabela Dinâmica**. Escolha o **Campo**. Tem de estar nas linhas ou nas colunas. Escreva um **Nome** e uma **Fórmula**. Escreva o nome de um item na fórmula, ou faça duplo clique nele na lista **Itens** para o colocar aí. Coloque um nome que tenha espaços entre aspas simples, como em `='Grande Este'+Oeste`. Pode usar números, `+ - * / ^ %` e parênteses.

O novo item fica com uma linha no relatório. Em cada coluna, mostra a fórmula calculada a partir das linhas dos itens que nomeia. O novo item conta para os totais. Um total de somas, como um total geral ou um subtotal, soma a linha do novo item às linhas dos outros itens. Assim, com Este em 35, Oeste em 14 e Ambos (Este mais Oeste) em 49, o total geral é 98. Se um item nomeado na fórmula estiver oculto, a célula mostra **#REF!**. Se a fórmula nomear um item que não existe, a célula mostra **#NOME?**.

Para alterar um item, escolha o seu nome na caixa **Nome**. A sua fórmula aparece. Altere-a e escolha **Modificar**. **Eliminar** retira o item e a sua linha. Cada uma destas ações é um passo de **Anular**. Um item calculado é guardado com o ficheiro e mantém-se quando o relatório é atualizado. Duas tabelas dinâmicas feitas a partir dos mesmos dados não podem ter um, porque um item calculado é uma parte dos dados que partilham.

### Ordem de cálculo

Por vezes, uma célula tem dois itens calculados. Um está na sua linha e o outro na sua coluna. A ordem pela qual são calculados pode alterar o resultado. Suponha que o item da linha é `=Este-Oeste` e o item da coluna é `=Jan+1`. Calculado numa ordem, a célula é (Este - Oeste) + 1. Na outra ordem, é (Este + 1) - (Oeste + 1), que é um número diferente.

**Ordem de Cálculo** no separador **Tabela Dinâmica** lista os itens calculados pela ordem em que são calculados. O item mais abaixo na lista é calculado por último. Escolha um item e clique em **Mover para Cima** ou **Mover para Baixo**. A ordem é guardada com o ficheiro como a ordem dos itens, e outros programas a leem da mesma forma. **Listar Fórmulas** mostra o lugar de cada item nela.

## Mostrar as linhas por trás de um valor

Faça duplo clique num valor de uma tabela dinâmica, ou escolha **Mostrar Detalhes** no separador **Tabela Dinâmica**. O JeffCrunch cria uma nova folha, com o nome Detalhe 1, 2, e assim por diante, com as linhas da lista que compõem o número. Os nomes dos campos estão na primeira linha. F2 continua a editar a célula, se a célula puder ser editada. **Anular** retira as linhas, e outro **Anular** retira a folha.

## Expandir e recolher

Um campo cujos itens têm outros por baixo deles pode mostrar ou ocultar as linhas por baixo dos seus itens. **Expandir Campo Inteiro** e **Recolher Campo Inteiro** fazem-no para todos os itens do campo que selecionar no painel.

## Opções da Tabela Dinâmica

**Opções** no separador **Tabela Dinâmica** abre uma caixa com as opções da tabela.

- **Nome** é o nome da tabela dinâmica. Nenhuma outra tabela dinâmica do livro o pode ter.
- **Para valores de erro, mostrar** e **Para células vazias, mostrar** colocam nessas células um texto à sua escolha.
- **Unir e centrar células com etiquetas**, **Ajustar larguras das colunas quando a tabela for atualizada** e **Manter a formatação das células quando a tabela for atualizada** são guardadas com a tabela.
- **Mostrar totais gerais para linhas** e **para colunas** são o mesmo que as opções de Totais Gerais.
- **Atualizar os dados quando o ficheiro for aberto** lê as células de novo sempre que o livro é aberto. É um passo de Anular, como qualquer atualização.

## Estilo: cabeçalhos, faixas e galeria

As **Opções de Estilo** no separador **Tabela Dinâmica** ativam ou desativam a linha de cabeçalho, a primeira coluna (cabeçalhos de linha) e as faixas de linhas e de colunas. **Estilos** tem uma galeria de aspetos, e **Limpar Estilo** retira o aspeto. Estes aspetos são desenhados da mesma forma que os das tabelas. **Linhas em Branco** e **Repetir Etiquetas** definem a linha em branco e as etiquetas repetidas para todos os campos das linhas de uma só vez.

## Estrutura: subtotais, totais gerais e esquema

- **Subtotais** mostra ou oculta o total de cada grupo, no topo ou na base do grupo. Quando há mais do que um campo nas Linhas, o total de cada campo exterior é um subtotal.
- **Totais Gerais** tem quatro opções: desativados para linhas e colunas, ativados para ambos, ativados só para linhas (uma coluna de totais à direita, um para cada linha), e ativados só para colunas (uma linha de totais em baixo).
- **Esquema do Relatório** é a forma das etiquetas. Na forma **compacta**, todos os campos das Linhas partilham uma coluna, e um novo grupo é mostrado um pouco mais à direita. Na forma de **destaques**, cada campo tem a sua própria coluna, e cada grupo tem uma linha própria acima dos seus itens. Na forma **tabular**, cada campo tem a sua própria coluna e o nome do grupo fica na sua primeira linha, com o subtotal por baixo do grupo.

## A mesma lista, um filtro

Mova **Mês** de **Colunas** para **Filtros**. O relatório perde as suas colunas, e uma linha por cima dele mostra **Mês** e **(Todos)**. Os números são agora o total de cada região em todos os meses.

## Gráfico dinâmico

Clique numa célula de uma tabela dinâmica e escolha **Inserir** e, depois, **Gráfico Dinâmico**. O JeffCrunch cria um gráfico de colunas com os valores da tabela. Deixa de fora os totais gerais. O gráfico fica marcado no ficheiro como o gráfico da tabela dinâmica. Lê as células da tabela, por isso os seus números mudam quando atualiza a tabela ou altera o seu esquema. Se a tabela ganhar mais linhas ou colunas, crie o gráfico de novo, porque o gráfico mantém as células a partir das quais foi feito. Um gráfico dinâmico que vem de um ficheiro é mostrado tal como foi guardado.

## Segmentações de dados e linhas do tempo

Uma segmentação de dados é um conjunto de botões que filtra uma tabela dinâmica, e uma linha do tempo é um cursor que faz o mesmo para as datas.

Clique numa célula de uma tabela dinâmica e, depois, clique em **Segmentações de Dados** no separador Tabela Dinâmica. Abre-se o painel Segmentações de Dados, que lista as segmentações e as linhas do tempo do livro. Uma segmentação tem um botão para cada item do seu campo. Os botões azuis são os itens que a tabela mostra.

Clique num botão para mostrar apenas esse item. Clique outra vez para mostrar todos os itens. Mantenha **Ctrl** premido e clique para acrescentar um item aos que estão a ser mostrados, ou para retirar um. Cada clique é um passo de Anular, e Anular repõe a tabela. Ao guardar, o ficheiro mantém a escolha da segmentação.

**Ligações de Relatório** indica quais as tabelas dinâmicas que uma segmentação filtra. Uma linha do tempo mostra o período de datas que escolheu. O JeffCrunch mostra as segmentações neste painel, e não sobre as células, e ainda não altera uma linha do tempo. Ambas são mantidas no ficheiro tal como foram guardadas.

## Limites

Uma tabela dinâmica pode ter até 2000 campos, uma lista até cinco milhões de linhas e um relatório até quatro milhões de células. Um ficheiro que ultrapasse estes valores é mostrado tal como foi guardado e não pode ser alterado. Ler de novo cem mil linhas de vinte colunas e calcular o relatório demorou cerca de meio segundo no computador onde isto foi medido; um computador mais lento demora mais.
