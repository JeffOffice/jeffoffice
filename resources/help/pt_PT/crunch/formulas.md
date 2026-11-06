# Como as fórmulas são mostradas e calculadas
<!-- keywords: erro de rastreio, calcular antes de guardar, fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, guardado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, ponto, apontar, clicar numa célula, referência, F4, verificação de erros, ignorar erro, avaliar fórmula, passo, janela de vigilância, vigiar, LET, dar nome a um valor, função de célula, função de informação, FILTERXML, xml, xpath-->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Quando abre um livro, o ficheiro guarda o último valor de cada fórmula, para que a folha possa ser mostrada de imediato. Depois, o JeffCrunch calcula novamente todas as fórmulas, algumas de cada vez, para que a janela se mantenha rápida. A barra de estado indica **A calcular** com uma percentagem enquanto o faz. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para calcular tudo novamente, clique em **Recalcular Tudo** no separador **Fórmulas** ou prima **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e continua a crescer. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o ficheiro guardou para ela. O mesmo acontece com uma fórmula que se refere a outro livro. As células que dependem destas são calculadas a partir dos valores guardados.

## Funções que requerem algumas notas

- **Obrigações e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMINC, e as outras funções de obrigações contam os dias da forma indicada pela **base** que indicar (0 são 30 dias por mês num ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDPRIMINC, RENDULTINC e TAXA encontram a resposta testando valores até que o preço ou o pagamento coincida. Param quando a resposta está certa até cerca de 15 algarismos, por isso os últimos algarismos podem ser diferentes de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE, e os nomes mais antigos como DIST.NORM.N e DISTT, dão as mesmas respostas: um nome antigo é mantido para que os ficheiros antigos continuem a funcionar. As funções *.INV também são encontradas testando valores.
- **Listas que preenchem várias células.** ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, PROCX, MMULT, MINVERSA e PROJ.LIN devolvem um bloco de valores. Introduza-as como fórmula matricial (**Ctrl+Shift+Enter**) sobre as células que devem preencher. Ainda não é possível que uma fórmula se estenda para as células vizinhas.
- **Textos em outros sistemas de escrita.** LENB, ESQUERDAB, SEQUÊNCIAB... não se aplicam aqui; use os equivalentes ESQUERDAB, DIREITAB, EXTRAIB, LOCALIZARB, PESQUISARB e SUBSTITUIRB, que contam cada carácter como um, tal como acontece num computador que não está configurado para texto de byte duplo. FONÉTICA devolve o texto da célula, porque o JeffCrunch não guarda as leituras do texto japonês.
- **CONVERTER.** As unidades são escritas como texto, com maiúsculas e minúsculas tal como indicado na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode ir à frente de uma unidade que o aceite, como "km" ou "mL".

## RESTO: o resto da divisão

**RESTO(n; d)** devolve o que sobra de **n** depois de retirar o maior número inteiro de vezes de **d**: é n - d * INT(n / d). Exemplo: **=RESTO(10; 3)** é 1, e **=RESTO(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** no separador **Fórmulas**, ou prima **Ctrl+'** (no Mac, a tecla Control e não a tecla Command), para ver a fórmula de cada célula em vez do seu valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas apareçam completas; voltam às larguras originais quando desativa a opção, e as larguras do seu ficheiro não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a sua célula, prima **F2**, edite-a e prima **Enter**. Também pode editá-la na barra de fórmulas. Quando altera uma célula, as fórmulas que dependem dela são calculadas novamente em segundo plano. Veja [Escrever nas células](edit-cells).

## Escrever uma fórmula

Escreva **=** e as primeiras letras de uma função, e surge uma lista curta com as funções e [nomes](names) que começam dessa forma, com uma frase sobre cada uma. Prima **Tab** para escolher a que está assinalada. Use as setas **Para Baixo** e **Para Cima** para assinalar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando está dentro dos parênteses de uma função, os seus argumentos aparecem sob a célula, com o que está a escrever em negrito. A lista e os argumentos são os mesmos que mostra **Escolher uma Função**. Os argumentos são separados por vírgulas, ou por ponto e vírgula quando a sua língua usa a vírgula como separador decimal.

## Apontar para células

Em vez de escrever o endereço de uma célula, pode apontar para ela. Enquanto escreve uma fórmula, coloque o cursor onde pode ir uma célula (logo a seguir a **=**, a um operador como **+**, a um parêntese de abertura ou a uma vírgula) e clique na célula. O seu endereço é escrito por si. Arraste sobre células para escrever um bloco como B2:D5. Clique novamente antes de escrever outra coisa, e a nova célula substitui a que acabou de ser escrita. Mantenha **Ctrl** premido e clique para acrescentar outro endereço depois de uma vírgula. Clique numa letra de coluna para escrever uma coluna inteira, como A:A, ou num número de linha para escrever uma linha inteira, como 3:3, e arraste ao longo das letras ou números para B:D ou 2:5.

As teclas de seta também funcionam. Prima uma seta e um apontador afasta-se da célula em que está a escrever, e o seu endereço é escrito; prima mais setas para continuar, ou mantenha **Shift** premido para estender o apontador sobre um bloco. A barra de estado indica **Apontar** enquanto as setas apontam, **Introduzir** enquanto escreve, e **Editar** enquanto as setas movem o cursor no texto. Prima **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula noutra folha, clique no separador dessa folha enquanto o cursor estiver onde pode ir uma célula, e depois clique na célula. O nome da folha é escrito com ela, entre aspas quando necessário, como em 'A Minha Folha'!B3. **Enter** leva-o de volta à folha onde começou e coloca a fórmula na sua célula. **Esc** cancela tudo, incluindo a mudança de folha.

Também pode arrastar a margem de um bloco com contorno na folha para mover esse endereço, ou arrastar o seu quadrado de canto para o estender; a fórmula muda enquanto arrasta.

Prima **F4** com o cursor dentro de um endereço ou logo a seguir a ele para fixar partes dele com um cifrão: A1 passa a $A$1, depois a A$1, depois a $A1, e depois novamente a A1.

Cada endereço da fórmula é colorido no texto, e as células a que se refere ficam com contorno na folha na mesma cor, para que veja de relance o que a fórmula lê. Um clique depois de um endereço completo, como em =A1, termina a entrada e seleciona a célula em que clicou, tal como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de hipóteses) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir um livro que tenha uma, o JeffCrunch calcula as suas células: para uma tabela com duas entradas, a fórmula está na célula do canto superior esquerdo da tabela, os valores a testar ao longo da linha vão para a célula de entrada de linha, e os valores ao longo da coluna vão para a célula de entrada de coluna. Para uma tabela de uma entrada, os valores ficam na coluna à sua esquerda (e as fórmulas estão na linha acima) ou ao longo da linha acima (e as fórmulas estão na coluna à sua esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para esse valor; nada mais no livro é alterado. No modo **Automático** as tabelas são calculadas com as fórmulas, e as fórmulas que leem as células de uma tabela acompanham-nas. Criar uma nova tabela de dados ainda não é possível. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda por si própria, como ALEATÓRIO ou AGORA) mantém os valores que o seu ficheiro contém.

## Calcular manualmente

Clique em **Modo de Cálculo** no separador **Fórmulas** para escolher a forma como o livro calcula as suas fórmulas:

- **Automático** calcula-as à medida que altera células.
- **Automático, Tabelas Manuais** faz o mesmo, mas deixa as células das tabelas de dados como estão até premir **F9**.
- **Manual** não calcula nada, incluindo as tabelas de dados, até que o peça. A barra de estado indica **Desatualizado** depois de uma alteração. Prima **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Folha**) para calcular as fórmulas da folha em que está, e tudo o mais que estava a aguardar. Um livro definido como Manual abre com os resultados que o seu ficheiro contém. **Calcular Antes de Guardar**, também no menu **Modo de Cálculo**, está ativado por predefinição: ao guardar um livro Manual desatualizado, as fórmulas são calculadas primeiro, para que o ficheiro contenha números atualizados. Desative-o para guardar os números como estão.

A escolha é guardada com o livro, e é um passo que pode anular.

## Fórmulas que leem a si próprias

Uma fórmula que lê o seu próprio resultado, diretamente ou através de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e assinala-as com contorno na folha; elas mantêm os valores que tinham. Clique em **Referências Circulares** no separador **Fórmulas** para ver a lista, e faça duplo clique numa célula dessa lista para ir até lá.

Alguns modelos precisam de um ciclo deste tipo de propósito. Escolha **Iterar Fórmulas Circulares** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, lendo cada uma os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade que indicar ou se esgote o número de iterações permitido.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a sua fórmula lê. Clique novamente para recuar mais um passo. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que leem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra folha. São desenhadas, no máximo, 2000 setas, e a barra de estado indica quando havia mais. **Limpar Setas** remove-as, e qualquer alteração ao livro também.

Quando uma célula mostra um erro, como #DIV/0!, selecione-a e clique em **Rastrear Erro**. São desenhadas setas para as células que ela lê e que também mostram um erro. Clique novamente para recuar mais um passo. Quando nenhuma das células que lê mostra um erro, a barra de estado indica que esta célula causa o erro, e é esta a que deve corrigir.

## Verificação de erros

Escolha **Verificação de Erros** no grupo **Verificar** do separador Fórmulas para procurar na folha células que possam estar erradas. Procura duas coisas: uma fórmula que dá um erro (como #DIV/0!), e um número guardado como texto, que as fórmulas podem ignorar.

A caixa de diálogo mostra uma célula de cada vez, vai até essa célula na folha, e explica numa frase o que está errado. **Seguinte** e **Anterior** percorrem as células. Um pequeno triângulo verde no canto superior esquerdo de uma célula assinala-a na folha. **Ignorar Erro** retira a célula da lista, retira-lhe o triângulo, e fica guardado no ficheiro. Um passo de Anular repõe-na. Se não houver nada errado, é informado disso.

## Avaliar fórmula

Use esta opção para ver como uma fórmula chega ao seu valor. Selecione uma célula que contenha uma fórmula e escolha **Avaliar Fórmula**. A fórmula aparece com a parte seguinte sublinhada. Prima **Avaliar** para calcular essa parte; o seu valor ocupa o seu lugar. As partes seguem esta ordem: cada célula isolada da esquerda, depois cada função com os seus parênteses, começando pela mais interior, e por fim o que resta. **Reiniciar** começa de novo. Se uma parte não puder ser calculada, a caixa de diálogo diz porquê e deixa a fórmula como estava.

## Janela de Vigilância

A **Janela de Vigilância** é uma pequena janela que fica visível enquanto trabalha. Lista as células que quer vigiar, com a sua folha, célula, valor e fórmula, mesmo quando estão noutra folha. Selecione células e prima **Adicionar Vigilância**. Selecione linhas da lista e prima **Eliminar Vigilância**. Faça duplo clique numa linha para ir até à célula. Os valores mudam quando a folha é calculada novamente. A lista é mantida enquanto o livro estiver aberto.

## Dar nomes a valores dentro de uma fórmula

**LET** dá um nome a um valor no meio de uma fórmula, para que o escreva uma vez e o use novamente. Os pares de nome e valor vêm primeiro, e o cálculo fica no fim. Em =LET(preço; B2; imposto; preço*0,08; preço+imposto) o nome **preço** representa o valor de B2, **imposto** é calculado a partir dele, e a última parte soma-os. Um nome pode usar os nomes que vêm antes dele. Um nome só funciona dentro do seu próprio LET, e um LET dentro de outro pode voltar a usar um nome. Um nome também pode representar um bloco de células, como em =LET(linhas; B2:B9; SOMA(linhas)/CONTAR(linhas)).

## Perguntar sobre uma célula ou o livro

**CÉLULA** indica algo sobre uma célula. O seu primeiro argumento diz o quê: "endereço", "linha", "coluna", "conteúdo", "tipo", "formato" (a forma como os números são mostrados, como "F2" para duas casas decimais, "P0" para uma percentagem, ou "D1" para uma data), "cor" (1 quando os números negativos são mostrados a cores), "parênteses" (1 quando os números positivos são mostrados entre parênteses), "prefixo" (a marca de um texto alinhado à esquerda, à direita ou ao centro), "proteger" (1 quando a célula está bloqueada), "largura" (a largura da coluna em carateres), e "nomeficheiro" (a pasta, o ficheiro e a folha, como /home/eu/[Orçamento.xlsx]Folha1; vazio até o livro ser guardado). **INFO** indica algo sobre o programa e o livro: "diretório", "numficheiro" (quantas folhas são mostradas), "origem", "versãosO", "recalcular" (Automático ou Manual), "versão" e "sistema".

## Extrair valores de XML

**FILTERXML** recebe um texto que contém XML e um XPath, que é uma forma breve de dizer que partes pretende, e devolve os valores que encontra. =FILTERXML(A1; "//item/price") devolve todos os preços do texto em A1, um por baixo do outro. Use "/loja/item[2]/nome" para o segundo item, "//item[@id='b']/nome" para o item cujo id é b, ou "count(//item)" para saber quantos há. Se o texto não for XML válido, ou o caminho não for um caminho válido, o resultado é #VALOR!. Se nada corresponder, é #N/D. XML que tenha um tipo de documento é recusado.
