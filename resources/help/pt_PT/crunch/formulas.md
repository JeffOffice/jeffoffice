# Como as fórmulas são apresentadas e calculadas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, guardado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar numa célula, referência, F4, verificação de erros, ignorar erro, avaliar fórmula, passo, janela de vigilância, vigiar -->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com as fórmulas

Quando abre um livro, o ficheiro guarda o último valor de cada fórmula, para que a folha possa ser mostrada de imediato. Depois, o JeffCrunch volta a calcular cada fórmula, poucas de cada vez, para que a janela se mantenha rápida. A barra de estado indica **A calcular** com uma percentagem enquanto o faz. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para voltar a calcular tudo, clique em **Recalcular Tudo** no separador **Fórmulas**, ou prima **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e continua a crescer. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o ficheiro guardou para ela. O mesmo acontece com uma fórmula que se refere a outro livro. As células que dependem dessas são calculadas a partir dos valores guardados.

## Funções que requerem algumas notas

- **Obrigações e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMIMPAR e as restantes funções de obrigações contam os dias segundo a **base** indicada (0 é 30 dias por mês num ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDIMENTOPRIMIMPAR, RENDIMENTOÚLTIMOIMPAR e TAXA encontram a resposta testando valores até que o preço ou o pagamento coincida. Param quando a resposta está certa até cerca de 15 algarismos, por isso os últimos algarismos podem diferir de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE e os nomes mais antigos, como DISTNORM e DISTT, dão as mesmas respostas: um nome antigo é mantido para que os ficheiros antigos continuem a funcionar. As funções *.INV também são encontradas testando valores.
- **Listas que preenchem várias células.** ORDENAR, ORDENARPOR, FILTRAR, ÚNICOS, SEQUÊNCIA, MATRIZALEATÓRIA, PROCX, MMULT, INVERSAMAT e PROJLIN devolvem um bloco de valores. Introduza-as como fórmula matricial (**Ctrl+Shift+Enter**) sobre as células que devem preencher. Ainda não é possível que uma fórmula se estenda para as células vizinhas.
- **Textos noutros sistemas de escrita.** LENB, ESQUERDAB, DIRB, DIREITAB, LOCALIZARB, PESQUISARB e SUBSTITUIRB contam cada carácter como um, tal como acontece num computador que não está configurado para texto de byte duplo. FONÉTICA devolve o texto da célula, porque o JeffCrunch não guarda as leituras do texto japonês.
- **CONVERTER.** As unidades são escritas como texto, com maiúsculas e minúsculas tal como indicado na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode ser colocado à frente de uma unidade que o aceite, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** dá o que resta de **n** depois de retirar o maior número inteiro de cópias de **d**: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** no separador **Fórmulas**, ou prima **Ctrl+`** (num Mac, a tecla Control e não a tecla Command), para ver a fórmula de cada célula em vez do seu valor. As colunas que contêm fórmulas ficam mais largas, até um limite, para que as fórmulas se vejam por inteiro; voltam às larguras anteriores quando desativa a opção, e as larguras no seu ficheiro não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a respetiva célula, prima **F2**, edite-a e prima **Enter**. Também pode editá-la na barra de fórmulas. Quando altera uma célula, as fórmulas que dependem dela são calculadas de novo em segundo plano. Veja [Escrever nas células](edit-cells).

## Escrever uma fórmula

Escreva **=** e as primeiras letras de uma função, e surge uma lista curta com as funções e [nomes](names) que começam dessa forma, com uma frase sobre cada uma. Prima **Tab** para escolher a que está marcada. Use as setas **Para baixo** e **Para cima** para marcar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Depois de entrar nos parênteses de uma função, os respetivos argumentos aparecem por baixo da célula, com o que está a escrever em negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas, ou por ponto e vírgula onde a sua língua usa a vírgula como separador decimal.

## Apontar para células

Em vez de escrever o endereço de uma célula, pode apontar para ela. Enquanto escreve uma fórmula, coloque o cursor onde pode entrar uma célula (logo a seguir a **=**, a um operador como **+**, a um parêntese de abertura ou a uma vírgula) e clique na célula. O respetivo endereço é escrito por si. Arraste sobre células para escrever um bloco, como B2:D5. Clique de novo antes de escrever outra coisa, e a nova célula substitui a que acabou de ser escrita. Mantenha **Ctrl** premido e clique para acrescentar outro endereço depois de uma vírgula. Clique numa letra de coluna para escrever uma coluna inteira, como A:A, ou num número de linha para escrever uma linha inteira, como 3:3, e arraste ao longo das letras ou dos números para B:D ou 2:5.

As teclas de seta também funcionam. Prima uma seta e um apontador afasta-se da célula que está a escrever, e o respetivo endereço é escrito; prima mais setas para avançar, ou mantenha **Shift** premido para estender o apontador sobre um bloco. A barra de estado indica **Apontar** enquanto as setas apontam, **Introduzir** enquanto escreve, e **Editar** enquanto as setas movem o cursor no texto. Prima **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula noutra folha, clique no separador dessa folha enquanto o cursor estiver onde pode entrar uma célula, e depois clique na célula. O nome da folha é escrito com ela, entre aspas quando são necessárias, como em 'A Minha Folha'!B3. **Enter** leva-o de volta à folha onde começou e coloca a fórmula na respetiva célula. **Esc** cancela tudo, incluindo a mudança de folha.

Também pode arrastar o limite de um bloco com contorno na folha para mover esse endereço, ou arrastar o quadrado do canto para o estender; a fórmula muda enquanto arrasta.

Prima **F4** com o cursor dentro ou logo a seguir a um endereço para fixar partes dele com um cifrão: A1 passa a $A$1, depois a A$1, depois a $A1, e depois de novo a A1.

Cada endereço na fórmula é colorido no texto, e as células que nomeia são assinaladas na folha com a mesma cor, para que veja de relance o que a fórmula lê. Um clique a seguir a um endereço completo, como =A1, termina a entrada e seleciona a célula em que clicou, como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de hipóteses) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir um livro que tem uma, o JeffCrunch calcula as respetivas células: numa tabela com duas entradas, a fórmula está na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada da linha, e os da vertical para a célula de entrada da coluna. Numa tabela de uma entrada, os valores ficam na coluna à esquerda (e as fórmulas na linha acima dela) ou na linha acima (e as fórmulas na coluna à esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para esse valor; nada mais no livro é alterado. No modo **Automático**, as tabelas são calculadas juntamente com as fórmulas, e as fórmulas que lêem células de uma tabela seguem-nas. Criar uma nova tabela de dados ainda não é possível. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda por si própria, como ALEATÓRIO ou AGORA) mantém os valores que o respetivo ficheiro contém.

## Calcular manualmente

Clique em **Modo de Cálculo** no separador **Fórmulas** para escolher a forma como o livro calcula as fórmulas:

- **Automático** calcula-as à medida que altera células.
- **Automático, Exceto Tabelas** faz o mesmo, mas deixa as células das tabelas de dados como estão até premir **F9**.
- **Manual** não calcula nada, incluindo as tabelas de dados, até que o peça. A barra de estado indica **Desatualizada** depois de uma alteração. Prima **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Folha**) para calcular as fórmulas da folha em que está, e tudo o que mais estava à espera. Um livro definido para Manual abre com os resultados que o seu ficheiro contém.

A escolha é guardada com o livro, e é um passo que pode anular.

## Fórmulas que se leem a si próprias

Uma fórmula que lê o seu próprio resultado, diretamente ou através de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e assinala-as na folha; essas mantêm os valores que tinham. Clique em **Referências Circulares** no separador **Fórmulas** para ver a lista, e faça duplo clique numa célula da lista para ir até lá.

Alguns modelos precisam de um ciclo deste tipo de propósito. Escolha **Iterar Fórmulas Circulares** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, lendo cada uma os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade indicada ou até se esgotar o número de iterações permitido.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a sua fórmula lê. Clique de novo para recuar mais um passo. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que lêem a célula, e cada clique avança mais um passo. Uma pequena página representa uma célula de outra folha. São desenhadas no máximo 2000 setas, e a barra de estado indica quando havia mais. **Limpar Setas** remove-as, e qualquer alteração ao livro também.

## Verificação de erros

Escolha **Verificação de Erros** no grupo **Verificar** do separador Fórmulas para procurar na folha células que possam estar erradas. Procura duas coisas: uma fórmula que devolve um erro (como #DIV/0!), e um número guardado como texto, que as fórmulas podem ignorar.

A caixa de diálogo mostra uma célula de cada vez, vai até essa célula na folha e explica numa frase qual é o problema. **Seguinte** e **Anterior** percorrem as células. Um pequeno triângulo verde no canto superior esquerdo de uma célula assinala-a na folha. **Ignorar Erro** retira a célula da lista, retira-lhe o triângulo e fica guardado no ficheiro. Um passo de Anular devolve-a à lista. Se não houver nenhum problema, é informado disso.

## Avaliar Fórmula

Use esta opção para ver como uma fórmula chega ao seu valor. Selecione uma célula que contenha uma fórmula e escolha **Avaliar Fórmula**. A fórmula é apresentada com a parte seguinte sublinhada. Prima **Avaliar** para calcular essa parte; o seu valor ocupa o seu lugar. As partes seguem esta ordem: primeiro cada célula isolada, da esquerda para a direita, depois cada função com os respetivos parênteses, começando pela mais interior, e por fim o que resta. **Reiniciar** recomeça. Se uma parte não puder ser calculada, a caixa de diálogo diz porquê e deixa a fórmula como estava.

## Janela de Vigilância

A **Janela de Vigilância** é uma pequena janela que fica visível enquanto trabalha. Lista as células que pretende acompanhar, com a respetiva folha, célula, valor e fórmula, mesmo quando estão noutra folha. Selecione células e prima **Adicionar Vigilância**. Selecione linhas na lista e prima **Eliminar Vigilância**. Faça duplo clique numa linha para ir até à célula. Os valores mudam quando a folha é calculada de novo. A lista é mantida enquanto o livro estiver aberto.
