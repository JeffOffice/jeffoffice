# Selecionar células e deslocar-se

<!-- keywords: selecionar, seleção, célula, intervalo, linha, coluna, clicar, arrastar, shift, ctrl, seta, home, end, page up, page down, tabulação, enter, linha inteira, coluna inteira, caixa de nome, ir para, mover células, arrastar contorno, deslocar, barra de estado, soma, média, contar -->

**Selecionar** células significa escolher aquelas a que o comando seguinte se aplica. As células selecionadas ficam com uma cor de fundo e têm uma moldura verde à volta. A **célula ativa** é a que tem o cursor, e mantém-se branca dentro da seleção.

## Com o rato

- **Clique** numa célula para a selecionar. Clique dentro de uma célula unida para selecionar a célula unida inteira.
- **Arraste** sobre as células para selecionar um bloco. Arraste para além da margem da janela e a folha desloca-se.
- **Shift+clique** noutra célula para selecionar desde a célula ativa até essa.
- Clique numa **letra de coluna** ou num **número de linha** para selecionar a coluna ou a linha inteira. Arraste sobre as letras ou os números, ou use Shift+clique, para selecionar várias.
- Clique no **canto** onde se encontram as letras e os números para selecionar tudo.
- Mantenha **Ctrl** premido enquanto clica ou arrasta para **adicionar um intervalo** à seleção. Os intervalos que adicionou ficam com cor de fundo, e o que tem o cursor tem a moldura verde. **Eliminar** limpa todos os intervalos, de uma só vez. Um comando que funciona com um intervalo de cada vez, como Copiar, Preencher, Ordenar, Localizar na seleção, Substituir Tudo ou uma formatação, indica-o e não altera nada; clique uma vez para voltar a um só intervalo. Mudar para outra folha, ou inserir ou remover linhas ou colunas, também deixa apenas um intervalo.
- **Arraste a margem** da seleção para mover as células para outro local. Aponte para a borda da moldura verde até o ponteiro mostrar quatro setas. Um contorno tracejado mostra onde as células vão ficar. Tudo o que aponte para as células, como uma fórmula, segue-as. Mantenha **Ctrl** premido enquanto arrasta para copiar as células. Para mover as células para outra folha, comece a arrastar a margem e, em seguida, mantenha **Alt** premido e solte o botão sobre o separador dessa folha: as células vão para as mesmas posições dessa folha (se também mantiver **Ctrl**, são copiadas para lá). Manter **Alt** premido antes de carregar no botão do rato não inicia este tipo de arrastamento. As células que já estão no local de destino são substituídas, tal como acontece ao mover células numa só folha, e **Anular** repõe as duas folhas num só passo. A folha onde soltar as células tem de o permitir: uma folha protegida, e células que atravessassem células unidas, uma tabela ou uma tabela dinâmica, são recusadas, e nada é alterado. Uma folha oculta não tem separador, e o botão **Nova folha** não é um separador, por isso nenhum dos dois recebe células. Premir e soltar **Alt** durante um arrastamento não mostra as Sugestões de Teclas. Para fazer o mesmo a partir do teclado, recorte as células, vá para a outra folha e cole. Mantenha **Shift** premido enquanto arrasta para colocar as células entre outras, em vez de por cima delas. Para fazer o mesmo sem rato, recorte ou copie as células e use **Inserir Células Recortadas ou Copiadas** (veja [Inserir e eliminar](insert-delete)). Se apenas clicar na margem sem mover, seleciona a célula como habitualmente.
- Rode a **roda do rato** para deslocar três linhas. Mantenha **Shift** premido para deslocar na horizontal, ou mantenha **Ctrl** premido para aplicar zoom.

## Com o teclado

As teclas de seta movem a célula ativa, saltando as linhas e colunas ocultas. Mantenha **Shift** premido para estender a seleção. **Ctrl** com uma seta vai até ao limite dos dados: até ao fim de uma sequência de células que contêm algo ou, a partir do fim de uma sequência, até à célula seguinte que contenha algo. Veja [Utilizar o JeffCrunch com o teclado](keyboard) para conhecer todas as teclas.

**Ctrl+A** seleciona o bloco de células à volta da célula ativa, delimitado por linhas e colunas vazias; prima-o novamente para selecionar a folha inteira. Quando a seleção já é maior do que esse bloco, Ctrl+A seleciona a folha inteira de uma só vez. **Ctrl+Shift+*** seleciona apenas o bloco. Prima **End** e, em seguida, uma seta, para ir até ao limite dos dados, tal como faz **Ctrl** com a seta; a barra de estado indica **End** até premir a seta, premir outra tecla ou clicar.

## Ir para uma célula

Escreva um endereço na **Caixa de nome**, como **C5**, **A1:D10**, **B:D** ou **3:7**, e prima **Enter**. Prima **F5** (ou Ctrl+G) para abrir Ir Para e escreva o endereço aí. Coloque vírgulas entre os endereços, como **A1:B2,D4**, para selecionar vários intervalos de uma só vez.

## Os números da seleção

Quando seleciona duas ou mais células, a barra de estado mostra a **média**, a **contagem** e a **soma** das células. A contagem é o número de células que contêm algo; a média e a soma usam apenas os números. As linhas e colunas ocultas não são incluídas. Para copiar a seleção como texto, prima **Ctrl+C**.

## O que diz a barra de estado

À esquerda, a barra de estado mostra **Pronto** quando pode trabalhar, **Introduzir** enquanto escreve numa célula, **Editar** depois de premir F2, **Apontar** enquanto pode clicar ou usar as setas para uma célula e colocar o respetivo endereço numa fórmula, e **End** depois de premir a tecla End. Quando um filtro está ativo, indica também quantas linhas o filtro deixou, como **3 de 8 registos encontrados**.
