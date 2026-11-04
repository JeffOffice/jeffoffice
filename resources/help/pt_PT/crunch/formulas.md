# Como as fórmulas são apresentadas e calculadas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, armazenado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar numa célula, referência, F4 -->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Quando abre um livro, o ficheiro guarda o último valor de cada fórmula, para que a folha possa ser mostrada de imediato. Depois, o JeffCrunch volta a calcular todas as fórmulas, algumas de cada vez, para que a janela continue rápida. A barra de estado indica **A calcular** com uma percentagem enquanto o faz. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para voltar a calcular tudo, clique em **Recalcular Tudo** no separador **Fórmulas**, ou prima **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e continua a crescer. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o ficheiro guardou para ela. O mesmo acontece com uma fórmula que se refere a outro livro. As células que dependem dessas são calculadas a partir dos valores guardados.

## Funções que precisam de algumas notas

- **Obrigações e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPARCPRIM, e as restantes funções de obrigações contam os dias de acordo com a **base** que indicar (0 é 30 dias por mês num ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDIMENTOPARCPRIM, RENDIMENTOPARCUL e TAXA encontram o resultado experimentando valores até que o preço ou o pagamento coincida. Param quando o resultado está certo até cerca de 15 algarismos, por isso os últimos algarismos podem ser diferentes dos de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE, e os nomes mais antigos, como DIST.NORM e DISTT, dão as mesmas respostas: um nome antigo é mantido para que os ficheiros antigos continuem a funcionar. As funções *.INV também são calculadas experimentando valores.
- **Listas que preenchem várias células.** ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, PROCX, MMULT, MINVERSA e PROJ.LIN devolvem um bloco de valores. Introduza-as como fórmula matricial (**Ctrl+Shift+Enter**) sobre as células que devem preencher. Deixar que uma fórmula se estenda para as células vizinhas ainda não está disponível.
- **Textos noutros sistemas de escrita.** LENB, ESQUERDAB, SEXTRAIB, DIREITAB, LOCALIZARB, PROCURARB e SUBSTITUIRB contam cada carácter como um, como acontece num computador que não está configurado para texto de byte duplo. FONÉTICO devolve o texto da célula, porque o JeffCrunch não guarda as leituras do texto japonês.
- **CONVERTER.** As unidades são escritas como texto, com maiúsculas e minúsculas tal como indicado na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode ir à frente de uma unidade que o aceite, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** devolve o que sobra de **n** depois de retirar o maior número inteiro de vezes possível de **d**: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** no separador **Fórmulas**, ou prima **Ctrl+'** (em Mac, a tecla Control e não a tecla Command), para ver a fórmula de cada célula em vez do seu valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas se vejam por completo; voltam às larguras originais quando desativar esta opção, e as larguras do seu ficheiro não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a respetiva célula, prima **F2**, edite-a e prima **Enter**. Também pode editá-la na barra de fórmulas. Quando altera uma célula, as fórmulas que dependem dela são recalculadas em segundo plano. Consulte [Escrever nas células](edit-cells).

## Escrever uma fórmula

Escreva **=** e as primeiras letras de uma função, e surge uma lista curta com as funções e [nomes](names) que começam dessa forma, com uma frase sobre cada um. Prima **Tab** para escolher a que está marcada. Use as setas **Para baixo** e **Para cima** para marcar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando está dentro dos parênteses de uma função, os respetivos argumentos aparecem por baixo da célula, com o que está a escrever em negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas, ou por ponto e vírgula quando na sua língua a vírgula é o separador decimal.

## Apontar para células

Em vez de escrever o endereço de uma célula, pode apontar para ela. Enquanto escreve uma fórmula, coloque o cursor onde pode estar uma célula (logo depois de **=**, de um operador como **+**, de um parêntese de abertura ou de uma vírgula) e clique na célula. O endereço é escrito por si. Arraste sobre células para escrever um bloco, como B2:D5. Clique de novo antes de escrever outra coisa, e a nova célula substitui a que acabou de ser escrita. Mantenha **Ctrl** premido e clique para acrescentar outro endereço depois de uma vírgula. Clique numa letra de coluna para escrever uma coluna inteira, como A:A, ou num número de linha para escrever uma linha inteira, como 3:3, e arraste ao longo das letras ou dos números para B:D ou 2:5.

As teclas de seta também funcionam. Prima uma seta e um apontador afasta-se da célula onde está a escrever, e o respetivo endereço é escrito; prima mais setas para continuar, ou mantenha **Shift** premido para estender o apontador sobre um bloco. A barra de estado indica **Apontar** enquanto as setas apontam, **Introduzir** enquanto escreve, e **Editar** quando as setas movem o cursor no texto. Prima **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula noutra folha, clique no separador dessa folha enquanto o cursor estiver onde pode estar uma célula, e depois clique na célula. O nome da folha é escrito junto, entre aspas quando necessário, como em 'A Minha Folha'!B3. **Enter** leva-o de volta à folha onde começou e coloca a fórmula na respetiva célula. **Esc** cancela tudo, incluindo a mudança de folha.

Também pode arrastar a margem de um bloco com contorno na folha para mover esse endereço, ou arrastar o quadrado do canto para o estender; a fórmula muda enquanto arrasta.

Prima **F4** com o cursor dentro de um endereço, ou logo a seguir a ele, para fixar partes dele com um cifrão: A1 passa a $A$1, depois a A$1, depois a $A1, e de novo a A1.

Cada endereço da fórmula é colorido no texto, e as células a que se refere são delineadas na folha com a mesma cor, para que veja de relance o que a fórmula lê. Um clique a seguir a um endereço completo, como em =A1, termina a escrita e seleciona a célula em que clicou, tal como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de hipóteses) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir um livro que tenha uma, o JeffCrunch calcula as respetivas células: numa tabela com duas entradas, a fórmula fica na célula do canto superior esquerdo da tabela, os valores a experimentar na horizontal vão para a célula de entrada de linha, e os da vertical vão para a célula de entrada de coluna. Numa tabela de uma entrada, os valores ficam na coluna à esquerda (e as fórmulas na linha por cima) ou na linha por cima (e as fórmulas na coluna à esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para esse valor; nada mais no livro é alterado. No modo **Automático**, as tabelas são calculadas juntamente com as fórmulas, e as fórmulas que leem células de uma tabela acompanham-na. Criar uma nova tabela de dados ainda não está disponível. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda por si própria, como ALEATÓRIO ou AGORA) mantém os valores que o respetivo ficheiro contém.

## Calcular manualmente

Clique em **Modo de Cálculo** no separador **Fórmulas** para escolher a forma como o livro calcula as fórmulas:

- **Automático** calcula-as à medida que altera células.
- **Automático, Exceto Tabelas** faz o mesmo, mas deixa as células das tabelas de dados como estão até premir **F9**.
- **Manual** não calcula nada, incluindo as tabelas de dados, até que o peça. A barra de estado indica **Desatualizada** depois de uma alteração. Prima **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Folha**) para calcular as fórmulas da folha em que está, e o que mais estivesse pendente. Um livro definido como Manual abre com os resultados que o respetivo ficheiro contém.

A escolha é guardada com o livro, e é um passo que pode anular.

## Fórmulas que leem o próprio resultado

Uma fórmula que lê o seu próprio resultado, diretamente ou através de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e delineia-as na folha; mantêm os valores que tinham. Clique em **Referências Circulares** no separador **Fórmulas** para ver a lista, e faça duplo clique numa célula da lista para lá ir.

Alguns modelos precisam de tal circularidade de propósito. Escolha **Iterar Fórmulas Circulares** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, lendo cada uma os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade indicada ou até se esgotar o número de iterações permitido.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a sua fórmula lê. Clique de novo para recuar mais um passo. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que lêem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra folha. São desenhadas, no máximo, 2000 setas, e a barra de estado indica quando havia mais. **Limpar Setas** remove-as, e qualquer alteração ao livro também as remove.
