# Selecionar células e deslocar-se

<!-- keywords: selecionar, seleção, célula, intervalo, linha, coluna, clicar, arrastar, shift, ctrl, seta, home, end, page up, page down, tab, enter, linha inteira, coluna inteira, caixa de nome, ir para, mover células, arrastar limite, deslocar, barra de estado, soma, média, contar, selecionar células, navegar -->

**Selecionar** células é escolher as que o próximo comando vai afetar. As células selecionadas ficam com uma cor de fundo esbatida e um contorno verde à volta. A **célula ativa** é a que tem o cursor, e mantém-se branca dentro da seleção.

## Com o rato

- **Clique** numa célula para a selecionar. Clique dentro de uma célula unida para selecionar a célula unida inteira.
- **Arraste** sobre as células para selecionar um bloco. Se arrastar para além da margem da janela, a folha desloca-se.
- **Shift+clique** noutra célula para selecionar desde a célula ativa até essa célula.
- Clique numa **letra de coluna** ou num **número de linha** para selecionar a coluna ou a linha inteira. Arraste sobre as letras ou os números, ou use Shift+clique, para selecionar várias.
- Clique no **canto** onde se encontram as letras e os números para selecionar tudo.
- Mantenha **Ctrl** premido enquanto clica ou arrasta para **adicionar um intervalo** à seleção. Os intervalos que adicionou ficam com cor de fundo, e o que tem o cursor tem o contorno verde. **Eliminar** limpa todos os intervalos de uma só vez. Um comando que trabalha com um intervalo de cada vez, como Copiar, Preencher, Ordenar, Localizar na seleção, Substituir Tudo ou uma formatação, indica-o e não altera nada; clique uma vez para voltar a um só intervalo. Mudar para outra folha, ou inserir ou remover linhas ou colunas, também deixa apenas um intervalo.
- **Arraste o limite** da seleção para mover as células para outro local. Aponte para a borda do contorno verde até o ponteiro mostrar quatro setas. Um contorno tracejado mostra onde as células vão ficar. Tudo o que aponta para as células, como uma fórmula, acompanha-as. Mantenha **Ctrl** premido enquanto arrasta para copiar as células em vez de as mover. Mantenha **Shift** premido enquanto arrasta para inserir as células entre outras, em vez de as colocar por cima. Para fazer o mesmo sem rato, recorte ou copie as células e use **Inserir Células Recortadas ou Copiadas** (ver [Inserir e eliminar](insert-delete)). Se apenas clicar no limite sem mover, a célula fica selecionada como de costume.
- Rode a **roda do rato** para deslocar três linhas. Mantenha **Shift** premido para deslocar na horizontal, ou **Ctrl** para ampliar ou reduzir o zoom.

## Com o teclado

As teclas de seta movem a célula ativa, saltando as linhas e colunas ocultas. Mantenha **Shift** premido para estender a seleção. **Ctrl** com uma seta vai até à extremidade dos dados: até ao fim de uma sequência de células com conteúdo ou, a partir do fim de uma sequência, até à célula seguinte com conteúdo. Consulte [Utilizar o JeffCrunch com o teclado](keyboard) para ver todas as teclas.

**Ctrl+A** seleciona o bloco de células à volta da célula ativa, delimitado por linhas e colunas vazias; prima-o de novo para selecionar a folha inteira. Quando a seleção já é maior do que esse bloco, Ctrl+A seleciona logo a folha inteira. **Ctrl+Shift+*** seleciona apenas o bloco. Prima **End** e, em seguida, uma seta, para ir até à extremidade dos dados, como faz **Ctrl** com a seta; a barra de estado mostra **End** até premir a seta, premir outra tecla ou clicar.

## Ir para uma célula

Escreva um endereço na **Caixa de nome**, como **C5**, **A1:D10**, **B:D** ou **3:7**, e prima **Enter**. Prima **F5** (ou Ctrl+G) para abrir Ir Para e escreva o endereço aí. Coloque vírgulas entre os endereços, como **A1:B2,D4**, para selecionar vários intervalos de uma só vez.

## Os números da seleção

Quando seleciona duas ou mais células, a barra de estado mostra a **média**, a **contagem** e a **soma** das células. A contagem é o número de células que contêm algo; a média e a soma usam apenas os números. As linhas e colunas ocultas são excluídas. Para copiar a seleção como texto, prima **Ctrl+C**.

## O que diz a barra de estado

À esquerda, a barra de estado mostra **Pronto** quando pode trabalhar, **Enter** enquanto escreve numa célula, **Editar** depois de premir F2, **Apontar** enquanto pode clicar ou usar as setas para colocar o endereço de uma célula numa fórmula, e **End** depois de premir a tecla End. Quando um filtro está ativo, mostra também quantas linhas o filtro deixou, como **3 de 8 registos encontrados**.
