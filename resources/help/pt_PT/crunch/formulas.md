# Como as fórmulas são mostradas e calculadas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, armazenado, F9, circular, referência circular, mostrar fórmulas, barra de fórmulas, erro, apontar, clicar numa célula, referência, referência absoluta, F4 -->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Quando abre um livro, o ficheiro contém o último valor de cada fórmula, para que a folha possa ser mostrada de imediato. Depois, o JeffCrunch volta a calcular todas as fórmulas, algumas de cada vez, para que a janela continue rápida. Enquanto isso acontece, a barra de estado indica **A calcular** com uma percentagem. Quando termina, as células contêm os resultados calculados pelo próprio JeffCrunch. Para voltar a calcular tudo, clique em **Recalcular Tudo** no separador **Fórmulas** ou prima **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 200 funções e continua a crescer. Uma fórmula que utilize uma função que o JeffCrunch ainda não tenha mantém o valor que o ficheiro guardou para ela. O mesmo acontece com uma fórmula que se refira a outro livro. As células que dependem dessas fórmulas são calculadas a partir dos valores guardados.

## RESTO: o resto da divisão

**RESTO(n; d)** devolve o que sobra de **n** depois de retirar o maior número possível de vezes inteiras **d**: é n - d * INT(n / d). Exemplo: **=RESTO(10; 3)** é 1 e **=RESTO(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** no separador **Fórmulas** ou prima **Ctrl+`** (num Mac, a tecla Control, não a tecla Command) para ver a fórmula de cada célula em vez do respetivo valor. As colunas que contêm fórmulas ficam mais largas, até um limite, para que as fórmulas sejam mostradas por inteiro; quando desativa esta opção, voltam às larguras anteriores, e as larguras guardadas no ficheiro não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a respetiva célula, prima **F2**, edite-a e prima **Enter**. Também pode editá-la na barra de fórmulas. Quando altera uma célula, as fórmulas que dependem dela são novamente calculadas em segundo plano. Consulte [Escrever em células](edit-cells).

## Escrever uma fórmula

Escreva **=** e as primeiras letras de uma função, e uma pequena lista mostra as funções e os [nomes](names) que começam dessa forma, com uma frase sobre cada um. Prima **Tab** para escolher o que está assinalado. Utilize as teclas **Seta para Baixo** e **Seta para Cima** para assinalar outro e, em seguida, prima **Tab** ou **Enter**. **Esc** fecha a lista. Quando estiver dentro dos parênteses de uma função, os respetivos argumentos são mostrados por baixo da célula, com o argumento que está a escrever a negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas ou, nos idiomas que utilizam a vírgula como separador decimal (como o português), por ponto e vírgula.

## Apontar para células

Em vez de escrever o endereço de uma célula, pode apontar para ela. Enquanto escreve uma fórmula, coloque o cursor num local onde possa ficar uma célula (logo a seguir a **=**, a um operador como **+**, a um parêntese de abertura ou a um separador de argumentos) e clique na célula. O endereço é escrito automaticamente. Arraste sobre várias células para escrever um bloco como B2:D5. Se clicar novamente antes de escrever qualquer outra coisa, a nova célula substitui a que acabou de ser escrita. Mantenha premida a tecla **Ctrl** e clique para adicionar outro endereço a seguir a um separador. Clique na letra de uma coluna para escrever uma coluna inteira, como A:A, ou no número de uma linha para escrever uma linha inteira, como 3:3, e arraste ao longo das letras ou números para obter B:D ou 2:5.

As teclas de seta também funcionam. Prima uma seta e um ponteiro afasta-se da célula onde está a escrever, sendo escrito o respetivo endereço; prima mais setas para continuar a mover o ponteiro ou mantenha premida a tecla **Shift** para alargar o ponteiro a um bloco. A barra de estado indica **Apontar** enquanto as teclas de seta apontam, **Introduzir** enquanto escreve e **Editar** enquanto as teclas de seta movem o cursor no texto. Prima **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula noutra folha, clique no separador dessa folha enquanto o cursor está num local onde possa ficar uma célula e, em seguida, clique na célula. O nome da folha é escrito com ela, entre plicas quando necessário, como em 'A Minha Folha'!B3. **Enter** leva-o de volta à folha onde começou e coloca a fórmula na respetiva célula. **Esc** cancela tudo, incluindo a mudança de folha.

Também pode arrastar o limite de um bloco destacado na folha para mover esse endereço ou arrastar o pequeno quadrado do canto para o alargar; a fórmula vai mudando à medida que arrasta.

Prima **F4** com o cursor dentro ou logo a seguir a um endereço para fixar partes dele com um cifrão: A1 passa a $A$1, depois a A$1, depois a $A1 e, por fim, novamente a A1.

Cada endereço da fórmula é colorido no texto e as células correspondentes são destacadas na folha com a mesma cor, para que possa ver rapidamente o que a fórmula lê. Um clique depois de um endereço completo, como =A1, termina a introdução e seleciona a célula em que clicou, tal como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de hipóteses) mostra o resultado de uma fórmula para vários valores de uma ou duas células. Abra um livro que contenha uma, e o JeffCrunch calcula as respetivas células: numa tabela com duas entradas, a fórmula está na célula do canto superior esquerdo da tabela, os valores a experimentar na horizontal vão para a célula de entrada de linha e os valores na vertical vão para a célula de entrada de coluna. Numa tabela com uma entrada, os valores ficam na coluna à esquerda da tabela (e as fórmulas na linha acima) ou na linha acima da tabela (e as fórmulas na coluna à esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para esse valor; nada mais no livro é alterado. No modo **Automático**, as tabelas são calculadas juntamente com as fórmulas, e as fórmulas que leem as células de uma tabela são calculadas a seguir. Ainda não é possível criar uma nova tabela de dados. Uma tabela que não possa ser calculada (porque uma fórmula que depende da entrada utiliza uma função que o JeffCrunch não tem, ou muda por si só, como ALEATÓRIO ou AGORA) mantém os valores guardados no ficheiro.

## Calcular manualmente

Clique em **Modo de Cálculo** no separador **Fórmulas** para escolher como o livro calcula as fórmulas:

- **Automático** calcula-as à medida que altera células.
- **Automático, Exceto Tabelas** faz o mesmo, mas deixa as células das tabelas de dados como estão até premir **F9**.
- **Manual** não calcula nada, incluindo as tabelas de dados, até que o peça. Após uma alteração, a barra de estado indica **Desatualizado**. Prima **F9** (**Recalcular Tudo**) para calcular tudo ou **Shift+F9** (**Recalcular Esta Folha**) para calcular as fórmulas da folha atual e tudo o que estava pendente. Um livro definido como Manual abre com os resultados guardados no ficheiro.

A opção é guardada com o livro e constitui um passo que pode anular.

## Fórmulas que se referem a si próprias

Uma fórmula que lê o seu próprio resultado, diretamente ou através de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa-o uma vez, indica as células e destaca-as na folha; essas células mantêm os valores que tinham. Clique em **Referências Circulares** no separador **Fórmulas** para ver a lista e faça duplo clique numa célula da lista para ir até ela.

Alguns modelos precisam de um ciclo destes propositadamente. Escolha **Iterar Fórmulas Circulares** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, cada uma lendo os valores mais recentes das outras, até nenhum valor mudar mais do que o valor que indicar ou até ser atingido o número de iterações permitido.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a fórmula lê. Clique novamente para recuar mais um nível. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que leem a célula, e cada clique avança mais um nível. Uma pequena página representa uma célula noutra folha. São desenhadas, no máximo, 2000 setas, e a barra de estado indica quando existiam mais. **Limpar Setas** remove-as, tal como qualquer alteração ao livro.
