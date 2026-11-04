# Como as fórmulas são exibidas e calculadas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, armazenado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar em uma célula, referência, F4 -->

Uma **fórmula** é uma célula que calcula seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Ao abrir uma pasta de trabalho, o arquivo contém o último valor de cada fórmula, para que a planilha possa ser exibida imediatamente. Em seguida, o JeffCrunch calcula todas as fórmulas novamente, poucas de cada vez, para que a janela continue rápida. A barra de status mostra **Calculando** com uma porcentagem enquanto isso acontece. Quando termina, as células contêm os resultados do próprio JeffCrunch. Para calcular tudo novamente, clique em **Recalcular Tudo** na guia **Fórmulas** ou pressione **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e continua crescendo. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o arquivo armazenou para ela. O mesmo acontece com uma fórmula que se refere a outra pasta de trabalho. As células que dependem dessas são calculadas a partir dos valores armazenados.

## Funções que precisam de algumas observações

- **Títulos e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMINC, e as demais funções de títulos contam os dias conforme a **base** que você informar (0 é 30 dias por mês em um ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDIMENTOPRIMINC, RENDIMENTOULTINC e TAXA encontram sua resposta testando valores até que o preço ou o pagamento correspondam. Elas param quando a resposta está correta com cerca de 15 dígitos, então os últimos dígitos podem diferir de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE e os nomes mais antigos, como DISTNORM e DISTT, dão as mesmas respostas: um nome antigo é mantido para que arquivos antigos continuem funcionando. As funções *.INV também são encontradas testando valores.
- **Listas que preenchem várias células.** SEQUÊNCIA, ALEATMATRIZ, CLASSIFICAR, CLASSIFICARPOR, FILTRAR, EXCLUIR.DUPLICATAS, PROCURARX, MMULT, MINVERSA e PROJ.LIN dão um bloco de valores. Insira-as como fórmula de matriz (**Ctrl+Shift+Enter**) sobre as células que devem preencher. Deixar uma fórmula se espalhar pelas células vizinhas ainda não é feito.
- **Textos em outros alfabetos.** ÀDIREITAB, ... (ver abaixo) ...

Corrigindo a linha acima, o trecho correto é:

- **Textos em outros alfabetos.** BDESLOCB, ESQUERDAB, EXT.TEXTOB, DIREITAB, LOCALIZARB, PROCURARB e SUBSTITUIRB contam cada caractere como um, como ocorre em um computador que não está configurado para textos de byte duplo. Os nomes com a letra B (como BDESLOCB) não são usados aqui: a função que conta bytes de um texto é **TAMANHOB**. PHONETIC dá o texto da célula, porque o JeffCrunch não guarda as leituras de textos em japonês.

Observação: a função que conta bytes é **TAMANHOB**, e as demais funções de byte duplo que o JeffCrunch oferece são ESQUERDAB, EXT.TEXTOB, DIREITAB, LOCALIZARB, PROCURARB, SUBSTITUIRB e DBCS, todas contando cada caractere como um.

- **CONVERTER.** As unidades são escritas como texto, com maiúsculas e minúsculas como mostrado na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode ser colocado antes de uma unidade que aceita prefixo, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** dá o que sobra de **n** depois que o maior número inteiro de vezes de **d** é retirado: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** na guia **Fórmulas**, ou pressione **Ctrl+'** (no Mac, a tecla Control, e não a tecla Command), para ver a fórmula de cada célula no lugar do valor. As colunas que contêm fórmulas ficam mais largas, até um limite, para que as fórmulas apareçam por inteiro; elas voltam às larguras originais quando você desliga a opção, e as larguras do seu arquivo não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a célula, pressione **F2**, edite e pressione **Enter**. Você também pode editá-la na barra de fórmulas. Quando você altera uma célula, as fórmulas que dependem dela são calculadas novamente em segundo plano. Veja [Digitar em células](edit-cells).

## Escrever uma fórmula

Digite **=** e as primeiras letras de uma função, e uma lista curta mostra as funções e [nomes](names) que começam assim, com uma frase sobre cada uma. Pressione **Tab** para escolher a que está marcada. Use as setas **Para Baixo** e **Para Cima** para marcar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando você está dentro dos parênteses de uma função, seus argumentos aparecem abaixo da célula, com o que você está digitando em negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas, ou por ponto e vírgula onde o seu idioma usa vírgula como separador decimal.

## Apontar para células

Em vez de digitar o endereço de uma célula, você pode apontar para ela. Enquanto digita uma fórmula, posicione o cursor onde pode entrar uma célula (logo depois de **=**, de um operador como **+**, de um parêntese de abertura ou de uma vírgula) e clique na célula. O endereço é escrito para você. Arraste sobre células para escrever um intervalo como B2:D5. Clique de novo antes de digitar qualquer outra coisa, e a nova célula substitui a que acabou de ser escrita. Mantenha **Ctrl** pressionada e clique para acrescentar outro endereço depois de uma vírgula. Clique em uma letra de coluna para escrever uma coluna inteira, como A:A, ou em um número de linha para escrever uma linha inteira, como 3:3, e arraste pelas letras ou números para B:D ou 2:5.

As teclas de seta também funcionam. Pressione uma seta e um ponteiro se afasta da célula em que você está digitando, e o endereço é escrito; pressione mais setas para continuar, ou mantenha **Shift** pressionada para estender o ponteiro sobre um intervalo. A barra de status mostra **Apontar** enquanto as setas apontam, **Enter** enquanto você digita, e **Editar** quando as setas movem o cursor no texto. Pressione **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula em outra planilha, clique na guia dessa planilha enquanto o cursor estiver onde pode entrar uma célula e, depois, clique na célula. O nome da planilha é escrito junto, entre aspas quando necessário, como em 'Minha Planilha'!B3. **Enter** leva você de volta à planilha de onde começou e coloca a fórmula em sua célula. **Esc** cancela tudo, inclusive a troca de planilha.

Você também pode arrastar a borda de um bloco com contorno na planilha para mover esse endereço, ou arrastar seu quadradinho de canto para esticá-lo; a fórmula muda enquanto você arrasta.

Pressione **F4** com o cursor dentro de um endereço ou logo depois dele para fixar partes dele com um cifrão: A1 vira $A$1, depois A$1, depois $A1, e depois A1 de novo.

Cada endereço da fórmula é colorido no texto, e as células que ele referencia são destacadas com contorno na planilha, na mesma cor, para que você veja de relance o que a fórmula lê. Um clique logo depois de um endereço completo, como =A1, encerra a entrada e seleciona a célula em que você clicou, como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de simulação) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir uma pasta de trabalho que tenha uma, o JeffCrunch calcula suas células: para uma tabela com duas entradas, a fórmula fica na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada de linha, e os da vertical vão para a célula de entrada de coluna. Para uma tabela de uma entrada, os valores ficam na coluna à esquerda (e as fórmulas ficam na linha acima) ou na linha acima (e as fórmulas ficam na coluna à esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para aquele valor; nada mais na pasta de trabalho é alterado. No modo **Automático**, as tabelas são calculadas junto com as fórmulas, e as fórmulas que leem as células de uma tabela acompanham o resultado. Criar uma nova tabela de dados ainda não é oferecido. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda sozinha, como ALEATÓRIO ou AGORA) mantém os valores armazenados no arquivo.

## Calcular manualmente

Clique em **Modo de Cálculo** na guia **Fórmulas** para escolher como a pasta de trabalho calcula suas fórmulas:

- **Automático** calcula as fórmulas à medida que você altera as células.
- **Automático, Tabelas Manuais** faz o mesmo, mas deixa as células das tabelas de dados como estão até que você pressione **F9**.
- **Manual** não calcula nada, inclusive as tabelas de dados, até que você peça. A barra de status mostra **Desatualizado** depois de uma alteração. Pressione **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Planilha**) para calcular as fórmulas da planilha em que você está, e o que mais estiver pendente. Uma pasta de trabalho definida como Manual abre com os resultados que o arquivo contém.

A escolha é salva com a pasta de trabalho, e é uma etapa que você pode desfazer.

## Fórmulas que leem a si mesmas

Uma fórmula que lê o próprio resultado, diretamente ou por meio de outras células, não pode ser calculada da maneira usual. O JeffCrunch avisa uma vez, indica as células e as destaca com contorno na planilha; elas mantêm os valores que tinham. Clique em **Referências Circulares** na guia **Fórmulas** para ver a lista, e clique duas vezes em uma célula dela para ir até ela.

Alguns modelos precisam de uma circularidade de propósito. Escolha **Calcular Fórmulas Circulares por Iteração** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, cada uma lendo os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade que você informar ou até que o número de rodadas permitido seja usado.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas das células que a fórmula lê. Clique de novo para voltar mais um passo. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que leem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra planilha. São desenhadas no máximo 2.000 setas, e a barra de status informa quando havia mais. **Limpar Setas** as remove, e qualquer alteração na pasta de trabalho também as remove.
