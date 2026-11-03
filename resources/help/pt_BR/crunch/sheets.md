# Trabalhar com planilhas
<!-- keywords: planilha, planilhas, aba, abas, guia, guias, adicionar, inserir, nova planilha, excluir, apagar, remover, renomear, nome, mover, copiar, duplicar, reordenar, arrastar, ocultar, esconder, reexibir, mostrar, cor da guia, cor, selecionar várias planilhas, Shift+F11, #REF!, referência 3D, Mover ou Copiar -->

Uma pasta de trabalho é formada por **planilhas**, e as **guias** na parte inferior da janela mostram os nomes delas. Clique em uma guia para mostrar essa planilha. Cada alteração abaixo é uma etapa que você pode desfazer e fica salva na pasta de trabalho.

## Adicionar uma planilha

- Clique no botão **+** depois da última guia para adicionar uma planilha no final.
- Pressione **Shift+F11**, ou use **Página Inicial > Inserir > Inserir Planilha**, para adicionar uma planilha antes da que você está vendo.

A nova planilha fica vazia, recebe o próximo nome livre (Planilha4, Planilha5 e assim por diante) e é aberta imediatamente. Uma pasta de trabalho pode ter até 1.000 planilhas; acima disso, o JeffCrunch avisa e não adiciona nenhuma, porque não consegue abrir um arquivo com mais planilhas.

## Renomear uma planilha

**Clique duas vezes** na guia, digite o novo nome e pressione **Enter** (**Esc** cancela). Você também pode usar **Página Inicial > Formatar > Renomear Planilha**.

Um nome pode ter até 31 caracteres. Ele não pode ficar vazio, não pode conter nenhum destes caracteres: **: \ / ? * [ ]**, não pode começar nem terminar com apóstrofo e não pode ser igual ao nome de outra planilha (maiúsculas e minúsculas não contam como diferença). O nome "History" é reservado para o programa. Se o nome for recusado, o JeffCrunch informa o motivo e a planilha mantém o nome atual.

Todas as fórmulas que citam a planilha passam a usar o novo nome. Exemplo: **=Orçamento!B2** passa a ser **='Orçamento T1'!B2** quando você renomeia Orçamento para Orçamento T1. Os nomes que apontam para a planilha, as regras de formatação condicional e de validação e os gráficos também acompanham a mudança.

## Excluir uma planilha

Clique com o botão direito na guia e escolha **Excluir Planilha**, ou use **Página Inicial > Excluir > Excluir Planilha**. Se a planilha tiver algum conteúdo, o JeffCrunch pede confirmação antes. As fórmulas de outras planilhas que leem a planilha excluída mostram **#REF!**, o que significa que aquilo que elas liam não existe mais. **Desfazer** traz de volta a planilha e essas fórmulas.

Uma pasta de trabalho sempre mantém pelo menos uma planilha visível.

## Mover ou copiar uma planilha

- **Arraste** uma guia para outro lugar para mover a planilha.
- Ou use **Página Inicial > Formatar > Mover ou Copiar Planilha**. Escolha a planilha antes da qual a planilha escolhida deve ficar (ou **(depois da última planilha)**) e marque a opção para copiar e deixar as originais onde estão (**Copy them, and leave the originals where they are**) para copiá-la em vez de movê-la. A cópia recebe um nome como **Orçamento (2)** e é aberta.

As fórmulas continuam lendo as mesmas planilhas, onde quer que elas estejam. A cópia tem as mesmas células, fórmulas, formatos e tamanhos da planilha original. Uma referência sem nome de planilha (como **A1**) lê a célula da própria cópia; uma referência que cita uma planilha (como **Planilha1!A1**) continua citando essa planilha, mesmo que seja a planilha da qual a cópia foi feita.

A cópia também recebe a proteção, o filtro, os links, a configuração de página e as quebras de página da original, os grupos de linhas e colunas e a aparência das barras de dados; os links acompanham as linhas inseridas ou excluídas, na original antes da cópia e na cópia depois dela. Ela não recebe as imagens, os gráficos, as anotações, as tabelas dinâmicas, os minigráficos nem os nomes que pertencem apenas à original, porque o JeffCrunch ainda não copia esses itens; quando isso acontece, ele informa quais ficaram de fora. Uma planilha com tabelas é copiada com cópias delas (cada uma com nome, identificação e parte próprios, e as fórmulas da cópia citam as tabelas dela). Excluir uma planilha exclui também as tabelas dela; uma fórmula de outra planilha que citava uma delas passa a mostrar #REF!, e Desfazer a traz de volta.

## Ocultar e mostrar uma planilha

Clique com o botão direito na guia e escolha **Ocultar Planilha** (ou use **Página Inicial > Formatar > Ocultar Planilha**). A planilha deixa de ser mostrada, mas continua na pasta de trabalho e as fórmulas continuam lendo seu conteúdo. Para mostrá-la de novo, clique com o botão direito em qualquer guia, escolha **Reexibir Planilha** e clique no nome dela na lista. A lista fica vazia quando não há planilhas ocultas.

## Colorir uma guia

Clique com o botão direito na guia, escolha **Cor da Guia** e selecione uma cor, ou escolha **Sem Cor**. A cor aparece como uma linha abaixo do nome. Uma cor escolhida na paleta do tema é salva como a cor fixa que ela é no momento.

## Várias planilhas de uma vez

Use **Ctrl+clique** nas guias para selecionar mais de uma, ou clique em uma guia e use **Shift+clique** em outra para selecionar todas as guias entre elas. As guias selecionadas ficam sombreadas. **Excluir**, **Ocultar**, **Cor da Guia** e **Mover ou Copiar** passam então a atuar em todas elas, em uma única etapa. Clique em uma guia que não esteja selecionada para selecionar somente ela.

O que você digitar ou formatar enquanto várias planilhas estiverem selecionadas altera apenas a planilha que você está vendo.

## O que não é possível fazer

- Em uma pasta de trabalho que tenha uma fórmula sobre um intervalo de planilhas, como **=SOMA(Planilha1:Planilha3!A1)**, ainda não é possível adicionar, excluir, renomear nem mover planilhas, porque isso mudaria as planilhas abrangidas pela fórmula.
- Depois que você salva, **Desfazer** não consegue voltar além de uma planilha adicionada, copiada, excluída, renomeada ou movida antes do salvamento. O arquivo salvo guarda essas planilhas como estão. Ocultar, mostrar e a cor da guia ainda podem ser desfeitos.
