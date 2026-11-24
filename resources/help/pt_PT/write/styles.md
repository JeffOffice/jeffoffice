# Usar estilos
<!-- keywords: estilo de lista, guardar um estilo de lista, estilo de lista multinível, estilos de lista, gerir estilos, estilo bloqueado, bloquear um estilo, limitar formatação, restringir formatação, exportar estilos, ficheiro de modelo, dotx, ocultar um estilo, estilos recomendados, estilos, estilo, painel de estilos, título, normal, criar estilo, novo estilo, modificar estilo, alterar estilo, baseado em, estilo seguinte, atualizar automaticamente, galeria, estilo de carácter, estilo de parágrafo, ctrl+alt+shift+s -->

Um **estilo** é um conjunto guardado de aspetos: um tipo de letra, um tamanho, um espaçamento e mais. Quando atribui um estilo a um parágrafo, o parágrafo adota todos os seus aspetos de uma só vez. Quando altera o estilo, todos os parágrafos que o têm também mudam. É por isso que os estilos mantêm os documentos longos organizados.

O JeffWrite tem dois tipos:

- Um **estilo de parágrafo** define o aspeto de parágrafos inteiros. **Título 1** e **Normal** são estilos de parágrafo.
- Um **estilo de carácter** define o aspeto das palavras que seleciona. **Forte** é um estilo de carácter.

## Atribuir um estilo ao seu texto

1. Clique num parágrafo ou selecione as palavras que pretende alterar.
2. Clique num estilo na galeria **Estilos** do separador **Base**. Ou prima **Ctrl+Shift+N** para Normal, ou **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** para Título 1, 2 ou 3.

Um estilo de parágrafo é aplicado a todos os parágrafos que toca. Um estilo de carácter é aplicado às palavras que selecionou.

## Abrir o painel Estilos

Clique na pequena seta no canto do grupo **Estilos**, ou prima **Ctrl+Alt+Shift+S**. O painel **Estilos** abre-se à direita. Lista os estilos do seu documento, cada um com o seu próprio aspeto.

- O estilo onde está o cursor aparece a negrito, com uma seta antes do nome.
- Clique num estilo para o atribuir à sua seleção.
- Use a lista por baixo dos estilos para escolher quais mostrar:
  - **Em Utilização** mostra apenas os estilos usados no texto.
  - **Todos os Estilos** mostra todos os estilos, incluindo os que o seu documento ainda não tem. Um estilo é adicionado ao documento na primeira vez que o usa.
  - **Recomendados** mostra os estilos que se pretende que sejam vistos. Um estilo que fica oculto até ser usado aparece aqui assim que o usar.
- Clique em **Gerir…** abaixo da lista para escolher, para cada estilo, se é **Recomendado** (mostrado na galeria), se está **Oculto** (excluído de todas as listas) e a sua **Ordem nas listas**. Um número mais baixo aparece primeiro. Clique em **OK**, e todas as alterações acontecem de uma só vez, por isso um **Anular** as desfaz todas.
- Em **Gerir…**, clique em **Eliminar** para remover um estilo que criou. Os parágrafos que o tinham voltam a Normal, e os estilos que se baseavam nele passam a basear-se naquilo em que ele se baseava. Os estilos que vêm com o JeffWrite não podem ser eliminados. Eliminar é um só passo, por isso **Anular** repõe o estilo.
- Em **Gerir…**, clique em **Importar de ficheiro…** para copiar estilos de outro documento. Escolha o ficheiro, marque os estilos que pretende e clique em **OK**. Um estilo que precise de outro estilo traz esse consigo. Os estilos que já tem ficam como estão, a menos que marque **Substituir estilos com o mesmo nome**. Importar é um só passo para anular.
- Em **Gerir…**, clique em **Exportar para ficheiro…** para guardar alguns dos seus estilos num ficheiro de modelo (um ficheiro **.dotx**). Marque os estilos que pretende, clique em **OK** e escolha onde guardar. Outro documento pode obtê-los desse ficheiro com **Importar de ficheiro…**. Um estilo que precise de outro estilo traz esse consigo.
- Marque **Bloqueado** num estilo para o bloquear. Depois, marque **Limitar formatação aos estilos que não estão bloqueados** para ativar o limite. Enquanto o limite estiver ativo, ninguém pode formatar o texto manualmente, como com **Negrito** ou **Centrar**, e ninguém pode usar um estilo bloqueado. Podem continuar a usar todos os outros estilos. A barra de estado indica quando um comando é recusado. Desmarque a caixa para desativar o limite. Se o documento estiver protegido com palavra-passe, pare primeiro essa proteção, em **Rever > Restringir Edição**. O limite e os bloqueios são guardados no ficheiro.
- Por baixo da lista, a caixa **Ordenar** coloca os estilos por ordem. **Alfabeticamente** ordena pelo nome. **Conforme Recomendado** coloca primeiro os estilos mais úteis. **Por Tipo** lista os estilos de parágrafo, depois os estilos ligados e depois os estilos de carácter. **Baseado Em** agrupa cada estilo com o estilo em que assenta.

## Criar um estilo a partir do seu texto

1. Formate um parágrafo como pretende. Por exemplo, coloque-o a negrito, centre-o e defina o tamanho.
2. Clique em **Novo Estilo** na parte inferior do painel, ou clique em **Criar um Estilo** no separador **Base**.
3. Escreva um **Nome**. Um nome pode ter até 255 letras e não pode ser o nome de outro estilo.
4. Verifique as outras opções e clique em **OK**.

O **Tipo de estilo** pode ser **Parágrafo**, **Carácter** ou **Ligado**. Um estilo ligado é um estilo de parágrafo com um estilo de carácter de aspeto igual ao lado (com o mesmo nome do estilo de parágrafo, acrescentado de "Carácter"), para que o aspeto possa ser aplicado a um parágrafo inteiro ou apenas a algumas palavras. Quando mais tarde alterar o estilo de parágrafo, o seu estilo de carácter também assume o novo aspeto.

Um estilo de **Tabela** trata das linhas, da linha de cabeçalho e das linhas alternadas de uma tabela, e um estilo de **Lista** trata dos números ou marcas de uma lista. Para estes dois, escreva o **Nome** e clique em **OK**, e a caixa desse tipo abre-se já com o nome preenchido. Uma lista criada desta forma é sempre guardada como estilo de lista. Um estilo de tabela ou de lista não é mantido ao guardar o documento como ficheiro .odt (estilos de tabela) ou .rtf (ambos); Verificar Compatibilidade (Ficheiro > Informações) indica-lhe isso.

**Disponível em** escolhe **Apenas neste documento** ou **Novos documentos baseados neste modelo**. Com a segunda opção, o estilo fica também guardado no modelo do qual partem os novos documentos.

O novo estilo é guardado com o seu documento, e o parágrafo assume-o. Um **Anular** (Ctrl+Z) retira o estilo.

## Alterar um estilo

1. Clique num parágrafo que tenha o estilo. Ou clique no estilo no painel.
2. Clique em **Modificar** na parte inferior do painel, ou clique em **Modificar Estilo** no separador **Base**.
3. Altere o que pretende e clique em **OK**.

A caixa de diálogo tem estas opções:

- **Nome** é o nome do estilo. Um estilo que vem com o JeffWrite mantém o seu nome.
- **Baseado em** é o estilo do qual este parte. O novo estilo parece-se com esse estilo, exceto no que alterar. Se mais tarde alterar o estilo base, este estilo acompanha-o.
- **Estilo do parágrafo seguinte** é o estilo que o parágrafo seguinte recebe quando prime **Enter** no fim deste. Normalmente, a um título segue-se Normal.
- **Tipo de letra**, **tamanho**, **Negrito**, **Itálico**, **Sublinhado** e **cor** definem o aspeto das palavras.
- **Alinhamento**, **avanço**, **espaçamento** e **espaçamento entre linhas** definem o aspeto dos parágrafos.
- **Adicionar à galeria de estilos** coloca o estilo na galeria do separador **Base**.
- **Atualizar automaticamente** faz com que o estilo acompanhe a sua formatação. Quando altera um parágrafo que tem este estilo, o estilo muda para corresponder, e o mesmo acontece com todos os outros parágrafos que o têm.

Quando altera um estilo, os parágrafos que o têm são alterados. Se tiver dado a um parágrafo um aspeto próprio, como uma palavra a vermelho, esse aspeto mantém-se.

Se clicar em **OK** sem alterar nada, o documento não é alterado.

## Ver de onde vem o aspeto do seu texto

Clique em **Inspetor de Estilos** na parte inferior do painel Estilos. O painel acompanha o cursor, no texto, num cabeçalho, num rodapé ou numa nota, e mostra três coisas:

- O **Estilo de parágrafo** do parágrafo. Se o parágrafo estiver numa tabela, o estilo da tabela aparece por baixo. Por baixo do nome, "Mais" lista o que o parágrafo define por si próprio, por cima do seu estilo, como o alinhamento, os avanços, o espaçamento, as linhas, as posições de tabulação, os limites, o sombreado, a quebra de página antes, o nível de contorno, a numeração ou as marcas e a direção do texto. **Limpar Formatação de Parágrafo** remove essas definições, para que o parágrafo apareça como o seu estilo indica. O que o estilo de uma tabela dá aos seus parágrafos não é listado, porque não é próprio do parágrafo.
- O **Estilo de carácter** do texto, se tiver um. **Limpar Estilo de Carácter** remove-o.
- A **Formatação de texto** que as letras têm por si próprias, como um tipo de letra, um tamanho, negrito, uma cor, texto oculto ou um limite. **Limpar Formatação de Carácter** remove-a, para que as letras apareçam como os seus estilos indicam. O idioma do texto e o seu estilo de carácter mantêm-se.

Selecione primeiro o texto para limpar apenas esse texto. Sem texto selecionado, o painel descreve a letra antes do cursor, e **Limpar Estilo de Carácter** e **Limpar Formatação de Carácter** atuam sobre todo o trecho de letras à volta do cursor que têm o mesmo aspeto que essa letra. Se selecionar vários parágrafos ou letras diferentes, o painel indica "Vários estilos na seleção" ou "Vários aspetos na seleção", lista o que a seleção contém, e os botões atuam sobre tudo.

Cada botão é um só passo, por isso **Anular** (Ctrl+Z) repõe o aspeto, e o cursor fica onde estava. Alterar um estilo nunca faz com que os seus parágrafos passem a definir o aspeto por si próprios, por isso um parágrafo que segue o seu estilo não mostra nada em "Mais". Shift+F1 continua a abrir Mostrar Formatação, que mostra o aspeto em si.

## Atualizar um estilo para corresponder ao seu texto

Formate um parágrafo como pretende. Depois, clique em **Atualizar Estilo para Corresponder à Seleção** no separador **Base**. O estilo desse parágrafo assume as alterações que fez, e todos os parágrafos com esse estilo acompanham-nas.

## Guardar uma lista como estilo de lista

Um **estilo de lista** é uma lista numerada ou com marcas, com um nome. Mantém os nove níveis: o número ou marca de cada nível, o seu texto, onde começa e onde se posiciona o texto.

1. Clique em **Lista Multinível** no separador **Base** e, depois, em **Definir Nova Lista Multinível…**.
2. Configure os níveis como pretende.
3. Marque **Guardar como estilo de lista** e escreva um nome. Se o nome estiver vazio ou já for usado, a caixa indica-o e continua aberta.
4. Clique em **OK**. Os parágrafos selecionados assumem o estilo de lista.

O estilo de lista passa a aparecer em dois locais: em **Estilos de Lista** na galeria **Lista Multinível** (os primeiros 100), e no painel **Estilos** (todos). Selecione parágrafos e clique no nome para lhes dar o estilo de lista.

- **Alterá-lo.** Clique em **Lista Multinível**, **Modificar Estilo de Lista** e no nome, ou escolha o estilo de lista no painel **Estilos** e clique em **Modificar…**. Todos os parágrafos que têm o estilo de lista mudam com ele, no texto e em cabeçalhos, rodapés e notas. No texto, é um só passo, por isso um **Anular** o desfaz.
- **Eliminá-lo.** Clique em **Lista Multinível**, **Eliminar Estilo de Lista** e no nome. Os parágrafos mantêm os seus números e marcas exatamente como estão. Apenas deixam de seguir um estilo.
- **Guardá-lo num ficheiro.** Um estilo de lista é guardado num ficheiro `.docx` como estilo de numeração, e num ficheiro `.odt` como estilo de lista. Volta a ser um estilo de lista quando abrir o ficheiro novamente. Um ficheiro pode ter até 1000 estilos de lista, e os restantes são usados como listas simples.

Um nível pode continuar ligado a um estilo de parágrafo, como em qualquer lista multinível. O estilo de parágrafo fica então numerado pelo estilo de lista, e **Modificar Estilo de Lista** mostra a ligação para que a possa alterar.

Se a caixa não tiver uma lista à qual atribuir o estilo (por exemplo, se o texto selecionado estiver bloqueado), nenhum estilo de lista é guardado, e uma mensagem informa disso.

## Relacionados

- [Alterar o aspeto do texto](font)
- [Copiar um aspeto com Copiar Formatação](format-painter)
- [Usar o JeffWrite com o teclado](keyboard)
