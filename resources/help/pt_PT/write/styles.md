# Usar estilos
<!-- keywords: gerir estilos, estilo bloqueado, bloquear um estilo, limitar formatação, restringir formatação, exportar estilos, ficheiro de modelo, dotx, ocultar um estilo, estilos recomendados, estilos, estilo, painel de estilos, título, normal, criar estilo, novo estilo, modificar estilo, alterar estilo, baseado em, estilo seguinte, atualizar automaticamente, galeria, estilo de carácter, estilo de parágrafo, ctrl+alt+shift+s -->

Um **estilo** é um conjunto guardado de aspetos: um tipo de letra, um tamanho, espaçamentos e mais. Quando aplica um estilo a um parágrafo, o parágrafo adota todos esses aspetos de uma só vez. Quando altera o estilo, todos os parágrafos que o usam também mudam. É por isso que os estilos mantêm os documentos longos organizados.

O JeffWrite tem dois tipos:

- Um **estilo de parágrafo** define o aspeto de parágrafos inteiros. **Título 1** e **Normal** são estilos de parágrafo.
- Um **estilo de carácter** define o aspeto das palavras que seleciona. **Forte** é um estilo de carácter.

## Aplicar um estilo ao texto

1. Clique num parágrafo ou selecione as palavras que pretende alterar.
2. Clique num estilo na galeria **Estilos** do separador **Base**. Ou prima **Ctrl+Shift+N** para Normal, ou **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** para Título 1, 2 ou 3.

Um estilo de parágrafo é aplicado a todos os parágrafos que toca. Um estilo de carácter é aplicado às palavras que selecionou.

## Abrir o painel Estilos

Clique na pequena seta no canto do grupo **Estilos**, ou prima **Ctrl+Alt+Shift+S**. O painel **Estilos** abre-se à direita. Mostra os estilos do seu documento, cada um com o respetivo aspeto.

- O estilo onde está o cursor aparece a negrito, com uma seta antes do nome.
- Clique num estilo para o aplicar à seleção.
- Use a lista por baixo dos estilos para escolher quais mostrar:
  - **Em Utilização** mostra apenas os estilos que o texto usa.
  - **Todos os Estilos** mostra todos os estilos, incluindo os que o documento ainda não tem. Um estilo é acrescentado ao documento na primeira vez que o utiliza.
  - **Recomendados** mostra os estilos que se destinam a ser vistos. Um estilo que fica oculto até ser utilizado aparece aqui assim que o utilizar.
- Clique em **Gerir…** por baixo da lista para escolher, para cada estilo, se é **Recomendado** (mostrado na galeria), se está **Oculto** (excluído de todas as listas) e a sua **Ordem nas listas**. Um número mais baixo aparece primeiro. Clique em **OK**: todas as alterações são feitas de uma só vez, pelo que um **Anular** as desfaz todas.
- Em **Gerir…**, clique em **Eliminar** para remover um estilo que tenha criado. Os parágrafos que o tinham voltam a Normal, e os estilos que se baseavam nele passam a basear-se no estilo em que ele se baseava. Os estilos que vêm com o JeffWrite não podem ser eliminados. Eliminar é um único passo, pelo que um **Anular** repõe o estilo.
- Em **Gerir…**, clique em **Importar de ficheiro…** para copiar estilos de outro documento. Escolha o ficheiro, marque os estilos que pretende e clique em **OK**. Um estilo que precise de outro estilo traz esse consigo. Os estilos que já tem ficam como estão, a menos que marque **Substituir estilos com o mesmo nome**. Importar é um único passo, que pode ser anulado.
- Em **Gerir…**, clique em **Exportar para ficheiro…** para guardar alguns dos seus estilos num ficheiro de modelo (um ficheiro **.dotx**). Marque os estilos que pretende, clique em **OK** e escolha onde guardar. Outro documento pode obtê-los desse ficheiro com **Importar de ficheiro…**. Um estilo que precise de outro estilo traz esse consigo.
- Marque **Bloqueada** num estilo para o bloquear. Depois marque **Limitar formatação aos estilos que não estão bloqueados** para ativar a limitação. Enquanto a limitação estiver ativa, ninguém pode formatar o texto manualmente, por exemplo com **Negrito** ou **Centrar**, nem pode usar um estilo bloqueado. Podem continuar a usar todos os outros estilos. A barra de estado indica quando um comando é recusado. Desmarque a caixa para desativar a limitação. Se o documento estiver protegido com palavra-passe, desative primeiro essa proteção, em **Rever > Restringir Edição**. A limitação e os bloqueios ficam guardados no ficheiro.
- Por baixo da lista, a caixa **Ordenar** coloca os estilos por ordem. **Alfabeticamente** ordena pelo nome. **Conforme Recomendado** coloca primeiro os estilos mais úteis. **Por Tipo** lista os estilos de parágrafo, depois os estilos ligados e, por fim, os estilos de carácter. **Baseado Em** agrupa cada estilo com o estilo em que assenta.

## Criar um estilo a partir do texto

1. Formate um parágrafo como pretende. Por exemplo, coloque-o a negrito, centre-o e defina o tamanho.
2. Clique em **Novo Estilo** na parte inferior do painel, ou clique em **Criar um Estilo** no separador **Base**.
3. Escreva um **Nome**. Um nome pode ter até 255 letras e não pode ser o nome de outro estilo.
4. Marque as outras opções e clique em **OK**.

O novo estilo fica guardado com o documento, e o seu parágrafo passa a usá-lo. Um **Anular** (Ctrl+Z) remove o estilo.

## Alterar um estilo

1. Clique num parágrafo que tenha o estilo. Ou clique no estilo no painel.
2. Clique em **Modificar** na parte inferior do painel, ou clique em **Modificar Estilo** no separador **Base**.
3. Altere o que pretende e clique em **OK**.

A caixa de diálogo tem estas opções:

- **Nome** é o nome do estilo. Um estilo que vem com o JeffWrite mantém o seu nome.
- **Baseado em** é o estilo de que este parte. O novo estilo tem o aspeto desse estilo, exceto no que alterar. Se alterar mais tarde o estilo base, este segue-o.
- **Estilo para o parágrafo seguinte** é o estilo que o parágrafo seguinte recebe quando prime **Enter** no fim deste. Normalmente, um título é seguido de Normal.
- **Tipo de letra**, **tamanho**, **Negrito**, **Itálico**, **Sublinhado** e **cor** definem o aspeto das palavras.
- **Alinhamento**, **avanços**, **espaçamento** e **espaçamento entre linhas** definem o aspeto dos parágrafos.
- **Acrescentar à galeria de estilos** coloca o estilo na galeria do separador **Base**.
- **Atualizar automaticamente** faz com que o estilo acompanhe a sua formatação. Quando altera um parágrafo que tem este estilo, o estilo muda para corresponder, e o mesmo acontece a todos os outros parágrafos que o têm.

Quando altera um estilo, os parágrafos que o usam mudam. Se deu a um parágrafo um aspeto próprio, como uma palavra a vermelho, esse aspeto mantém-se.

Se clicar em **OK** sem alterar nada, o documento não é alterado.

## Atualizar um estilo para corresponder ao texto

Formate um parágrafo como pretende. Depois clique em **Atualizar Estilo para Corresponder à Seleção** no separador **Base**. O estilo desse parágrafo adota as alterações que fez, e todos os parágrafos com esse estilo acompanham-nas.

## Ver também

- [Alterar o aspeto do texto](font)
- [Copiar um aspeto com Copiar Formatação](format-painter)
- [Usar o JeffWrite com o teclado](keyboard)
