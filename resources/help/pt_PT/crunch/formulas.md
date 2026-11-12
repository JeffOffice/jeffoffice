# Como as fórmulas são apresentadas e calculadas

<!-- keywords: derramamento, derramar, #DERRAMAMENTO!, matriz dinâmica, intervalo de derramamento, A1#, erro seguinte, erro anterior, percorrer erros, rastrear erro, calcular antes de guardar, fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, guardado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar numa célula, referência, F4, verificação de erros, ignorar erro, avaliar fórmula, passo, janela de vigilância, vigiar, let, nomear um valor, função cell, função info, filterxml, xml, xpath -->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com as fórmulas

Ao abrir um livro, o ficheiro guarda o último valor de cada fórmula, para que a folha possa ser mostrada de imediato. Depois, o JeffCrunch volta a calcular todas as fórmulas, algumas de cada vez, para que a janela continue rápida. A barra de estado indica **A calcular** com uma percentagem enquanto o faz. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para voltar a calcular tudo, clique em **Recalcular Tudo** no separador **Fórmulas**, ou prima **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e o número continua a crescer. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o ficheiro guardou para ela. O mesmo acontece com uma fórmula que se refere a outro livro. As células que dependem dessas são calculadas a partir dos valores guardados.

## Funções que precisam de algumas notas

- **Obrigações e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMIMPAR e as outras funções de obrigações contam os dias conforme a **base** indicada (0 são 30 dias por mês num ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDIMENTOPRIMIMPAR, RENDIMENTOÚLTIMOIMPAR e TAXA encontram a resposta experimentando valores até que o preço ou o pagamento coincidam. Param quando a resposta está certa até cerca de 15 algarismos, pelo que os últimos algarismos podem ser diferentes de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE e os nomes mais antigos, como DISTNORM e DISTT, dão as mesmas respostas: um nome antigo é mantido para que os ficheiros antigos continuem a funcionar. As funções *.INV também são calculadas experimentando valores.
- **Listas que preenchem várias células.** ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, PROCURARX, MMULT, MINVERSA e LINEST dão um bloco de valores. Escreva uma delas numa única célula e o bloco derrama-se pelas células abaixo e ao lado (veja **Fórmulas que preenchem as células ao lado**), ou introduza-a como fórmula matricial (**Ctrl+Shift+Enter**) sobre as células que deve preencher.
- **Textos noutros sistemas de escrita.** LENB, ESQUERDAB, SEMDIREITAB, DIREITAB, LOCALIZARB, PROCURARB e SUBSTITUIRB contam cada carácter como um, tal como acontece num computador que não está configurado para texto de byte duplo. FONÉTICA dá o texto da célula, porque o JeffCrunch não guarda as leituras do texto japonês.
- **CONVERTER.** As unidades são escritas como texto, com maiúsculas e minúsculas tal como indicado na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode ir à frente de uma unidade que o aceite, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** dá o que sobra de **n** depois de retirados o maior número de cópias inteiras de **d**: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** no separador **Fórmulas**, ou prima **Ctrl+'** (num Mac, a tecla Control e não a tecla Command), para ver a fórmula de cada célula em vez do seu valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas apareçam completas; voltam às larguras anteriores quando desativa esta opção, e as larguras do seu ficheiro não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a sua célula, prima **F2**, edite-a e prima **Enter**. Também pode editá-la na barra de fórmulas. Quando altera uma célula, as fórmulas que dependem dela são calculadas de novo em segundo plano. Veja [Escrever nas células](edit-cells).

## Escrever uma fórmula

Escreva **=** e as primeiras letras de uma função, e surge uma lista curta com as funções e [nomes](names) que começam assim, com uma frase sobre cada uma. Prima **Tab** para escolher a que está marcada. Use as setas **Para Baixo** e **Para Cima** para marcar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando está dentro dos parênteses de uma função, os seus argumentos aparecem por baixo da célula, com o que está a escrever em negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas, ou por ponto e vírgula onde a sua língua usa a vírgula como separador decimal.

## Apontar para células

Em vez de escrever o endereço de uma célula, pode apontar para ela. Enquanto escreve uma fórmula, coloque o cursor onde pode entrar uma célula (logo após **=**, um operador como **+**, um parêntese de abertura ou uma vírgula) e clique na célula. O seu endereço é escrito por si. Arraste sobre células para escrever um bloco como B2:D5. Clique de novo antes de escrever outra coisa, e a nova célula substitui a que acabou de escrever. Mantenha **Ctrl** premido e clique para acrescentar outro endereço depois de uma vírgula. Clique numa letra de coluna para escrever uma coluna inteira, como A:A, ou num número de linha para escrever uma linha inteira, como 3:3, e arraste ao longo das letras ou números para B:D ou 2:5.

As teclas de seta também funcionam. Prima uma seta e um apontador afasta-se da célula onde está a escrever, e o seu endereço é escrito; prima mais setas para avançar, ou mantenha **Shift** premido para estender o apontador sobre um bloco. A barra de estado indica **Apontar** enquanto as setas apontam, **Introduzir** enquanto escreve, e **Editar** enquanto as setas movem o cursor no texto. Prima **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula de outra folha, clique no separador dessa folha enquanto o cursor estiver onde pode entrar uma célula, e depois clique na célula. O nome da folha é escrito com ela, entre aspas quando são necessárias, como em 'A Minha Folha'!B3. **Enter** leva-o de volta à folha onde começou e coloca a fórmula na sua célula. **Esc** cancela tudo, incluindo a mudança de folha.

Também pode arrastar a margem de um bloco com contorno na folha para mover esse endereço, ou arrastar o seu quadrado de canto para o estender; a fórmula altera-se à medida que arrasta.

Prima **F4** com o cursor dentro ou logo após um endereço para fixar partes dele com um cifrão: A1 passa a $A$1, depois a A$1, depois a $A1, e depois de novo a A1.

Cada endereço da fórmula é colorido no texto, e as células que ele referencia são assinaladas na folha com a mesma cor, para que veja de relance o que a fórmula lê. Um clique depois de um endereço completo, como =A1, termina a entrada e seleciona a célula em que clicou, como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de hipóteses) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir um livro que tenha uma, o JeffCrunch calcula as suas células: numa tabela com duas entradas, a fórmula está na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada de linha, e os da vertical para a célula de entrada de coluna. Numa tabela de uma entrada, os valores ficam na coluna à sua esquerda (e as fórmulas estão na linha acima) ou na linha acima dela (e as fórmulas estão na coluna à sua esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para esse valor; nada mais no livro é alterado. No modo **Automático** as tabelas são calculadas juntamente com as fórmulas, e as fórmulas que leem as células de uma tabela seguem-nas. Criar uma nova tabela de dados ainda não é possível. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou se altera por si própria, como ALEATÓRIO ou AGORA) mantém os valores que o seu ficheiro contém.

## Calcular manualmente

Clique em **Modo de Cálculo** no separador **Fórmulas** para escolher a forma como o livro calcula as suas fórmulas:

- **Automático** calcula-as à medida que altera células.
- **Automático, Tabelas Manuais** faz o mesmo, mas deixa as células das tabelas de dados como estão até premir **F9**.
- **Manual** não calcula nada, incluindo as tabelas de dados, até que o peça. A barra de estado indica **Desatualizado** depois de uma alteração. Prima **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Folha**) para calcular as fórmulas da folha em que está, e o que mais estivesse à espera. Um livro definido para Manual abre com os resultados que o seu ficheiro contém. **Calcular Antes de Guardar**, também no menu **Modo de Cálculo**, está assinalado por predefinição: quando guarda um livro Manual que está desatualizado, as fórmulas são calculadas primeiro, para que o ficheiro contenha números atualizados. Desative-o para guardar os números tal como estão.

A escolha fica guardada com o livro, e é um passo que pode anular.

## Fórmulas que leem o próprio resultado

Uma fórmula que lê o seu próprio resultado, diretamente ou através de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e assinala-as na folha; elas mantêm os valores que tinham. Clique em **Referências Circulares** no separador **Fórmulas** para ver a lista, e faça duplo clique numa célula da lista para ir até lá.

Alguns modelos precisam de um circuito deste tipo de propósito. Escolha **Iterar Fórmulas Circulares** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, lendo cada uma os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade indicada ou até se esgotar o número de iterações permitido.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a sua fórmula lê. Clique de novo para recuar mais um passo. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que lêem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra folha. São desenhadas, no máximo, 2000 setas, e a barra de estado indica quando havia mais. **Limpar Setas** remove-as, e qualquer alteração ao livro também.

Quando uma célula mostra um erro, como #DIV/0!, selecione-a e clique em **Rastrear Erro**. São desenhadas setas para as células que ela lê e que também mostram um erro. Clique de novo para recuar mais um passo. Quando nenhuma das células que lê mostra um erro, a barra de estado indica que esta célula é a que provoca o erro, e é essa que deve corrigir.

Para percorrer todos os erros da folha, clique em **Erro Seguinte** ou **Erro Anterior**. As células que mostram um erro são visitadas uma linha de cada vez e, depois da última, recomeça-se na primeira. Cada célula é selecionada, e as setas são desenhadas até à célula que provoca o erro. A barra de estado indica qual é o erro atual e quantos existem.

## Verificação de erros

Escolha **Verificação de Erros** no grupo **Verificar** do separador Fórmulas para procurar na folha células que possam estar erradas. Procura duas coisas: uma fórmula que dá um erro (como #DIV/0!), e um número guardado como texto, que as fórmulas podem ignorar.

A caixa de diálogo mostra uma célula de cada vez, vai até essa célula na folha e diz numa frase qual é o problema. **Seguinte** e **Anterior** percorrem as células. Um pequeno triângulo verde no canto superior esquerdo de uma célula assinala-a na folha. **Ignorar Erro** retira a célula da lista, remove o seu triângulo e fica guardado no ficheiro. Um passo de Anular repõe-na. Se não houver nada errado, é informado disso.

## Avaliar Fórmula

Use esta opção para ver como uma fórmula chega ao seu valor. Selecione uma célula que contenha uma fórmula e escolha **Avaliar Fórmula**. A fórmula é mostrada com a parte seguinte sublinhada. Prima **Avaliar** para calcular essa parte; o seu valor ocupa o lugar dela. As partes seguem esta ordem: cada célula isolada, da esquerda para a direita, depois cada função com os seus parênteses, começando pela mais interna, e por fim o que resta. **Reiniciar** começa de novo. Se uma parte não puder ser calculada, a caixa de diálogo diz porquê e deixa a fórmula como estava.

## Janela de Vigilância

A **Janela de Vigilância** é uma pequena janela que fica visível enquanto trabalha. Lista as células que quer acompanhar, com a sua folha, célula, valor e fórmula, mesmo quando estão noutra folha. Selecione células e prima **Adicionar Vigilância**. Selecione linhas na lista e prima **Eliminar Vigilância**. Faça duplo clique numa linha para ir até à célula. Os valores mudam quando a folha é calculada de novo. A lista é mantida enquanto o livro estiver aberto.

## Fórmulas que preenchem as células ao lado

Quando uma fórmula dá um bloco de valores, o bloco preenche as células abaixo e ao lado da célula onde a escreveu. Isto chama-se derramamento. Experimente =SEQUÊNCIA(3) em A1: A1 mostra 1, e A2 e A3 mostram 2 e 3. Uma fórmula derrama-se quando chama uma função que dá um bloco (ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, TRANSPOR, PROCURARX ou MMULT), escreve um bloco entre chavetas, usa um intervalo de derramamento (abaixo), ou faz operações aritméticas sobre um intervalo, como em =A1:A3*2.

- **As células que preenche não têm fórmula própria.** Mostram os valores. Quando seleciona uma delas, a barra de fórmulas mostra, a cinzento, a fórmula da célula que a preenche. Premir Enter sobre ela tal como está não altera nada. Para alterar o bloco, altere primeiro essa célula.
- **O bloco segue o resultado.** Se a fórmula der mais ou menos valores depois de se alterar algo que ela lê, o bloco cresce ou diminui, e as fórmulas que lêem essas células são calculadas de novo.
- **Uma célula no caminho dá #DERRAMAMENTO!.** Se alguma célula onde o bloco iria ficar contiver um valor ou uma fórmula, a primeira célula mostra **#DERRAMAMENTO!** e nada é preenchido. Limpe a célula que está no caminho e o bloco preenche-se de imediato. Escrever numa célula que o bloco preenche produz o mesmo efeito: a primeira célula mostra #DERRAMAMENTO!.
- **Um bloco tem um limite de tamanho.** Um bloco maior do que a folha, ou com mais de 200 000 células, dá #DERRAMAMENTO!.
- **A1# é o bloco inteiro.** Escreva a célula que contém a fórmula seguida de **#**, como em =SOMA(A1#), para usar o bloco inteiro tal como está agora, nesta folha ou noutra (=SOMA(Folha1!A1#)). Se A1 não derramar, A1# é #REF!.

Um derramamento é guardado com o ficheiro como matriz dinâmica, pelo que o programa de origem do ficheiro também o mostra como derramamento. Guardar no formato mais antigo .xls ou .ods mantém os valores, e a fórmula como fórmula matricial sobre o bloco, nos casos em que o formato antigo tem as funções que ela usa. A Verificação de Compatibilidade indica estas fórmulas.

## Dar nomes a valores dentro de uma fórmula

**LET** dá um nome a um valor no meio de uma fórmula, para o escrever uma vez e o usar de novo. Os pares de nome e valor vêm primeiro, e o cálculo vem por último. Em =LET(preço; B2; imposto; preço*0,08; preço+imposto) o nome **preço** representa o valor de B2, **imposto** é calculado a partir dele, e a última parte soma-os. Um nome pode usar os nomes anteriores, e um LET pode ter até 126 pares. Um nome só funciona dentro do seu próprio LET, e um LET dentro de outro pode voltar a usar um nome. Um nome também pode representar um bloco de células, como em =LET(linhas; B2:B9; SOMA(linhas)/CONTAR(linhas)).

## Perguntar sobre uma célula ou sobre o livro

**CÉL** indica algo sobre uma célula. O seu primeiro argumento diz o quê: "endereço", "linha", "col", "conteúdo", "tipo", "formato" (como os números são mostrados, como "F2" para duas casas decimais, "P0" para uma percentagem, ou "D1" para uma data), "cor" (1 quando os números negativos aparecem a cores), "parênteses" (1 quando os números positivos aparecem entre parênteses), "prefixo" (a marca de um texto alinhado à esquerda, à direita ou ao centro), "proteger" (1 quando a célula está bloqueada), "largura" (a largura da coluna em carateres) e "nomeficheiro" (a pasta, o ficheiro e a folha, como /home/eu/[Orçamento.xlsx]Folha1; vazio até o livro ser guardado). **INFORMAÇÃO** indica dados sobre o programa e o livro: "diretório", "numficheiro" (quantas folhas são mostradas), "origem", "versãosistema" (a versão do sistema operativo), "recalcular" (Automático ou Manual), "versão" e "sistema".

## Extrair valores de XML

**FILTERXML** recebe um texto que contém XML e um XPath, que é uma forma abreviada de indicar as partes que pretende, e devolve os valores que encontra. =FILTERXML(A1; "//item/preço") dá todos os preços do texto em A1, um por baixo do outro. Use "/loja/item[2]/nome" para o segundo item, "//item[@id='b']/nome" para o item cujo id é b, ou "count(//item)" para saber quantos existem. Se o texto não for XML válido, ou o caminho não for um caminho válido, a resposta é #VALOR!. Se nada corresponder, é #N/D. XML que contenha um tipo de documento é recusado.
