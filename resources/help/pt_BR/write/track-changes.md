# Controlar alterações e aceitar ou rejeitar

<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações controladas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, exibir para revisão, marcação simples, todas as marcações, sem marcação, original, mostrar marcação, balões, painel de revisão, barra de alteração, inserção, exclusão, ctrl+shift+e, autor, revisor, nome de usuário, edições de outra pessoa -->

O **Controlar Alterações** registra cada edição que você faz, para que você ou outra pessoa possa ver o que mudou e decidir se deseja mantê-la. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai revisar depois de você.

## Ativar e desativar o Controlar Alterações

Clique em **Revisão > Controlar Alterações**, ou pressione **Ctrl+Shift+E**. A barra de status na parte inferior da janela indica se ele está ativado. Um clique nessas palavras alterna a configuração. A configuração é salva no documento, portanto continua ativada quando o documento voltar de outra pessoa.

Enquanto estiver ativado:

- **O que você digita** é marcado como inserção.
- **O que você exclui** não é removido. Ele permanece no texto, riscado, para que possa ser restaurado. A exceção é o texto que você mesmo digitou com o controle ativado: excluí-lo apenas o remove, porque ninguém mais o viu.
- **Uma quebra de parágrafo que você insere** (Enter) é marcada como inserida, e uma que você exclui (Backspace no início de um parágrafo) é marcada como excluída.
- **Uma mudança de aparência** (negrito, fonte, tamanho, cor, alinhamento, recuos ou estilo) é registrada com a aparência que o texto tinha antes.

Cada alteração é salva com quem a fez e quando. O nome é o que está em **Arquivo > Opções**, em **Nome de usuário**. Altere-o ali se estiver errado.

Cada edição é uma etapa, então **Ctrl+Z** a desfaz como faz com qualquer outra edição.

## Como uma alteração aparece

Por padrão, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento mostra recebe a primeira cor, a seguinte recebe a segunda, e assim por diante, e cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado na cor da pessoa, e o texto excluído é riscado. O texto que foi movido aparece em verde: riscado duplo no local de origem e sublinhado duplo no local de destino. O texto cuja formatação foi alterada fica em negrito. Uma barra na margem esquerda marca cada linha que tem alteração.

Você pode mudar tudo isso. Clique em **Revisão > Controle > Opções de Controle** para abrir a caixa **Opções de Controle de Alterações**. Em **Marcação**, cada tipo de alteração tem um **Estilo** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, tachado, negrito, itálico, somente cor ou (nenhum).
- **Exclusões**: tachado, tachado duplo, oculto, circunflexo ou somente cor. Com **Oculto**, as palavras excluídas são retiradas da linha. Com **Circunflexo**, elas também são retiradas, e um pequeno ^ indica onde estavam. O texto movido para outro lugar também é retirado quando você escolhe **Balões** para exclusões, mas não nos demais casos.
- **Movido de** e **Movido para**: os estilos e cores do texto movido. Essas opções ficam disponíveis enquanto **Controlar movimentos** estiver marcado. Se você o desmarcar, o texto movido aparece como uma exclusão comum e uma inserção comum.
- **Formatação**: negrito, somente cor ou (nenhum). Se você desmarcar **Controlar formatação**, as mudanças de formatação não são marcadas no texto.
- **Barras de alteração**: borda esquerda, borda direita, borda externa (o lado direito de uma página ímpar e o lado esquerdo de uma página par) ou (nenhuma), e a cor da barra.

Cada caixa **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma única cor para todas as pessoas. Essas escolhas são suas. Elas ficam guardadas neste computador, não no documento, valem para todos os documentos e todas as janelas abertas, e cada janela muda assim que você clica em **OK**. As barras são desenhadas da mesma forma na tela, em uma impressão e em um PDF sempre que a marcação estiver sendo exibida.

## Escolher o que você vê: Exibir para Revisão

Clique em **Revisão > Exibir para Revisão** e escolha uma opção:

- **Todas as Marcações** mostra cada alteração no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceitas, e uma barra na margem ao lado de cada linha que tem alteração.
- **Sem Marcação** mostra o documento como ficará com todas as alterações aceitas, e nada além disso.
- **Original** mostra o documento como era antes de qualquer alteração: o texto excluído volta, e o texto inserido desaparece.

Essas opções mudam apenas o que você vê. Elas nunca alteram o documento e não são salvas com ele. Um documento que tenha alterações é aberto em **Todas as Marcações**.

**Revisão > Mostrar Marcação** escolhe quais tipos de alteração são exibidos: **Inserções e Exclusões**, **Formatação** e **Comentários**. Abaixo deles, escolha como o texto excluído é mostrado. **Mostrar Todas as Revisões Embutidas** mantém o texto excluído no texto, riscado. **Mostrar Revisões em Balões** tira o texto excluído da linha e o coloca em um balão na margem. **Mostrar Somente Comentários e Formatação em Balões** mantém inserções e exclusões no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la definitivamente. Rejeitá-la é desfazê-la.

- **Revisão > Aceitar** aceita a alteração em que está o cursor (ou cada alteração do texto selecionado) e depois passa para a próxima. **Revisão > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta abaixo de **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controle**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcação** deixou ativados (e, no painel **Revisão**, do revisor que você escolheu).
- **Anterior** e **Próxima**, no grupo **Alterações**, passam de uma alteração para a seguinte sem mudar nada. Com o cursor no início absoluto do texto, de um cabeçalho ou de uma nota, **Próxima** vai para uma alteração que começa exatamente ali.

Cada um desses comandos é uma etapa, então **Ctrl+Z** devolve o documento exatamente ao estado anterior.

Aceitar uma exclusão remove o texto definitivamente. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que ele era novo. Rejeitar uma exclusão mantém o texto e remove o riscado. Rejeitar uma mudança de aparência restaura a aparência que o texto tinha. O texto movido é um par de alterações, uma onde ele saiu e outra onde ele chegou, unidas pelo nome do movimento. Quando o par está marcado com o mesmo nome, como outros programas o salvam, aceitar ou rejeitar qualquer uma das metades faz o mesmo com a outra, então o texto fica movido definitivamente ou volta ao lugar original. Um nome usado mais de uma vez, ou que tenha apenas uma das metades, não une nada: cada metade é uma alteração própria.

## O painel Revisão

Clique em **Revisão > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem a fez, quando e as palavras. Um clique em uma delas a mostra no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Próxima** abaixo da lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## Alterações em cabeçalhos, rodapés, notas e tabelas

As alterações feitas por outras pessoas em um cabeçalho, um rodapé, uma nota de rodapé ou uma nota de fim são exibidas e listadas como as alterações do texto, inclusive um cabeçalho ou rodapé que nenhuma página usa (o de páginas pares, quando o documento não usa páginas pares e ímpares diferentes): **Próxima** passa por ele, e **Aceitar** e **Rejeitar** fazem a alteração em que você parou por último. **Próxima**, **Anterior**, **Aceitar**, **Rejeitar** e o painel **Revisão** levam você ao cabeçalho, rodapé ou nota que contém a alteração, e a ordem é o texto primeiro, depois as notas de rodapé e, por último, os cabeçalhos e rodapés. **Aceitar Todas** e **Rejeitar Todas** abrangem todos eles.

As alterações em tabelas também são listadas, no texto e em cabeçalhos e rodapés: uma linha ou célula inserida ou excluída, células que foram mescladas e uma mudança de aparência de uma tabela, de uma linha ou de uma célula. Uma barra na margem marca as linhas da linha ou célula. Aceitar uma linha excluída, ou rejeitar uma inserida, remove a linha e suas palavras. Uma linha em que todas as células têm a alteração é a alteração dessa linha. Rejeitar uma mudança de aparência restaura a aparência que a tabela, a linha ou a célula tinham, e rejeitar uma mesclagem separa as células novamente. Quando retirar uma alteração quebraria a tabela, a alteração é mantida no lugar, e a mensagem informa quantas foram mantidas. Isso acontece com uma célula unida a outras abaixo dela, que permanece e perde a marca, e com uma mudança na grade que as linhas não conseguem acompanhar, que permanece como uma alteração que você pode aceitar. As alterações dentro de uma tabela em uma nota de rodapé são mantidas como foram escritas: elas não são listadas, e **Aceitar** e **Rejeitar** não agem sobre elas.

Um movimento ocupa uma linha na lista e uma alteração na contagem que **Aceitar Todas** e **Rejeitar Todas** exibem: sua outra metade e as marcas de parágrafo que o acompanhavam são resolvidas junto. As alterações dentro de uma caixa de texto são listadas uma a uma, depois das alterações do texto e antes das notas de rodapé. **Próxima** e **Anterior** abrem a caixa para digitação, com a alteração selecionada, e **Aceitar** e **Rejeitar** resolvem essa alteração, cada uma como uma etapa. Mover o cursor dentro da caixa aberta faz **Próxima** e **Anterior** continuarem a partir do cursor. Uma caixa resolvida uma alteração de cada vez é salva sem a cópia que programas mais antigos leem, como acontece quando você digita em uma caixa. As primeiras 200 caixas que têm alterações são listadas dessa forma. Uma caixa de texto que faz parte de um grupo, ou cujo texto continua em outras caixas, uma caixa dentro de outra caixa, uma caixa em um cabeçalho ou em uma nota, e as caixas além das primeiras 200 não podem ser abertas dessa forma: o painel **Revisão** conta as alterações delas em uma linha abaixo da lista, e somente **Aceitar Todas** e **Rejeitar Todas** as resolvem.

Uma alteração na numeração de um parágrafo é listada como **Numerado** ou **Numeração alterada**. Quando uma mudança de aparência de um parágrafo registrou a lista e o nível que o parágrafo tinha, rejeitá-la coloca o parágrafo de volta nessa lista, nesse nível (ou o retira da lista em que não estava). Uma alteração que apenas mantém a numeração antiga como rótulo (como alguns programas a salvam) não indica lista para voltar: aceitá-la mantém a numeração, e rejeitá-la retira a numeração somente quando o rótulo está vazio; caso contrário, a alteração permanece no lugar e é contada como mantida. Rejeitar uma numeração que foi acrescentada devolve ao parágrafo o que o estilo dele define (um estilo de título que tem numeração a mantém), e nenhuma numeração onde o estilo não tem nenhuma. Se a lista para a qual uma alteração deveria voltar não existe mais no documento, nada sobre esse parágrafo muda, e a alteração é contada como mantida.

Uma alteração em uma seção (tamanho da página, orientação, margens, colunas, numeração de páginas ou tipo de quebra) é listada como **Seção formatada**, no parágrafo que termina a seção, ou no fim do texto para a última seção. Aceitá-la mantém a seção como está e descarta o registro, e rejeitá-la devolve as propriedades que o registro guardou, e as páginas são diostribuídas novamente de imediato. Os cabeçalhos e rodapés que a seção indica não mudam. Se as propriedades antigas do registro não puderem ser lidas, rejeitar deixa a seção e a alteração como estão e conta a alteração como mantida; nada é substituído por uma página padrão. Quando **Aceitar Todas** ou **Rejeitar Todas** também remove ou recoloca uma quebra de seção, a alteração de cada seção é resolvida primeiro, para que nenhuma fique para trás.

Uma alteração que não pode ser resolvida sem danificar o documento (uma tabela que se quebraria, uma lista ou seção cujo estado antigo não pode ser recuperado, uma caixa de texto grande demais para ser gravada de volta) é mantida como está, e a mensagem exibida após **Aceitar Todas** ou **Rejeitar Todas** informa quantas foram mantidas.

## Quais alterações são registradas e quais não são

Estas alterações são registradas enquanto o Controlar Alterações estiver ativado: digitar, excluir e colar texto; Enter; substituir palavras; a aparência do texto e dos parágrafos (fontes, tamanhos, cores, alinhamento, espaçamento, bordas e estilos); Alterar Maiúsculas/Minúsculas; símbolos e caracteres especiais; quebras de página e de coluna; a marca de parágrafo de uma quebra de seção; e, em tabelas, uma linha que você insere (com **Inserir Acima** ou **Inserir Abaixo**, ou com Tab na última célula), uma linha que você exclui, uma tabela que você exclui e uma tabela que você insere. Uma linha inserida ou excluída aparece no painel **Revisão** como uma única alteração, com suas palavras, e **Aceitar** e **Rejeitar** a mantêm ou a retiram. Excluir uma linha que contém uma tabela marca também as linhas dessa tabela. Uma linha que outra pessoa inseriu e você exclui carrega as duas alterações, e tanto **Aceitar** quanto **Rejeitar** a retiram. Uma linha que você mesmo inseriu desaparece definitivamente quando você a exclui.

Estas alterações são feitas normalmente, sem marca, e a barra de status exibe "Esta alteração não foi registrada como alteração controlada." cada vez: marcadores, numeração e os demais comandos de lista; **Aumentar Recuo** e **Diminuir Recuo**; configuração de página, colunas, margens e numeração de linhas; cabeçalhos e rodapés, e digitar, excluir e substituir palavras em um cabeçalho, rodapé, nota de rodapé ou nota de fim; inserir uma nota de rodapé, uma nota de fim, um sumário, um índice, um campo, um link ou um controle de conteúdo; imagens, gráficos, equações, caixas de texto e as formas desenhadas; colunas, células, mesclagem, divisão e a aparência de uma tabela; excluir uma linha que faz parte de uma célula unida a outras linhas; classificar parágrafos ou as linhas de uma tabela; e mover uma tabela ou uma seção. Enquanto você digita em um cabeçalho ou em uma nota, a mensagem aparece uma vez, não a cada tecla. **Desfazer** desfaz qualquer um desses itens em uma única etapa.

Algumas alterações não são feitas de forma alguma enquanto o Controlar Alterações está ativado, porque, sem marca, danificariam uma alteração que está registrada: em uma tabela que tem uma linha inserida ou excluída, **Excluir Colunas**, **Excluir Células**, **Mesclar Células**, **Dividir Células**, **Converter em Texto** e colar sobre células. A barra de status informa isso. Aceite ou rejeite primeiro as alterações dessa tabela, ou desative o Controlar Alterações. Em um documento que bloqueia o Controlar Alterações como ativado, em que não é possível aceitar nem rejeitar, essas ações também são feitas, e a barra de status informa que a alteração não foi registrada.

Alterações que outros programas registraram em numeração e seções são listadas e podem ser aceitas e rejeitadas, como descrito acima.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e as mostra como alterações controladas.

1. Abra a versão mais antiga. Ela é o **original**.
2. Clique em **Comparar** no grupo **Comparar** da guia **Revisão**.
3. Clique em **Procurar** e escolha a versão mais recente. Ela é a versão **revisada**.
4. Digite o nome com que as alterações serão assinadas. Escolha as opções que desejar, como ignorar maiúsculas e minúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** ficam ativadas, a menos que você as desmarque.
5. Clique em **OK**.

Uma nova janela é aberta com o texto original e as alterações que o transformam no texto revisado. Seus dois arquivos não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração controlada. Se você aceitar todas as alterações, obterá o texto revisado. Se rejeitar todas, obterá o texto original.

Comparar analisa o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés nem notas de rodapé.

Um parágrafo de 12 palavras ou mais que foi retirado de um lugar e colocado em outro aparece como um **movimento**: um registro onde o texto saiu e outro onde ele chegou. Aceitar ou rejeitar qualquer um deles resolve os dois. O parágrafo precisa ser igual, ou ter pelo menos 9 de cada 10 palavras iguais e, na maior parte, na mesma ordem; caso contrário, aparece como uma exclusão e uma inserção. As palavras que diferem dentro de um parágrafo movido são mostradas como inserções e exclusões dentro do movimento.

Um texto igual nas duas versões, mas com aparência diferente, aparece como uma **alteração de formatação**. Comparar verifica negrito, itálico, sublinhado, tamanho, fonte, cor, realce e o estilo, alinhamento, recuos e espaçamento de um parágrafo. Rejeitar a alteração restaura a aparência antiga.

Se os dois documentos tiverem alterações demais, Comparar informa isso. Nesse caso, mostra alguns parágrafos como substituídos por inteiro e pode não procurar movimentos ou alterações de formatação em todos eles.
