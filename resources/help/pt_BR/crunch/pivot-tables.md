# Tabelas dinâmicas
<!-- keywords: outro arquivo, outra pasta de trabalho, fonte externa, listar fórmulas, campo calculado, item calculado, ordem de cálculo, campo de fórmula, mostrar páginas de filtro de relatório, tabela dinâmica, detalhar, subir nível, inserir tabela dinâmica, criar, fazer, selecionar, mover tabela dinâmica, getpivotdata, gerar getpivotdata, atualizar, atualizar tudo, alterar fonte de dados, lista de campos, campos, linhas, colunas, valores, filtros, filtro de relatório, total geral, subtotal, layout, compacto, estrutura de tópicos, tabular, limpar, configurações do campo de valor, soma, contagem, média, mostrar valores como, porcentagem do total, diferença, total acumulado, formato de número, configurações do campo, classificar, filtrar, primeiros, agrupar, desagrupar, datas, meses, anos, expandir, recolher, opções, estilo, linhas alternadas, linhas em branco, repetir rótulos, atualizar ao abrir o arquivo, gráfico dinâmico, segmentação de dados, linha do tempo, mostrar detalhes, detalhar, linhas por trás de um valor, dados de tabela dinâmica -->

Uma tabela dinâmica é um pequeno relatório feito a partir de uma lista longa. A lista tem uma linha para cada venda, por exemplo; o relatório soma as vendas por região e por mês. Você não escreve nenhuma fórmula. Diga quais colunas da lista ficam na lateral, quais ficam no topo e qual delas é somada, e o relatório se preenche sozinho.

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

Uma tabela dinâmica com **Região** na lateral, **Mês** no topo e a soma de **Valor** no centro fica assim.

| Soma de Valor | Mês | | |
|---|---|---|---|
| Região | Jan | Fev | Total Geral |
| Leste | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total Geral | 22 | 27 | 49 |

Leste em janeiro é 15 porque as duas linhas de Leste de janeiro, 10 e 5, são somadas. A coluna Total Geral soma cada linha, a linha Total Geral soma cada coluna, e a célula do canto soma tudo.

As palavras usadas aqui:

- **Campo** é uma coluna da lista. Região, Mês e Valor são os campos.
- **Linhas**, **Colunas** e **Valores** são os três lugares em que um campo pode ficar. Um campo em Linhas gera uma linha para cada um de seus itens diferentes; um campo em Colunas gera uma coluna para cada um; um campo em Valores é o que será somado. Um quarto lugar, **Filtros**, recebe um campo que deixa de fora as linhas que você não escolher.
- Um **cache** (o cache da tabela dinâmica) é uma cópia da sua lista que a tabela dinâmica guarda dentro do arquivo, para poder mostrar totais sem consultar as células. O relatório é calculado a partir do cache, e não a partir das células, por isso não muda quando as células mudam. **Atualizar** copia as células para o cache de novo.

## Criar uma tabela dinâmica

1. Clique em uma célula da lista ou selecione as células que você quer usar. A lista precisa de uma linha com nomes acima dos dados.
2. Escolha **Inserir** e depois **Tabela Dinâmica**.
3. A caixa mostra as células em que estão os dados. Altere-as se não estiverem corretas. Você também pode digitar o nome de uma tabela. Para usar uma lista que está em outro arquivo, escolha **Um intervalo em outro arquivo**, selecione o arquivo com **Procurar** e digite o intervalo, ou o nome de uma tabela ou de um nome que você criou nesse arquivo, como `Planilha1!A1:D100`. O JeffCrunch lê pastas de trabalho (`.xlsx`, `.xlsm`, `.xls` e `.ods`) e arquivos `.csv`.
4. Escolha onde a tabela dinâmica vai ficar: **Uma nova planilha** (o padrão), ou uma célula desta planilha ou de outra. Escolha um lugar em que não haja mais nada, porque a tabela vazia ocupa cerca de dezoito linhas e três colunas.
5. Escolha **OK**. A tabela começa vazia, e o painel **Campos da Tabela Dinâmica** se abre. Marque os campos que você quer: um campo de números é somado, e qualquer outro vira uma linha. Depois, mova os campos para as caixas, como explicam as próximas seções.

Quando os dados estão em outro arquivo, a tabela dinâmica guarda uma cópia deles, de modo que a tabela funciona e é salva sem esse arquivo. Veja Dados em outro arquivo, abaixo.

Uma nova planilha é uma etapa própria do Desfazer, e a tabela também. Ao salvar como .xlsx, a tabela é gravada da maneira como o programa original grava uma, de modo que ela se abre lá como tabela dinâmica. Uma cópia salva como .xls ou .ods mantém nas células os números que a tabela mostra, mas não a tabela em si, e o salvamento avisa você disso.

## GETPIVOTDATA: ler um número de uma tabela dinâmica

`GETPIVOTDATA` fornece o número que uma tabela dinâmica mostra para os valores e para as linhas e colunas que você indicar. Para o exemplo acima:

`=GETPIVOTDATA("Soma de Valor"; $E$3; "Região"; "Leste"; "Mês"; "Jan")` resulta em 15.

- O primeiro argumento é o nome do valor, como a tabela o mostra (por exemplo, Soma de Valor). O nome do campo que é somado (Valor) também funciona.
- O segundo argumento é qualquer célula da tabela dinâmica.
- Depois vêm pares de um campo e um de seus itens. Se você indicar apenas Região e Leste, obtém o total de Leste, 35. Se não indicar nada, obtém o total geral, 49. Os campos que você indicar devem ser os primeiros campos das linhas ou os primeiros campos das colunas, na ordem.
- Se a tabela não mostrar o número, a célula exibe `#REF!`.

Quando você clica em um valor de uma tabela dinâmica enquanto digita uma fórmula, o JeffCrunch escreve essa fórmula para você, para que ela continue lendo o número certo depois que a tabela for reorganizada. Para desativar isso, escolha **Opções** na guia **Tabela Dinâmica** e desmarque a caixa de seleção sobre GETPIVOTDATA.

## O que o JeffCrunch faz com uma tabela dinâmica

Uma tabela dinâmica em uma pasta de trabalho é mostrada como foi salva. Se você não alterar nada nela, o arquivo é salvo com a tabela dinâmica exatamente como estava. Quando você a atualiza ou altera, o JeffCrunch calcula o relatório de novo, coloca os novos números nas células e grava a tabela dinâmica de volta, para que o programa que a criou a abra sem problemas.

O JeffCrunch calcula tabelas dinâmicas que somam, contam, calculam médias e assim por diante, que agrupam datas e números, e que mostram valores como uma porcentagem, uma diferença ou um total acumulado. Uma tabela dinâmica que mostra um valor como classificação ou como porcentagem de um pai, que tem grupos de itens criados manualmente ou que usa um modelo de dados é mostrada e mantida como foi salva, e os comandos avisam que não é possível alterá-la.

Clique em qualquer célula de uma tabela dinâmica. A guia **Tabela Dinâmica** aparece.

## Atualizar

**Atualizar** lê as células de que a tabela dinâmica é feita e calcula o relatório de novo. Novas entradas aparecem depois das que já existem; entradas que deixaram de existir são retiradas. Larguras de coluna e o layout que você escolheu são mantidos. **Atualizar Tudo**, na guia **Dados** e na guia **Tabela Dinâmica**, faz o mesmo para todas as tabelas dinâmicas da pasta de trabalho.

Se o relatório fosse crescer sobre células que têm algo nelas, o JeffCrunch não atualiza. Ele informa quais células estão no caminho. Limpe-as ou mova a tabela dinâmica e atualize de novo.

**Desfazer** desfaz uma atualização em uma única etapa.

## Alterar Fonte de Dados

**Alterar Fonte de Dados** pede as células em que está a lista, incluindo a primeira linha de nomes. Digite-as como `Planilha1!A1:D100`, ou digite o nome de uma tabela ou de um nome que você criou. O JeffCrunch lê essas células e calcula o relatório de novo. Se um campo da lista tiver o mesmo nome nas novas células, o lugar dele no relatório é mantido.

Você também pode apontar a tabela para um intervalo em outro arquivo. Na caixa, escolha **Um intervalo em outro arquivo**, selecione o arquivo e digite o intervalo nele. Escolher **Um intervalo nesta pasta de trabalho** aponta a tabela de volta para células daqui. Cada uma dessas alterações é uma etapa do Desfazer.

## Dados em outro arquivo

Uma tabela dinâmica feita a partir de outro arquivo mantém os dados de que foi feita dentro desta pasta de trabalho, de modo que você pode abrir, alterar e salvar a pasta de trabalho quando o outro arquivo não estiver mais disponível. **Atualizar** lê o outro arquivo de novo. Se o arquivo tiver sido movido, não existir, for grande demais, não for uma pasta de trabalho ou não tiver mais o intervalo, a tabela mantém os dados que tinha, e uma mensagem informa que o arquivo não pôde ser lido. Um arquivo que está em outro computador da sua rede não é lido até que você o escolha em **Alterar Fonte de Dados**. Uma atualização lê cada arquivo uma vez, e no máximo 32 arquivos, 1 GB e 30 segundos no total; uma tabela que ultrapassar esses limites mantém seus dados.

Se uma pasta de trabalho indicar que a tabela deve ser lida de novo ao abrir, e os dados da tabela estiverem em outro arquivo, o JeffCrunch pergunta antes, do mesmo modo como faz com os vínculos para outras pastas de trabalho. Até que você diga sim, nenhum arquivo é lido.

Uma pasta de trabalho salva como .xlsx guarda onde o arquivo está. Ao escolher o arquivo na caixa, o endereço completo dele é guardado. Uma pasta de trabalho criada por outro programa, cuja tabela aponta para um arquivo por um caminho curto, como `Preços.xlsx`, mantém esse caminho, e o JeffCrunch procura o arquivo na mesma pasta da pasta de trabalho. Quando você usa **Salvar Como** para salvar a pasta de trabalho em outra pasta, um caminho desse tipo é reescrito para continuar apontando para o mesmo arquivo. Atualizar não segue os vínculos que o outro arquivo tenha com outros arquivos.

## Limpar

**Limpar** tem duas opções. **Limpar Filtros** remove todos os filtros e volta a mostrar todas as entradas. **Limpar Tudo** tira todos os campos do relatório e deixa-o vazio, pronto para ser montado de novo.

## Detalhar e subir nível

## Mostrar Páginas de Filtro do Relatório

Um filtro fica acima de uma tabela dinâmica e permite ver um item de cada vez, como um mês. **Mostrar Páginas de Filtro do Relatório**, na guia **Tabela Dinâmica**, cria uma página para cada item de uma vez. Cada página é uma nova planilha, com o nome do item, e com sua própria cópia da tabela. O filtro de cada cópia é definido para esse item. Se a tabela tiver mais de um filtro, você escolhe qual usar. Um único **Desfazer** remove todas as páginas.

Quando um relatório tem mais de um campo nas linhas, um item do primeiro campo pode ser aberto ou fechado. Clique na célula com o nome do item e escolha **Subir Nível** para fechá-lo: as linhas do campo seguinte abaixo dele ficam ocultas e só o total dele permanece. **Detalhar** o abre de novo. Um item do último campo não tem nada abaixo dele. Cada ação é um único **Desfazer**.

## Selecionar e mover

**Selecionar**, na guia **Tabela Dinâmica**, seleciona parte do relatório: **Tabela Dinâmica Inteira**, **Valores** (os números) ou **Rótulos** (os nomes na lateral; se o relatório não tiver nomes na lateral, os nomes no topo). **Mover Tabela Dinâmica…** pede uma célula na mesma planilha e desenha o relatório de novo com a primeira célula dele ali. As células que ele deixa são esvaziadas. Fórmulas que apontam para as células do relatório acompanham essas células, como acontece ao recortar e colar. Se o novo lugar tiver algo nele, o JeffCrunch não move o relatório e informa isso. Um único **Desfazer** o devolve ao lugar anterior.

## O painel de campos

**Lista de Campos**, na guia **Tabela Dinâmica**, abre o painel **Campos da Tabela Dinâmica**. Ele lista os campos da lista com uma caixa ao lado de cada um e, abaixo, os quatro lugares: **Filtros**, **Colunas**, **Linhas** e **Valores**.

- Marque um campo para adicioná-lo. Um campo que tem apenas números vai para **Valores** e é somado; qualquer outro vai para **Linhas**.
- Desmarque um campo para retirá-lo de todos os lugares.
- Clique em um campo em um dos quatro lugares e escolha **Mover Para** para colocá-lo em outro. As palavras **Mover para Cima** e **Mover para Baixo** alteram a ordem dele no seu lugar, e **Remover Campo** o tira dali.
- Com mais de um valor, aparece em **Colunas** uma linha chamada **Valores**. Mova-a para **Linhas** para colocar os valores na lateral.
- **Configurações do Campo de Valor** (um clique duplo em um valor, ou o botão) define o nome que o relatório mostra para o valor e como ele é totalizado: **Soma**, **Contagem**, **Contagem de Números**, **Média**, **Máx**, **Mín**, **Produto**, **DesvPad** e **DesvPadP** (o desvio padrão de uma amostra, ou de todos os valores), e **Var** e **VarP** (a variância dos mesmos). Também define como o valor é mostrado e o seu formato numérico. Veja a próxima seção.
- **Configurações do Campo** (um clique duplo em um campo de Filtros, Colunas ou Linhas, ou o botão) define um único campo. Veja "Configurações do Campo".

Cada alteração é uma etapa do **Desfazer**.

## Mostrar Valores Como, e o formato numérico de um valor

Em **Configurações do Campo de Valor**, **Mostrar valores como** altera o que uma célula do relatório mostra.

- **% do Total Geral**: a célula dividida pelo total de todo o relatório. No exemplo, Leste em janeiro é 15 de 49, que é 30,6%.
- **% do Total da Coluna** e **% do Total da Linha**: a célula dividida pelo total de sua coluna ou de sua linha.
- **Índice**: como a célula se compara com o que a sua linha e a sua coluna dariam se os dados fossem distribuídos igualmente. Um número acima de 1 significa mais do que a parcela igual.
- **Diferença De**, **% De** e **% Diferença De**: a célula comparada com outro item de um campo. Escolha o **Campo base** (um campo em Linhas ou Colunas) e o **Item base** (um dos itens dele, ou o anterior ou o seguinte). Por exemplo, com Mês como campo base e Jan como item base, fevereiro mostra quanto ele é a mais ou a menos que janeiro. O próprio item base fica vazio em uma diferença, e vale 100% em **% De**.
- **Total Acumulado Em**: cada célula soma as células anteriores ao longo do campo base.
- **Classificar do Menor para o Maior** e **Classificar do Maior para o Menor**: cada célula mostra a sua posição, 1, 2, 3 e assim por diante, entre as células do campo base na mesma linha ou coluna. A posição 1 é o menor número, ou o maior.
- **% do Total do Pai da Linha** e **% do Total do Pai da Coluna**: cada célula mostra que parte do seu pai ela é. O pai de um item é o item um nível acima, como o ano que contém um mês. Um item sem pai é mostrado como parte do total geral.

**Formato de número** aceita um código de formato, como `0,00`, `#.##0` ou `0,0%`. Deixe-o vazio para manter o formato que as células têm. Um valor mostrado como porcentagem recebe `0,00%`, a menos que você escolha um formato. As células do relatório assumem o formato imediatamente. **Desfazer** o desfaz junto com os números.

## Configurações do Campo

Selecione um campo na caixa **Filtros**, **Colunas** ou **Linhas** e escolha **Configurações do Campo** na guia **Tabela Dinâmica** (ou dê um clique duplo nele).

- **Nome personalizado** altera o nome que o relatório mostra para o campo.
- **Subtotais**: **Automático** mostra um total para cada grupo do campo; **Nenhum** não mostra.
- **Mostrar rótulos de itens em**: formato compacto, de estrutura de tópicos ou tabular, como em Layout do Relatório.
- **Inserir uma linha em branco após cada item** coloca uma linha vazia depois de cada grupo.
- **Repetir rótulos de itens** mostra o nome de um item externo em cada linha do seu grupo (formatos de estrutura de tópicos e tabular).
- **Classificar**: manter a ordem em que os itens estão (manual), ou classificar de A a Z ou de Z a A pelos rótulos. **Do menor para o maior por um valor** e **Do maior para o menor por um valor** classificam os itens pelos números de um campo de valor. O relatório mantém a classificação quando os dados mudam.
- **Filtro** deixa itens de fora pelos rótulos (começa com, termina com, contém, é igual a) ou por um valor (maior que, menor que, entre, e assim por diante), ou mantém apenas os primeiros ou últimos itens por um valor. O filtro é calculado quando você escolhe OK. Ele oculta os itens que deixa de fora; as caixas de seleção abaixo mostram quais são, e você pode marcar um item para que ele volte a aparecer.
- **Mostrar estes itens** é uma lista dos itens do campo, cada um com uma caixa de seleção. Desmarque uma caixa para ocultar o item, e os totais deixam de incluí-lo.

## Agrupar datas e números

Selecione um campo de datas ou de números em **Linhas** ou **Colunas** e escolha **Agrupar Campo** na guia **Tabela Dinâmica**. Os grupos são feitos a partir dos dados, por isso a coluna deve ter apenas números ou apenas datas, sem texto e sem células vazias.

- **Datas**: marque o que agrupar: **Segundos**, **Minutos**, **Horas**, **Dias**, **Meses**, **Trimestres** ou **Anos**. A parte mais detalhada que você marcar permanece no campo. Para cada parte mais ampla, o JeffCrunch acrescenta um campo com o nome dela (por exemplo, **Anos**) e o coloca antes do campo de datas, no mesmo lugar. **Iniciar em** e **Terminar em** são a primeira e a última data; deixe-os como estão para usar a menor e a maior data dos dados. Datas anteriores ao início ou posteriores ao fim vão para dois grupos extras, que começam com `<` e `>`.
- **Números**: **Iniciar em**, **Terminar em** e **Em intervalos de** formam grupos como 1-10, 11-20, e assim por diante.

**Desagrupar** remove os grupos e os campos que foram acrescentados para eles. Atualizar refaz os grupos com os dados novos. Quando duas tabelas dinâmicas são feitas dos mesmos dados, não é possível criar grupos, porque um grupo é uma parte dos dados que elas compartilham.

## Campos calculados

Um campo calculado é um campo que você cria a partir de uma fórmula. Ele usa outros campos da lista. Por exemplo, uma bonificação de um décimo do valor é `=Valor*0,1`.

Escolha **Campo Calculado** na guia **Tabela Dinâmica**. Digite um nome e uma fórmula. Escreva na fórmula o nome de um campo, ou clique duas vezes nele na lista **Campos** para inseri-lo ali. Coloque um nome que tenha espaços entre aspas simples, como em `='Preço unitário'*Quantidade`. Você pode usar números, `+ - * / ^ %`, comparações e algumas funções, como `SE`, `ARRED`, `SOMA`, `MÍN`, `MÁX`, `MÉDIA` e `ABS`.

Escolha **Adicionar**. O novo campo é colocado na caixa **Valores**. Ele também aparece na lista de campos, onde você pode marcá-lo ou desmarcá-lo como qualquer outro campo. Ele não pode ir para as linhas, as colunas ou os filtros.

Um campo calculado é calculado em cada célula do relatório. O nome de um campo representa o total desse campo na célula, de modo que `=Valor*Valor` eleva ao quadrado o total da célula. Ele não eleva ao quadrado cada linha e depois soma. Um total geral usa os totais de toda a lista.

**Listar Fórmulas** (ao lado de **Campo Calculado**) cria uma nova planilha que indica cada campo calculado e sua fórmula, e cada item calculado com sua fórmula, seu campo e sua ordem de cálculo. **Desfazer** remove as células, e outro **Desfazer** remove a planilha.

Para alterar um campo, escolha o nome dele na caixa **Nome**. Sua fórmula aparece. Altere-a e escolha **Modificar**. **Excluir** remove o campo e seu valor. Cada uma dessas ações é uma etapa do **Desfazer**. Um campo calculado é salvo com o arquivo e é mantido quando o relatório é atualizado. Duas tabelas dinâmicas feitas dos mesmos dados não podem ter um campo calculado, porque ele é parte dos dados que elas compartilham.

## Itens calculados

Um item calculado é um item que você cria a partir de outros itens do mesmo campo. Por exemplo, o campo Região tem os itens Leste e Oeste. Um novo item chamado Ambos pode ser `=Leste+Oeste`.

Clique em uma célula da tabela dinâmica e escolha **Item Calculado** na guia **Tabela Dinâmica**. Escolha o **Campo**. Ele precisa estar nas linhas ou nas colunas. Digite um **Nome** e uma **Fórmula**. Escreva na fórmula o nome de um item, ou clique duas vezes nele na lista **Itens** para inseri-lo ali. Coloque um nome que tenha espaços entre aspas simples, como em `='Grande Leste'+Oeste`. Você pode usar números, `+ - * / ^ %` e parênteses.

O novo item ganha uma linha no relatório. Em cada coluna, ele mostra a fórmula calculada a partir das linhas dos itens que ela cita. O novo item entra nos totais. Um total de somas, como um total geral ou um subtotal, soma a linha do novo item às linhas dos outros itens. Assim, com Leste em 35, Oeste em 14 e Ambos (Leste mais Oeste) em 49, o total geral é 98. Se um item citado na fórmula estiver oculto, a célula mostra **#REF!**. Se a fórmula citar um item que não existe, a célula mostra **#NOME?**.

Para alterar um item, escolha o nome dele na caixa **Nome**. Sua fórmula aparece. Altere-a e escolha **Modificar**. **Excluir** remove o item e a sua linha. Cada uma dessas ações é uma etapa do **Desfazer**. Um item calculado é salvo com o arquivo e é mantido quando o relatório é atualizado. Duas tabelas dinâmicas feitas dos mesmos dados não podem ter um, porque um item calculado é parte dos dados que elas compartilham.

### Ordem de cálculo

Às vezes, uma célula tem dois itens calculados. Um está na sua linha e o outro está na sua coluna. A ordem em que eles são calculados pode mudar o resultado. Suponha que o item da linha seja `=Leste-Oeste` e o item da coluna seja `=Jan+1`. Calculado em uma ordem, o valor da célula é (Leste - Oeste) + 1. Na outra ordem, é (Leste + 1) - (Oeste + 1), que é outro número.

**Ordem de Cálculo**, na guia **Tabela Dinâmica**, lista os itens calculados na ordem em que são calculados. O item mais abaixo na lista é calculado por último. Escolha um item e clique em **Mover para Cima** ou **Mover para Baixo**. A ordem é salva com o arquivo como a ordem dos itens, e outros programas a leem da mesma forma. **Listar Fórmulas** mostra o lugar de cada item nela.

## Mostrar as linhas por trás de um valor

Dê um clique duplo em um valor de uma tabela dinâmica ou escolha **Mostrar Detalhes** na guia **Tabela Dinâmica**. O JeffCrunch cria uma nova planilha, chamada Detalhe 1, 2 e assim por diante, com as linhas da lista que formam o número. Os nomes dos campos ficam na primeira linha. F2 ainda edita a célula, se ela puder ser editada. **Desfazer** remove as linhas, e outro **Desfazer** remove a planilha.

## Expandir e recolher

Um campo cujos itens têm outros abaixo deles pode mostrar ou ocultar as linhas abaixo dos seus itens. **Expandir Campo Inteiro** e **Recolher Campo Inteiro** fazem isso para todos os itens do campo que você selecionar no painel.

## Opções da Tabela Dinâmica

**Opções**, na guia **Tabela Dinâmica**, abre uma caixa com as escolhas da tabela.

- **Nome** é o nome da tabela dinâmica. Nenhuma outra tabela dinâmica da pasta de trabalho pode ter esse nome.
- **Para valores de erro, mostrar** e **Para células vazias, mostrar** colocam um texto próprio nessas células.
- **Mesclar e centralizar células com rótulos**, **Ajustar largura das colunas ao atualizar a tabela** e **Manter a formatação das células ao atualizar a tabela** são salvos com a tabela.
- **Mostrar totais gerais para linhas** e **para colunas** são o mesmo que as opções de Totais Gerais.
- **Atualizar os dados ao abrir o arquivo** lê as células de novo sempre que a pasta de trabalho é aberta. É uma etapa do Desfazer, como qualquer atualização.

## Estilo: cabeçalhos, faixas e galeria

As **Opções de Estilo**, na guia **Tabela Dinâmica**, ativam ou desativam a linha de cabeçalho, a primeira coluna (cabeçalhos de linha) e as faixas de linhas e de colunas. **Estilos** tem uma galeria de aparências, e **Limpar Estilo** remove a aparência. Essas aparências são desenhadas do mesmo modo que as aparências das tabelas. **Linhas em Branco** e **Repetir Rótulos** definem a linha em branco e os rótulos repetidos para todos os campos das linhas de uma vez.

## Design: subtotais, totais gerais e layout

- **Subtotais** mostra ou oculta o total de cada grupo, no início ou no fim do grupo. Quando há mais de um campo em Linhas, o total de cada campo externo é um subtotal.
- **Totais Gerais** tem quatro opções: desativado para linhas e colunas, ativado para ambos, ativado somente para linhas (uma coluna de totais à direita, um para cada linha) e ativado somente para colunas (uma linha de totais embaixo).
- **Layout do Relatório** é a forma dos rótulos. No formato **compacto**, todos os campos de Linhas compartilham uma coluna, e um novo grupo é mostrado um pouco à direita. No formato de **estrutura de tópicos**, cada campo tem sua própria coluna, e cada grupo tem uma linha própria acima dos seus itens. No formato **tabular**, cada campo tem sua própria coluna, o nome do grupo fica na sua primeira linha e o subtotal fica abaixo do grupo.

## A mesma lista, um filtro

Mova **Mês** de **Colunas** para **Filtros**. O relatório perde suas colunas, e uma linha acima dele mostra **Mês** e **(Todos)**. Os números passam a ser o total de cada região em todos os meses.

## Gráfico dinâmico

Clique em uma célula de uma tabela dinâmica e escolha **Inserir** e depois **Gráfico Dinâmico**. O JeffCrunch cria um gráfico de colunas com os valores da tabela. Ele deixa de fora os totais gerais. O gráfico é marcado no arquivo como o gráfico da tabela dinâmica. Ele lê as células da tabela, por isso seus números mudam quando você atualiza a tabela ou altera o layout dela. Se a tabela ganhar mais linhas ou colunas, crie o gráfico de novo, porque o gráfico mantém as células a partir das quais foi feito. Um gráfico dinâmico que vem de um arquivo é mostrado como foi salvo.

## Segmentações de dados e linhas do tempo

Uma segmentação de dados é um conjunto de botões que filtra uma tabela dinâmica, e uma linha do tempo é um controle deslizante que faz o mesmo para datas.

Clique em uma célula de uma tabela dinâmica e depois clique em **Segmentações de Dados** na guia Tabela Dinâmica. O painel Segmentações se abre e lista as segmentações e as linhas do tempo da pasta de trabalho. Uma segmentação tem um botão para cada item do seu campo. Os botões em azul são os itens que a tabela mostra.

Clique em um botão para mostrar apenas esse item. Clique nele de novo para mostrar todos os itens. Mantenha **Ctrl** pressionado e clique para adicionar um item aos que estão exibidos, ou para retirar um deles. Cada clique é uma etapa do Desfazer, e o Desfazer devolve a tabela ao estado anterior. Ao salvar, o arquivo guarda a escolha da segmentação.

**Conexões de Relatório** indica quais tabelas dinâmicas uma segmentação filtra. Uma linha do tempo mostra o período de datas que ela escolheu. O JeffCrunch mostra as segmentações neste painel, e não sobre as células, e ainda não altera uma linha do tempo. Ambas são mantidas no arquivo como foram salvas.

## Limites

Uma tabela dinâmica pode ter até 2.000 campos, uma lista de até cinco milhões de linhas e um relatório de até quatro milhões de células. Um arquivo que ultrapassa esses limites é mostrado como foi salvo e não pode ser alterado. Ler cem mil linhas de vinte colunas de novo e calcular o relatório levou cerca de meio segundo no computador em que isto foi medido; um computador mais lento leva mais tempo.
