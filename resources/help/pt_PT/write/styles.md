# Usar estilos
<!-- keywords: estilo de lista, guardar um estilo de lista, estilo de lista multinível, estilos de lista, gerir estilos, estilo bloqueado, bloquear um estilo, limitar formatação, restringir formatação, exportar estilos, ficheiro de modelo, dotx, ocultar um estilo, estilos recomendados, estilos, estilo, painel de estilos, título, normal, criar estilo, novo estilo, modificar estilo, alterar estilo, baseado em, estilo seguinte, atualizar automaticamente, galeria, estilo de carácter, estilo de parágrafo, ctrl+alt+shift+s -->

Um **estilo** é um conjunto guardado de aspetos: um tipo de letra, um tamanho, o espaçamento e outros. Quando aplica um estilo a um parágrafo, o parágrafo adquire todos esses aspetos de uma só vez. Se alterar o estilo, todos os parágrafos que o têm também mudam. É por isso que os estilos mantêm os documentos longos organizados.

O JeffWrite tem dois tipos:

- Um **estilo de parágrafo** define o aspeto de parágrafos inteiros. **Título 1** e **Normal** são estilos de parágrafo.
- Um **estilo de carácter** define o aspeto das palavras que selecionar. **Forte** é um estilo de carácter.

## Aplicar um estilo ao texto

1. Clique num parágrafo ou selecione as palavras que quer alterar.
2. Clique num estilo na galeria **Estilos** no separador **Base**. Ou prima **Ctrl+Shift+N** para Normal, ou **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** para Título 1, 2 ou 3.

Um estilo de parágrafo aplica-se a todos os parágrafos que tocar. Um estilo de carácter aplica-se às palavras que selecionou.

## Abrir o painel Estilos

Clique na pequena seta no canto do grupo **Estilos**, ou prima **Ctrl+Alt+Shift+S**. O painel **Estilos** abre-se à direita. Lista os estilos do seu documento, cada um com o seu próprio aspeto.

- O estilo onde está o cursor aparece a negrito, com uma seta antes do nome.
- Clique num estilo para o aplicar à sua seleção.
- Use a lista por baixo dos estilos para escolher quais mostrar:
  - **Em utilização** mostra apenas os estilos que o seu texto usa.
  - **Todos os estilos** mostra todos os estilos, incluindo os que o documento ainda não tem. Um estilo é adicionado ao documento na primeira vez que o usar.
  - **Recomendados** mostra os estilos que devem ser vistos. Um estilo que fica oculto até ser usado aparece aqui assim que o usar.
- Clique em **Gerir…** abaixo da lista para escolher, para cada estilo, se é **Recomendado** (mostrado na galeria), se está **Oculto** (excluído de todas as listas) e a sua **Ordem nas listas**. Um número mais baixo aparece primeiro. Clique em **OK** e todas as alterações acontecem num único passo, por isso um **Anular** as desfaz todas.
- Em **Gerir…**, clique em **Eliminar** para remover um estilo que criou. Os parágrafos que o tinham voltam a Normal, e os estilos que se baseavam nele passam a basear-se no que ele próprio tinha como base. Os estilos que vêm com o JeffWrite não podem ser eliminados. A eliminação é um só passo, por isso um **Anular** repõe o estilo.
- Em **Gerir…**, clique em **Importar de ficheiro…** para copiar estilos de outro documento. Escolha o ficheiro, marque os estilos que quer e clique em **OK**. Um estilo que precise de outro estilo traz esse consigo. Os estilos que já tem ficam como estão, a menos que marque **Substituir estilos com o mesmo nome**. Importar é um passo que se pode anular.
- Em **Gerir…**, clique em **Exportar para ficheiro…** para guardar alguns dos seus estilos num ficheiro de modelo (um ficheiro **.dotx**). Marque os estilos que quer, clique em **OK** e escolha onde guardar. Outro documento pode obtê-los desse ficheiro com **Importar de ficheiro…**. Um estilo que precise de outro estilo traz esse consigo.
- Marque **Bloqueado** para bloquear um estilo. Depois, marque **Limitar a formatação aos estilos que não estão bloqueados** para ativar o limite. Enquanto o limite estiver ativo, ninguém pode formatar o texto manualmente, por exemplo com **Negrito** ou **Centrar**, e ninguém pode usar um estilo bloqueado. Podem continuar a usar todos os outros estilos. A barra de estado indica quando um comando é recusado. Desmarque a caixa para desativar o limite. Se o documento estiver protegido com palavra-passe, pare primeiro essa proteção, em **Rever > Restringir Edição**. O limite e os bloqueios são guardados no ficheiro.
- Por baixo da lista, a caixa **Ordenar** coloca os estilos por ordem. **Alfabética** ordena pelo nome. **Conforme recomendado** coloca primeiro os estilos mais úteis. **Por tipo** lista os estilos de parágrafo, depois os estilos ligados e, por fim, os estilos de carácter. **Baseado em** agrupa cada estilo com o estilo em que assenta.

## Criar um estilo a partir do seu texto

1. Formate um parágrafo como pretende. Por exemplo, coloque-o a negrito, centrado e defina o tamanho.
2. Clique em **Novo Estilo** na parte inferior do painel, ou clique em **Criar um Estilo** no separador **Base**.
3. Escreva um **Nome**. Um nome pode ter até 255 letras, e não pode ser o nome de outro estilo.
4. Verifique as outras opções e clique em **OK**.

O **Tipo de estilo** pode ser **Parágrafo**, **Carácter** ou **Ligado**. Um estilo ligado é um estilo de parágrafo com um estilo de carácter de aspeto igual ao seu lado (com o mesmo nome do estilo de parágrafo, acrescentando "Char"), por isso o aspeto pode ser aplicado a um parágrafo inteiro ou apenas a algumas palavras. Quando alterar mais tarde o estilo de parágrafo, o seu estilo de carácter também adota o novo aspeto.

**Disponível em** escolhe **Apenas neste documento** ou **Novos documentos baseados neste modelo**. Com a segunda opção, o estilo fica também guardado no modelo com que os novos documentos começam.

O novo estilo é guardado com o seu documento, e o seu parágrafo passa a usá-lo. Um **Anular** (Ctrl+Z) remove o estilo.

## Alterar um estilo

1. Clique num parágrafo que tenha o estilo. Ou clique no estilo no painel.
2. Clique em **Modificar** na parte inferior do painel, ou clique em **Modificar Estilo** no separador **Base**.
3. Altere o que pretende e clique em **OK**.

A caixa de diálogo tem estas opções:

- **Nome** é o nome do estilo. Um estilo que vem com o JeffWrite mantém o seu nome.
- **Baseado em** é o estilo do qual este parte. O novo estilo tem o aspeto desse estilo, exceto no que alterar. Se mais tarde alterar o estilo base, este estilo acompanha essa alteração.
- **Estilo do parágrafo seguinte** é o estilo que o parágrafo seguinte recebe quando prime **Enter** no fim deste. Um título é normalmente seguido de Normal.
- **Tipo de letra**, **tamanho**, **Negrito**, **Itálico**, **Sublinhado** e **cor** definem o aspeto das palavras.
- **Alinhamento**, **avanços**, **espaçamento** e **espaçamento entre linhas** definem o aspeto dos parágrafos.
- **Adicionar à galeria de estilos** coloca o estilo na galeria do separador **Base**.
- **Atualizar automaticamente** faz com que o estilo acompanhe a sua formatação. Quando altera um parágrafo que tem este estilo, o estilo muda para corresponder, e o mesmo acontece com todos os outros parágrafos que o têm.

Quando altera um estilo, os parágrafos que o têm mudam também. Se tiver dado a um parágrafo um aspeto próprio, como uma palavra a vermelho, esse aspeto mantém-se.

Se clicar em **OK** sem alterar nada, o documento não é modificado.

## Atualizar um estilo para corresponder ao seu texto

Formate um parágrafo como pretende. Depois, clique em **Atualizar Estilo para Corresponder à Seleção** no separador **Base**. O estilo desse parágrafo assume as alterações que fez, e todos os parágrafos com esse estilo acompanham a mudança.

## Guardar uma lista como estilo de lista

Um **estilo de lista** é uma lista numerada ou com marcas que tem um nome. Mantém os nove níveis: o número ou a marca de cada nível, o seu texto, onde começa e onde se posiciona o texto.

1. Clique em **Lista Multinível** no separador **Base** e, depois, em **Definir Nova Lista Multinível…**.
2. Configure os níveis como pretende.
3. Marque **Guardar como estilo de lista** e escreva um nome. Se o nome estiver vazio ou já for usado, a caixa indica-o e mantém-se aberta.
4. Clique em **OK**. Os parágrafos selecionados passam a usar o estilo de lista.

O estilo de lista aparece agora em dois sítios: em **Estilos de Lista**, na galeria **Lista Multinível** (os primeiros 100), e no painel **Estilos** (todos). Selecione parágrafos e clique no nome para lhes aplicar o estilo de lista.

- **Alterar.** Clique em **Lista Multinível**, **Modificar Estilo de Lista** e o nome, ou escolha o estilo de lista no painel **Estilos** e clique em **Modificar…**. Todos os parágrafos com o estilo de lista mudam com ele, no texto, nos cabeçalhos, nos rodapés e nas notas. No texto é um só passo, por isso um **Anular** o desfaz.
- **Eliminar.** Clique em **Lista Multinível**, **Eliminar Estilo de Lista** e o nome. Os parágrafos mantêm os seus números e marcas exatamente como estão. Simplesmente deixam de seguir um estilo.
- **Guardar num ficheiro.** Um estilo de lista é guardado num ficheiro **.docx** como estilo de numeração, e num ficheiro **.odt** como estilo de lista. Volta a ser um estilo de lista quando abrir o ficheiro novamente. Um ficheiro pode ter até 1000 estilos de lista, e os restantes são usados como listas simples.

Um nível pode continuar ligado a um estilo de parágrafo, como em qualquer lista multinível. Nesse caso, o estilo de parágrafo é numerado pelo estilo de lista, e **Modificar Estilo de Lista** mostra a ligação para que a possa alterar.

Se não houver uma lista à qual aplicar o estilo (por exemplo, porque o texto selecionado está bloqueado), não é guardado nenhum estilo de lista, e uma mensagem indica-o.

## Relacionado

- [Alterar o aspeto do texto](font)
- [Copiar um aspeto com Copiar Formatação](format-painter)
- [Usar o JeffWrite com o teclado](keyboard)
