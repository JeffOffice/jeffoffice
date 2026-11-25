# Análise Rápida
<!-- keywords: análise rápida, analisar, análise, gráfico instantâneo, gráfico rápido, totais rápidos, totais, total acumulado, percentual do total, barras de dados, escala de cores, conjunto de ícones, minigráficos, tabela, tabela dinâmica, Ctrl+Q, visualizar, somar, soma -->

A **Análise Rápida** reúne em uma pequena caixa as maneiras mais comuns de analisar um bloco de números: colorir as células, criar um gráfico com elas, totalizá-las, transformá-las em uma tabela ou dar a cada linha um pequeno gráfico. Você escolhe o que quer e vê o resultado na sua planilha antes de decidir.

## Abrir a Análise Rápida

1. Selecione duas ou mais células que tenham algum conteúdo. Um pequeno botão aparece logo abaixo e à direita da última célula selecionada.
2. Clique no botão, pressione **Ctrl+Q** (no Mac, **Control+Q**) ou clique em **Página Inicial > Edição > Análise Rápida**.
3. Escolha uma guia e, em seguida, uma opção.

O botão nunca tira o foco do teclado da planilha. Ele desaparece enquanto você edita uma célula, quando você seleciona uma única célula ou células sem conteúdo, e quando a planilha está protegida de um modo que não permite fazer nada do que a caixa oferece.

## Veja antes de escolher

Aponte para uma opção ou pressione as teclas de **seta para a esquerda** e **seta para a direita** para percorrer as opções de uma guia (as teclas **seta para cima** e **seta para baixo** passam de uma guia para outra). A planilha mostra o que a opção faria, mas nada é alterado. Para totais de um número muito grande de células, a caixa informa que há células demais para mostrar a visualização, mas a opção continua funcionando quando você clica nela. Não há nenhuma etapa a desfazer, e um arquivo salvo naquele momento não contém nada dessa visualização. Pressione **Esc** ou clique em outro lugar, e a aparência desaparece. Clique em uma opção, ou pressione **Enter**, para aplicá-la. Cada opção que você aplica é uma etapa de **Desfazer**.

## As guias

- **Formatação**: **Barras de Dados**, **Escala de Cores** e **Conjunto de Ícones** mostram o tamanho de cada número. **Maior que** colore as células cujo número é maior que a média de todos os números. **Os 10% superiores** colore o décimo mais alto. **Limpar Formatação** remove a formatação condicional das células selecionadas. São as mesmas regras de **Página Inicial > Formatação Condicional**, e você pode alterá-las por lá.
- **Gráficos**: até quatro tipos de gráfico que combinam com o formato das suas células, os mesmos que **Inserir > Gráficos Recomendados** sugere. Uma prévia mostra o gráfico no canto da planilha. Ao escolher um, o gráfico é colocado na planilha.
- **Totais**: **Soma**, **Média**, **Contagem**, **% do Total** e **Total Acumulado**, escritos como fórmulas comuns. A primeira linha de opções coloca um total abaixo de cada coluna de números. A segunda linha (**Soma por Linha**, e assim por diante) coloca um total à direita de cada linha. Uma primeira linha ou primeira coluna que contenha apenas palavras é considerada como nomes e não entra no cálculo; uma primeira linha de números, como anos, é somada como as demais. Linhas ocultas e filtradas são incluídas. **% do Total** mostra a parcela de cada coluna (ou linha) em relação a todos os números, em forma de porcentagem. **Total Acumulado** soma à medida que avança, em novas células ao lado (ou abaixo) das suas células. Se alguma das células onde os totais serão colocados já tiver conteúdo, você é consultado antes que ela seja substituída. Os totais não são gravados sobre células preenchidas por um resultado que se estende para fora delas nem sobre células mescladas: a Análise Rápida informa isso e não altera nada.
- **Tabelas**: **Tabela** transforma as células em uma tabela, com botões para classificar e filtrar. **Tabela Dinâmica** cria uma tabela dinâmica que totaliza os dados, em uma nova planilha; a primeira linha deve conter os nomes das colunas.
- **Minigráficos**: **Linha**, **Coluna** e **Vitória/Derrota** desenham um pequeno gráfico de cada linha na célula à direita dela.

## Limites

A Análise Rápida examina até 250.000 células para totais e minigráficos. Se você selecionar mais do que isso, ela informa e não altera nada. Uma opção que não caberia (por exemplo, sem espaço abaixo das células) também informa isso.
