# Como as fórmulas são mostradas e calculadas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, função, funções, valor, resultado, guardado, F9, circular, referência circular, mostrar fórmulas, barra de fórmulas, erro -->

Uma **fórmula** é uma célula que calcula o seu valor a partir de outras células, como =SOMA(B3:B8). A célula mostra o valor e a **barra de fórmulas** mostra a fórmula.

## O que o JeffCrunch faz com as fórmulas

Quando abre um livro, o ficheiro contém o último valor de cada fórmula, para que a folha possa ser mostrada de imediato. Em seguida, o JeffCrunch volta a calcular todas as fórmulas, algumas de cada vez, para que a janela continue rápida. Enquanto isso acontece, a barra de estado indica **A calcular** com uma percentagem. Quando termina, as células contêm os resultados calculados pelo próprio JeffCrunch. Para voltar a calcular tudo, clique em **Recalcular Tudo** no separador **Fórmulas** ou prima **F9**.

## Quando o JeffCrunch não consegue calcular uma fórmula

O JeffCrunch tem mais de 200 funções e continua a crescer. Uma fórmula que utilize uma função que o JeffCrunch ainda não tem mantém o valor que o ficheiro guardou para ela. O mesmo acontece com uma fórmula que se refere a outro livro. As células que dependem dessas fórmulas são calculadas a partir dos valores guardados.

## Mostrar as fórmulas

Clique em **Mostrar Fórmulas** no separador **Fórmulas** ou prima **Ctrl+`** (num Mac, a tecla Control e não a tecla Command) para ver a fórmula de cada célula em vez do respetivo valor.

## Alterar fórmulas

Para alterar uma fórmula, selecione a respetiva célula, prima **F2**, edite-a e prima **Enter**. Também pode editá-la na barra de fórmulas. Quando altera uma célula, as fórmulas que dependem dela são novamente calculadas em segundo plano. Consulte [Escrever em células](edit-cells).

## Escrever uma fórmula

Escreva **=** e as primeiras letras de uma função e é apresentada uma pequena lista com as funções e os [nomes](names) que começam dessa forma, com uma frase sobre cada um. Prima **Tab** para escolher o que está assinalado. Utilize as teclas **Seta para Baixo** e **Seta para Cima** para assinalar outro e, em seguida, prima **Tab** ou **Enter**. **Esc** fecha a lista. Quando estiver dentro dos parênteses de uma função, os respetivos argumentos são mostrados por baixo da célula, com o argumento que está a escrever a negrito. A lista e os argumentos são os mesmos que **Escolher uma Função** mostra. Os argumentos são separados por vírgulas ou, nos idiomas que escrevem a vírgula como separador decimal, por ponto e vírgula.

## Calcular manualmente

Clique em **Modo de Cálculo** no separador **Fórmulas** para escolher como o livro calcula as fórmulas:

- **Automático** calcula-as à medida que altera as células.
- **Automático, Exceto Tabelas** faz o mesmo e é mantido no ficheiro para os programas que tratam as tabelas de dados à parte.
- **Manual** não calcula nada até que o peça. Depois de uma alteração, a barra de estado indica **Desatualizado**. Prima **F9** (**Recalcular Tudo**) para calcular tudo ou **Shift+F9** (**Recalcular Esta Folha**) para calcular as fórmulas da folha em que se encontra e tudo o que estava pendente. Um livro definido como Manual abre com os resultados que o ficheiro contém.

A escolha é guardada com o livro e constitui um passo que pode anular.

## Fórmulas que se referem a si próprias

Uma fórmula que se refere ao seu próprio resultado, diretamente ou através de outras células, não pode ser calculada da forma habitual. O JeffCrunch avisa uma vez, indica as células e contorna-as na folha; estas mantêm os valores que tinham. Clique em **Referências Circulares** no separador **Fórmulas** para ver a lista e faça duplo clique numa célula da lista para ir até ela.

Alguns modelos precisam deste tipo de ciclo intencionalmente. Escolha **Iterar Fórmulas Circulares** no menu **Modo de Cálculo** e as fórmulas são calculadas repetidamente, cada uma lendo os valores mais recentes das outras, até que nenhum valor mude mais do que a quantidade indicada ou até se esgotar o número de iterações permitido.

## Ver o que uma fórmula lê

Selecione uma célula e clique em **Mostrar o Que Alimenta Esta Célula** para desenhar setas a partir das células que a respetiva fórmula lê. Clique novamente para recuar mais um passo. **Mostrar o Que Usa Esta Célula** desenha setas para as fórmulas que leem a célula e cada clique avança mais um passo. Uma pequena página representa uma célula noutra folha. São desenhadas, no máximo, 2000 setas e a barra de estado indica quando existiam mais. **Limpar Setas** remove-as, tal como qualquer alteração ao livro.
