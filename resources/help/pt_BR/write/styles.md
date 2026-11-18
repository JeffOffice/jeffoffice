# Usar estilos
<!-- keywords: estilo de lista, salvar estilo de lista, estilo de lista de vários níveis, estilos de lista, gerenciar estilos, estilo bloqueado, bloquear estilo, limitar formatação, restringir formatação, exportar estilos, arquivo de modelo, dotx, ocultar estilo, estilos recomendados, estilos, estilo, painel de estilos, título, normal, criar estilo, novo estilo, modificar estilo, alterar estilo, baseado em, próximo estilo, atualizar automaticamente, galeria, estilo de caractere, estilo de parágrafo, ctrl+alt+shift+s -->

Um **estilo** é um conjunto salvo de aparências: uma fonte, um tamanho, espaçamentos e outros itens. Quando você aplica um estilo a um parágrafo, ele assume todas essas aparências de uma só vez. Quando você altera o estilo, todos os parágrafos que o usam também mudam. É por isso que os estilos mantêm documentos longos organizados.

O JeffWrite tem dois tipos:

- Um **estilo de parágrafo** define a aparência de parágrafos inteiros. **Título 1** e **Normal** são estilos de parágrafo.
- Um **estilo de caractere** define a aparência das palavras que você seleciona. **Forte** é um estilo de caractere.

## Aplicar um estilo ao seu texto

1. Clique em um parágrafo ou selecione as palavras que deseja alterar.
2. Clique em um estilo na galeria **Estilos** da guia **Página Inicial**. Ou pressione **Ctrl+Shift+N** para Normal, ou **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** para Título 1, 2 ou 3.

Um estilo de parágrafo é aplicado a todos os parágrafos que você tocar. Um estilo de caractere é aplicado às palavras selecionadas.

## Abrir o painel Estilos

Clique na pequena seta no canto do grupo **Estilos**, ou pressione **Ctrl+Alt+Shift+S**. O painel **Estilos** abre à direita e lista os estilos do seu documento, cada um com a sua própria aparência.

- O estilo onde está o cursor aparece em negrito, com uma seta antes do nome.
- Clique em um estilo para aplicá-lo à sua seleção.
- Use a lista abaixo dos estilos para escolher quais mostrar:
  - **Em uso** mostra apenas os estilos que o seu texto usa.
  - **Todos os estilos** mostra todos, inclusive os que o seu documento ainda não tem. Um estilo é adicionado ao documento na primeira vez que você o usa.
  - **Recomendados** mostra os estilos que devem ser vistos. Um estilo que fica oculto até ser usado aparece aqui depois que você o utiliza.
- Clique em **Gerenciar…** abaixo da lista para escolher, para cada estilo, se ele é **Recomendado** (mostrado na galeria), se é **Oculto** (retirado de todas as listas) e sua **Ordem nas listas**. Um número menor aparece primeiro. Clique em **OK**, e todas as alterações acontecem em uma única etapa, de modo que um **Desfazer** as desfaz.
- Em **Gerenciar…**, clique em **Excluir** para remover um estilo que você criou. Os parágrafos que o usavam voltam ao Normal, e os estilos baseados nele passam a ser baseados no que ele era baseado. Os estilos que acompanham o JeffWrite não podem ser excluídos. A exclusão é uma única etapa, então **Desfazer** restaura o estilo.
- Em **Gerenciar…**, clique em **Importar de arquivo…** para copiar estilos de outro documento. Escolha o arquivo, marque os estilos desejados e clique em **OK**. Um estilo que depende de outro traz esse outro junto. Os estilos que você já tem são mantidos, a menos que você marque **Substituir estilos que têm o mesmo nome**. A importação é uma etapa que pode ser desfeita.
- Em **Gerenciar…**, clique em **Exportar para arquivo…** para salvar alguns dos seus estilos em um arquivo de modelo (um arquivo **.dotx**). Marque os estilos desejados, clique em **OK** e escolha onde salvar. Outro documento pode obtê-los desse arquivo com **Importar de arquivo…**. Um estilo que depende de outro traz esse outro junto.
- Marque **Bloqueado** para bloquear um estilo. Depois, marque **Limitar formatação aos estilos que não estão bloqueados** para ativar a limitação. Enquanto ela estiver ativa, ninguém pode formatar o texto manualmente, por exemplo com **Negrito** ou **Centralizar**, e ninguém pode usar um estilo bloqueado. Os demais estilos continuam disponíveis. A barra de status informa quando um comando é recusado. Desmarque a caixa para desativar a limitação. Se o documento estiver protegido por senha, interrompa essa proteção antes, em **Revisão > Restringir Edição**. A limitação e os bloqueios são salvos no arquivo.
- Abaixo da lista, a caixa **Classificar** ordena os estilos. **Alfabética** segue o nome. **Como recomendado** coloca primeiro os estilos mais úteis. **Por tipo** lista os estilos de parágrafo, depois os estilos vinculados e depois os estilos de caractere. **Baseado em** agrupa cada estilo com o estilo em que ele se baseia.

## Criar um estilo a partir do seu texto

1. Formate um parágrafo como desejar. Por exemplo, deixe-o em negrito, centralize-o e defina o tamanho.
2. Clique em **Novo Estilo** na parte inferior do painel, ou clique em **Criar um Estilo** na guia **Página Inicial**.
3. Digite um **Nome**. O nome pode ter até 255 letras e não pode ser igual ao nome de outro estilo.
4. Confira as outras opções e clique em **OK**.

O novo estilo é salvo com o seu documento, e o seu parágrafo passa a usá-lo. Um **Desfazer** (Ctrl+Z) remove o estilo.

## Alterar um estilo

1. Clique em um parágrafo que tenha o estilo. Ou clique no estilo no painel.
2. Clique em **Modificar** na parte inferior do painel, ou clique em **Modificar Estilo** na guia **Página Inicial**.
3. Altere o que desejar e clique em **OK**.

A caixa de diálogo tem estas opções:

- **Nome** é o nome do estilo. Um estilo que acompanha o JeffWrite mantém o seu nome.
- **Baseado em** é o estilo do qual este parte. O novo estilo se parece com esse estilo, exceto pelo que você alterar. Se você alterar depois o estilo base, este estilo acompanha a mudança.
- **Estilo do parágrafo seguinte** é o estilo que o próximo parágrafo recebe quando você pressiona **Enter** no fim deste. Normalmente, um título é seguido por Normal.
- **Fonte**, **tamanho**, **Negrito**, **Itálico**, **Sublinhado** e **cor** definem a aparência das palavras.
- **Alinhamento**, **recuo**, **espaçamento** e **espaçamento entre linhas** definem a aparência dos parágrafos.
- **Adicionar à galeria de estilos** coloca o estilo na galeria da guia **Página Inicial**.
- **Atualizar automaticamente** faz o estilo acompanhar a sua formatação. Quando você altera um parágrafo que tem este estilo, o estilo muda para corresponder, e todos os outros parágrafos que o usam também mudam.

Ao alterar um estilo, os parágrafos que o usam são atualizados. Se você deu a um parágrafo uma aparência própria, como uma palavra em vermelho, ele mantém essa aparência.

Se você clicar em **OK** sem alterar nada, o documento não é modificado.

## Atualizar um estilo a partir do seu texto

Formate um parágrafo como desejar. Depois, clique em **Atualizar Estilo para Corresponder à Seleção** na guia **Página Inicial**. O estilo desse parágrafo assume as alterações feitas, e todos os parágrafos com esse estilo acompanham a mudança.

## Salvar uma lista como estilo de lista

Um **estilo de lista** é uma lista numerada ou com marcadores que tem um nome. Ele mantém os nove níveis: o número ou marcador de cada nível, seu texto, onde ele começa e onde o texto fica.

1. Clique em **Lista de Vários Níveis** na guia **Página Inicial** e, depois, em **Definir Nova Lista de Vários Níveis…**.
2. Configure os níveis como desejar.
3. Marque **Salvar como estilo de lista** e digite um nome. Se o nome estiver vazio ou já for usado, a caixa avisa e permanece aberta.
4. Clique em **OK**. Os parágrafos selecionados passam a usar o estilo de lista.

O estilo de lista agora aparece em dois lugares: em **Estilos de Lista**, na galeria **Lista de Vários Níveis** (os primeiros 100), e no painel **Estilos** (todos). Selecione parágrafos e clique no nome dele para aplicá-lo.

- **Alterar.** Clique em **Lista de Vários Níveis**, **Modificar Estilo de Lista** e escolha o nome; ou selecione o estilo de lista no painel **Estilos** e clique em **Modificar…**. Todos os parágrafos que usam o estilo de lista mudam junto, no texto e em cabeçalhos, rodapés e notas. No texto, é uma única etapa, então um **Desfazer** a desfaz.
- **Excluir.** Clique em **Lista de Vários Níveis**, **Excluir Estilo de Lista** e escolha o nome. Os parágrafos mantêm seus números e marcadores exatamente como estão; apenas deixam de seguir um estilo.
- **Salvar em um arquivo.** Um estilo de lista é guardado em um arquivo `.docx` como estilo de numeração, e em um arquivo `.odt` como estilo de lista. Ele volta como estilo de lista quando você abre o arquivo novamente. Um arquivo pode ter até 1.000 estilos de lista; os demais são usados como listas comuns.

Nível pode continuar vinculado a um estilo de parágrafo, como em qualquer lista de vários níveis. Nesse caso, o estilo de parágrafo é numerado pelo estilo de lista, e **Modificar Estilo de Lista** mostra o vínculo para que você possa alterá-lo.

Se a caixa não tiver uma lista à qual aplicar o estilo (por exemplo, quando o texto selecionado está bloqueado), nenhum estilo de lista é salvo, e uma mensagem informa isso.

## Relacionados

- [Alterar a aparência do texto](font)
- [Copiar uma aparência com Copiar formatação](format-painter)
- [Usar o JeffWrite com o teclado](keyboard)
