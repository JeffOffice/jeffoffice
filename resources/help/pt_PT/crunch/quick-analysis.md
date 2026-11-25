# Análise Rápida
<!-- keywords: análise rápida, analisar, análise, gráfico instantâneo, gráfico rápido, totais rápidos, totais, total acumulado, percentagem do total, barras de dados, escala de cores, conjunto de ícones, minigráficos, tabela, tabela dinâmica, Ctrl+Q, pré-visualizar -->

A **Análise Rápida** reúne numa pequena caixa as formas mais comuns de analisar um bloco de números: colorir as células, representá-las num gráfico, calcular totais, transformá-las numa tabela ou dar a cada linha um pequeno gráfico. Escolhe o que pretende e vê o resultado na sua folha antes de decidir.

## Abrir a Análise Rápida

1. Selecione duas ou mais células que tenham conteúdo. Aparece um pequeno botão logo abaixo e à direita da última célula selecionada.
2. Clique no botão, prima **Ctrl+Q** (num Mac, **Command+Q**) ou clique em **Base > Edição > Análise Rápida**.
3. Escolha um separador e, em seguida, uma opção.

O botão nunca tira o teclado à folha. Desaparece enquanto edita uma célula, quando seleciona apenas uma célula ou células sem conteúdo, e quando a folha está protegida de forma a não permitir nada do que a caixa oferece.

## Veja antes de escolher

Aponte para uma opção ou prima as teclas de seta **Esquerda** e **Direita** para percorrer as opções de um separador (as teclas de seta **Cima** e **Baixo** deslocam-se entre os separadores). A folha mostra o que a opção faria, mas nada é alterado (nos totais de um número muito grande de células, é indicado que há demasiadas células para mostrar uma pré-visualização, e a opção continua a funcionar quando clicar nela). Não há nenhum passo a anular, e um ficheiro guardado nesse momento não contém nada disso. Prima **Esc** ou clique noutro lado, e o aspeto desaparece. Clique numa opção, ou prima **Enter**, para a aplicar. Cada opção que escolher é um passo de **Anular**.

## Os separadores

- **Formatação**: **Barras de Dados**, **Escala de Cores** e **Conjunto de Ícones** mostram a grandeza de cada número. **Maior do Que** colore as células cujo número é superior à média de todos os números. **10% Superiores** colore a décima parte mais alta. **Limpar Formatação** remove a formatação condicional das células selecionadas. São as mesmas regras de **Base > Formatação Condicional**, e pode alterá-las ali.
- **Gráficos**: até quatro tipos de gráfico adequados à forma das suas células, os mesmos que **Inserir > Gráficos Recomendados** sugere. Um aspeto mostra o gráfico no canto da folha. Ao escolher um, o gráfico é colocado na folha.
- **Totais**: **Soma**, **Média**, **Contagem**, **% do Total** e **Total Acumulado**, escritos como fórmulas normais. A primeira linha de opções coloca um total por baixo de cada coluna de números. A segunda linha (**Soma da Linha**, e assim por diante) coloca um total à direita de cada linha. Uma primeira linha ou primeira coluna que contenha apenas palavras é considerada como nomes e fica de fora; uma primeira linha de números, como anos, é somada como as restantes. As linhas ocultas e filtradas são incluídas. **% do Total** mostra a parte de cada coluna (ou linha) no conjunto de todos os números, em percentagem. **Total Acumulado** vai somando à medida que avança, em novas células ao lado (ou por baixo) das suas células. Se alguma das células onde os totais são colocados já contiver algo, é-lhe perguntado antes de ser substituída. Os totais não são escritos sobre células preenchidas por um resultado que transborda nem sobre células unidas: a Análise Rápida informa-o e não altera nada.
- **Tabelas**: **Tabela** transforma as células numa tabela, com botões para ordenar e filtrar. **Tabela Dinâmica** cria uma tabela dinâmica que totaliza os dados, numa nova folha; a primeira linha deve conter os nomes das colunas.
- **Minigráficos**: **Linha**, **Coluna** e **Vitórias/Derrotas** desenham um pequeno gráfico de cada linha, na célula à direita dela.

## Limites

A Análise Rápida analisa até 250 000 células para totais e minigráficos. Se selecionar mais, é indicado e nada é alterado. Uma opção que não caiba (por exemplo, sem espaço por baixo das células) também o indica.
