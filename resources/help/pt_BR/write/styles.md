# Usar estilos
<!-- keywords: estilo de lista, salvar estilo de lista, estilo de lista multinível, estilos de lista, gerenciar estilos, estilo bloqueado, bloquear estilo, limitar formatação, restringir formatação, exportar estilos, arquivo de modelo, dotx, ocultar estilo, estilos recomendados, estilos, estilo, painel de estilos, título, normal, criar estilo, novo estilo, modificar estilo, alterar estilo, baseado em, próximo estilo, atualizar automaticamente, galeria, estilo de caractere, estilo de parágrafo, ctrl+alt+shift+s -->

Um **estilo** é um conjunto salvo de aparências: uma fonte, um tamanho, espaçamento e outras características. Quando você aplica um estilo a um parágrafo, ele recebe todas essas aparências de uma vez. Quando você altera o estilo, todos os parágrafos que o usam também mudam. É por isso que os estilos mantêm documentos longos organizados.

O JeffWrite tem dois tipos:

- Um **estilo de parágrafo** define a aparência de parágrafos inteiros. **Título 1** e **Normal** são estilos de parágrafo.
- Um **estilo de caractere** define a aparência das palavras que você seleciona. **Forte** é um estilo de caractere.

## Aplicar um estilo ao seu texto

1. Clique em um parágrafo ou selecione as palavras que deseja alterar.
2. Clique em um estilo na galeria **Estilos** da guia **Página Inicial**. Ou pressione **Ctrl+Shift+N** para Normal, ou **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** para Título 1, 2 ou 3.

Um estilo de parágrafo é aplicado a cada parágrafo que você tocar. Um estilo de caractere é aplicado às palavras selecionadas.

## Abrir o painel Estilos

Clique na pequena seta no canto do grupo **Estilos**, ou pressione **Ctrl+Alt+Shift+S**. O painel **Estilos** abre à direita. Ele lista os estilos do seu documento, cada um com a sua própria aparência.

- O estilo onde está o cursor aparece em negrito, com uma seta antes do nome.
- Clique em um estilo para aplicá-lo à sua seleção.
- Use a lista abaixo dos estilos para escolher quais mostrar:
  - **Em Uso** mostra apenas os estilos que o seu texto usa.
  - **Todos os Estilos** mostra todos os estilos, inclusive os que o documento ainda não tem. Um estilo é adicionado ao documento na primeira vez em que você o usa.
  - **Recomendados** mostra os estilos que devem ser vistos. Um estilo que fica oculto até ser usado aparece aqui assim que você o usa.
- Clique em **Gerenciar…** abaixo da lista para escolher, em cada estilo, se ele é **Recomendado** (aparece na galeria), se é **Oculto** (não aparece em nenhuma lista) e a sua **Ordem nas listas**. Um número menor aparece primeiro. Clique em **OK**, e todas as alterações acontecem de uma vez, então um **Desfazer** as desfaz juntas.
- Em **Gerenciar…**, clique em **Excluir** para remover um estilo que você criou. Os parágrafos que o usavam voltam para Normal, e os estilos baseados nele passam a se basear no que ele se baseava. Os estilos que acompanham o JeffWrite não podem ser excluídos. A exclusão é uma única etapa, então um **Desfazer** restaura o estilo.
- Em **Gerenciar…**, clique em **Importar de arquivo…** para copiar estilos de outro documento. Escolha o arquivo, marque os estilos desejados e clique em **OK**. Um estilo que depende de outro traz esse outro junto. Os estilos que você já tem são mantidos, a menos que você marque **Substituir estilos com o mesmo nome**. A importação é uma única etapa para desfazer.
- Em **Gerenciar…**, clique em **Exportar para arquivo…** para salvar alguns dos seus estilos em um arquivo de modelo (um arquivo **.dotx**). Marque os estilos desejados, clique em **OK** e escolha onde salvar. Outro documento pode obtê-los desse arquivo com **Importar de arquivo…**. Um estilo que depende de outro traz esse outro junto.
- Marque **Bloqueado** em um estilo para bloqueá-lo. Depois marque **Limitar formatação aos estilos que não estão bloqueados** para ativar a limitação. Com a limitação ativa, ninguém pode formatar o texto manualmente, por exemplo com **Negrito** ou **Centralizar**, e ninguém pode usar um estilo bloqueado. Os demais estilos continuam disponíveis. A barra de status informa quando um comando é recusado. Desmarque a caixa para desativar a limitação. Se o documento estiver protegido com senha, interrompa essa proteção primeiro, em **Revisão > Restringir Edição**. A limitação e os bloqueios são salvos no arquivo.
- Abaixo da lista, a caixa **Classificar** organiza os estilos. **Alfabética** ordena pelo nome. **Como Recomendado** coloca os estilos mais úteis primeiro. **Por Tipo** lista os estilos de parágrafo, depois os estilos vinculados e depois os estilos de caractere. **Baseado Em** agrupa cada estilo com aquele em que ele se baseia.

## Criar um estilo a partir do seu texto

1. Formate um parágrafo do jeito que você quer. Por exemplo, deixe-o em negrito, centralize-o e defina o tamanho.
2. Clique em **Novo Estilo** na parte inferior do painel, ou clique em **Criar um Estilo** na guia **Página Inicial**.
3. Digite um **Nome**. O nome pode ter até 255 letras e não pode ser o nome de outro estilo.
4. Verifique as outras opções e clique em **OK**.

O **Tipo de estilo** pode ser **Parágrafo**, **Caractere** ou **Vinculado**. Um estilo vinculado é um estilo de parágrafo com um estilo de caractere de mesma aparência ao lado (com o mesmo nome do estilo de parágrafo, acrescido de "Char"), para que a aparência possa ser aplicada a um parágrafo inteiro ou apenas a algumas palavras. Quando você alterar depois o estilo de parágrafo, o estilo de caractere também assume a nova aparência.

**Disponível em** permite escolher **Somente neste documento** ou **Novos documentos baseados neste modelo**. Com a segunda opção, o estilo também é salvo no modelo do qual os novos documentos partem.

O novo estilo é salvo com o seu documento, e o seu parágrafo passa a usá-lo. Um **Desfazer** (Ctrl+Z) remove o estilo.

## Alterar um estilo

1. Clique em um parágrafo que tenha o estilo. Ou clique no estilo no painel.
2. Clique em **Modificar** na parte inferior do painel, ou clique em **Modificar Estilo** na guia **Página Inicial**.
3. Altere o que desejar e clique em **OK**.

A caixa de diálogo tem estas opções:

- **Nome** é o nome do estilo. Um estilo que acompanha o JeffWrite mantém o seu nome.
- **Baseado em** é o estilo do qual este parte. O novo estilo tem a aparência desse estilo, exceto pelo que você alterar. Se você alterar depois o estilo base, este estilo acompanhará a mudança.
- **Estilo do parágrafo seguinte** é o estilo que o próximo parágrafo recebe quando você pressiona **Enter** no fim deste. Um título costuma ser seguido de Normal.
- **Fonte**, **tamanho**, **Negrito**, **Itálico**, **Sublinhado** e **cor** definem a aparência das palavras.
- **Alinhamento**, **recuo**, **espaçamento** e **espaçamento entre linhas** definem a aparência dos parágrafos.
- **Adicionar à galeria de estilos** coloca o estilo na galeria da guia **Página Inicial**.
- **Atualizar automaticamente** faz o estilo acompanhar a sua formatação. Quando você altera um parágrafo que usa este estilo, o estilo muda para acompanhar, e todos os outros parágrafos que o usam também mudam.

Quando você altera um estilo, os parágrafos que o usam também mudam. Se você deu a um parágrafo uma aparência própria, como uma palavra em vermelho, ela é mantida.

Se clicar em **OK** sem alterar nada, o documento não é modificado.

## Atualizar um estilo para corresponder ao seu texto

Formate um parágrafo do jeito que você quer. Depois clique em **Atualizar Estilo para Corresponder à Seleção** na guia **Página Inicial**. O estilo desse parágrafo assume as alterações feitas, e todos os parágrafos com esse estilo acompanham a mudança.

## Salvar uma lista como estilo de lista

Um **estilo de lista** é uma lista numerada ou com marcadores que tem um nome. Ele guarda os nove níveis: o número ou marcador de cada nível, o texto, onde ele começa e onde o texto fica.

1. Clique em **Lista Multinível** na guia **Página Inicial** e depois em **Definir Nova Lista Multinível…**.
2. Configure os níveis do jeito que você quer.
3. Marque **Salvar como estilo de lista** e digite um nome. Se o nome estiver vazio ou já estiver em uso, a caixa informa isso e permanece aberta.
4. Clique em **OK**. Os parágrafos selecionados passam a usar o estilo de lista.

O estilo de lista agora aparece em dois lugares: em **Estilos de Lista**, na galeria **Lista Multinível** (os primeiros 100), e no painel **Estilos** (todos eles). Selecione parágrafos e clique no nome dele para aplicar o estilo de lista.

- **Alterá-lo.** Clique em **Lista Multinível**, em **Modificar Estilo de Lista** e no nome; ou escolha o estilo de lista no painel **Estilos** e clique em **Modificar…**. Todos os parágrafos que usam o estilo de lista mudam junto, no texto, nos cabeçalhos, nos rodapés e nas notas. No texto, a mudança é uma única etapa, então um **Desfazer** a desfaz.
- **Excluí-lo.** Clique em **Lista Multinível**, em **Excluir Estilo de Lista** e no nome. Os parágrafos mantêm seus números e marcadores exatamente como estão. Eles apenas deixam de seguir um estilo.
- **Salvá-lo em um arquivo.** Um estilo de lista é guardado em um arquivo `.docx` como estilo de numeração, e em um arquivo `.odt` como estilo de lista. Ele volta a ser um estilo de lista quando você abre o arquivo de novo. Um arquivo pode ter até 1.000 estilos de lista, e os restantes são usados como listas simples.

Um nível ainda pode ser vinculado a um estilo de parágrafo, como em qualquer lista multinível. Nesse caso, o estilo de parágrafo é numerado pelo estilo de lista, e **Modificar Estilo de Lista** mostra o vínculo para que você possa alterá-lo.

Se a caixa não tiver uma lista à qual aplicar o estilo (por exemplo, quando o texto selecionado está bloqueado), nenhum estilo de lista é salvo, e uma mensagem informa isso.

## Veja também

- [Alterar a aparência do texto](font)
- [Copiar uma aparência com Copiar Formatação](format-painter)
- [Usar o JeffWrite pelo teclado](keyboard)
