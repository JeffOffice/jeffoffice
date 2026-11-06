# Usar estilos
<!-- keywords: gerenciar estilos, estilo bloqueado, bloquear estilo, limitar formatação, restringir formatação, exportar estilos, arquivo de modelo, dotx, ocultar estilo, estilos recomendados, estilos, estilo, painel de estilos, título, normal, criar estilo, novo estilo, modificar estilo, alterar estilo, baseado em, próximo estilo, atualizar automaticamente, galeria, estilo de caractere, estilo de parágrafo, ctrl+alt+shift+s -->

Um **estilo** é um conjunto salvo de aparências: uma fonte, um tamanho, espaçamentos e outros itens. Quando você aplica um estilo a um parágrafo, ele recebe todas essas aparências de uma vez. Quando você altera o estilo, todos os parágrafos que o usam também mudam. É por isso que os estilos mantêm documentos longos organizados.

O JeffWrite tem dois tipos:

- Um **estilo de parágrafo** define a aparência de parágrafos inteiros. **Título 1** e **Normal** são estilos de parágrafo.
- Um **estilo de caractere** define a aparência das palavras que você seleciona. **Forte** é um estilo de caractere.

## Aplicar um estilo ao seu texto

1. Clique em um parágrafo ou selecione as palavras que deseja alterar.
2. Clique em um estilo na galeria **Estilos**, na guia **Página Inicial**. Ou pressione **Ctrl+Shift+N** para Normal, ou **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** para Título 1, 2 ou 3.

Um estilo de parágrafo é aplicado a todos os parágrafos que você tocar. Um estilo de caractere é aplicado às palavras selecionadas.

## Abrir o painel Estilos

Clique na pequena seta no canto do grupo **Estilos** ou pressione **Ctrl+Alt+Shift+S**. O painel **Estilos** abre à direita. Ele lista os estilos do seu documento, cada um com a própria aparência.

- O estilo onde está o cursor aparece em negrito, com uma seta antes do nome.
- Clique em um estilo para aplicá-lo à sua seleção.
- Use a lista abaixo dos estilos para escolher quais mostrar:
  - **Em Uso** mostra apenas os estilos que o seu texto usa.
  - **Todos os Estilos** mostra todos os estilos, inclusive os que o seu documento ainda não tem. Um estilo é adicionado ao documento na primeira vez que você o usa.
  - **Recomendados** mostra os estilos que devem ser exibidos. Um estilo que fica oculto até ser usado aparece aqui assim que você o usar.
- Clique em **Gerenciar…** abaixo da lista para escolher, para cada estilo, se ele é **Recomendado** (aparece na galeria), se é **Oculto** (não aparece em nenhuma lista) e sua **Ordem nas listas**. Um número menor aparece primeiro. Clique em **OK**, e todas as alterações acontecem de uma vez, então um **Desfazer** as desfaz juntas.
- Em **Gerenciar…**, clique em **Excluir** para remover um estilo que você criou. Os parágrafos que o usavam voltam para Normal, e os estilos baseados nele passam a ser baseados no que ele era baseado. Os estilos que acompanham o JeffWrite não podem ser excluídos. A exclusão é feita em uma única etapa, então **Desfazer** restaura o estilo.
- Em **Gerenciar…**, clique em **Importar de arquivo…** para copiar estilos de outro documento. Escolha o arquivo, marque os estilos desejados e clique em **OK**. Um estilo que depende de outro traz esse outro junto. Os estilos que você já tem são mantidos, a menos que você marque **Substituir estilos que têm o mesmo nome**. A importação pode ser desfeita em uma única etapa.
- Em **Gerenciar…**, clique em **Exportar para arquivo…** para salvar alguns dos seus estilos em um arquivo de modelo (um arquivo **.dotx**). Marque os estilos desejados, clique em **OK** e escolha onde salvar. Outro documento pode obtê-los desse arquivo com **Importar de arquivo…**. Um estilo que depende de outro traz esse outro junto.
- Marque **Bloqueado** em um estilo para bloqueá-lo. Depois, marque **Limitar formatação aos estilos que não estão bloqueados** para ativar o limite. Com o limite ativo, ninguém consegue formatar o texto manualmente, por exemplo com **Negrito** ou **Centralizar**, e ninguém consegue usar um estilo bloqueado. Todos os outros estilos continuam disponíveis. A barra de status informa quando um comando é recusado. Desmarque a caixa para desativar o limite. Se o documento estiver protegido com senha, interrompa essa proteção primeiro, em **Revisão > Restringir Edição**. O limite e os bloqueios são salvos no arquivo.
- Abaixo da lista, a caixa **Classificar** organiza os estilos. **Alfabética** ordena pelo nome. **Como Recomendado** coloca primeiro os estilos mais úteis. **Por Tipo** lista os estilos de parágrafo, depois os estilos vinculados e depois os estilos de caractere. **Baseado Em** agrupa cada estilo com o estilo em que ele se baseia.

## Criar um estilo a partir do seu texto

1. Formate um parágrafo do jeito que deseja. Por exemplo, deixe-o em negrito, centralize-o e defina o tamanho.
2. Clique em **Novo Estilo** na parte inferior do painel, ou clique em **Criar um Estilo** na guia **Página Inicial**.
3. Digite um **Nome**. O nome pode ter até 255 letras e não pode ser igual ao nome de outro estilo.
4. Marque as outras opções desejadas e clique em **OK**.

O novo estilo é salvo com o documento, e o seu parágrafo passa a usá-lo. Um **Desfazer** (Ctrl+Z) remove o estilo.

## Alterar um estilo

1. Clique em um parágrafo que tenha o estilo. Ou clique no estilo no painel.
2. Clique em **Modificar** na parte inferior do painel, ou clique em **Modificar Estilo** na guia **Página Inicial**.
3. Altere o que desejar e clique em **OK**.

A caixa de diálogo tem estas opções:

- **Nome** é o nome do estilo. Um estilo que acompanha o JeffWrite mantém o seu nome.
- **Baseado em** é o estilo do qual este parte. O novo estilo se parece com esse estilo, exceto pelo que você alterar. Se você mudar o estilo base depois, este estilo acompanha a mudança.
- **Estilo do parágrafo seguinte** é o estilo que o próximo parágrafo recebe quando você pressiona **Enter** no fim deste. Um título normalmente é seguido de Normal.
- **Fonte**, **tamanho**, **Negrito**, **Itálico**, **Sublinhado** e **cor** definem a aparência das palavras.
- **Alinhamento**, **recuo**, **espaçamento** e **espaçamento entre linhas** definem a aparência dos parágrafos.
- **Adicionar à galeria de estilos** coloca o estilo na galeria da guia **Página Inicial**.
- **Atualizar automaticamente** faz o estilo acompanhar a sua formatação. Quando você altera um parágrafo que usa este estilo, o estilo muda para combinar, e todos os outros parágrafos que o usam também mudam.

Quando você altera um estilo, os parágrafos que o usam são alterados. Se você deu a um parágrafo uma aparência própria, como uma palavra em vermelho, ele mantém essa aparência.

Se você clicar em **OK** sem alterar nada, o documento não é modificado.

## Atualizar um estilo com base no seu texto

Formate um parágrafo do jeito que deseja. Depois, clique em **Atualizar Estilo para Corresponder à Seleção** na guia **Página Inicial**. O estilo desse parágrafo passa a incorporar as alterações feitas, e todos os parágrafos com esse estilo acompanham a mudança.

## Relacionados

- [Alterar a aparência do texto](font)
- [Copiar uma aparência com Copiar Formatação](format-painter)
- [Usar o JeffWrite com o teclado](keyboard)
