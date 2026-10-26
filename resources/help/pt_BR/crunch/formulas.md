# Como as fórmulas são mostradas e calculadas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, armazenado, F9, circular, referência circular, mostrar fórmulas, barra de fórmulas, erro -->

Uma **fórmula** é uma célula que calcula o próprio valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor, e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com elas

Quando você abre uma pasta de trabalho, o arquivo contém o último valor de cada fórmula, para que a planilha possa ser mostrada imediatamente. Em seguida, o JeffCrunch calcula novamente todas as fórmulas, algumas de cada vez, para que a janela continue respondendo rapidamente. Enquanto isso, a barra de status mostra **Calculando** com uma porcentagem. Quando termina, as células contêm os resultados calculados pelo próprio JeffCrunch. Para calcular tudo novamente, clique em **Recalcular Tudo** na guia **Fórmulas** ou pressione **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 200 funções, e esse número continua crescendo. Uma fórmula que usa uma função que o JeffCrunch ainda não tem mantém o valor armazenado no arquivo. O mesmo acontece com uma fórmula que faz referência a outra pasta de trabalho. As células que dependem delas são calculadas a partir dos valores armazenados.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** na guia **Fórmulas** ou pressione **Ctrl+`** (no Mac, a tecla Control, não a tecla Command) para ver a fórmula de cada célula no lugar do valor.

## Alterar fórmulas

Para alterar uma fórmula, selecione a célula, pressione **F2**, edite a fórmula e pressione **Enter**. Você também pode editá-la na barra de fórmulas. Quando você altera uma célula, as fórmulas que dependem dela são calculadas novamente em segundo plano. Consulte [Digitar em células](edit-cells).

## Escrever uma fórmula

Digite **=** e as primeiras letras de uma função para que uma pequena lista mostre as funções e os [nomes](names) que começam dessa forma, com uma frase sobre cada um. Pressione **Tab** para usar o item marcado. Use as setas **Para Baixo** e **Para Cima** para marcar outro item e pressione **Tab** ou **Enter**. **Esc** fecha a lista. Quando você estiver dentro dos parênteses de uma função, os argumentos dela aparecem abaixo da célula, com o argumento que você está digitando em negrito. A lista e os argumentos são os mesmos mostrados por **Escolher uma Função**. Os argumentos são separados por vírgulas, ou por ponto e vírgula nos idiomas que usam vírgula como separador decimal.

## Calcular manualmente

Clique em **Modo de Cálculo** na guia **Fórmulas** para escolher como a pasta de trabalho calcula as fórmulas:

- **Automático** calcula as fórmulas à medida que você altera as células.
- **Automático, Exceto Tabelas** faz o mesmo e fica registrado no arquivo para os programas que tratam as tabelas de dados separadamente.
- **Manual** não calcula nada até você pedir. A barra de status mostra **Desatualizado** após uma alteração. Pressione **F9** (**Recalcular Tudo**) para calcular tudo ou **Shift+F9** (**Recalcular Esta Planilha**) para calcular as fórmulas da planilha atual e tudo o que estava pendente. Uma pasta de trabalho definida como Manual é aberta com os resultados contidos no arquivo.

A escolha é salva com a pasta de trabalho e pode ser desfeita como uma única etapa.

## Fórmulas que fazem referência a si mesmas

Uma fórmula que lê o próprio resultado, diretamente ou por meio de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, informa quais são as células e as contorna na planilha; elas mantêm os valores que tinham. Clique em **Referências Circulares** na guia **Fórmulas** para ver a lista e clique duas vezes em uma célula da lista para ir até ela.

Alguns modelos precisam desse tipo de ciclo de propósito. Escolha **Calcular Fórmulas Circulares por Iteração** no menu **Modo de Cálculo** para que as fórmulas sejam calculadas repetidamente, cada uma lendo os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade que você definir ou até que o número de iterações permitido se esgote.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células lidas pela fórmula. Clique novamente para voltar mais um nível. **Mostrar o Que Usa Esta Célula** desenha setas até as fórmulas que leem a célula, e cada clique avança mais um nível. Uma pequena página representa uma célula de outra planilha. São desenhadas no máximo 2.000 setas, e a barra de status avisa quando havia mais. **Remover Setas** remove as setas, assim como qualquer alteração na pasta de trabalho.
