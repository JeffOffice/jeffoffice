# Desfazer, Refazer e Repetir
<!-- keywords: desfazer, refazer, repetir, f4, ctrl+y, histórico, voltar, fazer de novo, última ação, etapa, passo -->

**{key:Undo}** desfaz sua última alteração, e **{key:Redo}** a refaz. Ambos estão na barra de ferramentas rápida, na parte superior. Cada comando que você executa é uma etapa, e uma sequência de palavras digitadas também é uma etapa.

## Voltar várias etapas de uma vez

Cada um dos botões **Desfazer** e **Refazer** tem uma seta ao lado. Clique na seta para ver as etapas, da mais recente para a mais antiga, com até 100 listadas. Cada etapa tem o nome do comando que a fez, como "Negrito" ou "Digitação", e "Edição" quando a alteração veio de outro lugar. Mova o ponteiro para baixo na lista para marcar quantas etapas você quer, e clique para desfazê-las. Ao escolher a terceira etapa, as três primeiras são desfeitas. As etapas desfeitas permanecem na lista de Refazer até que você faça uma nova alteração.

## Repetir o último comando

**F4** executa novamente o seu último comando sobre o que está selecionado agora. Por exemplo, deixe uma palavra em negrito, selecione outra palavra e pressione **F4**. A segunda palavra fica em negrito, e um **Desfazer** a desfaz. Uma repetição é sempre uma etapa própria, mesmo logo depois de você ter digitado.

Comandos que ligam ou desligam algo, como negrito, itálico, sublinhado, marcadores e numeração, são repetidos pelo resultado obtido, não como uma alternância. Se você deixou uma palavra em negrito, **F4** deixa a próxima seleção em negrito, mesmo que parte dela já esteja em negrito, e uma seleção que já está em negrito é mantida como está. Se você tirou o negrito de uma palavra, **F4** tira o negrito da próxima seleção.

A repetição funciona para:

- Formatação: negrito, itálico, sublinhado, tachado, sobrescrito, subscrito, fonte, tamanho da fonte, cor da fonte, realce, sombreamento de texto, alterar maiúsculas e minúsculas, e limpar formatação.
- Configurações de parágrafo: alinhamento, recuos, espaçamento, marcadores, numeração, bordas e sombreamento.
- Estilos, da galeria, e os estilos Normal e de título.
- Comandos de tabela que inserem, excluem, mesclam, alinham ou definem bordas.
- Quebras de página e páginas em branco, e os caracteres especiais que você insere com um comando.
- Digitação: depois que você digita algumas palavras, **F4** digita as mesmas palavras novamente onde está o cursor. Somente as palavras de uma sequência são guardadas, até 1.000 letras, e uma sequência termina quando você move o cursor. A sequência é o texto como ficou depois de correções como a AutoCorreção, então a repetição insere essas palavras exatamente como estão. O Backspace encurta a sequência pelas letras que apaga.

A repetição não funciona para abrir e salvar, alterar o modo de exibição, Localizar e Substituir, copiar e colar, ou comandos que abrem uma janela para perguntar algo. Esses comandos nunca são o "último comando" para a repetição, então **F4** continua repetindo a formatação que você fez antes deles.

A dica do botão da barra de ferramentas rápida informa o que ele faria, por exemplo "Repetir Negrito (F4)". A repetição segue as mesmas regras do comando que repete: com o Controlar Alterações ativado, a repetição é registrada como alteração, e em um documento aberto somente para leitura, ou protegido, ela não faz nada e informa o motivo na barra de status. Cada documento tem seu próprio último comando, então a repetição nunca leva um comando de um documento para outro.

A repetição funciona onde você está editando. Em uma caixa de texto ou em uma forma, ela digita na caixa e executa os comandos que a caixa tem; um comando que a caixa não tem não é repetido ali, e a barra de status informa isso. Em um cabeçalho, um rodapé, uma nota ou uma célula de tabela, ela funciona naquele local.

**Ctrl+Y** é **Refazer** enquanto houver etapas para refazer. Quando não houver nenhuma, **Ctrl+Y** repete como **F4**.
