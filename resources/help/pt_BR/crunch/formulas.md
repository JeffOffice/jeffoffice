# Como as fórmulas são mostradas e calculadas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, armazenado, F9, circular, referência circular, mostrar fórmulas, barra de fórmulas, erro -->

Uma **fórmula** é uma célula que calcula seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Quando você abre uma pasta de trabalho, o arquivo contém o último valor de cada fórmula, para que a planilha possa ser mostrada imediatamente. Em seguida, o JeffCrunch calcula novamente todas as fórmulas, algumas de cada vez, para que a janela continue respondendo rápido. Enquanto isso, a barra de status mostra **Calculando** com uma porcentagem. Ao terminar, as células contêm os resultados calculados pelo próprio JeffCrunch. Para calcular tudo novamente, clique em **Recalcular Tudo** na guia **Fórmulas** ou pressione **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 200 funções, e esse número continua crescendo. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor armazenado no arquivo. O mesmo acontece com uma fórmula que faz referência a outra pasta de trabalho. As células que dependem dessas fórmulas são calculadas a partir dos valores armazenados.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** na guia **Fórmulas** ou pressione **Ctrl+`** (no Mac, a tecla Control, não a tecla Command) para ver a fórmula de cada célula no lugar do valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas apareçam inteiras; elas voltam às larguras originais quando você desativa a opção, e as larguras no arquivo não são alteradas.

## Alterar fórmulas

Para alterar uma fórmula, selecione a célula, pressione **F2**, edite a fórmula e pressione **Enter**. Você também pode editá-la na barra de fórmulas. Quando você altera uma célula, as fórmulas que dependem dela são calculadas novamente em segundo plano. Consulte [Digitar em células](edit-cells).

## Escrever uma fórmula

Digite **=** e as primeiras letras de uma função, e uma pequena lista mostrará as funções e os [nomes](names) que começam assim, com uma frase sobre cada um. Pressione **Tab** para usar o item marcado. Use as setas **para baixo** e **para cima** para marcar outro item e pressione **Tab** ou **Enter**. **Esc** fecha a lista. Quando você estiver dentro dos parênteses de uma função, os argumentos dela aparecem abaixo da célula, com o argumento que você está digitando em negrito. A lista e os argumentos são os mesmos mostrados em **Escolher uma Função**. Os argumentos são separados por vírgulas ou, nos idiomas que usam a vírgula como separador decimal, por ponto e vírgula.

## Tabelas de dados

Uma tabela de dados (tabela de hipóteses) mostra o resultado de uma fórmula para vários valores de uma ou duas células. Ao abrir uma pasta de trabalho que tenha uma tabela de dados, o JeffCrunch calcula as células dela: em uma tabela com duas entradas, a fórmula fica na célula do canto superior esquerdo da tabela, os valores a testar na horizontal vão para a célula de entrada da linha, e os valores na vertical vão para a célula de entrada da coluna. Em uma tabela com uma entrada, os valores ficam na coluna à esquerda da tabela (e as fórmulas, na linha acima dela) ou na linha acima da tabela (e as fórmulas, na coluna à esquerda dela). Cada célula da tabela é o resultado da fórmula, e de todas as fórmulas que dependem da célula de entrada, com a entrada definida como aquele valor; nada mais na pasta de trabalho é alterado. No modo **Automático**, as tabelas são calculadas junto com as fórmulas, e as fórmulas que leem as células de uma tabela são calculadas em seguida. Ainda não é possível criar uma nova tabela de dados. Uma tabela que não pode ser calculada (porque uma fórmula que depende da entrada usa uma função que o JeffCrunch não tem, ou muda por conta própria, como ALEATÓRIO ou AGORA) mantém os valores contidos no arquivo.

## Calcular manualmente

Clique em **Modo de Cálculo** na guia **Fórmulas** para escolher como a pasta de trabalho calcula as fórmulas:

- **Automático** calcula as fórmulas à medida que você altera as células.
- **Automático, Exceto Tabelas** faz o mesmo, mas mantém as células das tabelas de dados como estão até você pressionar **F9**.
- **Manual** não calcula nada, nem as tabelas de dados, até você pedir. Após uma alteração, a barra de status mostra **Desatualizado**. Pressione **F9** (**Recalcular Tudo**) para calcular tudo ou **Shift+F9** (**Recalcular Esta Planilha**) para calcular as fórmulas da planilha atual e tudo o que estava pendente. Uma pasta de trabalho definida como Manual é aberta com os resultados contidos no arquivo.

A escolha é salva com a pasta de trabalho e pode ser desfeita como uma única etapa.

## Fórmulas que fazem referência a si mesmas

Uma fórmula que lê o próprio resultado, diretamente ou por meio de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, informa quais são as células e as contorna na planilha; elas mantêm os valores que tinham. Clique em **Referências Circulares** na guia **Fórmulas** para ver a lista e clique duas vezes em uma célula da lista para ir até ela.

Alguns modelos precisam desse tipo de ciclo de propósito. Escolha **Calcular Fórmulas Circulares por Iteração** no menu **Modo de Cálculo**, e as fórmulas serão calculadas repetidamente, cada uma lendo os valores mais recentes das outras, até que nenhum valor mude mais que a quantidade informada ou até que o número de iterações permitido seja atingido.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a fórmula lê. Clique novamente para recuar mais um nível. **Mostrar o Que Usa Esta Célula** desenha setas até as fórmulas que leem a célula, e cada clique avança um nível. Uma pequena página representa uma célula em outra planilha. São desenhadas no máximo 2.000 setas, e a barra de status avisa quando havia mais. **Remover Setas** apaga as setas, assim como qualquer alteração na pasta de trabalho.
