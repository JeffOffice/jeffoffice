# Alterar a largura das colunas e a altura das linhas
<!-- keywords: largura predefinida, altura predefinida da linha, largura padrão, altura padrão, largura, altura, coluna, linha, tamanho, mais larga, mais alta, mais estreita, mais baixa, ajustar, ajuste automático, ocultar, mostrar, arrastar, limite, duplo clique, carateres, pontos, Ctrl+0, Ctrl+9, píxeis, número demasiado largo, cardinais -->

A **largura de uma coluna** é contada em carateres: quantos algarismos cabem nela. A **altura de uma linha** é contada em pontos, uma unidade usada em tipografia (72 pontos equivalem a uma polegada). A coluna habitual leva cerca de 8 algarismos e a linha habitual tem 15 pontos de altura.

## Arrastar um limite

Aponte para a linha entre duas letras de coluna (ou dois números de linha). O ponteiro muda para setas e pode arrastar a linha. Uma linha tracejada mostra onde ficará o limite, e nada muda até soltar. Arraste-a até ao outro limite para ocultar a coluna ou a linha.

Se estiverem selecionadas colunas (ou linhas) inteiras e arrastar o limite de uma delas, todas ficam com o mesmo tamanho.

## Ajustar ao conteúdo

**Faça duplo clique** no limite para que a coluna fique com a largura exata para o conteúdo mais comprido, ou a linha com a altura exata para a sua linha de texto mais alta. Também pode usar **Base > Formatar > Ajustar Automaticamente a Largura da Coluna** e **Ajustar Automaticamente a Altura da Linha**, para todas as colunas ou linhas selecionadas de uma só vez. Uma coluna ou linha sem conteúdo mantém o tamanho.

## Escrever um tamanho

Selecione as colunas ou linhas, utilize **Base > Formatar > Largura da Coluna** ou **Altura da Linha** e escreva o tamanho. Um tamanho de 0 oculta-as.

## Alterar o tamanho padrão

As colunas e linhas que não têm um tamanho próprio ficam com o tamanho **padrão**: 8,43 carateres de largura e 15 pontos de altura, salvo indicação em contrário no livro. Para o alterar na folha inteira, utilize **Base > Formatar > Largura Predefinida** ou **Altura Predefinida da Linha**, escreva o tamanho e clique em **OK**. Todas as colunas (ou linhas) sem tamanho próprio mudam de imediato; as que dimensionou manualmente mantêm o tamanho. É um único passo que pode anular, e o livro guarda o novo padrão.

## Ocultar e mostrar

**Ctrl+0** oculta as colunas selecionadas e **Ctrl+9** oculta as linhas selecionadas. Para mostrar as ocultas, selecione as colunas ou linhas de ambos os lados (ou um bloco que as abranja) e utilize **Base > Formatar > Mostrar Colunas** ou **Mostrar Linhas**. O menu de clique com o botão direito nas letras de coluna e nos números de linha também tem estes comandos.

## Um número demasiado largo para a célula

Quando escreve ou cola um número que não cabe na coluna, a coluna alarga para o mostrar, se ainda tiver a largura inicial. Uma coluna cuja largura definiu manualmente mantém-na, e um número que não caiba é apresentado como cardinais (**#**) até alargar a coluna. A escrita e o alargamento são um único passo, por isso **Anular** desfaz ambos.

Cada alteração é um passo que pode anular, e o ficheiro guarda os tamanhos. Para ter uma noção de escala: uma coluna com 8,43 carateres de largura tem cerca de 64 píxeis, e uma linha com 15 pontos de altura tem 20 píxeis, com zoom de 100%.

## Linhas que crescem com o texto

Uma linha à qual não atribuiu uma altura própria acompanha o conteúdo. Ative **Moldar Texto** e a linha cresce para mostrar todas as linhas de texto; aumente o tamanho do tipo de letra, ou escreva várias linhas numa célula (mantenha premida a tecla **Alt** e prima **Enter** para uma nova linha, o que também ativa Moldar Texto), e a linha cresce para as conter. Quando o texto desaparece, ou a coluna fica mais larga, a linha volta a encolher. Uma linha cuja altura definiu manualmente, ou que ocultou, fica como está. Estas alterações resultam do conteúdo das células, por isso não são passos próprios: Anular altera as células e a linha acompanha. Numa folha muito grande, só é medida a área que está a ver, e cada linha é medida quando se desloca até ela.
