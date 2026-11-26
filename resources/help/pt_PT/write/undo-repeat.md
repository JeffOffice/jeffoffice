# Anular, Refazer e Repetir

<!-- keywords: anular, refazer, repetir, f4, ctrl+y, histórico, voltar atrás, fazer novamente, última ação, passo -->

**{key:Undo}** anula a sua última alteração, e **{key:Redo}** volta a fazê-la. Ambos estão na barra de ferramentas rápida, no topo. Cada comando que dá é um passo, e um conjunto de palavras escritas seguidas é um passo.

## Recuar vários passos de uma só vez

Cada um dos botões **Anular** e **Refazer** tem, ao lado, uma seta. Clique na seta para ver os passos, do mais recente para o mais antigo, com até 100 passos listados. Um passo tem o nome do comando que o criou, como "Negrito" ou "Escrita", e "Edição" quando a alteração veio de outro lado. Desloque o ponteiro para baixo na lista para marcar quantos passos pretende e clique para os executar. Escolher o terceiro passo anula os três primeiros. Os passos anulados permanecem na lista de Refazer até fazer uma nova alteração.

## Repetir o último comando

**F4** executa novamente o seu último comando sobre o que está selecionado agora. Por exemplo, coloque uma palavra a negrito, selecione outra palavra e prima **F4**. A segunda palavra fica a negrito, e um **Anular** desfaz essa alteração. Uma repetição é sempre um passo próprio, mesmo logo a seguir a ter escrito.

Os comandos que ligam ou desligam uma função, como negrito, itálico, sublinhado, marcas e numeração, são repetidos com o resultado obtido, e não como um interruptor. Se colocou uma palavra a negrito, **F4** coloca a seleção seguinte a negrito, mesmo que parte dela já esteja a negrito, e uma seleção que já esteja a negrito fica como está. Se retirou o negrito de uma palavra, **F4** retira o negrito da seleção seguinte.

A repetição funciona para:

- Formatação: negrito, itálico, sublinhado, rasurado, superior à linha, inferior à linha, tipo de letra, tamanho do tipo de letra, cor do tipo de letra, realce, sombreado do texto, alterar maiúsculas e minúsculas, e limpar formatação.
- Definições de parágrafo: alinhamento, avanços, espaçamento, marcas, numeração, limites e sombreado.
- Estilos, da galeria, e o Normal e os estilos de título.
- Comandos de tabela que inserem, eliminam, unem, alinham ou definem limites.
- Quebras de página e páginas em branco, e os carateres especiais que insere com um comando.
- Escrita: depois de escrever algumas palavras, **F4** escreve as mesmas palavras novamente onde estiver o cursor. Só são guardadas as palavras de uma sequência, até 1000 letras, e uma sequência termina quando move o cursor. A sequência corresponde ao texto tal como ficou, após correções como a Correção Automática, pelo que uma repetição insere essas palavras tal como estão. O Retrocesso encurta a sequência pelas letras que elimina.

A repetição não funciona para abrir e guardar, alterar a vista, Localizar e Substituir, copiar e colar, nem para comandos que abrem uma janela para lhe perguntar algo. Esses comandos nunca são o "último comando" para a Repetição, pelo que **F4** continua a repetir a formatação que aplicou antes deles.

A dica do botão da barra de ferramentas rápida indica o que o comando faria, por exemplo "Repetir Negrito (F4)". A repetição segue as mesmas regras do comando que repete: com o Controlo de Alterações ativado, a repetição é registada como uma alteração, e num documento aberto apenas para leitura, ou protegido, não faz nada e indica o motivo na barra de estado. Cada documento tem o seu próprio último comando, pelo que a Repetição nunca transporta um comando de um documento para outro.

A repetição funciona onde estiver a editar. Numa caixa de texto ou numa forma, escreve na caixa e executa os comandos que a caixa tem; um comando que a caixa não tenha não é repetido ali, e a barra de estado indica-o. Num cabeçalho, num rodapé, numa nota ou numa célula de tabela, funciona nesse local.

**Ctrl+Y** é **Refazer** enquanto tiver passos para refazer. Quando não tiver nenhum, **Ctrl+Y** repete como **F4**.
