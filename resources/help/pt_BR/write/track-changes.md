# Controlar alterações, e aceitar ou rejeitar

<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações controladas, revisões, marcações, aceitar, rejeitar, aceitar tudo, rejeitar tudo, exibir para revisão, marcação simples, todas as marcações, sem marcações, original, mostrar marcações, balões, painel de revisão, barra de alteração, inserção, exclusão, ctrl+shift+e, autor, revisor, nome de usuário, edições de outra pessoa -->

O **Controlar Alterações** registra cada edição que você faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se quer manter a mudança. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai revisar depois de você.

## Ativar e desativar o Controlar Alterações

Clique em **Revisão > Controlar Alterações**, ou pressione **Ctrl+Shift+E**. A barra de status, na parte inferior da janela, informa se ele está ativado. Um clique nessas palavras alterna a opção. A configuração é salva no documento, por isso continua ativada quando o documento voltar de outra pessoa.

Enquanto estiver ativado:

- **O que você digita** é marcado como inserção.
- **O que você exclui** não é removido. Ele permanece no texto, riscado, para que possa ser restaurado. A única exceção é o texto que você mesmo digitou com o controle ativado: excluí-lo apenas o remove, porque ninguém mais chegou a vê-lo.
- **Uma quebra de parágrafo que você faz** (Enter) é marcada como inserida, e uma que você exclui (Backspace no início de um parágrafo) é marcada como excluída.
- **Uma mudança de aparência** (negrito, fonte, tamanho, cor, alinhamento, recuos ou estilo) é registrada com a aparência que o texto tinha antes.

Cada alteração é salva com quem a fez e quando. O nome é o que aparece em **Arquivo > Opções**, em **Nome de usuário**. Altere-o ali se estiver incorreto.

Cada edição é uma etapa, por isso **Ctrl+Z** a desfaz, como faz com qualquer outra edição.

## Como é uma alteração

Por padrão, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento exibe recebe a primeira cor, a seguinte recebe a segunda, e assim por diante, e cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado na cor da pessoa, e o texto excluído é riscado. O texto que foi movido aparece em verde: com risco duplo onde saiu e sublinhado duplo onde chegou. O texto cuja formatação foi alterada fica em negrito. Uma barra na margem esquerda marca cada linha que tenha alguma alteração.

Você pode alterar tudo isso. Clique em **Revisão > Controle > Opções de Controle de Alterações** para abrir a caixa **Opções de Controle de Alterações**. Em **Marcações**, cada tipo de alteração tem um **Estilo** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, tachado, negrito, itálico, somente cor, ou (nenhum).
- **Exclusões**: tachado, tachado duplo, oculto, circunflexo ou somente cor. Com **Oculto**, as palavras excluídas são retiradas da linha. Com **Circunflexo**, elas também são retiradas, e um pequeno ^ indica onde estavam. O texto movido para outro lugar também é retirado quando você escolhe **Balões** para exclusões, mas não nos demais casos.
- **Movido de** e **Movido para**: os estilos e cores do texto movido. Estão disponíveis enquanto **Controlar movimentos** estiver marcado. Se você desmarcá-lo, o texto movido aparece como uma exclusão comum e uma inserção comum.
- **Formatação**: negrito, somente cor, ou (nenhum). Se você desmarcar **Controlar formatação**, as mudanças de formatação não são marcadas no texto.
- **Barras de alteração**: borda esquerda, borda direita, borda externa (o lado direito de uma página ímpar e o lado esquerdo de uma página par), ou (nenhuma), e a cor da barra.

Cada caixa **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma única cor para todas as pessoas. Essas escolhas são suas. Elas ficam guardadas neste computador, não no documento, valem para todos os documentos e todas as janelas abertas, e cada janela muda assim que você clicar em **OK**. As barras são desenhadas da mesma forma na tela, em uma impressão e em um PDF sempre que as marcações estiverem visíveis.

## Escolher o que você vê: Exibir para Revisão

Clique em **Revisão > Exibir para Revisão** e escolha uma opção:

- **Todas as Marcações** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceitas, e uma barra na margem ao lado de cada linha que tenha alguma alteração.
- **Sem Marcações** mostra o documento como ficará com todas as alterações aceitas, e nada além disso.
- **Original** mostra o documento como era antes de qualquer alteração: o texto excluído volta, e o texto inserido desaparece.

Essas opções apenas mudam o que você vê. Elas nunca alteram o documento e não são salvas com ele. Um documento que tenha alterações é aberto com **Todas as Marcações**.

**Revisão > Mostrar Marcações** escolhe quais tipos de alteração são exibidos: **Inserções e Exclusões**, **Formatação** e **Comentários**. Abaixo deles, escolha como o texto excluído é mostrado. **Mostrar Todas as Revisões no Texto** mantém o texto excluído no texto, riscado. **Mostrar Revisões em Balões** retira o texto excluído da linha e o coloca em um balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e exclusões no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la definitivamente. Rejeitá-la é desfazê-la.

- **Revisão > Aceitar** aceita a alteração em que está o cursor (ou todas as alterações do texto selecionado) e depois vai para a próxima. **Revisão > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta sob **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controle**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcações** deixou ativados (e, no painel **Revisão**, do revisor que você escolheu).
- **Anterior** e **Próxima**, no grupo **Alterações**, passam de uma alteração para a seguinte sem mudar nada. Com o cursor no início absoluto do texto, de um cabeçalho ou de uma nota, **Próxima** vai para uma alteração que começa exatamente ali.

Cada uma dessas ações é uma etapa, por isso **Ctrl+Z** devolve o documento exatamente como estava.

Aceitar uma exclusão remove o texto definitivamente. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que ele era novo. Rejeitar uma exclusão mantém o texto e remove o risco. Rejeitar uma mudança de aparência restaura a aparência que o texto tinha. O texto movido é um par de alterações, uma onde saiu e outra onde chegou, ligadas pelo nome da movimentação. Quando o par está marcado com o mesmo nome, como outros programas o salvam, aceitar ou rejeitar qualquer uma das metades faz o mesmo com a outra, de modo que o texto é movido definitivamente ou volta ao lugar original. Um nome usado mais de uma vez, ou que tenha apenas uma metade, não liga nada: cada metade é uma alteração independente.

## O painel Revisão

Clique em **Revisão > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem a fez, quando e as palavras. Um clique em uma delas a mostra no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Próxima** sob a lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## Alterações em cabeçalhos, rodapés, notas e tabelas

As alterações feitas por outras pessoas em um cabeçalho, um rodapé, uma nota de rodapé ou uma nota de fim são exibidas e listadas como as alterações do texto, inclusive um cabeçalho ou rodapé que nenhuma página usa (o das páginas pares, quando o documento não usa páginas pares e ímpares diferentes): **Próxima** passa por ele, e **Aceitar** e **Rejeitar** fazem a alteração a que você foi por último. **Próxima**, **Anterior**, **Aceitar**, **Rejeitar** e o painel **Revisão** levam você até o cabeçalho, rodapé ou nota que contém a alteração, e a ordem é primeiro o texto, depois as notas de rodapé, depois os cabeçalhos e rodapés. **Aceitar Todas** e **Rejeitar Todas** abrangem todos eles.

Suas próprias alterações em um cabeçalho, rodapé, nota de rodapé ou nota de fim são registradas da mesma forma que no texto: digitação, exclusão, colagem, aparência do texto e substituição de palavras. Inserir Nota de Rodapé e Inserir Nota de Fim registram a marca no texto e a nota como uma única alteração inserida: **Aceitar** mantém as duas, e **Rejeitar** retira as duas; se você excluir uma nota que inseriu, ela simplesmente desaparece. Se você excluir uma nota feita por outra pessoa, a marca no texto e as palavras da nota são marcadas como excluídas. Se você recortar ou copiar um texto que tenha uma marca de nota e colá-lo com o Controlar Alterações ativado, a marca colada recebe uma nota nova e própria, uma cópia limpa das palavras, registrada como uma única alteração inserida. Se as palavras de uma nota copiada tiverem alterações controladas, a nota colada as mantém como estão lidas, com as inserções mantidas e as exclusões retiradas. Todas as notas do texto recortado ou copiado são copiadas; quando as notas juntas tiverem mais de 200.000 caracteres, nada é recortado ou copiado, e uma mensagem informa isso. Em um documento com primeira página diferente ou com páginas pares e ímpares diferentes, uma alteração vai para o cabeçalho ou rodapé da página em que você está. Um documento que você só pode ler não permite alterar cabeçalhos, rodapés ou notas. Ao salvar como arquivo de texto OpenDocument (.odt), as alterações em cabeçalhos e rodapés se tornam texto simples, e **Arquivo > Informações > Verificar Compatibilidade** informa isso. Inserir um cabeçalho ou rodapé que não existia não é marcado; as palavras que você digita nele são alterações inseridas, e rejeitá-las deixa o cabeçalho vazio, que tem a mesma aparência de um cabeçalho inexistente.

As alterações nas tabelas também são listadas, no texto e em cabeçalhos e rodapés: uma linha ou célula inserida ou excluída, células que foram mescladas e uma mudança de aparência de uma tabela, de uma linha ou de uma célula. Uma barra na margem marca as linhas da linha ou célula. Aceitar uma linha excluída, ou rejeitar uma inserida, remove a linha e as palavras dela. Uma linha em que todas as células têm a alteração é a alteração daquela linha. Rejeitar uma mudança de aparência restaura a aparência que a tabela, a linha ou a célula tinha, e rejeitar uma mesclagem separa as células novamente. Quando retirar uma alteração quebraria a tabela, a alteração é mantida no lugar, e a mensagem informa quantas foram mantidas. Isso acontece com uma célula unida a outras por baixo, que permanece e perde a marca, e com uma alteração da grade que as linhas não conseguem acompanhar, que permanece como uma alteração que você pode aceitar. Alterações dentro de uma tabela em uma nota de rodapé são mantidas como foram escritas: não são listadas, e Aceitar e Rejeitar não agem sobre elas.

Uma movimentação ocupa uma linha na lista e conta como uma alteração no total que Aceitar Todas e Rejeitar Todas informam: sua outra metade e as marcas de parágrafo que foram junto são resolvidas com ela. As alterações dentro de uma caixa de texto são listadas uma a uma, depois das alterações do texto e antes das notas de rodapé. **Próxima** e **Anterior** abrem a caixa para digitação, com a alteração selecionada, e **Aceitar** e **Rejeitar** resolvem essa alteração, cada uma como uma etapa. Mover o cursor dentro da caixa aberta faz **Próxima** e **Anterior** continuarem a partir do cursor. Uma caixa resolvida uma alteração por vez é salva sem a cópia que programas mais antigos leem, como acontece quando você digita em uma caixa. As primeiras 200 caixas que contêm alterações são listadas dessa forma. Uma caixa de texto que faz parte de um grupo, ou cujas palavras continuam em outras caixas, uma caixa dentro de outra caixa, uma caixa em um cabeçalho ou em uma nota, e as caixas além das primeiras 200 não podem ser abertas dessa forma: o painel **Revisão** conta as alterações delas em uma linha sob a lista, e somente **Aceitar Todas** e **Rejeitar Todas** as resolvem.

Uma alteração na numeração de um parágrafo é listada como **Numerado** ou **Numeração alterada**. Quando uma mudança de aparência de um parágrafo registrou a lista e o nível que ele tinha, rejeitá-la coloca o parágrafo de volta nessa lista, nesse nível (ou o retira da lista em que não estava). Uma alteração que apenas mantém a numeração antiga como rótulo (como alguns programas a salvam) não indica nenhuma lista para voltar: aceitá-la mantém a numeração, e rejeitá-la retira a numeração somente quando o rótulo estiver vazio; caso contrário, a alteração permanece e é contada como mantida. Rejeitar uma numeração que foi acrescentada devolve ao parágrafo o que o estilo dele define (um estilo de título que tenha numeração a mantém), e nenhuma numeração onde o estilo não tem nenhuma. Se a lista que a alteração restauraria não existe mais no documento, nada muda nesse parágrafo, e a alteração é contada como mantida.

Uma alteração em uma seção (tamanho da página, orientação, margens, colunas, numeração de páginas ou tipo de quebra) é listada como **Seção formatada**, no parágrafo que termina a seção, ou no fim do texto para a última seção. Aceitá-la mantém a seção como está e descarta o registro, e rejeitá-la devolve as propriedades que o registro guardou, e as páginas são diagramadas novamente de imediato. Os cabeçalhos e rodapés que uma seção indica não mudam. Se as propriedades antigas do registro não puderem ser lidas, rejeitar deixa a seção e a alteração como estão, e conta a alteração como mantida; nada é substituído por uma página padrão. Quando Aceitar Todas ou Rejeitar Todas também retira uma quebra de seção ou a recoloca, a alteração de cada seção é resolvida antes, para que nenhuma fique para trás.

Uma alteração que não pode ser resolvida sem danificar o documento (uma tabela que se quebraria, uma lista ou uma seção cujo estado antigo não pode ser restaurado, uma caixa de texto grande demais para ser regravada) permanece como foi escrita, e a mensagem após **Aceitar Todas** ou **Rejeitar Todas** informa quantas foram mantidas.

## Quais alterações são registradas e quais não são

Estas alterações são registradas enquanto o Controlar Alterações está ativado: digitar, excluir e colar texto, inclusive em cabeçalhos, rodapés, notas de rodapé e notas de fim; Enter; substituir palavras; inserir uma nota de rodapé ou de fim; a aparência de textos e parágrafos (fontes, tamanhos, cores, alinhamento, espaçamento, bordas e estilos); Alterar Maiúsculas/Minúsculas; símbolos e caracteres especiais; quebras de página e de coluna; o marcador de parágrafo de uma quebra de seção; e, em tabelas, uma linha que você insere (com **Inserir Acima** ou **Inserir Abaixo**, ou com Tab na última célula), uma linha que você exclui, uma tabela que você exclui e uma tabela que você insere. Uma linha inserida ou excluída aparece no painel **Revisão** como uma única alteração, com as palavras dela, e **Aceitar** e **Rejeitar** a mantêm ou a retiram. Excluir uma linha que contém uma tabela marca também as linhas dessa tabela. Uma linha que outra pessoa inseriu e você excluiu traz as duas alterações, e **Aceitar** ou **Rejeitar** a retira. Uma linha que você mesmo inseriu desaparece definitivamente quando você a exclui.

Estas alterações são feitas normalmente, sem marca, e a barra de status exibe "Esta alteração não foi registrada como alteração controlada." a cada vez: marcadores, numeração e os demais comandos de lista; **Aumentar Recuo** e **Diminuir Recuo**; configuração de página, colunas, margens e numeração de linhas; um número de página ou outro campo inserido em um cabeçalho ou rodapé; inserir um sumário, um índice remissivo, um campo, um link ou um controle de conteúdo; imagens, gráficos, equações, caixas de texto e formas desenhadas; colunas, células, mesclagem, divisão e aparência de uma tabela; excluir uma linha que faz parte de uma célula unida por baixo em várias linhas; classificar parágrafos ou as linhas de uma tabela; e mover uma tabela ou uma seção. Enquanto você digita, a mensagem aparece uma única vez, e não a cada tecla. **Desfazer** desfaz qualquer uma dessas ações como uma etapa.

Algumas alterações não são feitas de jeito nenhum enquanto o Controlar Alterações está ativado, porque, sem marca, danificariam uma alteração que foi registrada: em uma tabela que tenha uma linha inserida ou excluída, **Excluir Colunas**, **Excluir Células**, **Mesclar Células**, **Dividir Células**, **Converter em Texto** e colar sobre células. A barra de status informa isso. Aceite ou rejeite antes as alterações dessa tabela, ou desative o Controlar Alterações. Em um documento que bloqueia o Controlar Alterações como ativado, em que você não pode aceitar nem rejeitar, essas ações também são feitas, e a barra de status informa que a alteração não foi registrada.

Alterações que outros programas registraram na numeração e nas seções são listadas e podem ser aceitas e rejeitadas, como descrito acima.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e as mostra como alterações controladas.

1. Abra a versão mais antiga. Ela é o **original**.
2. Clique em **Comparar**, no grupo **Comparar** da guia **Revisão**.
3. Clique em **Procurar** e escolha a versão mais recente. Ela é a versão **revisada**.
4. Digite o nome com que as alterações serão assinadas. Escolha as opções que desejar, como ignorar maiúsculas e minúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** estão ativados, a menos que você os desmarque.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisado. Os dois arquivos não são alterados. Use **Aceitar** e **Rejeitar** como para qualquer alteração controlada. Se você aceitar todas as alterações, obterá o texto revisado. Se rejeitar todas, obterá o texto original.

Comparar analisa o texto do corpo do documento, suas notas de rodapé, seus cabeçalhos e rodapés e as tabelas do corpo. Ainda não analisa notas de fim nem caixas de texto. Quando algo diferente está em um lugar que a Comparação não consegue mostrar, ela informa quantas diferenças não pôde mostrar e de que tipo, e deixa essa parte como estava no original.

As tabelas do corpo são comparadas linha por linha e célula por célula. Uma linha que existe apenas na versão revisada é mostrada como linha inserida, e uma que existe apenas no original como linha excluída. Uma tabela que existe em apenas uma das versões é mostrada como tabela inserida ou excluída. As linhas presentes nas duas versões são comparadas célula por célula. Duas tabelas são a mesma tabela com linhas alteradas quando pelo menos metade das células que têm palavras na maior delas também têm as mesmas palavras na outra, contando cada célula uma vez. Se coincidirem menos células, a tabela antiga é mostrada como excluída e a nova como inserida. A Comparação não mostra uma coluna incluída ou retirada, células que foram unidas ou divididas, nem uma tabela dentro de outra tabela. Ela conta cada um desses casos, informa isso e deixa essa tabela como estava. Uma tabela em um cabeçalho, rodapé ou nota de rodapé é comparada apenas célula por célula.

Um parágrafo de 12 palavras ou mais que foi recortado de um lugar e colocado em outro é mostrado como uma **movimentação**: um registro onde o texto saiu e outro onde ele chegou. Aceitar ou rejeitar qualquer um deles resolve os dois. O parágrafo precisa ser o mesmo, ou pelo menos 9 em cada 10 palavras precisam ser iguais e a maioria na mesma ordem; caso contrário, ele é mostrado como uma exclusão e uma inserção. As palavras que diferem dentro de um parágrafo movido são mostradas como inserções e exclusões dentro da movimentação.

Um texto igual nas duas versões, mas com aparência diferente, é mostrado como uma **alteração de formatação**. A Comparação verifica negrito, itálico, sublinhado, tamanho, fonte, cor, realce e o estilo, o alinhamento, os recuos e o espaçamento de um parágrafo. Rejeitar a alteração restaura a aparência antiga.

Se os dois documentos tiverem alterações demais, a Comparação informa isso. Então mostra alguns parágrafos como substituídos por inteiro, e pode não procurar movimentações ou alterações de formatação em todos eles. Quando dois trechos longos de texto não têm nada em comum, a Comparação mostra o trecho antigo inteiro como excluído e o trecho novo inteiro como inserido, e informa isso. O texto novo mantém seus estilos, listas, negrito e itálico, mas não suas cores de texto, sombreamentos nem o espaçamento entre parágrafos. Imagens, campos, links e marcas de notas de rodapé ou de fim em um texto que a Comparação insere como novo não são levados para ele; a Comparação os conta e informa quantos são. Ela também deixa de marcar linhas e tabelas quando um documento tem mais de 5.000 linhas a marcar, e conta o restante.
