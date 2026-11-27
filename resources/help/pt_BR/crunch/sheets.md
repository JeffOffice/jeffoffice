# Trabalhar com planilhas
<!-- keywords: planilha, planilhas, aba, abas, guia, guias, adicionar, inserir, nova planilha, excluir, remover, renomear, nome, mover, copiar, duplicar, reordenar, arrastar, ocultar, reexibir, mostrar, cor da guia, cor, selecionar várias planilhas, Shift+F11, #REF!, referência 3D, Mover ou Copiar -->

Uma pasta de trabalho é formada por **planilhas**, e as **guias** na parte inferior da janela são os nomes delas. Clique em uma guia para exibir essa planilha. Cada alteração descrita abaixo é uma etapa que você pode desfazer e é salva na pasta de trabalho.

## Adicionar uma planilha

- Clique no botão **+** depois da última guia para adicionar uma planilha no final.
- Pressione **Shift+F11**, ou use **Página Inicial > Inserir > Inserir Planilha**, para adicionar uma planilha antes da que você está vendo.

A nova planilha fica vazia, recebe o próximo nome livre (Planilha4, Planilha5, e assim por diante) e é aberta imediatamente. Uma pasta de trabalho pode ter até 1.000 planilhas; ao chegar a esse limite, o JeffCrunch informa isso e não adiciona nenhuma, pois um arquivo com mais planilhas não é um que ele consegue abrir.

## Renomear uma planilha

Dê um **clique duplo** na guia, digite o novo nome e pressione **Enter** (**Esc** cancela). Você também pode usar **Página Inicial > Formatar > Renomear Planilha**.

Um nome pode ter até 31 caracteres. Ele não pode ficar vazio, não pode conter nenhum dos caracteres **: \ / ? * [ ]**, não pode começar ou terminar com apóstrofo e não pode ser igual ao nome de outra planilha (letras maiúsculas e minúsculas não fazem diferença). O nome "History" é reservado pelo programa. Se o nome for recusado, o JeffCrunch informa o motivo e a planilha mantém o nome anterior.

Toda fórmula que cita a planilha passa a usar o novo nome. Exemplo: **=Orçamento!B2** vira **='1º Trim Orçamento'!B2** ao renomear Orçamento para 1º Trim Orçamento. Nomes que apontam para a planilha, as regras de formatação condicional e de validação e os gráficos também acompanham a mudança.

## Excluir uma planilha

Clique com o botão direito na guia e escolha **Excluir Planilha**, ou use **Página Inicial > Excluir > Excluir Planilha**. Se a planilha tiver algum conteúdo, o JeffCrunch pergunta antes. As fórmulas de outras planilhas que leem a planilha excluída mostram **#REF!**, o que significa que o conteúdo que elas liam não existe mais. **Desfazer** restaura a planilha e essas fórmulas.

Uma pasta de trabalho sempre mantém pelo menos uma planilha visível.

## Mover ou copiar uma planilha

- **Arraste** uma guia para um novo lugar para mover a planilha.
- Ou use **Página Inicial > Formatar > Mover ou Copiar Planilha**. Escolha a planilha na frente da qual a planilha selecionada deve ficar (ou **(depois da última planilha)**) e marque **Criar uma cópia** para copiá-la em vez de movê-la. A cópia recebe um nome como **Orçamento (2)** e é aberta.

As fórmulas continuam lendo as planilhas que liam, onde quer que elas estejam. A cópia tem as mesmas células, fórmulas, formatações e tamanhos da planilha de origem. Uma referência sem nome de planilha (como **A1**) lê a célula da própria cópia; uma referência que cita uma planilha (como **Planilha1!A1**) continua apontando para essa planilha, mesmo que seja a planilha de origem da cópia.

A cópia também mantém a proteção, o filtro, os vínculos, a configuração de página e as quebras de página da original, os grupos de linhas e colunas e a aparência das barras de dados. Os vínculos acompanham as linhas inseridas ou excluídas: na original, antes da cópia, e na cópia, depois dela. Ela não tem as imagens, gráficos, anotações, tabelas dinâmicas, minigráficos ou nomes que pertencem apenas à original, porque o JeffCrunch ainda não copia esses itens; quando isso acontece, ele informa quais são. Uma planilha que tem tabelas é copiada com cópias delas (cada uma com nome, identificador e parte própria, e as fórmulas da cópia citam as próprias tabelas). Excluir uma planilha também exclui as tabelas dela; uma fórmula de outra planilha que citava uma dessas tabelas passa a mostrar #REF!, e **Desfazer** a restaura.

Para copiar uma planilha para **outra pasta de trabalho** que esteja aberta, escolha-a em **Para pasta de trabalho** na mesma caixa. A caixa lista o que a cópia não terá (por enquanto, as imagens, gráficos, formatações condicionais, tabelas, anotações e a forma como a planilha é impressa) e avisa isso antes de qualquer cópia. As fórmulas que leem outras planilhas da primeira pasta de trabalho se tornam vínculos com o arquivo dela, por isso a primeira pasta de trabalho precisa ter sido salva como arquivo .xlsx pelo menos uma vez. A cópia mostra os valores que a primeira pasta de trabalho tem agora, e eles mudam quando os vínculos são atualizados (**Atualizar Valores** em **Dados > Editar Vínculos**). Se a primeira pasta de trabalho tiver alterações não salvas, os vínculos leem o arquivo salvo dela quando forem atualizados; salve-a para manter esses valores. As fórmulas que leem a própria planilha copiada permanecem na cópia. Células que usam nomes, tabelas, uma matriz ou um derramamento, ou que leem uma terceira pasta de trabalho, são copiadas como os valores que exibem, e a caixa informa quantas são. A forma como a planilha é exibida (linhas e colunas congeladas, zoom, linhas de grade e o modo de leitura), a altura padrão das linhas e a largura padrão das colunas também são copiadas. A cópia mantém a mesma aparência: formatos, fontes e cores são adicionados à outra pasta de trabalho (uma cor do tema da primeira pasta é mantida como a cor que ela exibe, quando os dois temas são diferentes). As duas pastas de trabalho precisam contar as datas a partir do mesmo ano (1900 ou 1904). **Desfazer** na outra pasta de trabalho remove a cópia; a primeira pasta de trabalho não é alterada. Mover uma planilha para outra pasta de trabalho ainda não está disponível.

## Ocultar e mostrar uma planilha

Clique com o botão direito na guia e escolha **Ocultar Planilha** (ou use **Página Inicial > Formatar > Ocultar Planilha**). A planilha deixa de ser exibida, mas continua na pasta de trabalho e as fórmulas continuam lendo-a. Para exibi-la novamente, clique com o botão direito em qualquer guia, escolha **Reexibir Planilha** e clique no nome dela na lista. A lista fica vazia quando nenhuma planilha está oculta.

## Colorir uma guia

Clique com o botão direito na guia, escolha **Cor da Guia** e selecione uma cor, ou escolha **Sem Cor**. A cor aparece como uma linha sob o nome. Uma cor escolhida na paleta do tema é salva como a cor simples que ela tem no momento.

## Várias planilhas de uma vez

Use **Ctrl+clique** nas guias para selecionar mais de uma, ou clique em uma guia e use **Shift+clique** em outra para selecionar todas as guias entre elas. As guias selecionadas ficam sombreadas. **Excluir**, **Ocultar**, **Cor da Guia** e **Mover ou Copiar** então funcionam em todas elas como uma única etapa. Clique em uma guia que não está selecionada para selecionar apenas essa.

O que você digitar ou formatar enquanto várias planilhas estiverem selecionadas altera somente a planilha que está sendo exibida.

## O que não pode ser feito

- Uma pasta de trabalho que tenha uma fórmula sobre um intervalo de planilhas, como **=SOMA(Planilha1:Planilha3!A1)**, não pode ter planilhas adicionadas, excluídas, renomeadas ou movidas por enquanto, porque isso mudaria quais planilhas a fórmula abrange.
- Depois de salvar, **Desfazer** não volta além de uma planilha adicionada, copiada, excluída, renomeada ou movida antes do salvamento. O arquivo salvo mantém essas alterações. Ocultar, reexibir e a cor da guia ainda podem ser desfeitos.
