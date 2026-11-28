# Controlar alterações, e aceitar ou rejeitar

<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações registadas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, apresentar para revisão, marcação simples, toda a marcação, sem marcação, original, mostrar marcação, balões, painel de revisão, barra de alteração, inserção, eliminação, ctrl+shift+e, autor, revisor, nome de utilizador, edições de outra pessoa -->

**Controlar Alterações** regista cada edição que faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se a quer manter. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai rever depois de si.

## Ativar e desativar Controlar Alterações

Clique em **Rever > Controlar Alterações**, ou prima **Ctrl+Shift+E**. A barra de estado, no fundo da janela, indica se está ativado. Um clique nessas palavras muda o estado para o contrário. A definição é guardada no documento, por isso continua ativa quando o documento volta de outra pessoa.

Enquanto estiver ativado:

- **O que escreve** fica marcado como inserção.
- **O que elimina** não é removido. Mantém-se no texto, riscado, para poder ser recuperado. A exceção é o texto que escreveu a seguir com o controlo ativado: ao eliminá-lo, é simplesmente removido, porque ninguém mais o chegou a ver.
- **Uma quebra de parágrafo que faz** (Enter) fica marcada como inserida, e uma que elimina (Backspace no início de um parágrafo) fica marcada como eliminada.
- **Uma alteração de aspeto** (negrito, tipo de letra, tamanho, cor, alinhamento, avanços ou estilo) fica registada com a forma como o texto estava antes.

Cada alteração é guardada com o nome de quem a fez e a data e hora. O nome é o que está em **Ficheiro > Opções**, em **Nome de utilizador**. Altere-o aí se estiver errado.

Cada edição é um passo, por isso **Ctrl+Z** a anula como faz com qualquer outra edição.

## Como é apresentada uma alteração

Por predefinição, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento mostra recebe a primeira cor, a seguinte a segunda cor, e assim por diante, e cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado com a cor da pessoa, e o texto eliminado é riscado. O texto que foi movido é verde: duplamente riscado onde saiu, e duplamente sublinhado onde chegou. O texto cuja formatação foi alterada fica a negrito. Uma barra na margem esquerda assinala cada linha que tem uma alteração.

Pode alterar tudo isto. Clique em **Rever > Controlar Alterações > Opções de Controlo de Alterações** para abrir a caixa **Opções de Controlo de Alterações**. Em **Marcação**, cada tipo de alteração tem um **Aspeto** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, rasurado, negrito, itálico, apenas cor, ou (nenhum).
- **Eliminações**: rasurado, rasurado duplo, oculta, marcador de inserção, ou apenas cor. Com **Oculta**, as palavras eliminadas são retiradas da linha. Com **Marcador de inserção**, também são retiradas, e um pequeno ^ indica onde estavam. O texto movido para outro sítio também é retirado quando escolhe **Balões** para as eliminações, mas não de outra forma.
- **Movido de** e **Movido para**: os aspetos e cores do texto movido. Estão disponíveis enquanto **Controlar movimentos** estiver marcado. Se o desmarcar, o texto movido é apresentado como uma eliminação e uma inserção comuns.
- **Formatação**: negrito, apenas cor, ou (nenhum). Se desmarcar **Controlar formatação**, as alterações de formatação não são assinaladas no texto.
- **Barras de alteração**: limite esquerdo, limite direito, limite exterior (o lado direito das páginas ímpares e o lado esquerdo das pares), ou (nenhum), e a cor da barra.

Cada caixa **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma só cor para todas. Estas escolhas são suas. São guardadas neste computador, não no documento, aplicam-se a todos os documentos e a todas as janelas abertas, e cada janela muda assim que clicar em **OK**. As barras são desenhadas da mesma forma no ecrã, numa impressão e num PDF sempre que a marcação estiver visível.

## Escolher o que vê: Apresentar para Revisão

Clique em **Rever > Apresentar para Revisão** e escolha uma opção:

- **Toda a Marcação** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento tal como ficará com as alterações aceites, e uma barra na margem junto a cada linha que tenha uma alteração.
- **Sem Marcação** mostra o documento tal como ficará com todas as alterações aceites, e mais nada.
- **Original** mostra o documento tal como era antes de qualquer alteração: o texto eliminado volta a aparecer, e o texto inserido desaparece.

Estas opções alteram apenas o que vê. Nunca alteram o documento e não são guardadas com ele. Um documento que tenha alterações abre com **Toda a Marcação**.

**Rever > Mostrar Marcação** escolhe que tipos de alteração são mostrados: **Inserções e Eliminações**, **Formatação** e **Comentários**. Abaixo deles, escolha como é mostrado o texto eliminado. **Mostrar Todas as Revisões Inseridas no Texto** mantém-no no texto, riscado. **Mostrar Revisões em Balões** retira o texto eliminado da linha e coloca-o num balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e eliminações no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la de forma definitiva. Rejeitá-la é anulá-la.

- **Rever > Aceitar** aceita a alteração onde está o cursor (ou todas as alterações do texto selecionado) e depois passa para a seguinte. **Rever > Rejeitar** faz o mesmo, anulando a alteração.
- Clique na seta sob **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controlo**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. «Mostradas» significa as alterações dos tipos que **Mostrar Marcação** tem ativados (e, no painel **Revisão**, o revisor que escolheu).
- **Anterior** e **Seguinte**, no grupo **Alterações**, passam de uma alteração para a seguinte sem alterar nada. Com o cursor no início absoluto do texto, de um cabeçalho ou de uma nota, **Seguinte** vai para uma alteração que começa exatamente aí.

Cada uma destas ações é um passo, por isso **Ctrl+Z** repõe o documento exatamente como estava.

Aceitar uma eliminação remove o texto de forma definitiva. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que era novo. Rejeitar uma eliminação mantém o texto e remove o riscado. Rejeitar uma alteração de aspeto repõe o aspeto que o texto tinha. O texto movido é um par de alterações, uma onde saiu e outra onde chegou, ligadas pelo nome da movimentação. Quando o par está marcado com o mesmo nome, como outros programas o guardam, aceitar ou rejeitar uma das metades faz o mesmo à outra, por isso o texto fica movido de forma definitiva ou é reposto. Um nome usado mais do que uma vez, ou que tenha apenas uma metade, não liga nada: cada metade é uma alteração à parte.

## O painel Revisão

Clique em **Rever > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem as fez, quando e as palavras. Um clique numa delas mostra-a no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Seguinte** sob a lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## Alterações em cabeçalhos, rodapés, notas e tabelas

As alterações feitas por outras pessoas num cabeçalho, num rodapé, numa nota de rodapé ou numa nota de fim são apresentadas e listadas como as do texto, incluindo um cabeçalho ou rodapé que nenhuma página usa (o das páginas pares, quando o documento não usa páginas pares e ímpares diferentes): **Seguinte** passa por cima dele, e **Aceitar** e **Rejeitar** fazem a alteração a que chegou por último nesse ponto. **Seguinte**, **Anterior**, **Aceitar**, **Rejeitar** e o painel **Revisão** levam-no ao cabeçalho, rodapé ou nota que tem a alteração, e a ordem é primeiro o texto, depois as notas de rodapé e por fim os cabeçalhos e rodapés. **Aceitar Todas** e **Rejeitar Todas** abrangem todos eles.

As suas alterações num cabeçalho, num rodapé, numa nota de rodapé ou numa nota de fim são registadas da mesma forma que no texto: escrever, eliminar, colar, o aspeto do texto e substituir palavras. **Inserir Nota de Rodapé** e **Inserir Nota de Fim** registam a marca no texto e a nota como uma só alteração inserida: **Aceitar** mantém ambas e **Rejeitar** retira ambas, e se eliminar uma nota que inseriu você mesmo, ela desaparece. Se eliminar uma nota que outra pessoa criou, a marca no texto e as palavras da nota ficam ambas marcadas como eliminadas. Se recortar ou copiar texto que tenha uma marca de nota e o colar com o Controlar Alterações ativado, a marca colada recebe uma nota nova, uma cópia limpa das palavras, registada como uma só alteração inserida. Se as palavras de uma nota copiada tiverem alterações controladas, a nota colada fica com elas tal como se leem, mantendo as inserções e deixando de fora as eliminações. São copiadas todas as notas do texto que recortar ou copiar; quando as notas, em conjunto, têm mais de 200 000 carateres, nada é recortado ou copiado, e uma mensagem indica-o. Num documento com uma primeira página diferente ou com páginas pares e ímpares diferentes, uma alteração vai para o cabeçalho ou rodapé da página em que está. Um documento que só pode ser lido não permite alterar cabeçalhos, rodapés ou notas. Ao guardar como ficheiro de texto OpenDocument (.odt), as alterações nos cabeçalhos e rodapés passam a ser palavras simples, e **Ficheiro > Informações > Verificar Compatibilidade** indica-o. Inserir um cabeçalho ou rodapé que não existia não é assinalado; as palavras que escrever nele são alterações inseridas, e rejeitá-las deixa um cabeçalho vazio, que tem o mesmo aspeto de não haver nenhum.

As alterações às tabelas também são listadas, no texto e nos cabeçalhos e rodapés: uma linha ou célula inserida ou eliminada, células que foram unidas, e uma alteração ao aspeto de uma tabela, de uma linha ou de uma célula. Uma barra na margem assinala as linhas da linha ou célula. Aceitar uma linha eliminada, ou rejeitar uma inserida, remove a linha e as suas palavras. Uma linha em que todas as células têm a alteração é a alteração dessa linha. Rejeitar uma alteração de aspeto repõe o aspeto que a tabela, a linha ou a célula tinham, e rejeitar uma união volta a dividir as células. Quando retirar uma alteração estragaria a tabela, a alteração é mantida e a mensagem diz quantas foram mantidas. Isso acontece com uma célula unida a outras por baixo, que fica e perde a marca, e com uma alteração da grelha que as linhas não conseguem acompanhar, que fica como uma alteração que pode aceitar. As alterações dentro de uma tabela numa nota de rodapé são mantidas como foram escritas: não são listadas, e **Aceitar** e **Rejeitar** não atuam sobre elas.

Uma movimentação é uma linha na lista e uma alteração na contagem que **Aceitar Todas** e **Rejeitar Todas** indicam: a sua outra metade e as marcas de parágrafo que a acompanhavam ficam resolvidas com ela. As alterações dentro de uma caixa de texto são listadas uma a uma, depois das alterações do texto e antes das das notas de rodapé. **Seguinte** e **Anterior** abrem a caixa para escrever, com a alteração selecionada, e **Aceitar** e **Rejeitar** resolvem essa alteração, cada uma como um passo. Mover o cursor dentro da caixa aberta faz com que **Seguinte** e **Anterior** continuem a partir do cursor. Uma caixa que é resolvida uma a uma é guardada sem a cópia que os programas mais antigos leem, tal como acontece quando escreve numa caixa. As primeiras 200 caixas com alterações são listadas desta forma. Uma caixa de texto que faz parte de um grupo, ou cujas palavras continuam noutras caixas, uma caixa dentro de outra caixa, uma caixa num cabeçalho ou numa nota, e as caixas além das primeiras 200 não podem ser abertas desta forma: o painel **Revisão** conta as suas alterações numa linha abaixo da lista, e só **Aceitar Todas** e **Rejeitar Todas** as tratam.

Uma alteração à numeração de um parágrafo é listada como **Numerado** ou **Numeração alterada**. Quando uma alteração ao aspeto de um parágrafo registou a lista e o nível que o parágrafo tinha, rejeitá-la coloca o parágrafo de novo nessa lista e nesse nível (ou retira-o da lista em que não estava). Uma alteração que apenas mantém a numeração antiga como rótulo (como alguns programas a guardam) não indica nenhuma lista para onde voltar: aceitá-la mantém a numeração, e rejeitá-la retira a numeração apenas quando o rótulo está vazio; caso contrário, deixa a alteração no lugar e conta-a como mantida. Rejeitar uma numeração que foi acrescentada devolve ao parágrafo o que o seu estilo define (um estilo de título que tenha numeração mantém-na), e nenhuma numeração onde o estilo não tem. Se a lista que uma alteração repõe já não existir no documento, nada nesse parágrafo muda, e a alteração é contada como mantida.

Uma alteração a uma secção (tamanho da página, orientação, margens, colunas, numeração de páginas ou tipo de quebra) é listada como **Secção formatada**, no parágrafo que termina a secção, ou no fim do texto para a última secção. Aceitá-la mantém a secção como está e elimina o registo, e rejeitá-la devolve as propriedades que o registo guardou, e as páginas são compostas de novo de imediato. Os cabeçalhos e rodapés que a secção indica não mudam. Se as propriedades antigas do registo não puderem ser lidas, rejeitar deixa a secção e a alteração como estão e conta-a como mantida; nada é substituído por uma página predefinida. Quando **Aceitar Todas** ou **Rejeitar Todas** também retira uma quebra de secção ou a repõe, a alteração de cada secção é resolvida primeiro, para que nenhuma fique para trás.

Uma alteração que não pode ser resolvida sem danificar o documento (uma tabela que se estragaria, uma lista ou secção cujo estado antigo não pode ser reposto, uma caixa de texto demasiado grande para ser reescrita) é deixada como está, e a mensagem após **Aceitar Todas** ou **Rejeitar Todas** indica quantas foram mantidas.

## Que alterações são registadas, e quais não são

Estas alterações são registadas enquanto Controlar Alterações estiver ativado: escrever, eliminar e colar texto, também em cabeçalhos, rodapés, notas de rodapé e notas de fim; Enter; substituir palavras; inserir uma nota de rodapé ou uma nota de fim; o aspeto do texto e dos parágrafos (tipos de letra, tamanhos, cores, alinhamento, espaçamento, limites e estilos); Alterar Maiúsculas/Minúsculas; símbolos e carateres especiais; quebras de página e de coluna; a marca de parágrafo de uma quebra de secção; e, em tabelas, uma linha que insere (com **Inserir Acima** ou **Inserir Abaixo**, ou com Tab na última célula), uma linha que elimina, uma tabela que elimina e uma tabela que insere. Uma linha que insere ou elimina é listada no painel **Revisão** como uma alteração, com as suas palavras, e **Aceitar** e **Rejeitar** mantêm-na ou retiram-na. Eliminar uma linha que contém uma tabela marca também as linhas dessa tabela. Uma linha que outra pessoa inseriu e que você elimina tem ambas as alterações, e **Aceitar** ou **Rejeitar** retira-a. Uma linha que você mesmo inseriu desaparece de forma definitiva quando a eliminar.

Estas alterações são feitas normalmente, sem marca, e a barra de estado diz «Esta alteração não foi registada como alteração controlada» de cada vez: listas com marcas, numeração e os outros comandos de lista; **Aumentar Avanço** e **Diminuir Avanço**; configuração de página, colunas, margens e números de linha; um número de página ou outro campo colocado num cabeçalho ou rodapé; inserir um índice, um índice remissivo, um campo, uma ligação ou um controlo de conteúdo; imagens, gráficos, equações, caixas de texto e formas desenhadas; colunas, células, unir, dividir e o aspeto de uma tabela; eliminar uma linha que faz parte de uma célula unida a linhas abaixo; ordenar parágrafos ou as linhas de uma tabela; e mover uma tabela ou uma secção. Ao escrever, a mensagem aparece uma só vez, e não a cada tecla. **Anular** desfaz qualquer uma destas ações como um só passo.

Algumas alterações não são feitas de todo enquanto Controlar Alterações estiver ativado, porque sem marca danificariam uma alteração registada: numa tabela que tenha uma linha inserida ou eliminada, **Eliminar Colunas**, **Eliminar Células**, **Unir Células**, **Dividir Células**, **Converter em Texto** e colar sobre células. A barra de estado indica-o. Aceite ou rejeite primeiro as alterações dessa tabela, ou desative o Controlar Alterações. Num documento que bloqueie o Controlar Alterações de forma a não se poder aceitar nem rejeitar, estas ações são feitas, e a barra de estado indica que a alteração não foi registada.

As alterações que outros programas registaram na numeração e nas secções são listadas e podem ser aceites e rejeitadas, como descrito acima.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e mostra-as como alterações controladas.

1. Abra a versão mais antiga. É o **original**.
2. Clique em **Comparar** no grupo **Comparar** do separador **Rever**.
3. Clique em **Procurar** e escolha a versão mais recente. É a versão **revista**.
4. Escreva o nome com que as alterações serão assinadas. Escolha as opções que quiser, como ignorar maiúsculas e minúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** estão ativados, a menos que os desative.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisto. Os seus dois ficheiros não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração controlada. Se aceitar todas as alterações, obtém o texto revisto. Se rejeitar todas, obtém o texto original.

Comparar analisa o texto do corpo do documento, as suas notas de rodapé, os seus cabeçalhos e rodapés e as tabelas do corpo. Ainda não analisa notas de fim nem caixas de texto. Quando algo que difere está num sítio que Comparar não consegue mostrar, indica quantas diferenças não conseguiu mostrar e de que tipo, e deixa essa parte como estava no original.

As tabelas do corpo são comparadas linha a linha e célula a célula. Uma linha que existe apenas na versão revista é mostrada como linha inserida, e uma que existe apenas no original como linha eliminada. Uma tabela que existe apenas numa das versões é mostrada como tabela inserida ou tabela eliminada. As linhas que existem nas duas versões são comparadas célula a célula. Duas tabelas são a mesma tabela com linhas alteradas quando pelo menos metade das células que têm palavras na maior também têm as mesmas palavras na outra, contando cada célula uma vez. Se coincidirem menos células, a tabela antiga é mostrada como eliminada e a nova como inserida. Comparar não mostra uma coluna acrescentada ou retirada, células que foram unidas ou divididas, nem uma tabela dentro de outra tabela. Conta cada uma delas, indica-o, e deixa essa tabela como estava. Uma tabela num cabeçalho, num rodapé ou numa nota de rodapé é comparada apenas célula a célula.

Um parágrafo de 12 palavras ou mais que foi cortado de um sítio e colocado noutro é mostrado como uma **movimentação**: um registo onde o texto saiu, e outro onde chegou. Aceitar ou rejeitar um deles resolve ambos. O parágrafo tem de ser o mesmo, ou pelo menos 9 palavras em cada 10 iguais e, na maioria, pela mesma ordem; caso contrário, é mostrado como uma eliminação e uma inserção. As palavras que diferem dentro de um parágrafo movido são mostradas como inserções e eliminações dentro da movimentação.

O texto igual nas duas versões mas com aspeto diferente é mostrado como **alteração de formatação**. Comparar verifica negrito, itálico, sublinhado, tamanho, tipo de letra, cor, realce, e o estilo, alinhamento, avanços e espaçamento de um parágrafo. Rejeitar a alteração repõe o aspeto antigo.

Se os dois documentos tiverem alterações em excesso, Comparar indica-o. Nesse caso, mostra alguns parágrafos como substituídos por inteiro, e pode não procurar movimentações ou alterações de formatação em todos. Quando dois longos trechos de texto não têm nada em comum, Comparar mostra o trecho antigo inteiro como eliminado e o trecho novo inteiro como inserido, e indica-o. O texto novo mantém os seus estilos, listas, negrito e itálico, mas não as cores de texto, o sombreado nem o espaçamento entre parágrafos. As imagens, campos, ligações e marcas de notas de rodapé ou de fim no texto que Comparar apresenta como novo não são transportados para lá; Comparar conta-os e indica quantos são. Também deixa de assinalar linhas e tabelas quando um documento tem mais de 5 000 linhas para assinalar, e conta as restantes.

Um revisor encontrou estes problemas na sua última tradução; evite-os:
- "**Eliminações**: rasurado, rasurado duplo, oculta, cursor, ou apenas cor." (para "**Deletions**: strikethrough, double strikethrough, hidden, caret, or color only."): A opção chamada "Caret" (o marcador ^) foi traduzida como "cursor", que é o cursor de texto; o termo do glossário para caret é "Marcador de inserção", por isso o nome da opção na caixa de diálogo está errado.
