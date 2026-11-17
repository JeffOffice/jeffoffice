# Como as fórmulas são apresentadas e calculadas
<!-- keywords: derramamento, derramado, #SPILL!, matriz dinâmica, intervalo de derramamento, A1#, próximo erro, erro anterior, percorrer erros, rastrear erro, calcular antes de guardar, fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, guardado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar numa célula, referência, F4, verificação de erros, ignorar erro, avaliar fórmula, passo, vigilância, janela de vigilância, let, dar nome a um valor, função cell, função info, filterxml, xml, xpath, precisão como apresentada, definir precisão como apresentada, arredondar, arredondamento, casas decimais, dígitos-->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Quando abre um livro, o ficheiro guarda o último valor de cada fórmula, para que a folha possa ser mostrada de imediato. Depois, o JeffCrunch volta a calcular todas as fórmulas, algumas de cada vez, para que a janela se mantenha rápida. A barra de estado indica **A calcular** com uma percentagem enquanto o faz. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para voltar a calcular tudo, clique em **Recalcular Tudo** no separador **Fórmulas**, ou prima **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e continua a crescer. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o ficheiro guardou para ela. O mesmo acontece com uma fórmula que se refere a outro livro. As células que dependem dessas são calculadas a partir dos valores guardados.

## Funções que requerem algumas notas

- **Obrigações e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMINC, e as outras funções de obrigações contam os dias da forma indicada pela **base** que introduzir (0 são 30 dias por mês num ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDPRIMINC, RENDUPIMPAR e TAXA encontram a resposta testando valores até que o preço ou o pagamento coincida. Param quando a resposta está certa até cerca de 15 algarismos, pelo que os últimos algarismos podem diferir de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE, e os nomes mais antigos, como DIST.NORM ou DISTT, dão as mesmas respostas: um nome antigo mantém-se para que os ficheiros antigos continuem a funcionar. As funções *.INV também são encontradas testando valores.
- **Listas que preenchem várias células.** ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, PROCX, MMULT, MINVERSA e PROJLIN dão um bloco de valores. Escreva uma numa única célula e o bloco derrama-se para as células abaixo e ao lado (veja **Fórmulas que preenchem as células ao lado**), ou introduza-a como fórmula de matriz (**Ctrl+Shift+Enter**) sobre as células que deve preencher.
- **Textos noutros sistemas de escrita.** NÚM.CARACT.B, ESQUERDAB, SEQCARACTB, DIREITAB, LOCALIZARB, PROCURARB e SUBSTITUIRB contam cada carácter como um, tal como acontece num computador que não está configurado para texto de byte duplo. FONÉTICA dá o texto da célula, porque o JeffCrunch não guarda as leituras de texto japonês.
- **CONVERTER.** As unidades são escritas como texto, com maiúsculas e minúsculas como indicado na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode ir antes de uma unidade que o aceite, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** dá o que sobra de **n** depois de retirar o maior número de cópias inteiras de **d**: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. A resposta tem o sinal de **d**. Se **d** for 0, a resposta é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** no separador **Fórmulas**, ou prima **Ctrl+'** (num Mac, a tecla Control, e não a tecla Command), para ver a fórmula de cada célula no lugar do seu valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas apareçam por inteiro; voltam às larguras anteriores quando desativa a opção, e as larguras do seu ficheiro não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a sua célula, prima **F2**, edite-a e prima **Enter**. Também pode editá-la na barra de fórmulas. Quando altera uma célula, as fórmulas que dependem dela são calculadas de novo em segundo plano. Veja [Escrever nas células](edit-cells).

## Escrever uma fórmula

Escreva **=** e as primeiras letras de uma função, e surge uma lista curta com as funções e os [nomes](names) que começam assim, com uma frase sobre cada uma. Prima **Tab** para escolher a que está marcada. Use as setas **Para Baixo** e **Para Cima** para marcar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando está dentro dos parênteses de uma função, os seus argumentos aparecem sob a célula, com o que está a escrever em negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas, ou por pontos e vírgulas quando a sua língua usa a vírgula como separador decimal.

## Apontar para células

Em vez de escrever o endereço de uma célula, pode apontar para ela. Enquanto escreve uma fórmula, coloque o cursor onde pode ir uma célula (logo a seguir a **=**, a um operador como **+**, a um parêntese de abertura ou a uma vírgula) e clique na célula. O seu endereço é escrito por si. Arraste sobre células para escrever um bloco como B2:D5. Clique de novo antes de escrever qualquer outra coisa, e a nova célula substitui a que acabou de ser escrita. Mantenha **Ctrl** premido e clique para acrescentar outro endereço depois de uma vírgula. Clique numa letra de coluna para escrever uma coluna inteira, como A:A, ou num número de linha para escrever uma linha inteira, como 3:3, e arraste ao longo das letras ou dos números para B:D ou 2:5.

As teclas de seta também funcionam. Prima uma seta e um ponteiro afasta-se da célula onde está a escrever, e o seu endereço é escrito; prima mais setas para continuar, ou mantenha **Shift** premido para estender o ponteiro sobre um bloco. A barra de estado indica **Apontar** enquanto as setas apontam, **Introduzir** enquanto escreve, e **Editar** enquanto as setas movem o cursor no texto. Prima **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula de outra folha, clique no separador dessa folha enquanto o cursor estiver onde pode ir uma célula, e depois clique na célula. O nome da folha é escrito com ela, entre aspas quando são necessárias, como em 'A Minha Folha'!B3. **Enter** leva-o de volta à folha onde começou e coloca a fórmula na sua célula. **Esc** cancela tudo, incluindo a mudança de folha.

Também pode arrastar o contorno de um bloco assinalado na folha para mover esse endereço, ou arrastar o seu quadrado de canto para o alargar; a fórmula muda enquanto arrasta.

Prima **F4** com o cursor dentro de um endereço ou logo a seguir a ele para fixar partes dele com um sinal de dólar: A1 passa a $A$1, depois a A$1, depois a $A1, e depois de novo a A1.

Cada endereço da fórmula é colorido no texto, e as células que refere são assinaladas na folha com a mesma cor, para que veja de relance o que a fórmula lê. Um clique depois de um endereço completo, como =A1, termina a escrita e seleciona a célula em que clicou, tal como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de hipóteses) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir um livro que tem uma, o JeffCrunch calcula as suas células: numa tabela com duas entradas, a fórmula está na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada da linha, e os valores na vertical vão para a célula de entrada da coluna. Numa tabela com uma entrada, os valores seguem na coluna à sua esquerda (e as fórmulas estão na linha acima) ou na linha acima (e as fórmulas estão na coluna à sua esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para esse valor; nada mais no livro é alterado. No modo **Automático**, as tabelas são calculadas juntamente com as fórmulas, e as fórmulas que leem as células de uma tabela seguem-nas. Criar uma nova tabela de dados ainda não é disponibilizado. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda por si própria, como ALEATÓRIO ou AGORA) mantém os valores que o seu ficheiro contém.

## Calcular manualmente

Clique em **Modo de Cálculo** no separador **Fórmulas** para escolher a forma como o livro calcula as suas fórmulas:

- **Automático** calcula-as à medida que altera células.
- **Automático, Tabelas Manuais** faz o mesmo, mas deixa as células das tabelas de dados como estão até premir **F9**.
- **Manual** não calcula nada, incluindo as tabelas de dados, até que o peça. A barra de estado indica **Desatualizado** depois de uma alteração. Prima **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Folha**) para calcular as fórmulas da folha em que está, e tudo o mais que esteja pendente. Um livro definido como Manual abre com os resultados que o seu ficheiro contém. **Calcular Antes de Guardar**, também no menu **Modo de Cálculo**, vem assinalado por predefinição: quando guarda um livro Manual que está desatualizado, as fórmulas são calculadas primeiro, para que o ficheiro contenha números atualizados. Desative-a para guardar os números tal como estão.

A escolha é guardada com o livro, e é um passo que pode anular.

## Fórmulas que leem a si próprias

Uma fórmula que lê o seu próprio resultado, diretamente ou através de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e assinala-as na folha; elas mantêm os valores que tinham. Clique em **Referências Circulares** no separador **Fórmulas** para ver a lista, e faça duplo clique numa célula para lá ir.

Alguns modelos precisam de um ciclo desses de propósito. Escolha **Iterar Fórmulas Circulares** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, lendo cada uma os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade indicada ou até que se esgote o número de iterações permitido.

## Manter os números como as células os mostram

Uma célula pode mostrar menos algarismos do que os que contém. Se uma célula tiver 2,675 e mostrar 2,68, as fórmulas que a leem continuam a usar 2,675. Por vezes, quer que usem o que vê. Nesse caso, escolha **Definir Precisão como Apresentada** no menu **Opções de Cálculo** do separador **Fórmulas**.

O JeffCrunch avisa primeiro, porque os algarismos ocultos perdem-se. Escolha **Cancelar** e nada muda. Escolha **OK** e cada número do livro é arredondado para o que a sua célula mostra. Uma célula que mostra duas casas decimais mantém duas. Uma célula que mostra uma percentagem mantém a percentagem. Uma célula com uma data mantém o dia, e uma célula com uma hora mantém a menor unidade de tempo que mostra, como o minuto. Uma célula que mostra **Geral** mantém todos os seus algarismos. Num livro grande, aparece uma nota com um botão **Cancelar** enquanto isto é feito. Cancele enquanto a nota estiver visível, e nada é alterado.

A partir daí, cada número que escreve ou cola, e cada resposta que uma fórmula dá, é arredondado da mesma forma. As próprias fórmulas não são alteradas. A escolha é guardada com o livro.

O arredondamento é um passo que pode anular, e **Anular** repõe os números. Escolher **Definir Precisão como Apresentada** de novo desativa a opção. Isso apenas impede o arredondamento a partir desse momento. Os números que já tinham sido arredondados mantêm-se arredondados.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a sua fórmula lê. Clique de novo para recuar mais um passo. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que leem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra folha. São desenhadas, no máximo, 2000 setas, e a barra de estado indica quando havia mais. **Limpar Setas** remove-as, e qualquer alteração ao livro também.

Quando uma célula mostra um erro, como #DIV/0!, selecione-a e clique em **Rastrear Erro**. São desenhadas setas para as células que ela lê e que também mostram um erro. Clique de novo para recuar mais um passo. Quando nenhuma célula que lê mostra um erro, a barra de estado indica que esta célula é a que produz o erro, e é essa que deve corrigir.

Para percorrer todos os erros da folha, clique em **Próximo Erro** ou **Erro Anterior**. As células que mostram um erro são visitadas linha a linha, e depois da última recomeça-se na primeira. Cada célula é selecionada, e as setas são desenhadas até à célula que produz o erro. A barra de estado indica em que erro está e quantos existem.

## Verificação de erros

Escolha **Verificação de Erros** no grupo **Verificar** do separador Fórmulas para percorrer a folha à procura de células que possam estar erradas. Procura duas coisas: uma fórmula que dá um erro (como #DIV/0!), e um número guardado como texto, que as fórmulas podem ignorar.

A caixa de diálogo mostra uma célula de cada vez, vai até essa célula na folha, e explica numa frase qual é o problema. **Seguinte** e **Anterior** percorrem as células. Um pequeno triângulo verde no canto superior esquerdo de uma célula assinala-a na folha. **Ignorar Erro** retira a célula da lista, remove o seu triângulo, e fica guardado no ficheiro. Um passo de Anular repõe-na. Se não houver nenhum problema, é informado disso.

## Avaliar fórmula

Use esta opção para ver como uma fórmula chega ao seu valor. Selecione uma célula que contenha uma fórmula e escolha **Avaliar Fórmula**. A fórmula aparece com a parte seguinte sublinhada. Prima **Avaliar** para calcular essa parte; o seu valor ocupa o lugar dela. As partes seguem esta ordem: cada célula isolada da esquerda, depois cada função com os seus parênteses, começando pela mais interior, e depois o que restar. **Reiniciar** começa de novo. Se uma parte não puder ser calculada, a caixa de diálogo diz porquê e deixa a fórmula como estava.

## Janela de Vigilância

A **Janela de Vigilância** é uma pequena janela que permanece visível enquanto trabalha. Lista as células que quer vigiar, com a sua folha, célula, valor e fórmula, mesmo que estejam noutra folha. Selecione células e prima **Adicionar Vigilância**. Selecione linhas na lista e prima **Eliminar Vigilância**. Faça duplo clique numa linha para ir até à célula. Os valores mudam quando a folha é calculada de novo. A lista mantém-se enquanto o livro estiver aberto.

## Fórmulas que preenchem as células ao lado

Quando uma fórmula dá um bloco de valores, o bloco preenche as células abaixo e ao lado da célula onde a escreveu. Isto chama-se derramamento. Experimente =SEQUÊNCIA(3) em A1: A1 mostra 1, e A2 e A3 mostram 2 e 3. Uma fórmula derrama-se quando chama uma função que dá um bloco (ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, TRANSPOR, PROCX ou MMULT), escreve um bloco entre chavetas, usa um intervalo de derramamento (abaixo), ou faz uma operação sobre um intervalo, como em =A1:A3*2.

- **As células que preenche não têm fórmula própria.** Mostram os valores. Quando seleciona uma delas, a barra de fórmulas mostra, a cinzento, a fórmula da célula que a preenche. Premir Enter sobre ela tal como está não altera nada. Para alterar o bloco, altere primeiro essa célula.
- **O bloco segue o resultado.** Se a fórmula der mais ou menos valores depois de algo que lê mudar, o bloco cresce ou diminui, e as fórmulas que leem essas células são calculadas de novo.
- **Uma célula no caminho dá #SPILL!.** Se alguma célula onde o bloco iria ficar contiver um valor ou uma fórmula, a primeira célula mostra **#SPILL!** e nada é preenchido. Limpe a célula no caminho e o bloco preenche de imediato. Escrever numa célula que o bloco preencheria produz o mesmo efeito: a primeira célula mostra #SPILL!.
- **Um bloco tem um limite de tamanho.** Um bloco maior do que a folha, ou com mais de 200 000 células, dá #SPILL!.
- **A1# é o bloco inteiro.** Escreva a célula que contém a fórmula seguida de **#**, como em =SOMA(A1#), para usar o bloco inteiro tal como está agora, nesta folha ou noutra (=SOMA(Folha1!A1#)). Se A1 não derramar, A1# é #REF!.

Um derramamento é guardado com o ficheiro como matriz dinâmica, pelo que o programa de origem do ficheiro também o mostra como derramamento. Ao guardar no formato mais antigo .xls ou .ods, os valores são mantidos, e a fórmula passa a fórmula de matriz sobre o bloco, quando o formato antigo tem as funções que ela usa. Verificar Compatibilidade assinala estas fórmulas.

## Dar nomes a valores dentro de uma fórmula

**LET** dá um nome a um valor no meio de uma fórmula, para que o escreva uma vez e o use outras vezes. Os pares de nome e valor vêm primeiro, e o cálculo vem por último. Em =LET(preço; B2; imposto; preço*0,08; preço+imposto), o nome **preço** representa o valor de B2, **imposto** é calculado a partir dele, e a última parte soma-os. Um nome pode usar os nomes que o precedem, e um LET pode ter até 126 pares. Um nome só funciona dentro do seu próprio LET, e um LET dentro de outro pode voltar a usar um nome. Um nome também pode representar um bloco de células, como em =LET(linhas; B2:B9; SOMA(linhas)/CONTAR(linhas)).

## Perguntar sobre uma célula ou sobre o livro

**CELL** indica algo sobre uma célula. O seu primeiro argumento diz o quê: "address", "row", "col", "contents", "type", "format" (como os números são apresentados, como "F2" para duas casas decimais, "P0" para uma percentagem, ou "D1" para uma data), "color" (1 quando os números negativos aparecem a cor), "parentheses" (1 quando os números positivos aparecem entre parênteses), "prefix" (a marca de um texto alinhado à esquerda, à direita ou ao centro), "protect" (1 quando a célula está bloqueada), "width" (a largura da coluna em carateres), e "filename" (a pasta, o ficheiro e a folha, como /home/eu/[Orçamento.xlsx]Folha1; vazio até o livro ser guardado). **INFO** indica algo sobre o programa e o livro: "directory", "numfile" (quantas folhas são apresentadas), "origin", "osversion", "recalc" (Automatic ou Manual), "release", e "system".

## Extrair valores de XML

**FILTERXML** recebe um texto que contém XML e um XPath, que é uma forma abreviada de indicar que partes quer, e devolve os valores que encontra. =FILTERXML(A1; "//item/preço") dá cada preço do texto em A1, um por baixo do outro. Use "/loja/item[2]/nome" para o segundo item, "//item[@id='b']/nome" para o item cujo id é b, ou "count(//item)" para saber quantos existem. Se o texto não for XML válido, ou o caminho não for válido, a resposta é #VALOR!. Se nada corresponder, é #N/D. XML que tem um tipo de documento é recusado.
