# Trabalhar com folhas
<!-- keywords: folha, folhas, folha de cálculo, separador, separadores, adicionar, inserir, nova folha, eliminar, apagar, remover, mudar o nome, renomear, nome, mover, copiar, duplicar, reordenar, arrastar, ocultar, mostrar, reexibir, cor do separador, cor, selecionar várias folhas, Shift+F11, #REF!, referência 3D, Mover ou Copiar -->

Um livro é composto por **folhas**, e os **separadores** na parte inferior da janela são os respetivos nomes. Clique num separador para mostrar essa folha. Cada uma das alterações abaixo é um passo que pode anular e fica guardada no livro.

## Adicionar uma folha

- Clique no botão **+** a seguir ao último separador para adicionar uma folha no fim.
- Prima **Shift+F11** ou utilize **Base > Inserir > Inserir Folha** para adicionar uma folha antes da que está a ver.

A nova folha está vazia, recebe o nome livre seguinte (Folha4, Folha5, etc.) e abre de imediato. Um livro pode ter até 1000 folhas; acima desse número, o JeffCrunch informa-o e não adiciona nenhuma, porque não consegue abrir um ficheiro com mais folhas.

## Mudar o nome de uma folha

Faça **duplo clique** no separador, escreva o novo nome e prima **Enter** (**Esc** cancela). Também pode utilizar **Base > Formatar > Mudar o Nome da Folha**.

Um nome pode ter até 31 carateres. Não pode estar vazio, não pode conter nenhum dos carateres **: \ / ? * [ ]**, não pode começar nem terminar com um apóstrofo e não pode ser igual ao nome de outra folha (as maiúsculas não contam como diferença). O nome "History" está reservado para o programa. Se o nome for recusado, o JeffCrunch indica o motivo e a folha mantém o nome.

Todas as fórmulas que referem a folha passam a utilizar o novo nome. Exemplo: **=Orçamento!B2** passa a **='Orçamento T1'!B2** quando muda o nome de Orçamento para Orçamento T1. Os nomes que apontam para a folha, as regras de formatação condicional e de validação, e os gráficos também acompanham a alteração.

## Eliminar uma folha

Clique com o botão direito do rato no separador e escolha **Eliminar Folha**, ou utilize **Base > Eliminar > Eliminar Folha**. Se a folha tiver conteúdo, o JeffCrunch pede primeiro confirmação. As fórmulas noutras folhas que leem a folha eliminada mostram **#REF!**, o que significa que aquilo que liam já não existe. **Anular** repõe a folha e essas fórmulas.

Um livro mantém sempre pelo menos uma folha visível.

## Mover ou copiar uma folha

- **Arraste** um separador para uma nova posição para mover a folha.
- Em alternativa, utilize **Base > Formatar > Mover ou Copiar Folha…**. Escolha a folha antes da qual a folha escolhida deve ficar (ou **(depois da última folha)**) e selecione **Copiar e deixar os originais onde estão** para a copiar em vez de a mover. A cópia recebe um nome como **Orçamento (2)** e é aberta.

As fórmulas continuam a ler as mesmas folhas, independentemente da posição destas. Uma cópia tem as mesmas células, fórmulas, formatos e tamanhos da folha de origem. Uma referência sem nome de folha (como **A1**) lê a célula da própria cópia; uma referência que indica uma folha (como **Folha1!A1**) continua a indicar essa folha, mesmo que seja a folha de onde a cópia veio.

Uma cópia tem também a proteção, o filtro, as ligações, a configuração de página e as quebras de página do original, os grupos de linhas e colunas e o aspeto das barras de dados; as ligações acompanham as linhas inseridas ou eliminadas, no original antes da cópia e na cópia depois dela. Não inclui as imagens, gráficos, notas, tabelas dinâmicas, gráficos sparkline nem os nomes que pertencem apenas ao original, porque o JeffCrunch ainda não os copia; quando isto acontece, indica quais são. Uma folha com tabelas é copiada com cópias dessas tabelas (cada uma com nome, id e parte próprios, e as fórmulas da cópia referem as suas próprias tabelas). Eliminar uma folha elimina também as respetivas tabelas; uma fórmula noutra folha que referia uma delas passa a mostrar #REF!, e Anular repõe-na.

## Ocultar e mostrar uma folha

Clique com o botão direito do rato no separador e escolha **Ocultar Folha** (ou utilize **Base > Formatar > Ocultar Folha**). A folha deixa de ser mostrada, mas continua no livro e as fórmulas continuam a lê-la. Para a mostrar novamente, clique com o botão direito do rato em qualquer separador, escolha **Mostrar Folha** e clique no nome dela na lista. A lista está vazia quando não existem folhas ocultas.

## Colorir um separador

Clique com o botão direito do rato no separador, escolha **Cor do Separador** e selecione uma cor, ou escolha **Sem Cor**. A cor aparece como uma linha por baixo do nome. Uma cor escolhida na paleta do tema é guardada como a cor simples que é no momento.

## Várias folhas de uma vez

Faça **Ctrl+clique** nos separadores para selecionar mais do que um, ou clique num separador e faça **Shift+clique** noutro para selecionar todos os separadores entre eles. Os separadores selecionados ficam sombreados. **Eliminar**, **Ocultar**, **Cor do Separador** e **Mover ou Copiar** aplicam-se então a todos num só passo. Clique num separador não selecionado para selecionar apenas esse.

O que escrever ou formatar enquanto estão selecionadas várias folhas altera apenas a folha que está a ver.

## O que não é possível fazer

- Num livro com uma fórmula que abrange um bloco de folhas, como **=SOMA(Folha1:Folha3!A1)**, ainda não é possível adicionar, eliminar, mudar o nome ou mover folhas, porque isso alteraria as folhas abrangidas pela fórmula.
- Depois de guardar, **Anular** não consegue recuar para lá de uma folha que tenha adicionado, copiado, eliminado, cujo nome tenha mudado ou que tenha movido antes de guardar. O ficheiro guardado contém-nas tal como estão. Ocultar, mostrar e a cor do separador continuam a poder ser anulados.
