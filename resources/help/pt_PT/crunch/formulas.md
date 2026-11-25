# Como as fórmulas são apresentadas e calculadas
<!-- keywords: spill, spilled, #SPILL!, matriz dinâmica, intervalo de resultado, A1#, próximo erro, erro anterior, percorrer erros, rastrear erro, calcular antes de guardar, fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, guardado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar numa célula, referência, F4, verificação de erros, ignorar erro, avaliar fórmula, passo, janela de vigilância, vigiar, LET, nomear um valor, função de célula, função de informação, filterxml, xml, xpath, precisão tal como apresentada, definir precisão tal como apresentada, arredondar, arredondamento, casas decimais, algarismos-->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com as fórmulas

Quando abre um livro de cálculo, o ficheiro contém o último valor de cada fórmula, para que a folha possa ser mostrada de imediato. Depois, o JeffCrunch calcula novamente todas as fórmulas, algumas de cada vez, para que a janela continue rápida. A barra de estado indica **A calcular** com uma percentagem enquanto o faz. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para voltar a calcular tudo, clique em **Recalcular Tudo** no separador **Fórmulas**, ou prima **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e continua a crescer. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o ficheiro guardou para ela. O mesmo acontece com uma fórmula que se refere a outro livro de cálculo. As células que dependem dessas são calculadas a partir dos valores guardados.

## Funções que precisam de algumas notas

- **Obrigações e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMINC, e as restantes funções de obrigações contam os dias da forma indicada pela **base** que indicar (0 são 30 dias por mês num ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDMAT, RENDPRIMIN e TAXA encontram a resposta testando valores até que o preço ou o pagamento coincidam. Param quando a resposta está correta até cerca de 15 algarismos, pelo que os últimos algarismos podem diferir dos de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE, e os nomes mais antigos como DIST.NORM e DISTT, dão as mesmas respostas: um nome antigo é mantido para que os ficheiros antigos continuem a funcionar. As funções *.INV também são calculadas testando valores.
- **Listas que preenchem várias células.** ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, PROCX, MMULT, MINVERSA e PROJLIN devolvem um bloco de valores. Escreva uma delas numa única célula e o bloco transborda para as células abaixo e ao lado (veja **Fórmulas que preenchem as células ao lado**), ou introduza-a como fórmula matricial (**Ctrl+Shift+Enter**) sobre as células que deve preencher.
- **Textos noutros sistemas de escrita.** COMPRIMENTOB, ESQUERDAB, SEXT.B, DIREITAB, LOCALIZARB, PESQUISARB e SUBSTITUIRB contam cada carácter como um, tal como num computador que não está configurado para texto de byte duplo. FONÉTICA devolve o texto da célula, porque o JeffCrunch não guarda as leituras do texto japonês.
- **CONVERTER.** As unidades são escritas como texto, com maiúsculas e minúsculas tal como indicado na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode ser colocado antes de uma unidade que o aceite, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** dá o que sobra de **n** depois de retirar o maior número inteiro de cópias de **d**: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** no separador **Fórmulas**, ou prima **Ctrl+`** (em Mac, a tecla Control, e não a tecla Command), para ver a fórmula de cada célula em vez do seu valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas apareçam completas; voltam às larguras anteriores quando desativa a opção, e as larguras do seu ficheiro não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a sua célula, prima **F2**, edite-a e prima **Enter**. Também pode editá-la na barra de fórmulas. Quando altera uma célula, as fórmulas que dependem dela são calculadas novamente em segundo plano. Veja [Escrever em células](edit-cells).

## Escrever uma fórmula

Escreva **=** e as primeiras letras de uma função, e surge uma lista curta com as funções e [nomes](names) que começam por essas letras, com uma frase sobre cada uma. Prima **Tab** para escolher a que está marcada. Use as setas **Para Baixo** e **Para Cima** para marcar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando está dentro dos parênteses de uma função, os seus argumentos aparecem sob a célula, com o que está a escrever a negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas, ou por pontos e vírgulas quando a sua língua usa a vírgula como separador decimal.

## Nomes de funções na sua língua

Quando o JeffCrunch está apresentado em alemão, francês, espanhol, italiano, neerlandês, português ou russo, pode escrever uma fórmula com os nomes de funções usados na sua língua, como `=SUMME(A1;A2)` em alemão ou `=SOMME(A1;A2)` em francês. Os nomes em inglês também funcionam, pelo que uma fórmula copiada da Web não precisa de ser alterada. A barra de fórmulas, a célula, a lista de funções e **Escolher uma Função** mostram os nomes da sua língua.

O livro de cálculo não muda quando troca de língua. Um ficheiro guarda sempre os nomes em inglês, pelo que um livro criado numa língua abre e é lido corretamente noutra língua, e noutro programa qualquer. Os nomes locais estão disponíveis em alemão, francês, espanhol, italiano, neerlandês, português (Brasil e Portugal) e russo; os nomes em inglês funcionam sempre, em qualquer língua. Os programas em ucraniano e indonésio mantêm os nomes em inglês para quase todas as funções. Quando uma língua tem dois nomes comuns para uma função, ambos são reconhecidos e é mostrado o mais comum. Uma função que não tem nome habitual na sua língua mantém o nome em inglês. VERDADEIRO e FALSO também seguem a sua língua. Os valores de erro da sua língua, como `#NV` em alemão, são reconhecidos quando escreve uma fórmula e são mostrados na barra de fórmulas, mas uma célula com erro mostra sempre a forma padrão, como `#N/A`.

Quando as definições regionais do seu computador usam a vírgula como separador decimal, os argumentos de uma função são distinguidos por um ponto e vírgula. Uma matriz entre chavetas com mais de uma linha é mostrada com um ponto e vírgula entre os valores de uma linha e uma barra invertida entre linhas, como `{1;2\3;4}`, e pode escrevê-la dessa forma. Uma matriz escrita sem barra invertida é lida da forma como um ficheiro a escreve: vírgulas entre os valores de uma linha, pontos e vírgulas entre linhas, e um ponto como separador decimal.

Para escrever apenas nomes de funções em inglês, clique em **Ficheiro > Opções > Fórmulas** e marque **Utilizar nomes de funções em inglês**.

## Apontar para células

Em vez de escrever o endereço de uma célula, pode apontar para ela. Enquanto escreve uma fórmula, coloque o cursor onde pode entrar uma célula (logo a seguir a **=**, a um operador como **+**, a um parêntese de abertura ou a uma vírgula) e clique na célula. O seu endereço é escrito por si. Arraste sobre células para escrever um bloco, como B2:D5. Clique de novo antes de escrever outra coisa, e a nova célula substitui a que acabou de ser escrita. Mantenha **Ctrl** premido e clique para acrescentar outro endereço depois de uma vírgula. Clique numa letra de coluna para escrever uma coluna inteira, como A:A, ou num número de linha para escrever uma linha inteira, como 3:3, e arraste ao longo das letras ou números para B:D ou 2:5.

As teclas de seta também funcionam. Prima uma seta e um apontador afasta-se da célula que está a escrever, e o seu endereço é escrito; prima mais setas para avançar, ou mantenha **Shift** premido para estender o apontador sobre um bloco. A barra de estado indica **Apontar** enquanto as setas apontam, **Introduzir** enquanto escreve, e **Editar** enquanto as setas movem o cursor no texto. Prima **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula noutra folha, clique no separador dessa folha enquanto o cursor estiver onde pode entrar uma célula, e depois clique na célula. O nome da folha é escrito com ela, entre aspas quando são necessárias, como em 'A Minha Folha'!B3. **Enter** leva-o de volta à folha onde começou e coloca a fórmula na sua célula. **Esc** cancela tudo, incluindo a mudança de folha.

Também pode arrastar a margem de um bloco com contorno na folha para mover esse endereço, ou arrastar o seu quadrado de canto para o alargar; a fórmula altera-se à medida que arrasta.

Prima **F4** com o cursor dentro de um endereço ou logo a seguir a ele para fixar partes dele com um sinal de dólar: A1 passa a $A$1, depois a A$1, depois a $A1, e depois novamente a A1.

Cada endereço da fórmula é colorido no texto, e as células que menciona são delimitadas na folha com a mesma cor, para que veja de relance o que a fórmula lê. Um clique a seguir a um endereço completo, como =A1, termina a introdução e seleciona a célula em que clicou, como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de hipóteses) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir um livro que tenha uma, o JeffCrunch calcula as suas células: numa tabela com duas entradas, a fórmula está na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada da linha, e os da vertical vão para a célula de entrada da coluna. Numa tabela de uma entrada, os valores ficam na coluna à sua esquerda (e as fórmulas na linha acima) ou na linha acima (e as fórmulas na coluna à sua esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para esse valor; nada mais no livro é alterado. No modo **Automático**, as tabelas são calculadas juntamente com as fórmulas, e as fórmulas que leem as células de uma tabela acompanham-nas. Criar uma nova tabela de dados ainda não está disponível. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou se altera sozinha, como ALEATÓRIO ou AGORA) mantém os valores que o seu ficheiro contém.

## Calcular manualmente

Clique em **Modo de Cálculo** no separador **Fórmulas** para escolher a forma como o livro calcula as suas fórmulas:

- **Automático** calcula-as à medida que altera células.
- **Automático, Tabelas Manualmente** faz o mesmo, mas deixa as células das tabelas de dados como estão até premir **F9**.
- **Manual** não calcula nada, incluindo as tabelas de dados, até que o peça. A barra de estado mostra **Desatualizado** depois de uma alteração. Prima **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Folha**) para calcular as fórmulas da folha em que está, e tudo o que mais estava pendente. Um livro definido como Manual abre com os resultados que o seu ficheiro contém. **Calcular Antes de Guardar**, também no menu **Modo de Cálculo**, está marcado por predefinição: ao guardar um livro Manual que esteja desatualizado, as fórmulas são calculadas primeiro, para que o ficheiro contenha números atualizados. Desative-a para guardar os números tal como estão.

A escolha é guardada com o livro, e é um passo que pode anular.

## Fórmulas que leem o próprio resultado

Uma fórmula que lê o seu próprio resultado, diretamente ou através de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e delimita-as na folha; elas mantêm os valores que tinham. Clique em **Referências Circulares** no separador **Fórmulas** para ver a lista, e faça duplo clique numa célula da lista para ir até ela.

Alguns modelos precisam de um circuito destes de propósito. Escolha **Iterar Fórmulas Circulares** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, cada uma lendo os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade que indicar ou se esgote o número de iterações permitido.

## Manter os números tal como as células os mostram

Uma célula pode mostrar menos algarismos do que os que contém. Se uma célula contém 2,675 e mostra 2,68, as fórmulas que a leem continuam a usar 2,675. Por vezes, pretende que usem o que vê. Nesse caso, escolha **Definir Precisão Tal Como Apresentada** no menu **Opções de Cálculo** no separador **Fórmulas**.

O JeffCrunch avisa primeiro, porque os algarismos ocultos se perdem. Escolha **Cancelar** e nada muda. Escolha **OK** e cada número do livro é arredondado para o que a sua célula mostra. Uma célula que mostra duas casas decimais mantém duas. Uma célula que mostra uma percentagem mantém a percentagem. Uma célula com uma data mantém o dia, e uma célula com uma hora mantém a menor unidade de tempo que mostra, como o minuto. Uma célula que mostra **Geral** mantém todos os seus algarismos. Num livro grande, é apresentada uma nota com um botão **Cancelar** enquanto isto é feito. Se cancelar enquanto a nota estiver visível, nada é alterado.

Depois disso, cada número que escrever ou colar, e cada resposta que uma fórmula dê, é arredondado da mesma forma. As próprias fórmulas não são alteradas. A escolha é guardada com o livro.

O arredondamento é um passo que pode anular, e **Anular** repõe os números. Escolher **Definir Precisão Tal Como Apresentada** novamente desativa-o. Isso só impede o arredondamento a partir desse momento. Os números que já tinham sido arredondados continuam arredondados.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a sua fórmula lê. Clique novamente para recuar mais um passo. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que leem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra folha. São desenhadas, no máximo, 2000 setas, e a barra de estado indica quando havia mais. **Limpar Setas** remove-as, e qualquer alteração ao livro também as remove.

Quando uma célula mostra um erro, como #DIV/0!, selecione-a e clique em **Rastrear Erro**. São desenhadas setas para as células que ela lê e que também mostram um erro. Clique novamente para recuar mais um passo. Quando nenhuma das células que lê mostra um erro, a barra de estado indica que esta célula provoca o erro, e é esta que deve corrigir.

Para percorrer todos os erros da folha, clique em **Próximo Erro** ou **Erro Anterior**. As células que mostram um erro são visitadas uma linha de cada vez, e depois da última recomeça-se na primeira. Cada célula é selecionada, e as setas são desenhadas até à célula que provoca o erro. A barra de estado indica em que erro está e quantos existem.

## Verificação de Erros

Escolha **Verificação de Erros** no grupo **Verificar** do separador Fórmulas para procurar na folha células que possam estar erradas. Procura uma fórmula que resulte num erro (como #DIV/0!), um número guardado como texto (que as fórmulas podem ignorar), e mais: uma fórmula que não é igual às que estão ao lado, uma data guardada como texto com um ano de dois algarismos, uma fórmula numa célula que não está bloqueada, uma fórmula que deixa de fora um número junto às células que usa (apenas para uma função que soma ou totaliza um intervalo, como **SOMA**, **MÉDIA**, **CONTAR**, **MÍN** ou **MÁX**, quando o intervalo surge sozinho como argumento), e uma fórmula que usa uma célula vazia.

A caixa de diálogo mostra uma célula de cada vez, leva-o a essa célula na folha e explica numa frase o que está errado. **Seguinte** e **Anterior** movem-se entre as células. Um pequeno triângulo verde no canto superior esquerdo de uma célula assinala-a na folha. Quando a célula pode ser corrigida, aparece um botão com a correção (para uma fórmula que deixa de fora um número, **Incluir a Célula**); ele altera a fórmula, e um passo de Anular repõe a fórmula antiga. **Ignorar Erro** retira a célula da lista, remove-lhe o triângulo, e fica guardado no ficheiro. Um passo de Anular repõe-no. Se não houver nenhum problema, é informado disso. Para escolher quais destes são assinalados, abra **Opções** e a página **Fórmulas**: há uma caixa de verificação para cada um, e uma caixa que desmarque impede que esse tipo seja assinalado. A escolha é do programa, pelo que vale para todas as janelas abertas. Uma célula pode ter mais de um problema, e **Ignorar Erro** ignora apenas o que está a ser mostrado.

## Avaliar Fórmula

Use esta opção para ver como uma fórmula chega ao seu valor. Selecione uma célula que contenha uma fórmula e escolha **Avaliar Fórmula**. A fórmula é mostrada com a parte seguinte sublinhada. Prima **Avaliar** para calcular essa parte; o seu valor ocupa o seu lugar. As partes seguem esta ordem: cada célula individual da esquerda, depois cada função com os seus parênteses, começando pela mais interior, e depois o que resta. **Reiniciar** começa de novo. Se uma parte não puder ser calculada, a caixa de diálogo indica porquê e deixa a fórmula como estava.

## Janela de Vigilância

A **Janela de Vigilância** é uma pequena janela que se mantém visível enquanto trabalha. Lista as células que quer acompanhar, com a sua folha, célula, valor e fórmula, mesmo quando estão noutra folha. Selecione células e prima **Adicionar Vigilância**. Selecione linhas da lista e prima **Eliminar Vigilância**. Faça duplo clique numa linha para ir até à célula. Os valores mudam quando a folha é calculada novamente. A lista mantém-se enquanto o livro estiver aberto.

## Fórmulas que preenchem as células ao lado

Quando uma fórmula dá um bloco de valores, o bloco preenche as células abaixo e ao lado da célula em que a escreveu. Isto chama-se transbordo. Experimente =SEQUÊNCIA(3) em A1: A1 mostra 1, e A2 e A3 mostram 2 e 3. Uma fórmula transborda quando chama uma função que dá um bloco (ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, TRANSPOR, PROCX ou MMULT), escreve um bloco entre chavetas, usa um intervalo de resultado (abaixo), ou faz uma operação aritmética sobre um intervalo, como em =A1:A3*2.

- **As células que preenche não têm fórmula própria.** Mostram os valores. Quando seleciona uma delas, a barra de fórmulas mostra, a cinzento, a fórmula da célula que a preenche. Premir Enter sobre ela tal como está não altera nada. Para alterar o bloco, altere primeiro essa célula inicial.
- **O bloco acompanha o resultado.** Se a fórmula der mais ou menos valores depois de algo que lê mudar, o bloco cresce ou diminui, e as fórmulas que leem essas células são calculadas novamente.
- **Uma célula no caminho dá #SPILL!.** Se alguma célula onde o bloco iria ficar contiver um valor ou uma fórmula, a primeira célula mostra **#SPILL!** e nada é preenchido. Limpe a célula no caminho e o bloco preenche de imediato. Escrever numa célula que o bloco preencheria faz o mesmo: a primeira célula mostra #SPILL!.
- **Um bloco tem um limite de tamanho.** Um bloco maior do que a folha, ou com mais de 200 000 células, dá #SPILL!.
- **A1# é o bloco inteiro.** Escreva a célula que contém a fórmula seguida de **#**, como em =SOMA(A1#), para usar todo o bloco tal como está agora, nesta folha ou noutra (=SOMA(Folha1!A1#)). Se A1 não transbordar, A1# dá #REF!.

Um transbordo é guardado com o ficheiro como matriz dinâmica, pelo que o programa de origem do ficheiro também o mostra como transbordo. Ao guardar na forma mais antiga .xls ou .ods, os valores são mantidos, e a fórmula surge como fórmula matricial sobre o bloco, quando a forma antiga tem as funções que ela usa. A Verificação de Compatibilidade indica estas fórmulas.

## Dar nomes a valores dentro de uma fórmula

**LET** dá um nome a um valor no meio de uma fórmula, para que o escreva uma vez e o use de novo. Os pares de nome e valor vêm primeiro, e o cálculo no fim. Em =LET(preço; B2; imposto; preço*0,08; preço+imposto) o nome **preço** representa o valor de B2, **imposto** é calculado a partir dele, e a última parte soma-os. Um nome pode usar os nomes anteriores, e um LET pode ter até 126 pares. Um nome só funciona dentro do seu próprio LET, e um LET dentro de outro pode usar de novo um nome. Um nome pode também representar um bloco de células, como em =LET(linhas; B2:B9; SOMA(linhas)/CONTAR(linhas)).

## Perguntar sobre uma célula ou o livro

**CÉLULA** indica algo sobre uma célula. O seu primeiro argumento diz o quê: "endereço", "linha", "col", "conteúdo", "tipo", "formato" (como os números são mostrados, como "F2" para duas casas decimais, "P0" para uma percentagem, ou "D1" para uma data), "cor" (1 quando os números negativos são mostrados a cor), "parênteses" (1 quando os números positivos são mostrados entre parênteses), "prefixo" (a marca de um texto alinhado à esquerda, à direita ou ao centro), "proteger" (1 quando a célula está bloqueada), "largura" (a largura da coluna em carateres) e "nomeficheiro" (a pasta, o ficheiro e a folha, como /home/eu/[Orçamento.xlsx]Folha1; vazio até o livro ser guardado). **INFORMAÇÃO** indica dados sobre o programa e o livro: "diretório", "númficheiro" (quantas folhas são mostradas), "origem", "versãosO", "recálculo" (Automático ou Manual), "versão" e "sistema".

## Extrair valores de XML

**FILTERXML** recebe um texto com XML e um XPath, que é uma forma abreviada de indicar que partes pretende, e devolve os valores que encontra. =FILTERXML(A1; "//item/price") dá todos os preços do texto em A1, um por baixo do outro. Use "/shop/item[2]/name" para o segundo item, "//item[@id='b']/name" para o item cujo id é b, ou "count(//item)" para saber quantos existem. Se o texto não for XML válido, ou o caminho não for válido, o resultado é #VALUE!. Se nada corresponder, é #N/A. XML que tenha uma declaração de tipo de documento é recusado.
