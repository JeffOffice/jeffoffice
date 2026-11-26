# Como as fórmulas são exibidas e calculadas
<!-- keywords: transbordamento, transbordar, #SPILL!, matriz dinâmica, intervalo de transbordamento, A1#, próximo erro, erro anterior, percorrer erros, rastrear erro, calcular antes de salvar, fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, armazenado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar em uma célula, referência, F4, verificação de erros, ignorar erro, avaliar fórmula, passo, janela de inspeção, inspecionar, let, nomear um valor, função cell, função info, filterxml, xml, xpath, precisão como exibida, definir precisão como exibida, arredondar, arredondamento, casas decimais, dígitos -->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Quando você abre uma pasta de trabalho, o arquivo guarda o último valor de cada fórmula, para que a planilha possa ser exibida imediatamente. Em seguida, o JeffCrunch calcula todas as fórmulas novamente, poucas de cada vez, para que a janela continue rápida. A barra de status mostra **Calculando** com uma porcentagem enquanto isso acontece. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para calcular tudo de novo, clique em **Recalcular Tudo** na guia **Fórmulas**, ou pressione **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e a lista continua crescendo. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o arquivo armazenou para ela. O mesmo acontece com uma fórmula que se refere a outra pasta de trabalho. As células que dependem delas são calculadas a partir dos valores armazenados.

## Funções que precisam de algumas observações

- **Títulos e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMINC, e as demais funções de títulos contam os dias conforme a **base** que você informar (0 são 30 dias por mês em um ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDTOPRIMINC, RENDTOULTIMINC e TAXA encontram a resposta testando valores até que o preço ou o pagamento se ajuste. Elas param quando a resposta está correta com cerca de 15 dígitos, de modo que os últimos dígitos podem diferir de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE, e os nomes antigos como DISTNORM e DISTT, dão as mesmas respostas: um nome antigo é mantido para que arquivos antigos continuem funcionando. As funções *.INV também são encontradas testando valores.
- **Listas que preenchem várias células.** CLASSIFICAR, CLASSIFICARPOR, FILTRAR, EXCLUSIVO, SEQUÊNCIA, MATRIZALEATÓRIA, PROCX, MMULT, INVERSAM e ESTIMATIVALIN dão um bloco de valores. Digite uma delas em uma única célula e o bloco transborda para as células abaixo e ao lado (veja **Fórmulas que preenchem as células ao lado**), ou insira-a como fórmula de matriz (**Ctrl+Shift+Enter**) sobre as células que ela deve preencher.
- **Textos em outros alfabetos.** ÔNUMB, ESQUERDAB, EXT.TEXTOB, DIREITAB, LOCALIZARB, PROCURARB e SUBSTITUIRB contam cada caractere como um, como fazem em um computador que não está configurado para texto de dois bytes. FONÉTICA dá o texto da célula, porque o JeffCrunch não guarda as leituras de textos em japonês.
- **CONVERTER.** As unidades são escritas como texto, com letras maiúsculas e minúsculas como aparecem na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode vir antes de uma unidade que o aceite, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** dá o que sobra de **n** depois que retiramos o maior número inteiro de cópias de **d**: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** na guia **Fórmulas**, ou pressione **Ctrl+'** (em um Mac, a tecla Control, e não a tecla Command), para ver a fórmula de cada célula no lugar do valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas apareçam inteiras; elas voltam às larguras originais quando você desativa a opção, e as larguras do seu arquivo não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a célula, pressione **F2**, edite e pressione **Enter**. Você também pode editá-la na barra de fórmulas. Quando você altera uma célula, as fórmulas que dependem dela são recalculadas em segundo plano. Veja [Digitar em células](edit-cells).

## Escrever uma fórmula

Digite **=** e as primeiras letras de uma função, e uma lista curta mostra as funções e [nomes](names) que começam assim, com uma frase sobre cada uma. Pressione **Tab** para escolher a que está marcada. Use as setas **Para Baixo** e **Para Cima** para marcar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando você está dentro dos parênteses de uma função, os argumentos aparecem abaixo da célula, com o que você está digitando em negrito. A lista e os argumentos são os mesmos mostrados em **Inserir Função**. Os argumentos são separados por vírgulas, ou por ponto e vírgula onde o seu idioma usa vírgula como separador decimal.

## Nomes de funções no seu idioma

Quando o JeffCrunch é exibido em alemão, francês, espanhol, italiano, holandês, português ou russo, você pode digitar uma fórmula com os nomes das funções usados no seu idioma, como `=SUMME(A1;A2)` em alemão ou `=SOMME(A1;A2)` em francês. Os nomes em inglês também funcionam, de modo que uma fórmula copiada da web não precisa ser alterada. A barra de fórmulas, a célula, a lista de funções e **Inserir Função** mostram os nomes do seu idioma.

A pasta de trabalho não muda quando você troca de idioma. Um arquivo sempre guarda os nomes em inglês, de modo que uma pasta de trabalho feita em um idioma abre e é lida corretamente em outro, e em qualquer outro programa. Os nomes locais estão disponíveis em alemão, francês, espanhol, italiano, holandês, português (Brasil e Portugal) e russo; os nomes em inglês sempre funcionam em todos os idiomas. Programas em ucraniano e indonésio mantêm os nomes em inglês para quase todas as funções. Quando um idioma tem dois nomes comuns para uma função, ambos são reconhecidos e o mais comum é mostrado. Uma função que não tem um nome usual no seu idioma mantém o nome em inglês. VERDADEIRO e FALSO também seguem o seu idioma. Os valores de erro do seu idioma, como `#NV` em alemão, são reconhecidos quando você digita uma fórmula e aparecem na barra de fórmulas, mas uma célula com erro sempre mostra a forma padrão, como `#N/A`.

Quando as configurações regionais do seu computador usam a vírgula como separador decimal, os argumentos de uma função são separados por ponto e vírgula. Uma matriz entre chaves com mais de uma linha é mostrada com ponto e vírgula entre os valores de uma linha e barra invertida entre as linhas, como `{1;2\3;4}`, e você pode digitá-la desse modo. Uma matriz digitada sem barra invertida é lida da forma como um arquivo a grava: vírgulas entre os valores de uma linha, ponto e vírgula entre as linhas, e ponto como separador decimal.

Para digitar somente nomes em inglês, clique em **Arquivo > Opções > Fórmulas** e marque **Usar nomes de funções em inglês**.

## Apontar para células

Em vez de digitar o endereço de uma célula, você pode apontar para ela. Ao digitar uma fórmula, coloque o cursor onde pode entrar uma célula (logo depois de **=**, de um operador como **+**, de um parêntese de abertura ou de uma vírgula) e clique na célula. O endereço é escrito para você. Arraste sobre as células para escrever um intervalo, como B2:D5. Clique de novo antes de digitar qualquer outra coisa, e a nova célula substitui a que acabou de ser escrita. Mantenha **Ctrl** pressionado e clique para acrescentar outro endereço depois de uma vírgula. Clique em uma letra de coluna para escrever uma coluna inteira, como A:A, ou em um número de linha para escrever uma linha inteira, como 3:3, e arraste sobre as letras ou os números para B:D ou 2:5.

As teclas de seta também funcionam. Pressione uma seta e um ponteiro se afasta da célula em que você está digitando, e o endereço é escrito; pressione mais setas para continuar, ou mantenha **Shift** pressionado para estender o ponteiro sobre um intervalo. A barra de status mostra **Apontar** enquanto as setas apontam, **Enter** enquanto você digita, e **Editar** enquanto as setas movem o cursor no texto. Pressione **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula de outra planilha, clique na guia dessa planilha enquanto o cursor estiver onde pode entrar uma célula, e depois clique na célula. O nome da planilha é escrito junto, entre aspas quando necessário, como em 'Minha Planilha'!B3. **Enter** leva você de volta à planilha em que começou e coloca a fórmula na sua célula. **Esc** cancela tudo, inclusive a troca de planilha.

Você também pode arrastar a borda de um bloco com contorno na planilha para mover esse endereço, ou arrastar seu quadradinho de canto para esticá-lo; a fórmula muda enquanto você arrasta.

Pressione **F4** com o cursor dentro de um endereço, ou logo depois dele, para fixar partes dele com o sinal de cifrão: A1 vira $A$1, depois A$1, depois $A1, e depois A1 de novo.

Cada endereço da fórmula é colorido no texto, e as células que ele menciona recebem um contorno na planilha na mesma cor, para que você veja de relance o que a fórmula lê. Um clique depois de um endereço completo, como =A1, encerra a entrada e seleciona a célula em que você clicou, como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de simulação) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir uma pasta de trabalho que tenha uma, o JeffCrunch calcula as suas células: em uma tabela com duas entradas, a fórmula fica na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada de linha, e os da vertical vão para a célula de entrada de coluna. Em uma tabela de uma entrada, os valores ficam na coluna à esquerda (e as fórmulas na linha acima dela) ou na linha acima (e as fórmulas na coluna à esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para aquele valor; nada mais na pasta de trabalho é alterado. No modo **Automático**, as tabelas são calculadas junto com as fórmulas, e as fórmulas que leem as células de uma tabela são atualizadas em seguida. Criar uma nova tabela de dados ainda não é oferecido. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda sozinha, como ALEATÓRIO ou AGORA) mantém os valores que o arquivo contém.

## Calcular manualmente

Clique em **Modo de Cálculo** na guia **Fórmulas** para escolher como a pasta de trabalho calcula as suas fórmulas:

- **Automático** calcula as fórmulas à medida que você altera as células.
- **Automático, Exceto Tabelas de Dados** faz o mesmo, mas deixa as células das tabelas de dados como estão até que você pressione **F9**.
- **Manual** não calcula nada, inclusive as tabelas de dados, até que você peça. A barra de status mostra **Desatualizado** depois de uma alteração. Pressione **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Planilha**) para calcular as fórmulas da planilha em que você está, e o que mais estava pendente. Uma pasta de trabalho definida como Manual abre com os resultados que o arquivo contém. **Calcular Antes de Salvar**, também no menu **Modo de Cálculo**, vem marcado por padrão: ao salvar uma pasta de trabalho Manual que está desatualizada, as fórmulas são calculadas primeiro, para que o arquivo contenha números atualizados. Desative a opção para salvar os números como estão.

A escolha é salva com a pasta de trabalho, e é uma etapa que pode ser desfeita.

## Fórmulas que leem a si mesmas

Uma fórmula que lê o próprio resultado, diretamente ou por meio de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e as destaca na planilha; elas mantêm os valores que tinham. Clique em **Referências Circulares** na guia **Fórmulas** para ver a lista, e clique duas vezes em uma célula dela para ir até ela.

Alguns modelos precisam de uma circularidade de propósito. Escolha **Calcular Fórmulas Circulares por Iteração** no menu **Modo de Cálculo**, e as fórmulas são calculadas várias vezes, cada uma lendo os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade que você informar, ou até que o número de rodadas permitido seja usado.

## Manter os números como as células os mostram

Uma célula pode mostrar menos dígitos do que ela contém. Se uma célula contém 2,675 e mostra 2,68, as fórmulas que a leem continuam usando 2,675. Às vezes você quer que elas usem o que você vê. Nesse caso, escolha **Definir Precisão como Exibida** no menu **Opções de Cálculo** na guia **Fórmulas**.

O JeffCrunch avisa antes, porque os dígitos ocultos se perdem. Escolha **Cancelar** e nada muda. Escolha **OK** e cada número da pasta de trabalho é arredondado para o que a sua célula mostra. Uma célula que mostra duas casas decimais mantém duas. Uma célula que mostra uma porcentagem mantém a porcentagem. Uma célula com data mantém o dia, e uma célula com hora mantém a menor parte da hora que ela mostra, como o minuto. Uma célula que mostra **Geral** mantém todos os seus dígitos. Em uma pasta de trabalho grande, uma nota com um botão **Cancelar** aparece enquanto isso é feito. Cancele enquanto a nota estiver na tela, e nada é alterado.

Depois disso, cada número que você digitar ou colar, e cada resposta que uma fórmula der, é arredondado da mesma forma. As fórmulas em si não são alteradas. A escolha é salva com a pasta de trabalho.

O arredondamento é uma etapa que pode ser desfeita, e **Desfazer** devolve os números originais. Escolher **Definir Precisão como Exibida** de novo desativa a opção. Isso só impede o arredondamento daí em diante. Os números que já foram arredondados continuam arredondados.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar O Que Alimenta Esta Célula** para desenhar setas a partir das células que a sua fórmula lê. Clique de novo para ir mais um passo para trás. **Mostrar O Que Usa Esta Célula** desenha setas até as fórmulas que leem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra planilha. São desenhadas no máximo 2.000 setas, e a barra de status informa quando havia mais. **Limpar Setas** as remove, e qualquer alteração na pasta de trabalho também.

Quando uma célula mostra um erro, como #DIV/0!, selecione-a e clique em **Rastrear Erro**. São desenhadas setas até as células que ela lê e que também mostram um erro. Clique de novo para ir mais um passo para trás. Quando nenhuma célula que ela lê mostra um erro, a barra de status informa que esta célula causa o erro, e é essa que deve ser corrigida.

Para percorrer todos os erros da planilha, clique em **Próximo Erro** ou **Erro Anterior**. As células que mostram um erro são visitadas uma linha após a outra, e depois da última o percurso recomeça na primeira. Cada célula é selecionada, e as setas são desenhadas até a célula que causa o erro. A barra de status informa qual erro está sendo exibido e quantos há.

## Verificação de Erros

Escolha **Verificação de Erros** no grupo **Verificar** da guia Fórmulas para procurar na planilha células que podem estar erradas. Ela procura uma fórmula que resulta em erro (como #DIV/0!), um número armazenado como texto (que as fórmulas podem ignorar), e mais: uma fórmula diferente das vizinhas, uma data armazenada como texto com um ano de dois dígitos, uma fórmula em uma célula que não está bloqueada, uma fórmula que deixa de fora um número ao lado das células que ela usa (somente para uma função que soma um intervalo, como **SOMA**, **MÉDIA**, **CONT.NÚM**, **MÍNIMO** ou **MÁXIMO**, quando o intervalo aparece sozinho como argumento), e uma fórmula que usa uma célula vazia.

A caixa de diálogo mostra uma célula de cada vez, vai até essa célula na planilha e diz em uma frase qual é o problema. **Próxima** e **Anterior** percorrem as células. Um pequeno triângulo verde no canto superior esquerdo de uma célula a marca na planilha. Clique no próprio triângulo para abrir um menu ao lado da célula: ele diz qual é o problema, oferece a correção quando há uma, e tem **Ignorar Erro** e **Verificação de Erros…**. Quando a célula pode ser corrigida, aparece um botão com a correção (para uma fórmula que deixa de fora um número, **Incluir a Célula**); ele altera a fórmula, e uma etapa de Desfazer traz a fórmula antiga de volta. **Ignorar Erro** remove a célula da lista, retira o seu triângulo e é salvo no arquivo. Uma etapa de Desfazer o traz de volta. Se nada estiver errado, você é avisado. Para escolher quais desses itens são marcados, abra **Opções** e a página **Fórmulas**: há uma caixa de seleção para cada um, e uma caixa desmarcada impede que aquele tipo seja marcado. A escolha vale para o programa, de modo que vale para todas as janelas abertas. Uma célula pode ter mais de um problema, e **Ignorar Erro** ignora apenas o que está sendo exibido.

## Avaliar Fórmula

Use esta opção para ver como uma fórmula chega ao seu valor. Selecione uma célula que contenha uma fórmula e escolha **Avaliar Fórmula**. A fórmula é mostrada com a próxima parte sublinhada. Pressione **Avaliar** para calcular essa parte; o valor dela ocupa o seu lugar. As partes seguem esta ordem: primeiro cada célula isolada, da esquerda para a direita, depois cada função com seus parênteses, começando pela mais interna, e por fim o que restar. **Reiniciar** começa de novo. Se uma parte não puder ser calculada, a caixa de diálogo diz por quê e deixa a fórmula como estava.

## Janela de Inspeção

A **Janela de Inspeção** é uma pequena janela que permanece visível enquanto você trabalha. Ela lista as células que você quer acompanhar, com a planilha, a célula, o valor e a fórmula, mesmo quando estão em outra planilha. Selecione células e pressione **Adicionar Inspeção**. Selecione linhas da lista e pressione **Excluir Inspeção**. Clique duas vezes em uma linha para ir até a célula. Os valores mudam quando a planilha é recalculada. A lista é mantida enquanto a pasta de trabalho estiver aberta.

## Fórmulas que preenchem as células ao lado

Quando uma fórmula dá um bloco de valores, o bloco preenche as células abaixo e ao lado da célula em que você a digitou. Isso se chama transbordamento. Experimente =SEQUÊNCIA(3) em A1: A1 mostra 1, e A2 e A3 mostram 2 e 3. Uma fórmula transborda quando chama uma função que dá um bloco (CLASSIFICAR, CLASSIFICARPOR, FILTRAR, EXCLUSIVO, SEQUÊNCIA, MATRIZALEATÓRIA, TRANSPOR, PROCX ou MMULT), escreve um bloco entre chaves, usa um intervalo de transbordamento (abaixo), ou faz uma operação com um intervalo, como em =A1:A3*2.

- **As células que ele preenche não têm fórmula própria.** Elas mostram os valores. Quando você seleciona uma delas, a barra de fórmulas mostra, em cinza, a fórmula da célula que a preenche. Pressionar Enter nela sem alterações não muda nada. Para alterar o bloco, altere antes a primeira célula.
- **O bloco acompanha o resultado.** Se a fórmula der mais ou menos valores depois que algo que ela lê mudar, o bloco cresce ou diminui, e as fórmulas que leem essas células são recalculadas.
- **Uma célula no caminho gera #SPILL!.** Se alguma célula onde o bloco iria aparecer contiver um valor ou uma fórmula, a primeira célula mostra **#SPILL!** e nada é preenchido. Limpe a célula no caminho e o bloco é preenchido imediatamente. Digitar em uma célula que o bloco preenche produz o mesmo resultado: a primeira célula mostra #SPILL!.
- **Um bloco tem limite de tamanho.** Um bloco maior que a planilha, ou com mais de 200.000 células, gera #SPILL!.
- **A1# é o bloco inteiro.** Escreva a célula que contém a fórmula seguida de **#**, como em =SOMA(A1#), para usar o bloco inteiro como ele está agora, nesta planilha ou em outra (=SOMA(Planilha1!A1#)). Se A1 não transbordar, A1# é #REF!.

Um transbordamento é salvo com o arquivo como matriz dinâmica, de modo que o programa de origem do arquivo também o mostra como transbordamento. Ao salvar no formato .xls ou .ods mais antigo, os valores são mantidos, e a fórmula é gravada como fórmula de matriz sobre o bloco, quando o formato antigo tem as funções que ela usa. A Verificação de Compatibilidade aponta essas fórmulas.

## Dar nomes a valores dentro de uma fórmula

**LET** dá um nome a um valor no meio de uma fórmula, de modo que você o escreve uma vez e o usa de novo. Os pares de nome e valor vêm primeiro, e o cálculo vem por último. Em =LET(preço; B2; imposto; preço*0,08; preço+imposto), o nome **preço** representa o valor de B2, **imposto** é calculado a partir dele, e a última parte soma os dois. Um nome pode usar os nomes que vêm antes dele, e um LET pode ter até 126 pares. Um nome funciona somente dentro do seu próprio LET, e um LET dentro de outro pode usar um nome de novo. Um nome também pode representar um intervalo de células, como em =LET(linhas; B2:B9; SOMA(linhas)/CONT.VALORES(linhas)).

## Perguntar sobre uma célula ou sobre a pasta de trabalho

**CÉL** informa algo sobre uma célula. Seu primeiro argumento diz o quê: "endereço", "linha", "col", "conteúdo", "tipo", "formatar" (como os números aparecem, como "F2" para duas casas decimais, "P0" para porcentagem, ou "D1" para data), "cor" (1 quando números negativos aparecem em cor), "parênteses" (1 quando números positivos aparecem entre parênteses), "prefixo" (a marca de um texto alinhado à esquerda, à direita ou centralizado), "proteger" (1 quando a célula está bloqueada), "largura" (a largura da coluna em caracteres), e "nomearquivo" (a pasta, o arquivo e a planilha, como /home/eu/[Orçamento.xlsx]Plan1; vazio até que a pasta de trabalho seja salva). **INFORMAÇÃO** informa sobre o programa e a pasta de trabalho: "diretório", "numarquivo" (quantas planilhas aparecem), "origem", "versãosO", "recalcular" (Automático ou Manual), "versão" e "sistema".

## Extrair valores de XML

**FILTRARXML** recebe um texto que contém XML e um XPath, que é uma forma curta de dizer quais partes você quer, e devolve os valores que encontra. =FILTRARXML(A1; "//item/preço") dá todos os preços do texto em A1, um embaixo do outro. Use "/loja/item[2]/nome" para o segundo item, "//item[@id='b']/nome" para o item cujo id é b, ou "contar(//item)" para saber quantos há. Se o texto não for XML válido, ou se o caminho não for válido, o resultado é #VALOR!. Se nada corresponder, o resultado é #N/A. XML que tem declaração de tipo de documento é recusado.
