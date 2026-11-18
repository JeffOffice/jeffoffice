# Controlar alterações, e aceitar ou rejeitar

<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações controladas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, exibir para revisão, marcação simples, todas as marcações, sem marcação, original, mostrar marcações, balões, painel de revisão, barra de alteração, inserção, exclusão, ctrl+shift+e, autor, revisor, nome de usuário, edições de outra pessoa -->

O **Controlar Alterações** guarda um registro de cada edição que você faz, para que você ou outra pessoa possa ver o que mudou e decidir se a mudança deve ficar. Use-o quando você editar um documento que pertence a outra pessoa, ou que alguém vai revisar depois de você.

## Ligar e desligar o Controlar Alterações

Clique em **Revisão > Controlar Alterações**, ou pressione **Ctrl+Shift+E**. A barra de status, na parte inferior da janela, mostra se ele está ativado. Um clique nessas palavras inverte a configuração. A configuração é salva no documento, por isso continua ativada quando o documento volta de outra pessoa.

Enquanto estiver ativado:

- **O que você digita** é marcado como inserção.
- **O que você exclui** não é removido. Ele permanece no texto, riscado, para que possa ser restaurado. O texto que você mesmo digitou enquanto o controle estava ativado é a única exceção: excluí-lo apenas o remove, porque ninguém mais o viu.
- **Uma quebra de parágrafo que você faz** (Enter) é marcada como inserida, e uma que você exclui (Backspace no início de um parágrafo) é marcada como excluída.
- **Uma mudança de aparência** (negrito, fonte, tamanho, cor, alinhamento, recuos ou estilo) é registrada com a aparência que o texto tinha antes.

Cada alteração é salva com quem a fez e quando. O nome é o que está em **Arquivo > Opções**, em **Nome de usuário**. Altere-o ali se estiver incorreto.

Cada edição é uma etapa, então **Ctrl+Z** a desfaz como faz com qualquer outra edição.

## Como é uma alteração

Por padrão, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento mostra recebe a primeira cor, a seguinte recebe a segunda, e assim por diante; cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado na cor da pessoa, e o texto excluído é riscado. O texto movido é verde: duplamente riscado onde saiu, e duplamente sublinhado onde chegou. O texto cuja formatação foi alterada fica em negrito. Uma barra na margem esquerda marca cada linha que tem uma alteração.

Você pode mudar tudo isso. Clique em **Revisão > Controle > Opções de Controle** para abrir a caixa **Opções de Controlar Alterações**. Em **Marcação**, cada tipo de alteração tem um **Estilo** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, tachado, negrito, itálico, somente cor ou (nenhum).
- **Exclusões**: tachado, tachado duplo, oculto, acento circunflexo ou somente cor. Com **Oculto**, as palavras excluídas são retiradas da linha. Com **Acento circunflexo**, elas também são retiradas, e um pequeno ^ mostra onde estavam. O texto movido para outro lugar também é retirado quando você escolhe **Balões** para exclusões, mas não nos demais casos.
- **Movido de** e **Movido para**: os estilos e cores do texto movido. Essas opções ficam disponíveis enquanto **Controlar movimentos** estiver marcado. Se você desmarcá-lo, o texto movido aparece como uma exclusão comum e uma inserção comum.
- **Formatação**: negrito, somente cor ou (nenhum). Se você desmarcar **Controlar formatação**, as mudanças de formatação não são marcadas no texto.
- **Barras de alteração**: borda esquerda, borda direita, borda externa (o lado direito de uma página ímpar e o lado esquerdo de uma página par), ou (nenhuma), e a cor da barra.

Cada caixa de **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma só cor para todas as pessoas. Essas escolhas são suas. Elas ficam guardadas neste computador, não no documento, valem para todos os documentos e todas as janelas abertas, e cada janela muda assim que você clica em **OK**. As barras são desenhadas da mesma forma na tela, em uma impressão e em um PDF sempre que as marcações estiverem visíveis.

## Escolha o que ver: Exibir para Revisão

Clique em **Revisão > Exibir para Revisão** e escolha uma opção:

- **Todas as Marcações** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceitas, e uma barra na margem ao lado de cada linha que tem uma alteração.
- **Sem Marcação** mostra o documento como ficará com todas as alterações aceitas, e nada mais.
- **Original** mostra o documento como era antes de qualquer alteração: o texto excluído volta, e o texto inserido desaparece.

Essas opções mudam apenas o que você vê. Elas nunca alteram o documento e não são salvas com ele. Um documento que tenha alterações é aberto com **Todas as Marcações**.

**Revisão > Mostrar Marcações** escolhe quais tipos de alteração são mostrados: **Inserções e Exclusões**, **Formatação** e **Comentários**. Abaixo deles, escolha como o texto excluído é mostrado. **Mostrar Todas as Revisões Embutidas** mantém o texto excluído no corpo, riscado. **Mostrar Revisões em Balões** tira o texto excluído da linha e o coloca em um balão na margem. **Mostrar Somente Comentários e Formatação em Balões** mantém as inserções e exclusões no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la em definitivo. Rejeitá-la é desfazê-la.

- **Revisão > Aceitar** aceita a alteração em que está o cursor (ou todas as alterações do texto selecionado) e depois passa para a próxima. **Revisão > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta abaixo de **Aceitar** ou **Rejeitar** para ver mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controle**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcações** deixou ativados (e, no painel **Painel de Revisão**, o revisor escolhido).
- **Anterior** e **Próxima**, no grupo **Alterações**, passam de uma alteração para a seguinte sem mudar nada.

Cada uma dessas ações é uma etapa, então **Ctrl+Z** devolve o documento exatamente como estava.

Aceitar uma exclusão remove o texto em definitivo. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que ele era novo. Rejeitar uma exclusão mantém o texto e remove o risco. Rejeitar uma mudança de aparência restaura a aparência que o texto tinha. O texto movido é um par de alterações, uma onde ele saiu e outra onde chegou. Quando o par está marcado com o mesmo nome, como outros programas o salvam, aceitar ou rejeitar qualquer uma das metades faz o mesmo com a outra, e o texto fica movido em definitivo ou volta ao lugar original.

## O painel de revisão

Clique em **Revisão > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem a fez, quando e as palavras. Um clique em uma delas a mostra no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Próxima** abaixo da lista, ou escolha um revisor na caixa acima dela para ver somente as alterações dessa pessoa.

## O que ainda não é controlado

Alterações em tabelas (linhas, células, mesclagem), em numeração e listas, em seções e configuração de página, e em textos de cabeçalhos, rodapés e notas são feitas normalmente, sem registro. As alterações que outros programas registraram nesses pontos são mantidas e salvas como estavam.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e as mostra como alterações controladas.

1. Abra a versão mais antiga. Ela é o **original**.
2. Clique em **Comparar** no grupo **Comparar** da guia **Revisão**.
3. Clique em **Procurar** e escolha a versão mais nova. Ela é a versão **revisada**.
4. Digite o nome com que as alterações serão assinadas. Escolha as opções que quiser, como ignorar maiúsculas e minúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** ficam ativados, a menos que você os desative.
5. Clique em **OK**.

Uma nova janela se abre com o texto original e as alterações que o transformam no texto revisado. Os seus dois arquivos não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração controlada. Se você aceitar todas as alterações, terá o texto revisado. Se rejeitar todas, terá o texto original.

A comparação analisa o texto do corpo do documento. Ela não analisa tabelas, cabeçalhos, rodapés nem notas de rodapé.

Um parágrafo de 12 palavras ou mais que foi recortado de um lugar e colocado em outro é mostrado como um **movimento**: um registro onde o texto saiu e outro onde ele chegou. Aceitar ou rejeitar um dos dois afeta os dois. O parágrafo precisa ser igual, ou ter pelo menos 9 em cada 10 palavras iguais e, na maioria, na mesma ordem; caso contrário, é mostrado como uma exclusão e uma inserção. As palavras que diferem dentro de um parágrafo movido são mostradas como inserções e exclusões dentro do movimento.

O texto que é igual nas duas versões, mas aparece de forma diferente, é mostrado como uma **alteração de formatação**. A comparação verifica negrito, itálico, sublinhado, tamanho, fonte, cor, realce, e o estilo, o alinhamento, os recuos e o espaçamento de um parágrafo. Rejeitar a alteração restaura a aparência antiga.

Se os dois documentos tiverem alterações demais, a comparação informa isso. Nesse caso, mostra alguns parágrafos como substituídos por inteiro e pode não procurar movimentos ou alterações de formatação em todos eles.
