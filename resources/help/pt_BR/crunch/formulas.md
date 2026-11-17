# Como as fórmulas são exibidas e calculadas
<!-- keywords: transbordamento, transbordar, #SPILL!, matriz dinâmica, intervalo de transbordamento, A1#, próximo erro, erro anterior, percorrer erros, rastrear erro, calcular antes de salvar, fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, armazenado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar em uma célula, referência, F4, verificação de erros, ignorar erro, avaliar fórmula, passo a passo, inspeção, janela de inspeção, LET, nomear um valor, função CELL, função INFO, FILTERXML, xml, xpath, precisão como exibida, definir precisão como exibida, arredondar, arredondamento, casas decimais, dígitos -->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Ao abrir uma pasta de trabalho, o arquivo guarda o último valor de cada fórmula, para que a planilha possa ser exibida de imediato. Depois, o JeffCrunch calcula todas as fórmulas novamente, poucas de cada vez, para que a janela continue rápida. A barra de status mostra **Calculando** com uma porcentagem enquanto isso acontece. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para calcular tudo novamente, clique em **Recalcular Tudo** na guia **Fórmulas** ou pressione **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e a lista continua crescendo. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o arquivo guardou para ela. O mesmo acontece com uma fórmula que se refere a outra pasta de trabalho. As células que dependem dessas são calculadas a partir dos valores armazenados.

## Funções que precisam de algumas observações

- **Títulos e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMINC, e as demais funções de títulos contam os dias da forma indicada pela **base** que você informar (0 é 30 dias por mês em um ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDPRIMINC, RENDULTINC e TAXA encontram o resultado testando valores até que o preço ou o pagamento coincidam. Elas param quando a resposta está certa com cerca de 15 dígitos, de modo que os últimos dígitos podem ser diferentes dos de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE, e os nomes mais antigos como DIST.NORM e DISTT, dão as mesmas respostas: um nome antigo é mantido para que arquivos antigos continuem funcionando. As funções *.INV também são encontradas testando valores.
- **Listas que preenchem várias células.** CLASSIFICAR, CLASSIFICARPOR, FILTRAR, EXCLUSIVO, SEQUÊNCIA, MATRIZALEATÓRIA, PROCX, MMULT, MATRIZINVERSA e PROJ.LIN dão um bloco de valores. Digite uma delas em uma única célula e o bloco se estende pelas células abaixo e ao lado (veja **Fórmulas que preenchem as células ao lado**), ou insira-a como fórmula de matriz (**Ctrl+Shift+Enter**) sobre as células que ela deve preencher.
- **Textos em outros alfabetos.** ÉTEXTOB, ESQUERDAB, EXTRAIRB, DIREITAB, LOCALIZARB, PROCURARB e SUBSTITUIRB contam cada caractere como um, como ocorre em um computador que não está configurado para texto de dois bytes. FONÉTICA devolve o texto da célula, pois o JeffCrunch não guarda as leituras de textos em japonês.
- **CONVERTER.** As unidades são escritas como texto, com letras maiúsculas e minúsculas como aparecem na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode vir antes de uma unidade que aceita prefixo, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** dá o que sobra de **n** depois de retirar o maior número inteiro de vezes de **d**: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** na guia **Fórmulas**, ou pressione **Ctrl+'** (em um Mac, a tecla Control, e não a tecla Command), para ver a fórmula de cada célula no lugar do valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas apareçam inteiras; elas voltam às larguras originais quando você desativa a opção, e as larguras do seu arquivo não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a célula, pressione **F2**, edite e pressione **Enter**. Você também pode editá-la na barra de fórmulas. Quando você altera uma célula, as fórmulas que dependem dela são calculadas novamente em segundo plano. Veja [Digitar em células](edit-cells).

## Escrever uma fórmula

Digite **=** e as primeiras letras de uma função, e uma lista curta mostra as funções e [nomes](names) que começam assim, com uma frase sobre cada um. Pressione **Tab** para escolher o que está marcado. Use as setas **Para Baixo** e **Para Cima** para marcar outro, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando você está dentro dos parênteses de uma função, os argumentos dela aparecem abaixo da célula, com o que você está digitando em negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por ponto e vírgula, ou por vírgula onde o seu idioma usa vírgula como separador decimal.

## Apontar para células

Em vez de digitar o endereço de uma célula, você pode apontar para ela. Enquanto digita uma fórmula, coloque o cursor onde pode entrar uma célula (logo depois de **=**, de um operador como **+**, de um parêntese de abertura ou de uma vírgula) e clique na célula. O endereço é escrito para você. Arraste sobre as células para escrever um intervalo como B2:D5. Clique de novo antes de digitar outra coisa, e a nova célula substitui a que acabou de ser escrita. Mantenha **Ctrl** pressionado e clique para acrescentar outro endereço depois de uma vírgula. Clique em uma letra de coluna para escrever uma coluna inteira, como A:A, ou em um número de linha para escrever uma linha inteira, como 3:3, e arraste pelas letras ou números para B:D ou 2:5.

As teclas de seta também funcionam. Pressione uma seta e um ponteiro se afasta da célula em que você está digitando, e o endereço é escrito; pressione mais setas para continuar, ou mantenha **Shift** pressionado para estender o ponteiro sobre um intervalo. A barra de status mostra **Apontar** enquanto as setas apontam, **Inserir** enquanto você digita, e **Editar** quando as setas movem o cursor no texto. Pressione **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula de outra planilha, clique na guia dessa planilha enquanto o cursor estiver onde pode entrar uma célula, e depois clique na célula. O nome da planilha é escrito junto, entre aspas quando necessário, como em 'Minha Planilha'!B3. **Enter** volta para a planilha em que você começou e coloca a fórmula na sua célula. **Esc** cancela tudo, inclusive a troca de planilha.

Você também pode arrastar a borda de um bloco com contorno na planilha para mover esse endereço, ou arrastar seu quadradinho de canto para esticá-lo; a fórmula muda enquanto você arrasta.

Pressione **F4** com o cursor dentro ou logo depois de um endereço para fixar partes dele com um cifrão: A1 vira $A$1, depois A$1, depois $A1, e depois A1 de novo.

Cada endereço da fórmula é colorido no texto, e as células que ele menciona são destacadas na planilha com a mesma cor, para que você veja de relance o que a fórmula lê. Um clique depois de um endereço completo, como =A1, encerra a entrada e seleciona a célula em que você clicou, como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de simulação) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir uma pasta de trabalho que tenha uma, o JeffCrunch calcula as suas células: para uma tabela com duas entradas, a fórmula fica na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada de linha, e os da vertical vão para a célula de entrada de coluna. Para uma tabela de uma entrada, os valores ficam na coluna à esquerda (e as fórmulas estão na linha acima dela) ou na linha acima (e as fórmulas estão na coluna à esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para aquele valor; nada mais na pasta de trabalho é alterado. No modo **Automático**, as tabelas são calculadas junto com as fórmulas, e as fórmulas que leem as células de uma tabela são atualizadas em seguida. Criar uma nova tabela de dados ainda não é possível. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda sozinha, como ALEATÓRIO ou AGORA) mantém os valores que o arquivo guarda.

## Calcular manualmente

Clique em **Modo de Cálculo** na guia **Fórmulas** para escolher como a pasta de trabalho calcula as suas fórmulas:

- **Automático** calcula as fórmulas à medida que você altera as células.
- **Automático, Tabelas Manuais** faz o mesmo, mas deixa as células das tabelas de dados como estão até você pressionar **F9**.
- **Manual** não calcula nada, inclusive as tabelas de dados, até que você peça. A barra de status mostra **Desatualizado** depois de uma alteração. Pressione **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Planilha**) para calcular as fórmulas da planilha em que você está, e o que mais estava pendente. Uma pasta de trabalho definida como Manual abre com os resultados que o arquivo guarda. **Calcular Antes de Salvar**, também no menu **Modo de Cálculo**, vem marcado por padrão: ao salvar uma pasta de trabalho Manual que está desatualizada, as fórmulas são calculadas antes, para que o arquivo guarde números atualizados. Desative a opção para salvar os números como estão.

A escolha é guardada com a pasta de trabalho, e é uma etapa que pode ser desfeita.

## Fórmulas que leem a si mesmas

Uma fórmula que lê o próprio resultado, diretamente ou por meio de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e as destaca na planilha; elas mantêm os valores que tinham. Clique em **Referências Circulares** na guia **Fórmulas** para ver a lista, e clique duas vezes em uma célula dela para ir até lá.

Alguns modelos precisam desse ciclo de propósito. Escolha **Calcular Fórmulas Circulares por Iteração** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidas vezes, cada uma lendo os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade que você informar ou até que se esgote o número de rodadas permitido.

## Manter os números como as células os mostram

Uma célula pode mostrar menos dígitos do que contém. Se uma célula tem 2,675 e mostra 2,68, as fórmulas que a leem continuam usando 2,675. Às vezes você quer que elas usem o que você vê. Nesse caso, escolha **Definir Precisão como Exibida** no menu **Opções de Cálculo**, na guia **Fórmulas**.

O JeffCrunch avisa antes, porque os dígitos ocultos se perdem. Escolha **Cancelar** e nada muda. Escolha **OK** e cada número da pasta de trabalho é arredondado para o que a sua célula mostra. Uma célula que mostra duas casas decimais mantém duas. Uma célula que mostra uma porcentagem mantém a porcentagem. Uma célula com data mantém o dia, e uma célula com hora mantém a menor parte da hora que ela mostra, como o minuto. Uma célula que mostra **Geral** mantém todos os seus dígitos. Em uma pasta de trabalho grande, aparece um aviso com um botão **Cancelar** enquanto isso é feito. Cancele enquanto o aviso estiver visível, e nada é alterado.

Depois disso, todo número que você digitar ou colar, e toda resposta que uma fórmula der, é arredondado da mesma forma. As fórmulas em si não são alteradas. A escolha é guardada com a pasta de trabalho.

O arredondamento é uma etapa que pode ser desfeita, e **Desfazer** devolve os números originais. Escolher **Definir Precisão como Exibida** de novo desativa a opção. Isso só impede o arredondamento daí em diante. Os números que já foram arredondados continuam arredondados.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a fórmula lê. Clique de novo para ir mais um passo para trás. **Mostrar o Que Usa Esta Célula** desenha setas até as fórmulas que leem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra planilha. São desenhadas no máximo 2.000 setas, e a barra de status avisa quando havia mais. **Limpar Setas** as remove, e qualquer alteração na pasta de trabalho também.

Quando uma célula mostra um erro, como #DIV/0!, selecione-a e clique em **Rastrear Erro**. São desenhadas setas até as células que ela lê e que também mostram um erro. Clique de novo para ir mais um passo para trás. Quando nenhuma célula que ela lê mostra um erro, a barra de status informa que esta célula causa o erro, e essa é a que deve ser corrigida.

Para percorrer todos os erros da planilha, clique em **Próximo Erro** ou **Erro Anterior**. As células que mostram um erro são visitadas uma linha após a outra, e depois da última ele recomeça na primeira. Cada célula é selecionada, e as setas são desenhadas até a célula que causa o erro. A barra de status informa qual erro é o atual e quantos existem.

## Verificação de erros

Escolha **Verificação de Erros** no grupo **Verificar** da guia Fórmulas para procurar na planilha células que podem estar erradas. Ela procura duas coisas: uma fórmula que resulta em erro (como #DIV/0!), e um número guardado como texto, que as fórmulas podem ignorar.

A caixa de diálogo mostra uma célula de cada vez, vai até essa célula na planilha e explica em uma frase o que há de errado. **Próximo** e **Anterior** percorrem as células. Um pequeno triângulo verde no canto superior esquerdo de uma célula a marca na planilha. **Ignorar Erro** retira a célula da lista, tira o triângulo dela e fica salvo no arquivo. Uma etapa de Desfazer a traz de volta. Se não houver nenhum problema, você é avisado.

## Avaliar Fórmula

Use esta opção para ver como uma fórmula chega ao seu valor. Selecione uma célula que contenha uma fórmula e escolha **Avaliar Fórmula**. A fórmula aparece com a próxima parte sublinhada. Pressione **Avaliar** para calcular essa parte; o valor dela ocupa o seu lugar. As partes são avaliadas nesta ordem: cada célula isolada, da esquerda para a direita, depois cada função com seus parênteses, começando pela mais interna, e por fim o que restar. **Reiniciar** começa de novo. Se uma parte não puder ser calculada, a caixa de diálogo diz o motivo e deixa a fórmula como estava.

## Janela de Inspeção

A **Janela de Inspeção** é uma janela pequena que fica visível enquanto você trabalha. Ela lista as células que você quer acompanhar, com a planilha, a célula, o valor e a fórmula, mesmo quando estão em outra planilha. Selecione células e pressione **Adicionar Inspeção**. Selecione linhas na lista e pressione **Excluir Inspeção**. Clique duas vezes em uma linha para ir até a célula. Os valores mudam quando a planilha é calculada novamente. A lista é mantida enquanto a pasta de trabalho estiver aberta.

## Fórmulas que preenchem as células ao lado

Quando uma fórmula dá um bloco de valores, o bloco preenche as células abaixo e ao lado da célula em que você a digitou. Isso se chama transbordamento. Experimente =SEQUÊNCIA(3) em A1: A1 mostra 1, e A2 e A3 mostram 2 e 3. Uma fórmula transborda quando chama uma função que dá um bloco (CLASSIFICAR, CLASSIFICARPOR, FILTRAR, EXCLUSIVO, SEQUÊNCIA, MATRIZALEATÓRIA, TRANSPOR, PROCX ou MMULT), escreve um bloco entre chaves, usa um intervalo de transbordamento (abaixo), ou faz uma operação aritmética com um intervalo, como em =A1:A3*2.

- **As células que ele preenche não têm fórmula própria.** Elas mostram os valores. Quando você seleciona uma delas, a barra de fórmulas mostra, em cinza, a fórmula da célula que a preenche. Pressionar Enter sobre ela, como está, não muda nada. Para alterar o bloco, altere primeiro essa célula inicial.
- **O bloco acompanha o resultado.** Se a fórmula der mais ou menos valores depois que algo que ela lê muda, o bloco cresce ou diminui, e as fórmulas que leem essas células são calculadas novamente.
- **Uma célula no caminho gera #SPILL!.** Se qualquer célula por onde o bloco passaria contiver um valor ou uma fórmula, a primeira célula mostra **#SPILL!** e nada é preenchido. Limpe a célula no caminho e o bloco é preenchido imediatamente. Digitar em uma célula que o bloco preencheria produz o mesmo efeito: a primeira célula mostra #SPILL!.
- **Um bloco tem limite de tamanho.** Um bloco maior que a planilha, ou com mais de 200.000 células, gera #SPILL!.
- **A1# é o bloco inteiro.** Escreva a célula que contém a fórmula seguida de **#**, como em =SOMA(A1#), para usar o bloco inteiro como ele está agora, nesta planilha ou em outra (=SOMA(Planilha1!A1#)). Se A1 não transbordar, A1# dá #REF!.

Um transbordamento é salvo no arquivo como matriz dinâmica, de modo que o programa de origem do arquivo também o mostra como transbordamento. Salvar nos formatos mais antigos .xls ou .ods mantém os valores, e a fórmula como fórmula de matriz sobre o bloco, quando o formato antigo tem as funções que ela usa. Verificar Compatibilidade indica essas fórmulas.

## Dar nomes a valores dentro de uma fórmula

**LET** dá um nome a um valor no meio de uma fórmula, para que você o escreva uma vez e o use de novo. Os pares de nome e valor vêm primeiro, e o cálculo vem por último. Em =LET(preço; B2; imposto; preço*0,08; preço+imposto) o nome **preço** representa o valor de B2, **imposto** é calculado a partir dele, e a última parte soma os dois. Um nome pode usar os nomes que vêm antes dele, e um LET pode ter até 126 pares. Um nome funciona somente dentro do seu próprio LET, e um LET dentro de outro pode usar o mesmo nome de novo. Um nome também pode representar um intervalo de células, como em =LET(linhas; B2:B9; SOMA(linhas)/CONT.VALORES(linhas)).

## Perguntar sobre uma célula ou sobre a pasta de trabalho

**CÉLULA** informa algo sobre uma célula. Seu primeiro argumento diz o quê: "endereço", "linha", "col", "conteúdo", "tipo", "formato" (como os números são exibidos, como "F2" para duas casas decimais, "P0" para uma porcentagem, ou "D1" para uma data), "cor" (1 quando os números negativos aparecem em cor), "parênteses" (1 quando os números positivos aparecem entre parênteses), "prefixo" (a marca de um texto alinhado à esquerda, à direita ou ao centro), "proteger" (1 quando a célula está bloqueada), "largura" (a largura da coluna em caracteres) e "nomedearquivo" (a pasta, o arquivo e a planilha, como /home/eu/[Orçamento.xlsx]Planilha1; vazio até que a pasta de trabalho seja salva). **INFO** informa sobre o programa e a pasta de trabalho: "diretório", "numarquivo" (quantas planilhas aparecem), "origem", "versãodoso", "recalc" (Automático ou Manual), "versão" e "sistema".

## Extrair valores de XML

**FILTERXML** recebe um texto que contém XML e um XPath, que é uma forma curta de dizer quais partes você quer, e devolve os valores que encontrar. =FILTERXML(A1; "//item/preço") dá todos os preços do texto em A1, um embaixo do outro. Use "/loja/item[2]/nome" para o segundo item, "//item[@id='b']/nome" para o item cujo id é b, ou "count(//item)" para saber quantos existem. Se o texto não for XML válido, ou se o caminho não for válido, o resultado é #VALOR!. Se nada corresponder, o resultado é #N/D. XML que tenha declaração de tipo de documento é recusado.
