# Usar estilos
<!-- keywords: estilo de lista, guardar um estilo de lista, estilo de lista multinível, estilos de lista, gerir estilos, estilo bloqueado, bloquear um estilo, limitar formatação, restringir formatação, exportar estilos, ficheiro de modelo, dotx, ocultar um estilo, estilos recomendados, estilos, estilo, painel de estilos, título, normal, criar estilo, novo estilo, modificar estilo, alterar estilo, baseado em, estilo seguinte, atualizar automaticamente, galeria, estilo de carácter, estilo de parágrafo, ctrl+alt+shift+s -->

Um **estilo** é um conjunto guardado de aspetos: um tipo de letra, um tamanho, o espaçamento e outros. Quando aplica um estilo a um parágrafo, este adota todos os seus aspetos de uma só vez. Se alterar o estilo, todos os parágrafos que o usam também mudam. É por isso que os estilos mantêm os documentos longos organizados.

O JeffWrite tem dois tipos:

- Um **estilo de parágrafo** define o aspeto de parágrafos inteiros. **Título 1** e **Normal** são estilos de parágrafo.
- Um **estilo de carácter** define o aspeto das palavras que seleciona. **Forte** é um estilo de carácter.

## Aplicar um estilo ao texto

1. Clique num parágrafo ou selecione as palavras que quer alterar.
2. Clique num estilo na galeria **Estilos** do separador **Base**. Ou prima **Ctrl+Shift+N** para Normal, ou **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** para Título 1, 2 ou 3.

Um estilo de parágrafo aplica-se a todos os parágrafos que toca. Um estilo de carácter aplica-se às palavras que selecionou.

## Abrir o painel Estilos

Clique na pequena seta no canto do grupo **Estilos**, ou prima **Ctrl+Alt+Shift+S**. O painel **Estilos** abre-se à direita. Lista os estilos do seu documento, cada um com o seu próprio aspeto.

- O estilo onde está o cursor aparece a negrito, com uma seta antes do nome.
- Clique num estilo para o aplicar à seleção.
- Use a lista abaixo dos estilos para escolher quais mostrar:
  - **Em utilização** mostra apenas os estilos que o seu texto usa.
  - **Todos os estilos** mostra todos os estilos, incluindo os que o seu documento ainda não tem. Um estilo é acrescentado ao documento na primeira vez que o usa.
  - **Recomendados** mostra os estilos que devem ser visíveis. Um estilo que fica oculto até ser usado aparece aqui assim que o utilizar.
- Clique em **Gerir…** abaixo da lista para escolher, para cada estilo, se é **Recomendado** (mostrado na galeria), se está **Oculto** (excluído de todas as listas) e a sua **Ordem nas listas**. Um número mais baixo aparece primeiro. Clique em **OK**, e todas as alterações acontecem num único passo, pelo que um **Anular** as desfaz todas.
- Em **Gerir…**, clique em **Eliminar** para remover um estilo que criou. Os parágrafos que o tinham voltam a Normal, e os estilos baseados nele passam a basear-se no estilo em que ele próprio se baseava. Os estilos que vêm com o JeffWrite não podem ser eliminados. Eliminar é um passo único, pelo que um **Anular** repõe o estilo.
- Em **Gerir…**, clique em **Importar de ficheiro…** para copiar estilos de outro documento. Escolha o ficheiro, marque os estilos que pretende e clique em **OK**. Um estilo que necessite de outro estilo traz esse outro consigo. Os estilos que já tem ficam intactos, a menos que marque **Substituir estilos com o mesmo nome**. Importar é um passo que pode ser anulado.
- Em **Gerir…**, clique em **Exportar para ficheiro…** para guardar alguns dos seus estilos num ficheiro de modelo (um ficheiro **.dotx**). Marque os estilos que pretende, clique em **OK** e escolha onde guardar. Outro documento pode obtê-los desse ficheiro com **Importar de ficheiro…**. Um estilo que necessite de outro estilo traz esse outro consigo.
- Marque **Bloqueado** num estilo para o bloquear. Depois, marque **Limitar formatação aos estilos que não estão bloqueados** para ativar a limitação. Enquanto a limitação estiver ativa, ninguém pode formatar o texto manualmente, por exemplo com **Negrito** ou **Centrar**, nem usar um estilo bloqueado. Podem continuar a usar todos os outros estilos. A barra de estado indica quando um comando é recusado. Desmarque a caixa para desativar a limitação. Se o documento estiver protegido com palavra-passe, interrompa primeiro essa proteção, em **Rever > Restringir Edição**. A limitação e os bloqueios ficam guardados no ficheiro.
- Abaixo da lista, a caixa **Ordenar** coloca os estilos por ordem. **Alfabeticamente** ordena pelo nome. **Conforme recomendado** coloca primeiro os estilos mais úteis. **Por tipo** lista os estilos de parágrafo, depois os estilos associados e, por fim, os estilos de carácter. **Baseado em** agrupa cada estilo com o estilo em que se baseia.

## Criar um estilo a partir do texto

1. Formate um parágrafo como pretende. Por exemplo, coloque-o a negrito, centrado e defina o tamanho.
2. Clique em **Novo Estilo** na parte inferior do painel, ou clique em **Criar um Estilo** no separador **Base**.
3. Escreva um **Nome**. Um nome pode ter até 255 letras e não pode ser o nome de outro estilo.
4. Verifique as outras opções e clique em **OK**.

O novo estilo fica guardado com o seu documento, e o parágrafo passa a usá-lo. Um **Anular** (Ctrl+Z) repõe o estado anterior à criação do estilo.

## Alterar um estilo

1. Clique num parágrafo que tenha o estilo. Ou clique no estilo no painel.
2. Clique em **Modificar** na parte inferior do painel, ou clique em **Modificar Estilo** no separador **Base**.
3. Altere o que pretende e clique em **OK**.

A caixa de diálogo tem estas opções:

- **Nome** é o nome do estilo. Um estilo que vem com o JeffWrite mantém o seu nome.
- **Baseado em** é o estilo de que este parte. O novo estilo parece-se com esse estilo, exceto no que alterar. Se mais tarde alterar o estilo base, este estilo acompanha-o.
- **Estilo do parágrafo seguinte** é o estilo que o parágrafo seguinte recebe quando prime **Enter** no fim deste. Normalmente, um título é seguido de Normal.
- **Tipo de letra**, **tamanho**, **Negrito**, **Itálico**, **Sublinhado** e **cor** definem o aspeto das palavras.
- **Alinhamento**, **avanços**, **espaçamento** e **espaçamento entre linhas** definem o aspeto dos parágrafos.
- **Adicionar à galeria de estilos** coloca o estilo na galeria do separador **Base**.
- **Atualizar automaticamente** faz com que o estilo acompanhe a sua formatação. Quando altera um parágrafo que tem este estilo, o estilo muda para corresponder, e o mesmo acontece com todos os outros parágrafos que o têm.

Quando altera um estilo, os parágrafos que o usam mudam. Se deu a um parágrafo um aspeto próprio, como uma palavra a vermelho, esse aspeto mantém-se.

Se clicar em **OK** sem alterar nada, o documento não é modificado.

## Atualizar um estilo para corresponder ao texto

Formate um parágrafo como pretende. Depois, clique em **Atualizar Estilo para Corresponder à Seleção** no separador **Base**. O estilo desse parágrafo adota as alterações que fez, e todos os parágrafos com esse estilo acompanham-nas.

## Guardar uma lista como estilo de lista

Um **estilo de lista** é uma lista numerada ou com marcas que tem um nome. Mantém os nove níveis: o número ou a marca de cada nível, o respetivo texto, onde começa e onde se situa o texto.

1. Clique em **Lista Multinível** no separador **Base** e, depois, em **Definir Nova Lista Multinível…**.
2. Configure os níveis como pretender.
3. Marque **Guardar como estilo de lista** e escreva um nome. Se o nome estiver vazio ou já for usado, a caixa indica-o e continua aberta.
4. Clique em **OK**. Os parágrafos selecionados passam a usar o estilo de lista.

O estilo de lista aparece agora em dois sítios: em **Estilos de Lista** na galeria **Lista Multinível** (os primeiros 100), e no painel **Estilos** (todos). Selecione parágrafos e clique no nome para lhes atribuir o estilo de lista.

- **Alterar.** Clique em **Lista Multinível**, **Modificar Estilo de Lista** e no nome; ou escolha o estilo de lista no painel **Estilos** e clique em **Modificar…**. Todos os parágrafos que têm o estilo de lista mudam com ele, no texto, nos cabeçalhos, nos rodapés e nas notas. No texto, é um único passo, pelo que um **Anular** o desfaz.
- **Eliminar.** Clique em **Lista Multinível**, **Eliminar Estilo de Lista** e no nome. Os parágrafos mantêm os números e as marcas exatamente como estão. Simplesmente deixam de seguir um estilo.
- **Guardar num ficheiro.** Um estilo de lista é guardado num ficheiro `.docx` como estilo de numeração, e num ficheiro `.odt` como estilo de lista. Volta a ser um estilo de lista quando abrir o ficheiro novamente. Um ficheiro pode ter até 1000 estilos de lista, e os restantes são usados como listas simples.

Um nível pode continuar associado a um estilo de parágrafo, como em qualquer lista multinível. Nesse caso, o estilo de parágrafo é numerado pelo estilo de lista, e **Modificar Estilo de Lista** mostra a ligação para que a possa alterar.

Se a caixa não tiver nenhuma lista à qual atribuir o estilo (por exemplo, se o texto selecionado estiver bloqueado), nenhum estilo de lista é guardado, e uma mensagem informa disso.

## Relacionados

- [Alterar o aspeto do texto](font)
- [Copiar um aspeto com Copiar Formatação](format-painter)
- [Usar o JeffWrite com o teclado](keyboard)
