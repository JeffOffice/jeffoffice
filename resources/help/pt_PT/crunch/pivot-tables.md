# Tabelas dinâmicas
<!-- keywords: outro ficheiro, outro livro, origem externa, listar fórmulas, campo calculado, item calculado, ordem de cálculo, campo de fórmula, páginas de filtro de relatório, tabela dinâmica, detalhar, resumir, inserir tabela dinâmica, criar, fazer, selecionar, mover tabela dinâmica, getpivotdata, gerar getpivotdata, atualizar, atualizar tudo, alterar origem de dados, lista de campos, campos, linhas, colunas, valores, filtros, filtro de relatório, total geral, subtotal, esquema, compacto, estrutura de tópicos, tabular, limpar, definições do campo de valor, soma, contar, média, mostrar valores como, percentagem do total, diferença, total acumulado, formato de número, definições do campo, ordenar, filtrar, superiores, agrupar, desagrupar, datas, meses, anos, expandir, recolher, opções, estilo, linhas em faixas, linhas em branco, repetir etiquetas, atualizar ao abrir o ficheiro, gráfico dinâmico, segmentação de dados, linha do tempo, mostrar detalhes, detalhar, linhas por trás de um valor -->

Uma tabela dinâmica é um pequeno relatório feito a partir de uma lista longa. A lista tem uma linha para cada venda, por exemplo; o relatório soma as vendas por região e por mês. Não escreve nenhuma fórmula. Indica quais as colunas da lista que vão na vertical, quais vão na horizontal no topo e qual é somada, e o relatório preenche-se sozinho.

## Um exemplo

Aqui está uma lista nas células A1 a C7. Cada linha é uma venda.

| Região | Mês | Montante |
|---|---|---|
| Leste | Jan | 10 |
| Leste | Jan | 5 |
| Leste | Fev | 20 |
| Oeste | Jan | 7 |
| Oeste | Fev | 3 |
| Oeste | Fev | 4 |

Uma tabela dinâmica com **Região** na vertical, **Mês** na horizontal e a soma de **Montante** no centro tem este aspeto.

| Soma de Montante | Mês | | |
|---|---|---|---|
| Região | Jan | Fev | Total Geral |
| Leste | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total Geral | 22 | 27 | 49 |

Leste em janeiro é 15 porque se somam as duas linhas de Leste de janeiro, 10 e 5. A coluna Total Geral soma cada linha, a linha Total Geral soma cada coluna, e a célula do canto soma tudo.

As palavras usadas aqui:

- **Campo** é uma coluna da lista. Região, Mês e Montante são campos.
- **Linhas**, **Colunas** e **Valores** são os três sítios onde um campo pode ficar. Um campo em Linhas dá uma linha para cada um dos seus valores diferentes; um campo em Colunas dá uma coluna para cada um; um campo em Valores é o que é somado. Um quarto sítio, **Filtros**, contém um campo que deixa de fora as linhas que não escolher.
- Uma **cache** (a cache dinâmica) é uma cópia da lista que a tabela dinâmica guarda dentro do ficheiro, para poder mostrar totais sem ler as células. O relatório é calculado a partir da cache, e não das células, por isso não muda quando as células mudam. **Atualizar** copia de novo as células para a cache.

## Criar uma tabela dinâmica

1. Clique numa célula da lista, ou selecione as células que quer usar. A lista precisa de uma linha com nomes por cima dos dados.
2. Escolha **Inserir** e, em seguida, **Tabela Dinâmica**.
3. A caixa mostra as células onde estão os dados. Altere-as se não estiverem certas. Também pode escrever o nome de uma tabela. Para usar uma lista que está noutro ficheiro, escolha **Um intervalo noutro ficheiro**, selecione o ficheiro com **Procurar** e escreva o intervalo, ou o nome de uma tabela ou de um nome que tenha criado nesse ficheiro, como `Folha1!A1:D100`. O JeffCrunch lê livros (`.xlsx`, `.xlsm`, `.xls` e `.ods`) e ficheiros `.csv`.
4. Escolha onde fica a tabela dinâmica: **Uma nova folha** (a predefinição), ou uma célula desta folha ou de outra. Escolha um lugar onde não haja mais nada, porque a tabela vazia ocupa cerca de dezoito linhas e três colunas.
5. Escolha **OK**. A tabela começa vazia, e abre-se o painel **Campos da Tabela Dinâmica**. Marque os campos que quer: um campo de números é somado, e qualquer outro torna-se uma linha. Depois, mova os campos para as caixas, como indicam as secções seguintes.

Quando os dados estão noutro ficheiro, a tabela dinâmica guarda uma cópia deles, pelo que a tabela funciona e grava-se sem esse ficheiro. Veja Dados noutro ficheiro, mais abaixo.

Uma nova folha é um passo próprio de Anular, e o mesmo acontece com a tabela. Quando guarda como .xlsx, a tabela é escrita da forma como o programa original a escreve, pelo que abre ali como tabela dinâmica. Uma cópia guardada como .xls ou .ods mantém os números que a tabela mostra nas suas células, mas não a própria tabela, e ao guardar é indicado isso.

## GETPIVOTDATA: ler um número de uma tabela dinâmica

`OBTERDADOSDINÂMICOS` devolve o número que uma tabela dinâmica mostra para os valores e as linhas e colunas que indicar. Para o exemplo acima:

`=OBTERDADOSDINÂMICOS("Soma de Montante"; $E$3; "Região"; "Leste"; "Mês"; "Jan")` devolve 15.

- O primeiro argumento é o nome do valor, tal como a tabela o mostra (por exemplo, Soma de Montante). O nome do campo que é somado (Montante) também funciona.
- O segundo argumento é qualquer célula da tabela dinâmica.
- Depois seguem pares de um campo e um dos seus itens. Indicando apenas Região e Leste, obtém o total de Leste, 35. Sem indicar nada, obtém o total geral, 49. Os campos que indicar têm de ser os primeiros campos das linhas, ou os primeiros campos das colunas, por ordem.
- Se a tabela não mostrar o número, a célula mostra `#REF!`.

Quando clica num valor de uma tabela dinâmica enquanto escreve uma fórmula, o JeffCrunch escreve esta fórmula por si, para que continue a ler o número certo depois de a tabela ser reorganizada. Para desativar isto, escolha **Opções** no separador **Tabela Dinâmica** e desmarque a caixa de verificação sobre OBTERDADOSDINÂMICOS.

## O que o JeffCrunch faz com uma tabela dinâmica

Uma tabela dinâmica num livro é mostrada tal como foi guardada. Se não alterar nada nela, o ficheiro é guardado com a tabela dinâmica exatamente como estava. Quando a atualiza ou a altera, o JeffCrunch volta a calcular o relatório, coloca os novos números nas células e escreve a tabela dinâmica de novo, para que o programa que a criou a abra sem problemas.

O JeffCrunch calcula tabelas dinâmicas que somam, contam, calculam médias e outras funções, que agrupam datas e números e que mostram valores como percentagem, diferença ou total acumulado. Uma tabela dinâmica que mostre um valor como classificação ou como percentagem de um item pai, que tenha grupos de itens criados manualmente, ou que use um modelo de dados, é mostrada e mantida como foi guardada, e os comandos indicam que não a podem alterar.

Clique em qualquer célula de uma tabela dinâmica. Aparece o separador **Tabela Dinâmica**.

## Atualizar

**Atualizar** lê as células de que a tabela dinâmica é feita e volta a calcular o relatório. Os novos itens aparecem depois dos que já lá estavam; os itens que deixaram de existir são retirados. As larguras das colunas e o esquema que escolheu mantêm-se. **Atualizar Tudo**, no separador **Dados** e no separador **Tabela Dinâmica**, faz o mesmo para todas as tabelas dinâmicas do livro.

Se o relatório fosse ocupar células que já têm conteúdo, o JeffCrunch não atualiza. Indica quais as células que estão no caminho. Limpe-as ou mova a tabela dinâmica, e volte a atualizar.

**Anular** desfaz uma atualização num único passo.

## Alterar Origem de Dados

**Alterar Origem de Dados** pede as células onde está a lista, incluindo a primeira linha com os nomes. Escreva-as como `Folha1!A1:D100`, ou escreva o nome de uma tabela ou de um nome que tenha criado. O JeffCrunch lê-as e volta a calcular o relatório. Se um campo da lista tiver o mesmo nome nas novas células, o seu lugar no relatório mantém-se.

Também pode apontar a tabela para um intervalo noutro ficheiro. Na caixa, escolha **Um intervalo noutro ficheiro**, selecione o ficheiro e escreva o intervalo nele. Escolher **Um intervalo neste livro** volta a apontar a tabela para células daqui. Cada uma destas alterações é um passo de Anular.

## Dados noutro ficheiro

Uma tabela dinâmica feita a partir de outro ficheiro guarda os dados de que foi feita dentro deste livro, pelo que pode abrir, alterar e guardar o livro quando o outro ficheiro já não estiver disponível. **Atualizar** lê o outro ficheiro de novo. Se o ficheiro tiver sido movido, estiver em falta, for demasiado grande, não for um livro, ou já não tiver o intervalo, a tabela mantém os dados que tinha, e uma mensagem indica que o ficheiro não pôde ser lido. Um ficheiro que esteja noutro computador da sua rede não é lido até o escolher você próprio em **Alterar Origem de Dados**. Uma atualização lê cada ficheiro uma só vez, e no máximo 32 ficheiros, 1 GB e 30 segundos no total; uma tabela que ultrapasse estes limites mantém os seus dados.

Se um livro indicar que a tabela deve ser lida de novo ao ser aberto, e os dados da tabela estiverem noutro ficheiro, o JeffCrunch pergunta primeiro, da mesma forma como faz para as ligações a outros livros. Até responder que sim, nenhum ficheiro é lido.

Um livro guardado como .xlsx armazena onde está o ficheiro. Escolher o ficheiro na caixa guarda o seu endereço completo. Um livro criado por outro programa, cuja tabela aponta para um ficheiro através de um caminho curto, como `Preços.xlsx`, mantém esse caminho, e o JeffCrunch procura o ficheiro na mesma pasta que o livro. Quando usa **Guardar Como** para guardar o livro noutra pasta, um caminho deste tipo é reescrito para continuar a indicar o mesmo ficheiro. A atualização não segue quaisquer ligações que o outro ficheiro tenha para outros ficheiros.

## Limpar

**Limpar** tem duas opções. **Limpar Filtros** remove todos os filtros e volta a mostrar todos os itens. **Limpar Tudo** retira todos os campos do relatório e deixa-o vazio, pronto para ser construído de novo.

## Detalhar e resumir

Quando um relatório tem mais de um campo nas linhas, um item do primeiro campo pode ser aberto ou fechado. Clique na célula com o nome do item e escolha **Resumir** para o fechar: as linhas do campo seguinte que estão por baixo dele ficam ocultas e só o seu total se mantém. **Detalhar** volta a abri-lo. Um item do último campo não tem nada por baixo. Cada uma destas ações é um Anular.

## Páginas de filtro de relatório

Um filtro fica acima de uma tabela dinâmica e permite ver um item de cada vez, por exemplo um mês. **Mostrar Páginas de Filtro do Relatório**, no separador **Tabela Dinâmica**, cria uma página para cada item de uma só vez. Cada página é uma nova folha, com o nome do item, e com a sua própria cópia da tabela. O filtro de cada cópia fica definido para esse item. Se a tabela tiver mais de um filtro, escolhe qual usar. Um único **Anular** remove todas as páginas.

## Selecionar e mover

**Selecionar**, no separador **Tabela Dinâmica**, seleciona parte do relatório: **Tabela Dinâmica Inteira**, **Valores** (os números), ou **Etiquetas** (os nomes na vertical; se o relatório não tiver nomes na vertical, os nomes na horizontal). **Mover Tabela Dinâmica…** pede uma célula na mesma folha e desenha o relatório de novo com a primeira célula aí. As células que deixa ficam vazias. As fórmulas que apontam para as células do relatório seguem-nas, como acontece quando recorta e cola. Se o novo local tiver conteúdo, o JeffCrunch não move o relatório e indica-o. Um **Anular** repõe a tabela no sítio original.

## O painel de campos

**Lista de Campos**, no separador **Tabela Dinâmica**, abre o painel **Campos da Tabela Dinâmica**. Lista os campos da lista, com uma caixa ao lado de cada um, e por baixo os quatro sítios: **Filtros**, **Colunas**, **Linhas** e **Valores**.

- Marque um campo para o adicionar. Um campo que tenha apenas números vai para **Valores** e é somado; qualquer outro vai para **Linhas**.
- Desmarque um campo para o retirar de todo o lado.
- Clique num campo em um dos quatro sítios e escolha **Mover Para** para o pôr noutro. As palavras **Mover para Cima** e **Mover para Baixo** alteram a sua ordem dentro do sítio, e **Remover Campo** retira-o.
- Com mais de um valor, aparece uma linha chamada **Valores** em **Colunas**. Mova-a para **Linhas** para pôr os valores na vertical.
- **Definições do Campo de Valor** (duplo clique num valor, ou o botão) define o nome que o relatório mostra para o valor e a forma como é totalizado: **Soma**, **Contar**, **Contar Números**, **Média**, **Máx**, **Mín**, **Produto**, **DesvPad** e **DesvPadp** (o desvio padrão de uma amostra, ou de todos os valores), e **Var** e **Varp** (a variância dos mesmos). Define também a forma como o valor é mostrado e o seu formato de número. Veja a secção seguinte.
- **Definições do Campo** (duplo clique num campo em Filtros, Colunas ou Linhas, ou o botão) define um único campo. Veja "Definições do Campo".

Cada alteração é um passo de **Anular**.

## Mostrar valores como, e o formato de número de um valor

Em **Definições do Campo de Valor**, **Mostrar valores como** altera o que mostra uma célula do relatório.

- **% do Total Geral**: a célula dividida pelo total de todo o relatório. No exemplo, Leste em janeiro é 15 de 49, o que dá 30,6%.
- **% do Total da Coluna** e **% do Total da Linha**: a célula dividida pelo total da sua coluna, ou da sua linha.
- **Índice**: a comparação da célula com o que a sua linha e a sua coluna dariam se os dados estivessem distribuídos de forma uniforme. Um número acima de 1 significa mais do que a parte uniforme.
- **Diferença de**, **% de** e **% da diferença de**: a célula comparada com outro item de um campo. Escolha o **Campo base** (um campo nas Linhas ou nas Colunas) e o **Item base** (um dos seus itens, ou o anterior ou o seguinte). Por exemplo, com Mês como campo base e Jan como item base, fevereiro mostra quanto é mais ou menos do que janeiro. O próprio item base fica vazio numa diferença, e é 100% em **% de**.
- **Total Acumulado em**: cada célula soma as células anteriores ao longo do campo base.
- **Classificar do Menor para o Maior** e **Classificar do Maior para o Menor**: cada célula mostra a sua posição, 1, 2, 3, e assim sucessivamente, entre as células do campo base na mesma linha ou coluna. A posição 1 é o número mais pequeno, ou o maior.
- **% do Total da Linha Pai** e **% do Total da Coluna Pai**: cada célula mostra que parte do seu pai representa. O pai de um item é o item um nível acima, como o ano que contém um mês. Um item sem pai é mostrado como uma parte do total geral.

**Formato de número** aceita um código de formato, como `0,00`, `#.##0` ou `0,0%`. Deixe-o vazio para manter o formato que as células já têm. Um valor mostrado como percentagem recebe `0,00%`, a menos que escolha um formato. As células do relatório adotam o formato de imediato. **Anular** o desfaz, juntamente com os números.

## Definições do Campo

Selecione um campo na caixa **Filtros**, **Colunas** ou **Linhas** e escolha **Definições do Campo** no separador **Tabela Dinâmica** (ou faça duplo clique nele).

- **Nome personalizado** altera o nome que o relatório mostra para o campo.
- **Subtotais**: **Automático** mostra um total para cada grupo do campo, **Nenhum** não mostra.
- **Mostrar etiquetas de itens em**: forma compacta, de estrutura de tópicos ou tabular, como em Esquema do Relatório.
- **Inserir uma linha em branco após cada item** coloca uma linha vazia depois de cada grupo.
- **Repetir etiquetas de itens** mostra o nome de um item exterior em cada linha do seu grupo (formas de estrutura de tópicos e tabular).
- **Ordenar**: manter a ordem em que os itens estão (manual), ou ordenar de A a Z ou de Z a A pelas etiquetas. **Do menor para o maior por um valor** e **Do maior para o menor por um valor** ordenam os itens pelos números de um campo de valor. O relatório mantém a ordenação quando os dados mudam.
- **Filtrar** deixa itens de fora pelas suas etiquetas (começa por, termina em, contém, é igual a) ou por um valor (maior que, menor que, entre, e assim por diante), ou mantém apenas os primeiros ou últimos itens por um valor. O filtro é calculado quando escolhe OK. Oculta os itens que deixa de fora; as caixas de verificação abaixo mostram quais são, e pode marcar um item para o voltar a mostrar.
- **Mostrar estes itens** é uma lista dos itens do campo, com uma caixa de verificação em cada um. Desmarque uma caixa para ocultar o item, e os totais deixam-no de fora.

## Agrupar datas e números

Selecione um campo de datas ou de números nas **Linhas** ou nas **Colunas** e escolha **Agrupar Campo** no separador **Tabela Dinâmica**. Os grupos são criados a partir dos dados, pelo que a coluna tem de ter apenas números ou datas, sem texto e sem células vazias.

- **Datas**: marque o que agrupar: **Segundos**, **Minutos**, **Horas**, **Dias**, **Meses**, **Trimestres** ou **Anos**. A parte mais fina que marcar permanece no campo. Para cada parte mais grossa, o JeffCrunch acrescenta um campo com o respetivo nome (por exemplo **Anos**) e coloca-o antes do campo de datas, no mesmo sítio. **Começar em** e **Terminar em** são a primeira e a última data; deixe-as como estão para usar a data mais antiga e a mais recente dos dados. As datas anteriores ao início ou posteriores ao fim ficam em dois grupos extra, que começam por `<` e `>`.
- **Números**: **Começar em**, **Terminar em** e **Com intervalos de** criam grupos como 1-10, 11-20, e assim por diante.

**Desagrupar** retira os grupos e os campos que foram acrescentados para eles. Atualizar volta a criar os grupos a partir dos novos dados. Quando duas tabelas dinâmicas são feitas a partir dos mesmos dados, não é possível criar grupos, porque um grupo é uma parte dos dados que partilham.

## Campos calculados

Um campo calculado é um campo que cria a partir de uma fórmula. Usa outros campos da lista. Por exemplo, um bónus de um décimo do montante é `=Montante*0,1`.

Escolha **Campo Calculado** no separador **Tabela Dinâmica**. Escreva um nome e uma fórmula. Escreva o nome de um campo na fórmula, ou faça duplo clique nele na lista **Campos** para o colocar lá. Coloque entre plicas um nome que tenha espaços, como em `='Preço unitário'*Quantidade`. Pode usar números, `+ - * / ^ %`, comparações e algumas funções, como `SE`, `ARRED`, `SOMA`, `MÍN`, `MÁX`, `MÉDIA` e `ABS`.

Escolha **Adicionar**. O novo campo é colocado na caixa **Valores**. Aparece também na lista de campos, onde pode marcá-lo ou desmarcá-lo como qualquer outro campo. Não pode ir para as linhas, as colunas ou os filtros.

Um campo calculado é calculado em cada célula do relatório. O nome de um campo representa o total desse campo na célula, por isso `=Montante*Montante` eleva ao quadrado o total da célula. Não eleva ao quadrado cada linha e depois soma. Um total geral usa os totais de toda a lista.

**Listar Fórmulas** (ao lado de **Campo Calculado**) cria uma nova folha que indica cada campo calculado e a sua fórmula, e cada item calculado com a sua fórmula, o seu campo e a sua ordem de cálculo. **Anular** retira as células, e outro **Anular** retira a folha.

Para alterar um campo, escolha o seu nome na caixa **Nome**. A sua fórmula aparece. Altere-a e escolha **Modificar**. **Eliminar** retira o campo e o seu valor. Cada uma destas ações é um passo de **Anular**. Um campo calculado é guardado com o ficheiro e mantém-se quando o relatório é atualizado. Duas tabelas dinâmicas feitas a partir dos mesmos dados não podem ter um, porque um campo calculado é uma parte dos dados que partilham.

## Itens calculados

Um item calculado é um item criado a partir de outros itens do mesmo campo. Por exemplo, o campo Região tem os itens Leste e Oeste. Um novo item chamado Ambos pode ser `=Leste+Oeste`.

Clique numa célula da tabela dinâmica e escolha **Item Calculado** no separador **Tabela Dinâmica**. Escolha o **Campo**. Tem de estar nas linhas ou nas colunas. Escreva um **Nome** e uma **Fórmula**. Escreva o nome de um item na fórmula, ou faça duplo clique nele na lista **Itens** para o colocar lá. Coloque entre plicas um nome que tenha espaços, como em `='Leste grande'+Oeste`. Pode usar números, `+ - * / ^ %` e parênteses.

O novo item recebe uma linha no relatório. Em cada coluna, mostra a fórmula calculada a partir das linhas dos itens que refere. O novo item conta para os totais. Um total de somas, como um total geral ou um subtotal, adiciona a linha do novo item às linhas dos outros itens. Assim, com Leste em 35, Oeste em 14 e Ambos (Leste mais Oeste) em 49, o total geral é 98. Se um item referido na fórmula estiver oculto, a célula mostra **#REF!**. Se a fórmula referir um item que não existe, a célula mostra **#NOME?**.

Para alterar um item, escolha o seu nome na caixa **Nome**. A sua fórmula aparece. Altere-a e escolha **Modificar**. **Eliminar** retira o item e a sua linha. Cada uma destas ações é um passo de **Anular**. Um item calculado é guardado com o ficheiro e mantém-se quando o relatório é atualizado. Duas tabelas dinâmicas feitas a partir dos mesmos dados não podem ter um, porque um item calculado é uma parte dos dados que partilham.

### Ordem de cálculo

Por vezes, uma célula tem dois itens calculados. Um está na sua linha e o outro na sua coluna. A ordem pela qual são calculados pode alterar o resultado. Suponha que o item de linha é `=Leste-Oeste` e o item de coluna é `=Jan+1`. Calculado numa ordem, a célula é (Leste - Oeste) + 1. Na outra ordem, é (Leste + 1) - (Oeste + 1), que é um número diferente.

**Ordem de Cálculo**, no separador **Tabela Dinâmica**, lista os itens calculados pela ordem em que são calculados. O item mais abaixo na lista é calculado por último. Escolha um item e clique em **Mover para Cima** ou **Mover para Baixo**. A ordem fica guardada com o ficheiro como a ordem dos itens, e outros programas leem-na da mesma forma. **Listar Fórmulas** mostra o lugar de cada item nela.

## Mostrar as linhas por trás de um valor

Faça duplo clique num valor de uma tabela dinâmica, ou escolha **Mostrar Detalhes** no separador **Tabela Dinâmica**. O JeffCrunch cria uma nova folha, chamada Detalhe 1, 2, e assim por diante, com as linhas da lista que compõem o número. Os nomes dos campos estão na primeira linha. F2 continua a editar a célula, se a célula puder ser editada. **Anular** retira as linhas, e outro **Anular** retira a folha.

## Expandir e recolher

Um campo cujos itens têm outros por baixo deles pode mostrar ou ocultar as linhas por baixo dos seus itens. **Expandir Campo Inteiro** e **Recolher Campo Inteiro** fazem-no para todos os itens do campo que selecionar no painel.

## Opções da Tabela Dinâmica

**Opções**, no separador **Tabela Dinâmica**, abre uma caixa com as escolhas da tabela.

- **Nome** é o nome da tabela dinâmica. Nenhuma outra tabela dinâmica do livro o pode ter.
- **Para valores de erro, mostrar** e **Para células vazias, mostrar** colocam um texto à sua escolha nessas células.
- **Unir e centrar células com etiquetas**, **Ajustar larguras das colunas quando a tabela for atualizada** e **Manter a formatação das células quando a tabela for atualizada** ficam guardadas com a tabela.
- **Mostrar totais gerais para linhas** e **para colunas** são o mesmo que as opções de Totais Gerais.
- **Atualizar os dados quando o ficheiro for aberto** lê as células de novo sempre que o livro é aberto. É um passo de Anular, como qualquer atualização.

## Estilo: cabeçalhos, faixas e galeria

As **Opções de Estilo**, no separador **Tabela Dinâmica**, ativam ou desativam a linha de cabeçalho, a primeira coluna (cabeçalhos de linha) e as faixas de linhas e de colunas. **Estilos** tem uma galeria de aspetos, e **Limpar Estilo** retira o aspeto. Estes aspetos são desenhados da mesma forma que os aspetos das tabelas. **Linhas em Branco** e **Repetir Etiquetas** definem a linha em branco e as etiquetas repetidas para todos os campos das linhas de uma só vez.

## Estrutura: subtotais, totais gerais e esquema

- **Subtotais** mostra ou oculta o total de cada grupo, no início ou no fim do grupo. Quando há mais de um campo nas Linhas, o total de cada campo exterior é um subtotal.
- **Totais Gerais** tem quatro opções: desativado para linhas e colunas, ativado para ambos, ativado apenas para linhas (uma coluna de totais à direita, um para cada linha), e ativado apenas para colunas (uma linha de totais em baixo).
- **Esquema do Relatório** é a forma das etiquetas. Na forma **compacta**, todos os campos das Linhas partilham uma coluna, e um novo grupo é mostrado um pouco mais à direita. Na forma de **estrutura de tópicos**, cada campo tem a sua própria coluna, e cada grupo tem uma linha própria acima dos seus itens. Na forma **tabular**, cada campo tem a sua própria coluna e o nome do grupo fica na primeira linha, com o subtotal abaixo do grupo.

## A mesma lista, um filtro

Mova **Mês** de **Colunas** para **Filtros**. O relatório perde as suas colunas, e uma linha por cima mostra **Mês** e **(Todos)**. Os números são agora o total de cada região em todos os meses.

## Gráfico dinâmico

Clique numa célula de uma tabela dinâmica e escolha **Inserir** e, em seguida, **Gráfico Dinâmico**. O JeffCrunch cria um gráfico de colunas com os valores da tabela. Deixa de fora os totais gerais. O gráfico fica marcado no ficheiro como o gráfico da tabela dinâmica. Lê as células da tabela, pelo que os seus números mudam quando atualiza a tabela ou altera o seu esquema. Se a tabela ganhar mais linhas ou colunas, crie o gráfico de novo, porque o gráfico mantém as células a partir das quais foi feito. Um gráfico dinâmico que venha de um ficheiro é mostrado tal como foi guardado.

## Segmentações de dados e linhas do tempo

Uma segmentação de dados é um conjunto de botões que filtra uma tabela dinâmica, e uma linha do tempo é um cursor que faz o mesmo para as datas.

Clique numa célula de uma tabela dinâmica e, em seguida, clique em **Segmentações de Dados** no separador Tabela Dinâmica. Abre-se o painel Segmentações de Dados, que lista as segmentações de dados e as linhas do tempo do livro. Uma segmentação de dados tem um botão para cada item do seu campo. Os botões que estão a azul são os itens que a tabela mostra.

Clique num botão para mostrar apenas esse item. Clique de novo para mostrar todos os itens. Mantenha **Ctrl** premido e clique para acrescentar um item aos que estão mostrados, ou para retirar um. Cada clique é um passo de Anular, e Anular repõe a tabela. Ao guardar, o ficheiro mantém a escolha da segmentação.

Uma segmentação de dados que veio com o ficheiro também é desenhada sobre as células, no sítio onde o ficheiro a coloca. Mostra o seu título e um botão para cada item. Os botões que estão a azul são os itens que a tabela mostra. Clique num botão para mostrar apenas esse item, mantenha **Ctrl** premido e clique para acrescentar um item ou retirar um, e clique no funil no canto superior direito para mostrar de novo todos os itens. Um clique é um passo de Anular. Quando a tabela é filtrada de outra forma, ou quando clica em Anular, os botões acompanham essa alteração. Quando uma segmentação é demasiado pequena para mostrar todos os seus botões, a última linha diz "Mais itens" e quantos há (e quantos estão escolhidos); clique nessa linha para abrir o painel Segmentações de Dados, que lista todos os itens. Uma segmentação que filtra uma tabela que não é dinâmica, ou cuja tabela dinâmica não está no ficheiro, é desenhada com o seu título e uma frase que indica que o JeffCrunch ainda não a consegue usar; fica guardada no ficheiro tal como estava. Um livro aberto apenas para leitura, e uma folha protegida contra alterações às tabelas dinâmicas, não deixam que um clique filtre a tabela. Uma segmentação é impressa com a folha, a menos que o ficheiro indique o contrário. Ainda não é possível mover, redimensionar ou eliminar uma segmentação na folha.

**Ligações de Relatório** indica que tabelas dinâmicas são filtradas por uma segmentação. Uma linha do tempo mostra o intervalo de datas que escolheu. O JeffCrunch ainda não desenha uma linha do tempo sobre as células nem a altera. Ambas ficam guardadas no ficheiro tal como foram gravadas.

## Limites

Uma tabela dinâmica pode ter até 2000 campos, uma lista de até cinco milhões de linhas e um relatório de até quatro milhões de células. Um ficheiro que ultrapasse estes valores é mostrado tal como foi guardado e não pode ser alterado. Ler de novo cem mil linhas de vinte colunas e calcular o relatório demorou cerca de meio segundo no computador em que isto foi medido; um computador mais lento demora mais.
