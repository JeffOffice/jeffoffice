# Como as fórmulas são exibidas e calculadas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, armazenado, F9, circular, mostrar fórmulas, barra de fórmulas, erro, apontar, apontando, clicar em uma célula, referência, F4, verificação de erros, ignorar erro, avaliar fórmula, etapa, inspeção, inspecionar, janela de inspeção, watch -->

Uma **fórmula** é uma célula que calcula seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Ao abrir uma pasta de trabalho, o arquivo guarda o último valor de cada fórmula, para que a planilha possa ser exibida de imediato. Em seguida, o JeffCrunch calcula todas as fórmulas novamente, poucas de cada vez, para que a janela continue rápida. A barra de status mostra **Calculando** com uma porcentagem enquanto isso acontece. Quando termina, as células têm os resultados calculados pelo próprio JeffCrunch. Para calcular tudo novamente, clique em **Recalcular Tudo** na guia **Fórmulas** ou pressione **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 400 funções, e a lista continua crescendo. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor que o arquivo guardou para ela. O mesmo acontece com uma fórmula que se refere a outra pasta de trabalho. As células que dependem dessas fórmulas são calculadas a partir dos valores armazenados.

## Funções que precisam de algumas observações

- **Títulos e empréstimos.** PREÇO, RENDIMENTO, DURAÇÃO, PREÇOPRIMINC, e as demais funções de títulos contam os dias conforme a **base** informada (0 é 30 dias por mês em um ano de 360 dias, 1 são os dias reais, e assim por diante). RENDIMENTO, RENDTITPRIMINC, RENDTITULTINC e TAXA encontram a resposta testando valores até que o preço ou o pagamento coincida. Elas param quando a resposta está correta com cerca de 15 dígitos, por isso os últimos dígitos podem ser diferentes de os de outro programa.
- **Distribuições e testes.** As funções que terminam em .DIST, .INV, .TESTE e os nomes mais antigos, como DISTNORM e DISTT, dão as mesmas respostas: um nome antigo é mantido para que os arquivos antigos continuem funcionando. As funções *.INV também são encontradas testando valores.
- **Listas que preenchem várias células.** CLASSIFICAR, CLASSIFICARPOR, FILTRO, EXCLUSIVO, SEQUÊNCIA, ALEATMATRIZ, PROCX, MMULT, MATRIZINVERSA e PROJ.LIN dão um bloco de valores. Digite-as como fórmula de matriz (**Ctrl+Shift+Enter**) sobre as células que devem preencher. Fazer uma fórmula se espalhar automaticamente para as células vizinhas ainda não é possível.
- **Textos em outros sistemas de escrita.** LOCALIZARB, ESQUERDAB, EXTRAIRB, DIREITAB, LOCALIZARB, PROCURARB e SUBSTITUIRB contam cada caractere como um, como fazem em um computador que não está configurado para texto de dois bytes. FONÉTICO dá o texto da célula, porque o JeffCrunch não guarda as leituras de textos em japonês.
- **CONVERTER.** As unidades são escritas como texto, com maiúsculas e minúsculas como aparecem na lista da função: "kg", "lbm", "mi", "F". Um prefixo métrico pode ser colocado antes de uma unidade que aceita prefixo, como "km" ou "mL".

## MOD: o resto

**MOD(n; d)** dá o que sobra de **n** depois que são retiradas as cópias inteiras de **d** possíveis: é n - d * INT(n / d). Exemplo: **=MOD(10; 3)** é 1, e **=MOD(-10; 3)** é 2, porque INT(-10 / 3) é -4 e -10 - 3 * -4 é 2. O resultado tem o sinal de **d**. Se **d** for 0, o resultado é #DIV/0!.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** na guia **Fórmulas**, ou pressione **Ctrl+'** (no Mac, a tecla Control, e não a tecla Command), para ver a fórmula de cada célula no lugar do valor. As colunas que têm fórmulas ficam mais largas, até um limite, para que as fórmulas apareçam por inteiro; elas voltam às larguras originais quando você desliga a opção, e as larguras do seu arquivo não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a célula, pressione **F2**, edite e pressione **Enter**. Você também pode editá-la na barra de fórmulas. Quando você altera uma célula, as fórmulas que dependem dela são calculadas novamente em segundo plano. Veja [Digitar em células](edit-cells).

## Escrever uma fórmula

Digite **=** e as primeiras letras de uma função; uma lista curta mostra as funções e [nomes](names) que começam assim, com uma frase sobre cada uma. Pressione **Tab** para escolher a que está marcada. Use as setas **Para Baixo** e **Para Cima** para marcar outra, e depois **Tab** ou **Enter**. **Esc** fecha a lista. Quando você está dentro dos parênteses de uma função, os argumentos dela aparecem abaixo da célula, com o que você está digitando em negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas, ou por ponto e vírgula quando o seu idioma usa a vírgula como separador decimal.

## Apontar para células

Em vez de digitar o endereço de uma célula, você pode apontar para ela. Enquanto digita uma fórmula, coloque o cursor onde pode entrar uma célula (logo depois de **=**, de um operador como **+**, de um parêntese de abertura ou de uma vírgula) e clique na célula. O endereço é escrito para você. Arraste sobre as células para escrever um intervalo como B2:D5. Clique de novo antes de digitar qualquer outra coisa, e a nova célula substitui a que acabou de ser escrita. Segure **Ctrl** e clique para acrescentar outro endereço depois de uma vírgula. Clique em uma letra de coluna para escrever uma coluna inteira, como A:A, ou em um número de linha para escrever uma linha inteira, como 3:3, e arraste pelas letras ou pelos números para B:D ou 2:5.

As teclas de seta também funcionam. Pressione uma seta e um ponteiro se afasta da célula em que você está digitando, e o endereço é escrito; pressione mais setas para continuar, ou segure **Shift** para estender o ponteiro sobre um intervalo. A barra de status mostra **Apontar** enquanto as setas apontam, **Inserir** enquanto você digita e **Editar** quando as setas movem o cursor no texto. Pressione **F2** para alternar entre apontar e editar o texto.

Para apontar para uma célula de outra planilha, clique na guia dessa planilha enquanto o cursor estiver onde pode entrar uma célula, e depois clique na célula. O nome da planilha é escrito junto, entre aspas quando necessário, como em 'Minha Planilha'!B3. **Enter** leva você de volta à planilha de onde começou e coloca a fórmula na célula. **Esc** cancela tudo, inclusive a troca de planilha.

Você também pode arrastar a borda de um bloco com contorno na planilha para mover esse endereço, ou arrastar seu quadradinho de canto para esticá-lo; a fórmula muda enquanto você arrasta.

Pressione **F4** com o cursor dentro de um endereço ou logo depois dele para fixar partes dele com um cifrão: A1 vira $A$1, depois A$1, depois $A1, e depois A1 de novo.

Cada endereço da fórmula é colorido no texto, e as células que ele menciona ganham um contorno na planilha na mesma cor, para que você veja de relance o que a fórmula lê. Um clique depois de um endereço completo, como =A1, encerra a entrada e seleciona a célula em que você clicou, como acontece com qualquer outro texto.

## Tabelas de dados

Uma tabela de dados (tabela de simulação) mostra o resultado de uma fórmula para muitos valores de uma ou duas células. Ao abrir uma pasta de trabalho que tenha uma, o JeffCrunch calcula suas células: em uma tabela com duas entradas, a fórmula fica na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada de linha, e os da vertical vão para a célula de entrada de coluna. Em uma tabela de uma entrada, os valores ficam na coluna à esquerda (e as fórmulas ficam na linha acima dela) ou na linha acima (e as fórmulas ficam na coluna à esquerda). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida para aquele valor; nada mais na pasta de trabalho é alterado. No modo **Automático**, as tabelas são calculadas junto com as fórmulas, e as fórmulas que leem as células de uma tabela são atualizadas. Criar uma nova tabela de dados ainda não é oferecido. Uma tabela que não pode ser calculada (uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda sozinha, como ALEATÓRIO ou AGORA) mantém os valores que o arquivo contém.

## Calcular manualmente

Clique em **Modo de Cálculo** na guia **Fórmulas** para escolher como a pasta de trabalho calcula suas fórmulas:

- **Automático** calcula as fórmulas à medida que você altera as células.
- **Automático, Exceto Tabelas** faz o mesmo, mas deixa as células das tabelas de dados como estão até você pressionar **F9**.
- **Manual** não calcula nada, inclusive as tabelas de dados, até que você peça. A barra de status mostra **Desatualizado** depois de uma alteração. Pressione **F9** (**Recalcular Tudo**) para calcular tudo, ou **Shift+F9** (**Recalcular Esta Planilha**) para calcular as fórmulas da planilha em que você está, e o que mais estiver pendente. Uma pasta de trabalho definida como Manual abre com os resultados que o arquivo contém.

A escolha é salva com a pasta de trabalho, e é uma etapa que você pode desfazer.

## Fórmulas que leem a si mesmas

Uma fórmula que lê o próprio resultado, diretamente ou por meio de outras células, não pode ser calculada da maneira usual. O JeffCrunch avisa uma vez, indica as células e as destaca com contorno na planilha; elas mantêm os valores que tinham. Clique em **Referências Circulares** na guia **Fórmulas** para ver a lista, e clique duas vezes em uma célula dela para ir até lá.

Alguns modelos precisam de um ciclo assim de propósito. Escolha **Calcular Fórmulas Circulares por Iteração** no menu **Modo de Cálculo**, e as fórmulas são calculadas repetidamente, cada uma lendo os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade informada ou que o número de rodadas permitido seja usado.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar O Que Alimenta Esta Célula** para desenhar setas a partir das células que a fórmula lê. Clique de novo para voltar mais um passo. **Mostrar O Que Usa Esta Célula** desenha setas até as fórmulas que leem a célula, e cada clique avança um passo. Uma pequena página representa uma célula de outra planilha. São desenhadas no máximo 2.000 setas, e a barra de status informa quando havia mais. **Remover Setas** as apaga, e qualquer alteração na pasta de trabalho também as apaga.

## Verificação de erros

Escolha **Verificação de Erros** no grupo **Verificar** da guia Fórmulas para procurar na planilha células que podem estar erradas. Ela procura duas coisas: uma fórmula que resulta em erro (como #DIV/0!) e um número guardado como texto, que as fórmulas podem ignorar.

A caixa de diálogo mostra uma célula por vez, vai até essa célula na planilha e explica em uma frase qual é o problema. **Próximo** e **Anterior** percorrem as células. Um pequeno triângulo verde no canto superior esquerdo de uma célula a marca na planilha. **Ignorar Erro** tira a célula da lista, remove o triângulo dela e fica salvo no arquivo. Uma etapa de Desfazer a traz de volta. Se não houver nenhum problema, você é avisado.

## Avaliar fórmula

Use esta opção para ver como uma fórmula chega ao seu valor. Selecione uma célula que tenha uma fórmula e escolha **Avaliar Fórmula**. A fórmula é mostrada com a próxima parte sublinhada. Pressione **Avaliar** para calcular essa parte; o valor dela ocupa o lugar. As partes seguem esta ordem: cada célula isolada da esquerda para a direita, depois cada função com seus parênteses, começando pela mais interna, e por fim o que restar. **Reiniciar** começa de novo. Se uma parte não puder ser calculada, a caixa de diálogo diz o motivo e deixa a fórmula como estava.

## Janela de Inspeção

A **Janela de Inspeção** é uma pequena janela que permanece visível enquanto você trabalha. Ela lista as células que você quer acompanhar, com a planilha, a célula, o valor e a fórmula delas, mesmo quando estão em outra planilha. Selecione células e pressione **Adicionar Inspeção**. Selecione linhas na lista e pressione **Excluir Inspeção**. Clique duas vezes em uma linha para ir até a célula. Os valores mudam quando a planilha é calculada novamente. A lista é mantida enquanto a pasta de trabalho estiver aberta.
