# Trabalhar com folhas
<!-- keywords: folha, folhas, separador, separadores, adicionar, inserir, nova folha, eliminar, remover, mudar o nome, nome, mover, copiar, duplicar, reordenar, arrastar, ocultar, mostrar, cor do separador, cor, selecionar várias folhas, Shift+F11, #REF!, referência 3D, Mover ou Copiar -->

Um livro é composto por **folhas**, e os **separadores** na parte inferior da janela são os respetivos nomes. Clique num separador para mostrar essa folha. Cada alteração indicada abaixo é um passo que pode anular e fica guardada no livro.

## Adicionar uma folha

- Clique no botão **+** depois do último separador para adicionar uma folha no fim.
- Prima **Shift+F11**, ou utilize **Base > Inserir > Inserir Folha**, para adicionar uma folha antes da que está a ver.

A nova folha está vazia, tem o próximo nome livre (Folha4, Folha5, e assim por diante) e abre de imediato. Um livro pode ter até 1000 folhas; depois disso, o JeffCrunch informa e não adiciona mais, porque um ficheiro com mais do que isso não é um que o programa abra.

## Mudar o nome de uma folha

**Clique duas vezes** no separador, escreva o novo nome e prima **Enter** (**Esc** cancela). Também pode utilizar **Base > Formatar > Mudar o Nome da Folha**.

Um nome pode ter até 31 carateres. Não pode estar vazio, não pode conter nenhum dos carateres **: \ / ? * [ ]**, não pode começar nem terminar por um apóstrofo, e não pode ser igual ao nome de outra folha (as maiúsculas não contam como diferença). O nome "History" está reservado pelo programa. Se o nome for recusado, o JeffCrunch indica o motivo e a folha mantém o nome que tinha.

Todas as fórmulas que referem a folha passam a usar o novo nome. Exemplo: **=Orçamento!B2** passa a ser **='1.º Trimestre'!B2** quando muda o nome de Orçamento para 1.º Trimestre. Os nomes que apontam para a folha, as regras de formatação condicional e de validação, e os gráficos também o acompanham.

## Eliminar uma folha

Clique com o botão direito no separador e escolha **Eliminar Folha**, ou utilize **Base > Eliminar > Eliminar Folha**. Se a folha tiver algum conteúdo, o JeffCrunch pergunta primeiro. As fórmulas de outras folhas que leem a folha eliminada mostram **#REF!**, o que significa que aquilo que liam já não existe. **Anular** repõe a folha e essas fórmulas.

Um livro mantém sempre pelo menos uma folha visível.

## Mover ou copiar uma folha

- **Arraste** um separador para um novo lugar para mover a folha.
- Ou utilize **Base > Formatar > Mover ou Copiar Folha**. Escolha a folha à frente da qual a folha escolhida deve ficar (ou **(depois da última folha)**), e marque **Copiar, e deixar os originais onde estão** para copiar a folha em vez de a mover. Uma cópia recebe um nome como **Orçamento (2)** e abre.

As fórmulas continuam a ler as folhas que liam, independentemente de onde as folhas estejam. Uma cópia tem as mesmas células, fórmulas, formatos e tamanhos da folha de origem. Uma referência sem nome de folha (como **A1**) lê a célula da própria cópia; uma referência que indica uma folha (como **Folha1!A1**) continua a referir essa folha, mesmo que seja a folha de onde a cópia foi feita.

Uma cópia também tem a proteção, o filtro, as ligações, a configuração de página e as quebras de página do original, os grupos de linhas e colunas e o aspeto das barras de dados; as ligações acompanham as linhas que inseriu ou eliminou, no original antes da cópia e na cópia depois dela. Não tem as imagens, gráficos, notas, tabelas dinâmicas, minigráficos nem os nomes que pertencem apenas ao original, porque o JeffCrunch ainda não os copia; quando isso acontece, indica quais são. Uma folha que tenha tabelas é copiada com cópias delas (cada uma com um nome, um identificador e uma parte própria, e as fórmulas da cópia referem as suas próprias tabelas). Eliminar uma folha leva consigo as suas tabelas; uma fórmula de outra folha que as referia fica então com #REF!, e **Anular** repõe-na.

Para copiar uma folha para **outro livro** que esteja aberto, escolha-o em **Para o livro** na mesma caixa. A caixa indica o que a cópia não terá (por agora, as imagens, gráficos, formatações condicionais, tabelas, notas e a forma como a folha é impressa) e avisa antes de copiar seja o que for. As fórmulas que leem outras folhas do primeiro livro passam a ser ligações ao respetivo ficheiro, por isso o primeiro livro tem de ter sido guardado pelo menos uma vez como ficheiro .xlsx. A cópia mostra os valores que o primeiro livro tem neste momento, e esses valores mudam quando as ligações forem atualizadas (**Atualizar Valores** em **Dados > Editar Ligações**). Se o primeiro livro tiver alterações por guardar, as ligações leem o seu ficheiro guardado quando forem atualizadas; por isso, guarde-o para manter estes valores. As fórmulas que leem a própria folha copiada permanecem na cópia. As células que usam nomes, tabelas, uma matriz ou um transbordo, ou que leem um terceiro livro, chegam como os valores que mostram, e a caixa indica quantas são. A forma como a folha é apresentada (linhas e colunas fixas, zoom, linhas de grelha e a forma como é lida) e a altura predefinida das linhas e a largura predefinida das colunas também são copiadas. A cópia tem o mesmo aspeto: os formatos, tipos de letra e cores são acrescentados ao outro livro (uma cor do tema do primeiro livro é mantida como a cor que apresenta quando os dois temas são diferentes). Os dois livros têm de contar as datas a partir do mesmo ano (1900 ou 1904). **Anular** no outro livro remove a cópia; o primeiro livro não é alterado. Mover uma folha para outro livro ainda não está disponível.

## Ocultar e mostrar uma folha

Clique com o botão direito no separador e escolha **Ocultar Folha** (ou utilize **Base > Formatar > Ocultar Folha**). A folha deixa de ser mostrada, mas continua no livro e as fórmulas continuam a lê-la. Para a mostrar de novo, clique com o botão direito em qualquer separador e escolha **Mostrar Folha**, e depois clique no nome dela na lista. A lista está vazia quando não há nenhuma folha oculta.

## Colorir um separador

Clique com o botão direito no separador e escolha **Cor do Separador**, depois escolha uma cor, ou escolha **Sem Cor**. A cor aparece como uma linha por baixo do nome. Uma cor escolhida na paleta do tema é guardada como a cor simples que tem agora.

## Várias folhas de uma vez

**Ctrl+clique** em separadores para escolher mais do que um, ou clique num separador e depois **Shift+clique** noutro para escolher todos os separadores entre eles. Os separadores escolhidos ficam sombreados. **Eliminar**, **Ocultar**, **Cor do Separador** e **Mover ou Copiar** passam então a atuar sobre todos como um só passo. Clique num separador que não esteja escolhido para o selecionar sozinho.

O que escrever ou formatar enquanto várias folhas estiverem selecionadas altera apenas a folha que está a ver.

## O que não é possível fazer

- Um livro que tenha uma fórmula sobre um intervalo de folhas, como **=SOMA(Folha1:Folha3!A1)**, não pode ter folhas adicionadas, eliminadas, renomeadas ou movidas por agora, porque isso alteraria as folhas que a fórmula abrange.
- Depois de guardar, **Anular** não pode voltar atrás além de uma folha que tenha adicionado, copiado, eliminado, renomeado ou movido antes de guardar. O ficheiro guardado tem-nas como estão. Ocultar, mostrar e a cor do separador ainda podem ser anulados.
