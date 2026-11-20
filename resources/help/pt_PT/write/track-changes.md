# Controlar alterações, e aceitar ou rejeitar
<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações registadas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, apresentar para revisão, marcação simples, toda a marcação, sem marcação, original, mostrar marcação, balões, painel de revisão, barra de alteração, inserção, eliminação, ctrl+shift+e, autor, revisor, nome de utilizador, edições de outra pessoa -->

**Controlar Alterações** regista cada edição que faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se a mantém. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai rever depois de si.

## Ativar e desativar Controlar Alterações

Clique em **Rever > Controlar Alterações**, ou prima **Ctrl+Shift+E**. A barra de estado, na parte inferior da janela, indica se está ativado. Um clique nessas palavras inverte a definição. A definição é guardada no documento, por isso continua ativada quando o documento regressa de outra pessoa.

Enquanto estiver ativado:

- **O que escreve** fica marcado como inserção.
- **O que elimina** não é removido. Continua no texto, riscado, para poder ser recuperado. O texto que escreveu a si próprio enquanto o controlo estava ativado é a única exceção: eliminá-lo remove-o, porque ninguém mais o chegou a ver.
- **Uma quebra de parágrafo que faz** (Enter) fica marcada como inserida, e uma que elimina (Backspace no início de um parágrafo) fica marcada como eliminada.
- **Uma alteração de aspeto** (negrito, tipo de letra, tamanho, cor, alinhamento, avanços ou estilo) fica registada com o aspeto que o texto tinha antes.

Cada alteração é guardada com o nome de quem a fez e a data e hora. O nome é o que está em **Ficheiro > Opções**, em **Nome de utilizador**. Altere-o aí se estiver errado.

Cada edição conta como um passo, por isso **Ctrl+Z** a anula como faz com qualquer outra edição.

## Aspeto de uma alteração

Por predefinição, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento mostra recebe a primeira cor, a seguinte recebe a segunda, e assim por diante, e cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido fica sublinhado na cor da pessoa, e o texto eliminado fica riscado. O texto que foi movido fica verde: duplamente riscado onde saiu, e duplamente sublinhado onde chegou. O texto cuja formatação foi alterada fica a negrito. Uma barra na margem esquerda assinala cada linha que tem uma alteração.

Pode alterar tudo isto. Clique em **Rever > Controlo > Opções de Controlo** para abrir a caixa **Opções de Controlo de Alterações**. Em **Marcação**, cada tipo de alteração tem um **Aspeto** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, rasurado, negrito, itálico, apenas cor, ou (nenhum).
- **Eliminações**: rasurado, rasurado duplo, oculta, circunflexo, ou apenas cor. Com **Oculta**, as palavras eliminadas são retiradas da linha. Com **Circunflexo**, também são retiradas, e um pequeno ^ mostra onde estavam. O texto movido para outro sítio também é retirado quando escolhe **Balões** para as eliminações, mas não de outra forma.
- **Movido de** e **Movido para**: os aspetos e cores do texto movido. Estão disponíveis enquanto **Controlar movimentos** estiver marcado. Se o desmarcar, o texto movido aparece como uma eliminação e uma inserção normais.
- **Formatação**: negrito, apenas cor, ou (nenhum). Se desmarcar **Controlar formatação**, as alterações de formatação não são assinaladas no texto.
- **Barras de alteração**: limite esquerdo, limite direito, limite exterior (o lado direito de uma página ímpar e o lado esquerdo de uma par), ou (nenhum), e a cor da barra.

Cada caixa de **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma cor única para todas. Estas escolhas são suas. Ficam guardadas neste computador, não no documento, aplicam-se a todos os documentos e a todas as janelas abertas, e cada janela muda assim que clicar em **OK**. As barras são desenhadas da mesma forma no ecrã, numa impressão e num PDF sempre que a marcação estiver visível.

## Escolher o que vê: Apresentar para Revisão

Clique em **Rever > Apresentar para Revisão** e escolha uma opção:

- **Toda a Marcação** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento tal como ficará com as alterações aceites, e uma barra na margem junto a cada linha que tem uma alteração.
- **Sem Marcação** mostra o documento tal como ficará com todas as alterações aceites, e mais nada.
- **Original** mostra o documento tal como era antes de qualquer alteração: o texto eliminado volta e o texto inserido desaparece.

Estas opções alteram apenas o que vê. Nunca alteram o documento, e não são guardadas com ele. Um documento que tenha alterações abre com **Toda a Marcação**.

**Rever > Mostrar Marcação** escolhe que tipos de alteração são mostrados: **Inserções e Eliminações**, **Formatação** e **Comentários**. Abaixo deles, escolha como é mostrado o texto eliminado. **Mostrar Todas as Revisões Inseridas no Texto** mantém-no no texto, riscado. **Mostrar Revisões em Balões** retira o texto eliminado da linha e coloca-o num balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e eliminações no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la de forma definitiva. Rejeitá-la é anulá-la.

- **Rever > Aceitar** aceita a alteração onde está o cursor (ou todas as alterações do texto selecionado) e passa à seguinte. **Rever > Rejeitar** faz o mesmo, anulando a alteração.
- Clique na seta por baixo de **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações**, e **Aceitar Todas as Alterações e Parar o Controlo**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcação** tem ativados (e, no painel **Revisão**, o revisor que escolheu).
- **Anterior** e **Seguinte**, no grupo **Alterações**, passam de uma alteração para a seguinte sem alterar nada.

Cada uma destas ações é um passo, por isso **Ctrl+Z** devolve o documento exatamente ao estado anterior.

Aceitar uma eliminação remove o texto de forma definitiva. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que era novo. Rejeitar uma eliminação mantém o texto e remove o rasurado. Rejeitar uma alteração de aspeto repõe o aspeto que o texto tinha. O texto movido é um par de alterações, uma onde saiu e outra onde chegou, ligadas pelo nome da movimentação. Quando o par está marcado com o mesmo nome, como outros programas o guardam, aceitar ou rejeitar uma das metades faz o mesmo à outra, por isso o texto fica movido de forma definitiva ou é reposto. Um nome usado mais do que uma vez, ou que tenha apenas uma metade, não liga nada: cada metade é uma alteração independente.

## O painel Revisão

Clique em **Rever > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem a fez, quando, e as palavras. Um clique numa delas mostra-a no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Seguinte** por baixo da lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## Alterações em cabeçalhos, rodapés, notas e tabelas

As alterações feitas por outras pessoas num cabeçalho, num rodapé, numa nota de rodapé ou numa nota de fim são apresentadas e listadas como as alterações do texto, incluindo um cabeçalho ou rodapé que nenhuma página usa (o das páginas pares quando o documento não usa páginas pares e ímpares diferentes): **Seguinte** passa por cima dele, e **Aceitar** e **Rejeitar** fazem a alteração a que chegou por último. **Seguinte**, **Anterior**, **Aceitar**, **Rejeitar** e o painel **Revisão** levam-no ao cabeçalho, rodapé ou nota que tem a alteração, e a ordem é primeiro o texto, depois as notas de rodapé e por fim os cabeçalhos e rodapés. **Aceitar Tudo** e **Rejeitar Tudo** abrangem todos eles.

As alterações às tabelas também são listadas, no texto e nos cabeçalhos e rodapés: uma linha ou célula inserida ou eliminada, células que foram unidas, e uma alteração de aspeto de uma tabela, de uma linha ou de uma célula. Uma barra na margem assinala as linhas da linha ou da célula. Aceitar uma linha eliminada, ou rejeitar uma inserida, remove a linha e as respetivas palavras. Uma linha em que todas as células têm a alteração é a alteração dessa linha. Rejeitar uma alteração de aspeto repõe o aspeto que a tabela, a linha ou a célula tinham, e rejeitar uma união volta a separar as células. Quando retirar uma alteração estragaria a tabela, a alteração é mantida e a mensagem indica quantas foram mantidas. Isso acontece com uma célula unida a outras para baixo, que fica e perde a marca, e com uma alteração da grelha que as linhas não conseguem acompanhar, que fica como uma alteração que pode aceitar. As alterações dentro de uma tabela numa nota de rodapé são mantidas como foram escritas: não são listadas, e **Aceitar** e **Rejeitar** não atuam sobre elas.

Uma movimentação é uma linha na lista, e uma alteração na contagem que **Aceitar Tudo** e **Rejeitar Tudo** indicam: a outra metade e as marcas de parágrafo que a acompanhavam ficam resolvidas com ela. As alterações dentro de caixas de texto não são listadas uma a uma. O painel **Revisão** indica quantas são (por exemplo, "3 alterações estão dentro de caixas de texto"), e **Aceitar Tudo** e **Rejeitar Tudo** tratam-nas juntamente com as restantes.

## O que ainda não é registado

As alterações que faz diretamente a tabelas (linhas, células, união), à numeração e às listas, às secções e à configuração da página, e ao texto de cabeçalhos, rodapés e notas são feitas como de costume, sem registo. As alterações registadas por outros programas na numeração e nas secções são mantidas e guardadas como estavam, mas não são listadas, e **Aceitar** e **Rejeitar** não atuam sobre elas.

## Comparar duas versões

**Comparar** encontra as diferenças entre duas versões de um documento e mostra-as como alterações controladas.

1. Abra a versão mais antiga. É o **original**.
2. Clique em **Comparar** no grupo **Comparar** do separador **Rever**.
3. Clique em **Procurar** e escolha a versão mais recente. É a versão **revista**.
4. Escreva o nome com que assinar as alterações. Escolha as opções que pretender, como ignorar maiúsculas e minúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** estão ativados, a menos que os desative.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisto. Os seus dois ficheiros não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração controlada. Se aceitar todas as alterações, obtém o texto revisto. Se rejeitar todas, obtém o texto original.

A comparação analisa o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés nem notas de rodapé.

Um parágrafo de 12 palavras ou mais que foi cortado de um sítio e colocado noutro é mostrado como uma **movimentação**: um registo onde o texto saiu e outro onde chegou. Aceitar ou rejeitar um deles resolve os dois. O parágrafo tem de ser igual, ou ter pelo menos 9 em cada 10 palavras iguais e, na maioria, na mesma ordem; caso contrário, é mostrado como uma eliminação e uma inserção. As palavras que diferem dentro de um parágrafo movido são mostradas como inserções e eliminações dentro da movimentação.

O texto que é igual nas duas versões, mas tem aspeto diferente, é mostrado como uma **alteração de formatação**. A comparação verifica negrito, itálico, sublinhado, tamanho, tipo de letra, cor, realce, e o estilo, alinhamento, avanços e espaçamento de um parágrafo. Rejeitar a alteração repõe o aspeto antigo.

Se os dois documentos tiverem alterações em excesso, a comparação indica-o. Mostra então alguns parágrafos como substituídos por inteiro, e pode não procurar movimentações ou alterações de formatação em todos eles.
