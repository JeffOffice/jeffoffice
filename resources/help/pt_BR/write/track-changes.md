# Controlar alterações, e aceitar ou rejeitar

<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações controladas, revisões, marcações, aceitar, rejeitar, aceitar tudo, rejeitar tudo, exibir para revisão, marcação simples, todas as marcações, sem marcações, original, mostrar marcações, balões, painel de revisão, barra de alteração, inserção, exclusão, ctrl+shift+e, autor, revisor, nome de usuário, edições de outra pessoa -->

O **Controlar Alterações** registra cada edição que você faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se quer mantê-la. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai revisar depois de você.

## Ligar e desligar o Controlar Alterações

Clique em **Revisão > Controlar Alterações**, ou pressione **Ctrl+Shift+E**. A barra de status, na parte inferior da janela, mostra se ele está ativado. Um clique nessas palavras inverte a configuração. A configuração é salva no documento, então continua ativada quando o documento volta de outra pessoa.

Enquanto estiver ativado:

- **O que você digita** é marcado como inserção.
- **O que você exclui** não é removido. Ele permanece no texto, riscado, para que possa ser restaurado. A única exceção é o texto que você mesmo digitou enquanto o controle estava ativado: excluí-lo apenas o remove, porque ninguém mais chegou a vê-lo.
- **Uma quebra de parágrafo que você faz** (Enter) é marcada como inserida, e uma que você exclui (Backspace no início de um parágrafo) é marcada como excluída.
- **Uma mudança de aparência** (negrito, fonte, tamanho, cor, alinhamento, recuos ou estilo) é registrada com a aparência que o texto tinha antes.

Cada alteração é salva com o nome de quem a fez e a data e hora. O nome é o que está em **Arquivo > Opções**, em **Nome de usuário**. Altere-o ali se estiver errado.

Cada edição é uma etapa, por isso **Ctrl+Z** a desfaz, como faz com qualquer outra edição.

## Como uma alteração aparece

Por padrão, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento mostra recebe a primeira cor, a seguinte recebe a segunda, e assim por diante; cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado na cor da pessoa, e o texto excluído é riscado. O texto que foi movido é verde: duplamente riscado onde saiu, e duplamente sublinhado onde chegou. O texto cuja formatação foi alterada fica em negrito. Uma barra na margem esquerda marca cada linha que tem alteração.

Você pode mudar tudo isso. Clique em **Revisão > Controle > Opções de Controle** para abrir a caixa **Opções de Controle de Alterações**. Em **Marcações**, cada tipo de alteração tem um **Estilo** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, tachado, negrito, itálico, somente cor, ou (nenhum).
- **Exclusões**: tachado, tachado duplo, oculto, circunflexo, ou somente cor. Com **Oculto**, as palavras excluídas são retiradas da linha. Com **Circunflexo**, elas também são retiradas, e um pequeno ^ indica onde estavam. O texto movido para outro lugar também é retirado quando você escolhe **Balões** para exclusões, mas não nos demais casos.
- **Movido de** e **Movido para**: os estilos e cores do texto movido. Essas opções ficam disponíveis enquanto **Controlar movimentos** estiver marcado. Se você desmarcá-lo, o texto movido aparece como uma exclusão comum e uma inserção comum.
- **Formatação**: negrito, somente cor, ou (nenhum). Se você desmarcar **Controlar formatação**, as mudanças de formatação não são marcadas no texto.
- **Barras de alteração**: borda esquerda, borda direita, borda externa (o lado direito de uma página ímpar e o lado esquerdo de uma página par), ou (nenhuma), e a cor da barra.

Cada caixa de **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma única cor para todas. Essas escolhas são suas. Elas ficam guardadas neste computador, não no documento, valem para todos os documentos e todas as janelas abertas, e cada janela muda assim que você clica em **OK**. As barras são desenhadas da mesma forma na tela, em uma impressão e em um PDF sempre que as marcações estiverem visíveis.

## Escolha o que você vê: Exibir para Revisão

Clique em **Revisão > Exibir para Revisão** e escolha uma opção:

- **Todas as Marcações** mostra cada alteração no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceitas, e uma barra na margem ao lado de cada linha que tem alteração.
- **Sem Marcações** mostra o documento como ficará com todas as alterações aceitas, e nada mais.
- **Original** mostra o documento como era antes de qualquer alteração: o texto excluído volta, e o texto inserido desaparece.

Essas opções apenas mudam o que você vê. Elas nunca alteram o documento e não são salvas com ele. Um documento que tem alterações abre em **Todas as Marcações**.

**Revisão > Mostrar Marcações** escolhe quais tipos de alteração são exibidos: **Inserções e Exclusões**, **Formatação** e **Comentários**. Abaixo deles, escolha como o texto excluído é mostrado. **Mostrar Todas as Revisões Embutidas** mantém o texto excluído no texto, riscado. **Mostrar Revisões em Balões** tira o texto excluído da linha e o coloca em um balão na margem. **Mostrar Somente Comentários e Formatação em Balões** mantém as inserções e exclusões no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la definitivamente. Rejeitá-la é desfazê-la.

- **Revisão > Aceitar** aceita a alteração em que está o cursor (ou todas as alterações do texto selecionado) e depois passa para a próxima. **Revisão > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta sob **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controle**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcações** deixou ativados (e, no painel **Revisão**, o revisor escolhido).
- **Anterior** e **Próxima**, no grupo **Alterações**, passam de uma alteração para a outra sem mudar nada.

Cada uma dessas ações é uma etapa, por isso **Ctrl+Z** devolve o documento exatamente como estava.

Aceitar uma exclusão remove o texto definitivamente. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que era novo. Rejeitar uma exclusão mantém o texto e remove o riscado. Rejeitar uma mudança de aparência restaura a aparência que o texto tinha. O texto movido é um par de alterações, uma onde saiu e outra onde chegou, unidas pelo nome do movimento. Quando o par está marcado com o mesmo nome, como outros programas o salvam, aceitar ou rejeitar uma das metades faz o mesmo com a outra, então o texto fica movido definitivamente ou volta ao lugar. Um nome usado mais de uma vez, ou que tem apenas uma metade, não une nada: cada metade é uma alteração independente.

## O painel Revisão

Clique em **Revisão > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem as fez, quando, e as palavras. Um clique em uma delas a mostra no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Próxima** sob a lista, ou escolha um revisor na caixa acima dela para ver somente as alterações dessa pessoa.

## Alterações em cabeçalhos, rodapés, notas e tabelas

As alterações feitas por outras pessoas em um cabeçalho, um rodapé, uma nota de rodapé ou uma nota de fim são exibidas e listadas como as alterações do texto, inclusive em um cabeçalho ou rodapé que nenhuma página usa (o de páginas pares, quando o documento não usa páginas pares e ímpares diferentes): **Próxima** passa por ele, e **Aceitar** e **Rejeitar** fazem a alteração em que você parou por último ali. **Próxima**, **Anterior**, **Aceitar**, **Rejeitar** e o painel **Revisão** levam você ao cabeçalho, rodapé ou nota que contém a alteração, e a ordem é o texto primeiro, depois as notas de rodapé, depois os cabeçalhos e rodapés. **Aceitar Todas** e **Rejeitar Todas** abrangem todos eles.

As alterações em tabelas também são listadas, no texto e em cabeçalhos e rodapés: uma linha ou célula inserida ou excluída, células que foram mescladas, e uma mudança de aparência de uma tabela, de uma linha ou de uma célula. Uma barra na margem marca as linhas da linha ou da célula. Aceitar uma linha excluída, ou rejeitar uma inserida, remove a linha e o seu texto. Uma linha em que todas as células têm a alteração é a alteração dessa linha. Rejeitar uma mudança de aparência restaura a aparência que a tabela, a linha ou a célula tinha, e rejeitar uma mesclagem separa as células de novo. Quando tirar uma alteração quebraria a tabela, a alteração é mantida, e a mensagem informa quantas foram mantidas. Isso acontece com uma célula unida a outras para baixo, que fica e perde sua marca, e com uma mudança da grade que as linhas não conseguem acompanhar, que fica como uma alteração que você pode aceitar. As alterações dentro de uma tabela em uma nota de rodapé são mantidas como foram escritas: não são listadas, e Aceitar e Rejeitar não agem sobre elas.

Um movimento ocupa uma linha na lista e uma alteração na contagem que Aceitar Todas e Rejeitar Todas apresentam: sua outra metade e as marcas de parágrafo que o acompanhavam são resolvidas junto com ele. As alterações dentro de caixas de texto não são listadas uma a uma. O painel **Revisão** informa quantas são (por exemplo, "3 outras alterações estão dentro de caixas de texto"), e **Aceitar Todas** e **Rejeitar Todas** as tratam junto com as demais.

## O que ainda não é controlado

As alterações que você faz pessoalmente em tabelas (linhas, células, mesclagem), em numeração e listas, em seções e configuração de página, e em textos de cabeçalhos, rodapés e notas são feitas normalmente, sem registro. As alterações que outros programas registraram em numeração e em seções são mantidas e salvas como foram, mas não são listadas, e Aceitar e Rejeitar não agem sobre elas.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e as mostra como alterações controladas.

1. Abra a versão mais antiga. Ela é o **original**.
2. Clique em **Comparar**, no grupo **Comparar** da guia **Revisão**.
3. Clique em **Procurar** e escolha a versão mais recente. Ela é a versão **revisada**.
4. Digite o nome com que as alterações serão assinadas. Escolha as opções que quiser, como ignorar letras maiúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** estão ativadas, a não ser que você as desative.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisado. Os seus dois arquivos não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração controlada. Se você aceitar todas as alterações, obtém o texto revisado. Se rejeitar todas, obtém o texto original.

Comparar analisa o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés nem notas de rodapé.

Um parágrafo de 12 palavras ou mais que foi cortado de um lugar e colocado em outro é mostrado como um **movimento**: um registro onde o texto saiu e outro onde ele chegou. Aceitar ou rejeitar um deles resolve os dois. O parágrafo precisa ser igual, ou ter pelo menos 9 de cada 10 palavras iguais e, na maior parte, na mesma ordem; caso contrário, é mostrado como uma exclusão e uma inserção. Palavras que diferem dentro de um parágrafo movido são mostradas como inserções e exclusões dentro do movimento.

Texto que é igual nas duas versões, mas tem aparência diferente, é mostrado como uma **alteração de formatação**. A comparação verifica negrito, itálico, sublinhado, tamanho, fonte, cor, realce, e o estilo, alinhamento, recuos e espaçamento de um parágrafo. Rejeitar a alteração restaura a aparência antiga.

Se os dois documentos tiverem alterações demais, a Comparação informa isso. Nesse caso, mostra alguns parágrafos como substituídos por inteiro, e pode não procurar movimentos ou alterações de formatação em todos eles.
