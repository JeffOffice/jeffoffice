# Selecionar células e navegar pela planilha
<!-- keywords: selecionar, seleção, célula, intervalo, linha, coluna, clicar, arrastar, shift, ctrl, seta, home, end, page up, page down, tab, enter, linha inteira, coluna inteira, caixa de nome, ir para, mover células, arrastar borda, rolar, barra de status, soma, média, contagem -->

**Selecionar** células é escolher as que o próximo comando vai afetar. As células selecionadas ficam com uma cor de fundo suave e uma borda verde ao redor. A **célula ativa** é a que tem o cursor, e ela continua branca dentro da seleção.

## Com o mouse

- **Clique** em uma célula para selecioná-la. Clique dentro de uma célula mesclada para selecionar a célula mesclada inteira.
- **Arraste** sobre as células para selecionar um bloco. Se você arrastar além da borda da janela, a planilha rola.
- **Shift+clique** em outra célula para selecionar da célula ativa até ali.
- Clique em uma **letra de coluna** ou em um **número de linha** para selecionar a coluna ou a linha inteira. Arraste sobre as letras ou os números, ou use Shift+clique, para selecionar várias.
- Clique no **canto** onde as letras e os números se encontram para selecionar a planilha inteira.
- Mantenha **Ctrl** pressionado enquanto clica ou arrasta para **adicionar um intervalo** à seleção. Os intervalos que você adicionou ficam com a cor de fundo, e o que tem o cursor tem a borda verde. **Delete** limpa todos os intervalos de uma vez. Um comando que funciona em um intervalo por vez, como Copiar, Preencher, Classificar, Localizar na seleção, Substituir Tudo ou uma formatação, avisa isso e não altera nada; clique uma vez para voltar a um só intervalo. Mudar para outra planilha, ou inserir ou excluir linhas ou colunas, também deixa apenas um intervalo.
- **Arraste a borda** da seleção para mover as células para outro lugar. Aponte para a borda verde até o ponteiro mostrar quatro setas. Um contorno tracejado mostra onde as células vão parar. Tudo o que aponta para as células, como uma fórmula, acompanha a movimentação. Mantenha **Ctrl** pressionado enquanto arrasta para copiar as células em vez disso. Mantenha **Shift** pressionado enquanto arrasta para inserir as células entre outras, em vez de sobrescrevê-las. Para fazer o mesmo sem mouse, recorte ou copie as células e use **Inserir Células Recortadas ou Copiadas** (veja [Inserir e excluir](insert-delete)). Se você apenas clicar na borda sem mover, a célula é selecionada normalmente.
- Gire a **roda do mouse** para rolar três linhas. Mantenha **Shift** pressionado para rolar para os lados, ou **Ctrl** para ampliar ou reduzir o zoom.

## Com o teclado

As teclas de seta movem a célula ativa, pulando as linhas e colunas ocultas. Mantenha **Shift** pressionado para estender a seleção. **Ctrl** com uma seta vai até a borda dos dados: até o fim de uma sequência de células que contêm algo ou, a partir do fim de uma sequência, até a próxima célula que contém algo. Veja [Usar o JeffCrunch com o teclado](keyboard) para conhecer todas as teclas.

**Ctrl+A** seleciona o bloco de células ao redor da célula ativa, delimitado por linhas e colunas vazias; pressione de novo para selecionar a planilha inteira. Quando a seleção já é maior que esse bloco, Ctrl+A seleciona a planilha inteira de uma vez. **Ctrl+Shift+\*** seleciona apenas o bloco. Pressione **End** e depois uma seta para ir até a borda dos dados, como faz **Ctrl** com a seta; a barra de status mostra **End** até você pressionar a seta, outra tecla ou clicar.

## Ir para uma célula

Digite um endereço na **Caixa de Nome**, como **C5**, **A1:D10**, **B:D** ou **3:7**, e pressione **Enter**. Pressione **F5** (ou Ctrl+G) para abrir a caixa **Ir Para** e digite o endereço ali. Separe os endereços com vírgulas, como **A1:B2,D4**, para selecionar vários intervalos de uma vez.

## Os números da seleção

Quando você seleciona duas ou mais células, a barra de status mostra a **média**, a **contagem** e a **soma** das células. A contagem é o número de células que contêm algo; a média e a soma usam apenas os números. Linhas e colunas ocultas são desconsideradas. Para copiar a seleção como texto, pressione **Ctrl+C**.

## O que a barra de status informa

No lado esquerdo da barra de status, aparece **Pronto** quando você pode trabalhar, **Inserir** enquanto você digita em uma célula, **Editar** depois que você pressiona F2, **Apontar** enquanto você pode clicar ou usar as setas para escolher uma célula e colocar o endereço dela em uma fórmula, e **End** depois que você pressiona a tecla End. Quando um filtro está ativo, a barra também informa quantas linhas o filtro deixou, como **3 de 8 registros encontrados**.
