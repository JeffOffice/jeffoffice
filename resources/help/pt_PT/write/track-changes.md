# Controlar alterações e aceitá-las ou rejeitá-las
<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações registadas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, apresentar para revisão, marcação simples, toda a marcação, sem marcação, original, mostrar marcação, balões, painel de revisão, barra de alteração, inserção, eliminação, ctrl+shift+e, autor, revisor, nome de utilizador, edições de outra pessoa -->

O **Controlar Alterações** guarda um registo de cada edição que faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se deve manter a alteração. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai rever depois de si.

## Ativar e desativar o Controlar Alterações

Clique em **Rever > Controlar Alterações**, ou prima **Ctrl+Shift+E**. A barra de estado, na parte inferior da janela, indica se está ativado. Um clique nessas palavras inverte a opção. A definição é guardada no documento, por isso continua ativada quando o documento regressa de outra pessoa.

Enquanto estiver ativado:

- **O que escreve** é marcado como inserção.
- **O que elimina** não é removido. Mantém-se no texto, riscado, para que possa ser reposto. A exceção é o texto que escreveu a seguir a ativar o controlo: se o eliminar, é simplesmente removido, porque ninguém mais o chegou a ver.
- **Uma mudança de parágrafo (Enter)** é marcada como inserida, e uma que elimine (Backspace no início de um parágrafo) é marcada como eliminada.
- **Uma alteração de aspeto** (negrito, tipo de letra, tamanho, cor, alinhamento, avanços ou estilo) fica registada com a forma como o texto estava antes.

Cada alteração é guardada com quem a fez e quando. O nome é o que está em **Ficheiro > Opções**, em **Nome de utilizador**. Altere-o aí, se estiver errado.

Cada edição conta como um passo, por isso **Ctrl+Z** a anula, como faz com qualquer outra edição.

## Como é uma alteração

Por predefinição, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento mostra recebe a primeira cor, a seguinte recebe a segunda, e assim por diante, e cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado na cor da pessoa, e o texto eliminado é riscado. O texto que foi movido é verde: riscado duplo onde saiu, e sublinhado duplo onde chegou. O texto cuja formatação foi alterada fica a negrito. Uma barra na margem esquerda assinala cada linha que tem uma alteração.

Pode alterar tudo isto. Clique em **Rever > Controlo > Opções de Controlo de Alterações** para abrir a caixa **Opções de Controlo de Alterações**. Em **Marcação**, cada tipo de alteração tem um **Aspeto** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, riscado, negrito, itálico, apenas cor, ou (nenhum).
- **Eliminações**: riscado, riscado duplo, oculto, circunflexo, ou apenas cor. Com **Oculto**, as palavras eliminadas deixam de aparecer na linha. Com **Circunflexo**, também ficam de fora, e um pequeno ^ mostra onde estavam. O texto movido para outro local também fica de fora quando escolhe **Balões** para as eliminações, mas não nos outros casos.
- **Movido de** e **Movido para**: os aspetos e as cores do texto movido. Estão disponíveis enquanto **Controlar movimentos** estiver marcado. Se o desmarcar, o texto movido é apresentado como uma eliminação e uma inserção comuns.
- **Formatação**: negrito, apenas cor, ou (nenhum). Se desmarcar **Controlar formatação**, as alterações de formatação não são marcadas no texto.
- **Barras de alteração**: limite esquerdo, limite direito, limite exterior (o lado direito das páginas ímpares e o lado esquerdo das pares), ou (nenhum), e a cor da barra.

Cada caixa **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma só cor para todas. Estas opções são suas. São guardadas neste computador, não no documento, aplicam-se a todos os documentos e a todas as janelas abertas, e cada janela muda assim que clicar em **OK**. As barras são desenhadas da mesma forma no ecrã, numa impressão e num PDF sempre que a marcação estiver visível.

## Escolher o que vê: Apresentar para Revisão

Clique em **Rever > Apresentar para Revisão** e escolha uma opção:

- **Toda a Marcação** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceites, e uma barra na margem junto a cada linha que tem uma alteração.
- **Sem Marcação** mostra o documento como ficará com todas as alterações aceites, e mais nada.
- **Original** mostra o documento como era antes de qualquer alteração: o texto eliminado volta, e o texto inserido desaparece.

Estas opções mudam apenas o que vê. Nunca alteram o documento, e não são guardadas com ele. Um documento que tenha alterações abre sempre com **Toda a Marcação**.

**Rever > Mostrar Marcação** escolhe que tipos de alteração são apresentados: **Inserções e Eliminações**, **Formatação** e **Comentários**. Por baixo, escolha como o texto eliminado é mostrado. **Mostrar Todas as Revisões Inseridas no Texto** mantém-no no texto, riscado. **Mostrar Revisões em Balões** tira o texto eliminado da linha e coloca-o num balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e eliminações no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la de forma definitiva. Rejeitá-la é anulá-la.

- **Rever > Aceitar** aceita a alteração onde está o cursor (ou cada alteração no texto que selecionou) e passa à seguinte. **Rever > Rejeitar** faz o mesmo, anulando a alteração.
- Clique na seta sob **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controlo**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcação** tem ativados (e, no painel **Revisão**, do revisor que escolheu).
- **Anterior** e **Seguinte**, no grupo **Alterações**, passam de uma alteração para a seguinte sem alterar nada. Com o cursor no início absoluto do texto, de um cabeçalho ou de uma nota, **Seguinte** vai para uma alteração que começa exatamente aí.

Cada uma destas ações conta como um passo, por isso **Ctrl+Z** repõe o documento exatamente como estava.

Aceitar uma eliminação remove o texto de forma definitiva. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que era novo. Rejeitar uma eliminação mantém o texto e remove o riscado. Rejeitar uma alteração de aspeto repõe o aspeto que o texto tinha. O texto movido é um par de alterações, uma onde saiu e outra onde chegou, ligadas pelo nome do movimento. Quando o par está marcado com o mesmo nome, como outros programas o guardam, aceitar ou rejeitar uma das metades faz o mesmo à outra, por isso o texto fica movido de forma definitiva ou volta ao lugar original. Um nome usado mais do que uma vez, ou que tenha apenas uma metade, não liga nada: cada metade é uma alteração própria.

## O painel Revisão

Clique em **Rever > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem a fez, quando e as palavras. Um clique numa delas mostra-a no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Seguinte** sob a lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## Alterações em cabeçalhos, rodapés, notas e tabelas

As alterações feitas por outras pessoas num cabeçalho, num rodapé, numa nota de rodapé ou numa nota de fim são apresentadas e listadas como as alterações do texto, incluindo um cabeçalho ou rodapé que nenhuma página usa (o das páginas pares, quando o documento não usa páginas pares e ímpares diferentes): **Seguinte** passa por cima, e **Aceitar** e **Rejeitar** fazem a alteração a que chegou por último. **Seguinte**, **Anterior**, **Aceitar**, **Rejeitar** e o painel **Revisão** levam-no ao cabeçalho, rodapé ou nota que tem a alteração, e a ordem é primeiro o texto, depois as notas de rodapé, depois os cabeçalhos e rodapés. **Aceitar Tudo** e **Rejeitar Tudo** abrangem todos eles.

As alterações às tabelas também são listadas, no texto e em cabeçalhos e rodapés: uma linha ou célula inserida ou eliminada, células que foram unidas e uma alteração do aspeto de uma tabela, de uma linha ou de uma célula. Uma barra na margem assinala as linhas da linha ou célula. Aceitar uma linha eliminada, ou rejeitar uma inserida, remove a linha e as respetivas palavras. Uma linha em que todas as células têm a alteração é a alteração dessa linha. Rejeitar uma alteração de aspeto repõe o aspeto que a tabela, a linha ou a célula tinham, e rejeitar uma união volta a separar as células. Quando retirar uma alteração partiria a tabela, a alteração é mantida e a mensagem indica quantas foram mantidas. Isto acontece com uma célula unida a outras em baixo, que fica e perde a marca, e com uma alteração da grelha que as linhas não conseguem acompanhar, que fica como uma alteração que pode aceitar. As alterações dentro de uma tabela numa nota de rodapé são mantidas como foram escritas: não são listadas, e **Aceitar** e **Rejeitar** não atuam sobre elas.

Um movimento é uma linha na lista e uma alteração na contagem que **Aceitar Tudo** e **Rejeitar Tudo** apresentam: a sua outra metade e as marcas de parágrafo que o acompanhavam ficam resolvidas com ele. As alterações dentro de uma caixa de texto são listadas uma a uma, depois das alterações do texto e antes das das notas de rodapé. **Seguinte** e **Anterior** abrem a caixa para escrita, com a alteração selecionada, e **Aceitar** e **Rejeitar** resolvem essa alteração, cada uma como um passo. Mover o cursor dentro da caixa aberta faz com que **Seguinte** e **Anterior** continuem a partir do cursor. Uma caixa resolvida alteração a alteração é guardada sem a cópia que os programas mais antigos leem, tal como acontece quando escreve numa caixa. São listadas desta forma as primeiras 200 caixas que têm alterações. Uma caixa de texto que faz parte de um grupo, ou cujas palavras continuam noutras caixas, uma caixa dentro de outra caixa, uma caixa num cabeçalho ou numa nota, e as caixas além das primeiras 200 não podem ser abertas desta forma: o painel **Revisão** conta as respetivas alterações numa linha por baixo da lista, e apenas **Aceitar Tudo** e **Rejeitar Tudo** as resolvem.

Uma alteração à numeração de um parágrafo é listada como **Numerada** ou **Numeração alterada**. Quando uma alteração de aspeto de um parágrafo registou a lista e o nível que o parágrafo tinha, rejeitá-la coloca o parágrafo de volta nessa lista, nesse nível (ou retira-o da lista em que não estava). Uma alteração que apenas mantém a numeração antiga como rótulo (como alguns programas a guardam) não indica nenhuma lista para onde voltar: aceitá-la mantém a numeração, e rejeitá-la retira a numeração apenas quando o rótulo está vazio; caso contrário, deixa a alteração no lugar e conta-a como mantida. Rejeitar uma numeração que foi acrescentada dá ao parágrafo o que o respetivo estilo lhe atribui (um estilo de título que tenha numeração mantém-na), e nenhuma numeração onde o estilo não tem nenhuma. Se a lista que a alteração repõria já não existe no documento, nada desse parágrafo muda, e a alteração é contada como mantida.

Uma alteração a uma secção (tamanho da página, orientação, margens, colunas, numeração de páginas ou tipo de quebra) é listada como **Secção formatada**, no parágrafo que termina a secção, ou no fim do texto para a última secção. Aceitá-la mantém a secção como está e descarta o registo, e rejeitá-la devolve as propriedades que o registo guardou, e as páginas são novamente dispostas de imediato. Os cabeçalhos e rodapés que a secção indica não mudam. Se as propriedades antigas do registo não puderem ser lidas, rejeitar deixa a secção e a alteração como estão e conta-a como mantida; nada é substituído por uma página predefinida. Quando **Aceitar Tudo** ou **Rejeitar Tudo** também retira uma quebra de secção ou a repõe, a alteração de cada secção é resolvida primeiro, para que nenhuma fique para trás.

Uma alteração que não pode ser resolvida sem danificar o documento (uma tabela que se partiria, uma lista ou uma secção cujo estado antigo não pode ser reposto, uma caixa de texto grande demais para ser reescrita) fica como está, e a mensagem após **Aceitar Tudo** ou **Rejeitar Tudo** indica quantas foram mantidas.

## O que ainda não é controlado

As alterações que faz diretamente às tabelas (linhas, células, união), à numeração e às listas, às secções e à configuração da página, e ao texto de cabeçalhos, rodapés e notas são feitas como de costume, sem registo. As alterações que outros programas registaram na numeração e nas secções são listadas e podem ser aceites e rejeitadas, como descrito acima.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e mostra-as como alterações controladas.

1. Abra a versão mais antiga. É o **original**.
2. Clique em **Comparar** no grupo **Comparar** do separador **Rever**.
3. Clique em **Procurar** e escolha a versão mais recente. É a versão **revista**.
4. Escreva o nome com que assinar as alterações. Escolha as opções que quiser, como ignorar maiúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** estão ativados, a não ser que os desmarque.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisto. Os seus dois ficheiros não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração controlada. Se aceitar todas as alterações, obtém o texto revisto. Se rejeitar todas, obtém o texto original.

A comparação analisa o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés nem notas de rodapé.

Um parágrafo de 12 palavras ou mais que foi cortado de um sítio e colocado noutro é mostrado como um **movimento**: um registo onde o texto saiu e outro onde chegou. Aceitar ou rejeitar um deles resolve os dois. O parágrafo tem de ser igual, ou pelo menos 9 em cada 10 palavras iguais e na maioria na mesma ordem; caso contrário, é mostrado como uma eliminação e uma inserção. As palavras que diferem dentro de um parágrafo movido são mostradas como inserções e eliminações dentro do movimento.

O texto que é igual nas duas versões, mas tem um aspeto diferente, é mostrado como uma **alteração de formatação**. A comparação verifica negrito, itálico, sublinhado, tamanho, tipo de letra, cor, realce, e o estilo, o alinhamento, os avanços e o espaçamento de um parágrafo. Rejeitar a alteração repõe o aspeto antigo.

Se os dois documentos tiverem alterações em excesso, a comparação indica-o. Mostra então alguns parágrafos como substituídos por inteiro, e pode não procurar movimentos ou alterações de formatação em todos eles.
