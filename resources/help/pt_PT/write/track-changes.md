# Controlar alterações, e aceitá-las ou rejeitá-las
<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações controladas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, mostrar para revisão, marcação simples, toda a marcação, sem marcação, original, mostrar marcação, balões, painel de revisão, barra de alteração, inserção, eliminação, ctrl+shift+e, autor, revisor, nome de utilizador, edições de outra pessoa -->

O **Controlo de Alterações** regista cada edição que faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se a mantém. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai rever depois de si.

## Ativar e desativar o Controlo de Alterações

Clique em **Rever > Controlar Alterações**, ou prima **Ctrl+Shift+E**. A barra de estado, na parte inferior da janela, indica se está ativado. Se clicar nessas palavras, o estado inverte-se. A definição é guardada no documento, por isso continua ativada quando o documento regressa de outra pessoa.

Enquanto estiver ativado:

- **O que escreve** é assinalado como uma inserção.
- **O que elimina** não é removido. Continua no texto, riscado, para que possa ser recuperado. A única exceção é o texto que escreveu a seguir à ativação do controlo: ao eliminá-lo, ele é simplesmente removido, porque ninguém mais o chegou a ver.
- **Uma quebra de parágrafo que faz** (Enter) é assinalada como inserida, e uma que elimina (Backspace no início de um parágrafo) é assinalada como eliminada.
- **Uma alteração de aspeto** (negrito, tipo de letra, tamanho, cor, alinhamento, avanços ou estilo) é registada com o aspeto que o texto tinha antes.

Cada alteração é guardada com o nome de quem a fez e a data e hora. O nome é o que está em **Ficheiro > Opções**, em **Nome de utilizador**. Altere-o aí se estiver errado.

Cada edição conta como um passo, por isso **Ctrl+Z** a anula, tal como faz com qualquer outra edição.

## Como é uma alteração

Por predefinição, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento mostra recebe a primeira cor, a seguinte recebe a segunda, e assim sucessivamente, e cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado na cor da pessoa, e o texto eliminado é riscado. O texto que foi movido aparece a verde: com risco duplo onde saiu e com sublinhado duplo onde chegou. O texto cuja formatação foi alterada fica a negrito. Uma barra na margem esquerda assinala cada linha que tenha uma alteração.

Pode alterar tudo isto. Clique em **Rever > Controlo > Opções de Controlo de Alterações** para abrir a caixa **Opções de Controlo de Alterações**. Em **Marcação**, cada tipo de alteração tem um **Aspeto** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, riscado, negrito, itálico, apenas cor, ou (nenhum).
- **Eliminações**: riscado, riscado duplo, oculto, circunflexo, ou apenas cor. Com **Oculto**, as palavras eliminadas deixam de aparecer na linha. Com **Circunflexo**, também ficam de fora, e um pequeno ^ mostra onde estavam. O texto movido para outro lugar também fica de fora quando escolhe **Balões** para as eliminações, mas não de outra forma.
- **Movido de** e **Movido para**: os aspetos e cores do texto movido. Estão disponíveis enquanto **Controlar movimentos** estiver assinalado. Se o desmarcar, o texto movido é mostrado como uma eliminação e uma inserção comuns.
- **Formatação**: negrito, apenas cor, ou (nenhum). Se desmarcar **Controlar formatação**, as alterações de formatação não são assinaladas no texto.
- **Barras de alteração**: limite esquerdo, limite direito, limite exterior (o lado direito de uma página ímpar e o lado esquerdo de uma par), ou (nenhum), e a cor da barra.

Cada caixa **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma só cor para todas. Estas escolhas são suas. São guardadas neste computador, não no documento, aplicam-se a todos os documentos e a todas as janelas abertas, e cada janela muda assim que clicar em **OK**. As barras são desenhadas da mesma forma no ecrã, numa impressão e num PDF sempre que a marcação estiver visível.

## Escolher o que vê: Mostrar para Revisão

Clique em **Rever > Mostrar para Revisão** e escolha uma opção:

- **Toda a Marcação** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento tal como ficará com as alterações aceites, e uma barra na margem junto a cada linha que tenha uma alteração.
- **Sem Marcação** mostra o documento tal como ficará com todas as alterações aceites, e nada mais.
- **Original** mostra o documento tal como era antes de qualquer alteração: o texto eliminado volta e o texto inserido desaparece.

Estas opções só alteram o que vê. Nunca alteram o documento e não são guardadas com ele. Um documento que tenha alterações abre-se com **Toda a Marcação**.

**Rever > Mostrar Marcação** escolhe que tipos de alteração são apresentados: **Inserções e Eliminações**, **Formatação** e **Comentários**. Por baixo, escolha como é mostrado o texto eliminado. **Mostrar Todas as Revisões no Texto** mantém-no no texto, riscado. **Mostrar Revisões em Balões** tira o texto eliminado da linha e coloca-o num balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e as eliminações no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la para sempre. Rejeitá-la é desfazê-la.

- **Rever > Aceitar** aceita a alteração onde está o cursor (ou todas as alterações do texto selecionado) e passa à seguinte. **Rever > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta por baixo de **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controlo**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcação** tem ativados (e, no painel **Revisão**, o revisor que escolheu).
- **Anterior** e **Seguinte**, no grupo **Alterações**, passam de uma alteração para a seguinte sem alterar nada. Com o cursor no início absoluto do texto, de um cabeçalho ou de uma nota, **Seguinte** vai para uma alteração que começa exatamente aí.

Cada uma destas ações é um passo, por isso **Ctrl+Z** repõe o documento exatamente como estava.

Aceitar uma eliminação remove o texto para sempre. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que era novo. Rejeitar uma eliminação mantém o texto e remove o riscado. Rejeitar uma alteração de aspeto repõe o aspeto que o texto tinha. O texto movido é um par de alterações, uma onde saiu e outra onde chegou, ligadas pelo nome do movimento. Quando o par está marcado com o mesmo nome, como outros programas o guardam, aceitar ou rejeitar uma das metades faz o mesmo à outra, por isso o texto fica movido para sempre ou volta ao sítio original. Um nome usado mais do que uma vez, ou que só tem uma metade, não liga nada: cada metade é uma alteração própria.

## O painel Revisão

Clique em **Rever > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem a fez, a data e hora, e as palavras. Ao clicar numa delas, ela é mostrada no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Seguinte** por baixo da lista, ou escolha um revisor na caixa por cima dela para ver apenas as alterações dessa pessoa.

## Alterações em cabeçalhos, rodapés, notas e tabelas

As alterações feitas por outras pessoas num cabeçalho, num rodapé, numa nota de rodapé ou numa nota de fim são apresentadas e listadas como as alterações do texto, incluindo um cabeçalho ou rodapé que nenhuma página usa (o de páginas pares, quando o documento não usa páginas pares e ímpares diferentes): **Seguinte** passa por cima dele, e **Aceitar** e **Rejeitar** fazem a alteração a que chegou por último. **Seguinte**, **Anterior**, **Aceitar**, **Rejeitar** e o painel **Revisão** levam-no até ao cabeçalho, rodapé ou nota que tem a alteração, e a ordem é primeiro o texto, depois as notas de rodapé e por fim os cabeçalhos e rodapés. **Aceitar Tudo** e **Rejeitar Tudo** abrangem todos eles.

As alterações a tabelas também são listadas, no texto e nos cabeçalhos e rodapés: uma linha ou célula inserida ou eliminada, células que foram unidas e uma alteração de aspeto de uma tabela, de uma linha ou de uma célula. Uma barra na margem assinala as linhas da linha ou célula. Aceitar uma linha eliminada, ou rejeitar uma inserida, remove a linha e as suas palavras. Uma linha em que todas as células têm a alteração é a alteração dessa linha. Rejeitar uma alteração de aspeto repõe o aspeto que a tabela, a linha ou a célula tinham, e rejeitar uma união volta a separar as células. Quando retirar uma alteração partiria a tabela, a alteração é mantida e a mensagem indica quantas foram mantidas. Isso acontece com uma célula unida a outras em baixo, que fica e perde a marca, e com uma alteração da grelha que as linhas não conseguem acompanhar, que fica como uma alteração que pode aceitar. As alterações dentro de uma tabela numa nota de rodapé são mantidas como foram escritas: não são listadas, e **Aceitar** e **Rejeitar** não atuam sobre elas.

Um movimento é uma linha na lista e uma alteração na contagem que **Aceitar Tudo** e **Rejeitar Tudo** indicam: a sua outra metade e as marcas de parágrafo que foram com ele ficam tratadas. As alterações dentro de uma caixa de texto são listadas uma a uma, depois das alterações do texto e antes das das notas de rodapé. **Seguinte** e **Anterior** abrem a caixa para escrita, com a alteração selecionada, e **Aceitar** e **Rejeitar** resolvem essa alteração, cada uma como um passo. Mover o cursor dentro da caixa aberta faz com que **Seguinte** e **Anterior** continuem a partir do cursor. Uma caixa que é resolvida uma de cada vez fica guardada sem a cópia que os programas mais antigos leem, tal como acontece quando escreve numa caixa. São listadas desta forma as primeiras 200 caixas que têm alterações. Uma caixa de texto que faz parte de um grupo, ou cujo texto continua noutras caixas, uma caixa dentro de outra caixa, uma caixa num cabeçalho ou numa nota, e as caixas para além das primeiras 200, não podem ser abertas desta forma: o painel **Revisão** conta as respetivas alterações numa linha por baixo da lista, e só **Aceitar Tudo** e **Rejeitar Tudo** as tratam.

Uma alteração à numeração de um parágrafo é listada como **Numerado** ou **Numeração alterada**. Quando uma alteração de aspeto de um parágrafo registou a lista e o nível que o parágrafo tinha, rejeitá-la coloca o parágrafo de volta nessa lista, nesse nível (ou retira-o da lista em que não estava). Uma alteração que apenas mantém a numeração antiga como rótulo (como alguns programas a guardam) não indica nenhuma lista para onde voltar: aceitá-la mantém a numeração, e rejeitá-la retira a numeração apenas quando o rótulo está vazio; caso contrário, deixa a alteração no lugar e conta-a como mantida. Rejeitar uma numeração que foi acrescentada devolve ao parágrafo o que o seu estilo lhe dá (um estilo de título que tenha numeração mantém-na), e nenhuma numeração onde o estilo não tem nenhuma. Se a lista que uma alteração repõria já não existe no documento, nada nesse parágrafo muda, e a alteração é contada como mantida.

Uma alteração a uma secção (tamanho da página, orientação, margens, colunas, numeração de páginas ou tipo de quebra) é listada como **Secção formatada**, no parágrafo que termina a secção, ou no fim do texto para a última secção. Aceitá-la mantém a secção como está e elimina o registo, e rejeitá-la devolve as propriedades que o registo guardou, sendo as páginas recompostas de imediato. Os cabeçalhos e rodapés que uma secção indica não mudam. Se as propriedades antigas do registo não puderem ser lidas, rejeitar deixa a secção e a alteração como estão, e conta-a como mantida; nada é substituído por uma página predefinida. Quando **Aceitar Tudo** ou **Rejeitar Tudo** também retira uma quebra de secção ou a repõe, a alteração de cada secção é resolvida primeiro, para que não fique nenhuma para trás.

Uma alteração que não pode ser resolvida sem danificar o documento (uma tabela que se partiria, uma lista ou uma secção cujo estado antigo não pode ser reposto, uma caixa de texto demasiado grande para ser escrita de novo) fica como está escrita, e a mensagem após **Aceitar Tudo** ou **Rejeitar Tudo** indica quantas foram mantidas.

## Que alterações são registadas, e quais não são

Estas alterações são registadas enquanto o Controlo de Alterações está ativado: escrever, eliminar e colar texto; Enter; substituir palavras; o aspeto do texto e dos parágrafos (tipos de letra, tamanhos, cores, alinhamento, espaçamento, limites e estilos); **Alterar Maiúsculas/Minúsculas**; símbolos e carateres especiais; quebras de página e de coluna; a marca de parágrafo de uma quebra de secção; e, em tabelas, uma linha que insere (com **Inserir Linha Acima** ou **Inserir Linha Abaixo**, ou com Tab na última célula), uma linha que elimina, uma tabela que elimina e uma tabela que insere. Uma linha inserida ou eliminada é listada no painel **Revisão** como uma alteração, com as suas palavras, e **Aceitar** e **Rejeitar** mantêm-na ou retiram-na. Eliminar uma linha que contém uma tabela marca também as linhas dessa tabela. Uma linha que outra pessoa inseriu e que você elimina tem as duas alterações, e **Aceitar** ou **Rejeitar** retira-a. Uma linha que você próprio inseriu desaparece para sempre quando a eliminar.

Estas alterações são feitas como de costume, sem marca, e a barra de estado indica "Esta alteração não foi registada como alteração controlada." de cada vez: marcas, numeração e os outros comandos de listas; **Aumentar Avanço** e **Diminuir Avanço**; configuração de página, colunas, margens e números de linha; cabeçalhos e rodapés, e escrever, eliminar e substituir palavras num cabeçalho, rodapé, nota de rodapé ou nota de fim; inserir uma nota de rodapé, uma nota de fim, um índice, um índice remissivo, um campo, uma ligação ou um controlo de conteúdo; imagens, gráficos, equações, caixas de texto e as formas desenhadas; colunas, células, união, divisão e aspeto de uma tabela; eliminar uma linha que faz parte de uma célula unida em baixo através de linhas; ordenar parágrafos ou as linhas de uma tabela; e mover uma tabela ou uma secção. Enquanto escreve num cabeçalho ou numa nota, a mensagem aparece uma só vez, e não a cada tecla. **Anular** desfaz qualquer uma destas ações como um só passo.

Algumas alterações não são feitas de todo enquanto o Controlo de Alterações está ativado, porque sem marca danificariam uma alteração que está registada: numa tabela que tem uma linha inserida ou eliminada, **Eliminar Colunas**, **Eliminar Células**, **Unir Células**, **Dividir Células**, **Converter em Texto** e colar por cima de células. A barra de estado indica-o. Aceite ou rejeite primeiro as alterações dessa tabela, ou desative o Controlo de Alterações. Num documento que bloqueia o Controlo de Alterações como ativado, onde não é possível aceitar nem rejeitar, estas alterações são feitas também, e a barra de estado indica que a alteração não foi registada.

As alterações que outros programas registaram na numeração e nas secções são listadas e podem ser aceites e rejeitadas, como descrito acima.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e mostra-as como alterações controladas.

1. Abra a versão mais antiga. É o **original**.
2. Clique em **Comparar**, no grupo **Comparar** do separador **Rever**.
3. Clique em **Procurar** e escolha a versão mais recente. É a versão **revista**.
4. Escreva o nome com que as alterações serão assinadas. Escolha as opções que pretender, como ignorar maiúsculas e minúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** estão ativados, salvo se os desativar.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisto. Os dois ficheiros não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração controlada. Se aceitar todas as alterações, obtém o texto revisto. Se rejeitar todas, obtém o texto original.

A comparação analisa o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés nem notas de rodapé.

Um parágrafo de 12 palavras ou mais que foi cortado de um lugar e colocado noutro é mostrado como um **movimento**: um registo onde o texto saiu e outro onde chegou. Aceitar ou rejeitar um deles aplica-se aos dois. O parágrafo tem de ser igual, ou pelo menos 9 em cada 10 palavras iguais e na maioria na mesma ordem; caso contrário, é mostrado como uma eliminação e uma inserção. As palavras que diferem dentro de um parágrafo movido são mostradas como inserções e eliminações dentro do movimento.

O texto que é igual nas duas versões mas tem outro aspeto é mostrado como uma **alteração de formatação**. A comparação verifica negrito, itálico, sublinhado, tamanho, tipo de letra, cor, realce, estilo de parágrafo, alinhamento, avanços e espaçamento. Rejeitar a alteração repõe o aspeto antigo.

Se os dois documentos tiverem alterações em excesso, a comparação indica-o. Mostra então alguns parágrafos substituídos por inteiro, e pode não procurar movimentos ou alterações de formatação em todos eles.
