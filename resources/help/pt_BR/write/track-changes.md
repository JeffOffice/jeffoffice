# Controlar alterações, e aceitar ou rejeitá-las
<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações controladas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, exibir para revisão, marcação simples, todas as marcações, sem marcação, original, mostrar marcações, balões, painel de revisão, barra de alteração, inserção, exclusão, ctrl+shift+e, autor, revisor, nome de usuário, edições de outra pessoa -->

**Controlar Alterações** registra cada edição que você faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se quer mantê-la. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai revisar depois de você.

## Ativar e desativar o Controlar Alterações

Clique em **Revisão > Controlar Alterações** ou pressione **Ctrl+Shift+E**. A barra de status, na parte inferior da janela, indica se ele está ativado. Um clique nessas palavras inverte a configuração. A configuração é salva no documento, por isso continua ativada quando o documento volta de outra pessoa.

Enquanto estiver ativado:

- **O que você digita** é marcado como inserção.
- **O que você exclui** não é removido. Ele permanece no texto, tachado, para que possa ser restaurado. A única exceção é o texto que você mesmo digitou com o controle ativado: excluí-lo simplesmente o remove, porque ninguém mais chegou a vê-lo.
- **Uma quebra de parágrafo que você faz** (Enter) é marcada como inserida, e uma que você exclui (Backspace no início de um parágrafo) é marcada como excluída.
- **Uma mudança de aparência** (negrito, fonte, tamanho, cor, alinhamento, recuos ou estilo) é registrada junto com a aparência que o texto tinha antes.

Cada alteração é salva com o nome de quem a fez e a data e hora. O nome é o que aparece em **Arquivo > Opções**, em **Nome de usuário**. Altere-o ali se estiver errado.

Cada edição é uma etapa, por isso **Ctrl+Z** a desfaz como faz com qualquer outra edição.

## Como é uma alteração

Por padrão, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração aparece no documento recebe a primeira cor, a seguinte recebe a segunda, e assim por diante. Uma pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado na cor da pessoa, e o texto excluído é tachado. O texto que foi movido é verde: tachado duplamente onde saiu e sublinhado duplamente onde chegou. O texto cuja formatação foi alterada fica em negrito. Uma barra na margem esquerda marca cada linha que tem alguma alteração.

Você pode mudar tudo isso. Clique em **Revisão > Controle > Opções de Controle de Alterações** para abrir a caixa **Opções de Controle de Alterações**. Em **Marcação**, cada tipo de alteração tem uma **Aparência** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, tachado, negrito, itálico, somente cor, ou (nenhuma).
- **Exclusões**: tachado, tachado duplo, oculto, acento circunflexo, ou somente cor. Com **Oculto**, as palavras excluídas são retiradas da linha. Com **Acento circunflexo**, também são retiradas, e um pequeno ^ mostra onde estavam. O texto que foi movido também é retirado quando você escolhe **Balões** para exclusões, mas não nos demais casos.
- **Movido de** e **Movido para**: as aparências e cores do texto movido. Estão disponíveis enquanto **Controlar movimentos** estiver marcado. Se você desmarcá-lo, o texto movido aparece como uma exclusão comum e uma inserção comum.
- **Formatação**: negrito, somente cor, ou (nenhuma). Se você desmarcar **Controlar formatação**, as mudanças de formatação não são marcadas no texto.
- **Barras de alteração**: borda esquerda, borda direita, borda externa (o lado direito de uma página ímpar e o lado esquerdo de uma página par), ou (nenhuma), e a cor da barra.

Cada caixa **Cor** oferece **Por Autor** (a cor própria de cada pessoa) ou uma única cor para todas as pessoas. Essas escolhas são suas. Elas ficam guardadas neste computador, não no documento, valem para todos os documentos e todas as janelas abertas, e cada janela muda assim que você clica em **OK**. As barras são desenhadas da mesma forma na tela, em uma impressão e em um PDF, sempre que a marcação estiver sendo mostrada.

## Escolha o que você vê: Exibir para Revisão

Clique em **Revisão > Exibir para Revisão** e escolha uma opção:

- **Todas as Marcações** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceitas, e uma barra na margem ao lado de cada linha que tem alguma alteração.
- **Sem Marcação** mostra o documento como ficará com todas as alterações aceitas, e nada além disso.
- **Original** mostra o documento como era antes de qualquer alteração: o texto excluído volta, e o texto inserido desaparece.

Essas opções mudam apenas o que você vê. Elas nunca alteram o documento e não são salvas com ele. Um documento que tenha alterações é aberto com **Todas as Marcações**.

**Revisão > Mostrar Marcações** escolhe quais tipos de alteração são mostrados: **Inserções e Exclusões**, **Formatação** e **Comentários**. Abaixo deles, escolha como o texto excluído é mostrado. **Mostrar Todas as Revisões Embutidas** mantém o texto excluído no texto, tachado. **Mostrar Revisões em Balões** retira o texto excluído da linha e o coloca em um balão na margem. **Mostrar Somente Comentários e Formatação em Balões** mantém as inserções e exclusões no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la definitivamente. Rejeitá-la é desfazê-la.

- **Revisão > Aceitar** aceita a alteração em que está o cursor (ou todas as alterações do texto selecionado) e depois passa para a próxima. **Revisão > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta abaixo de **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controle**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcações** deixou ativados (e, no **Painel de Revisão**, do revisor escolhido).
- **Anterior** e **Próxima**, no grupo **Alterações**, passam de uma alteração para a seguinte sem modificar nada.

Cada uma dessas ações é uma etapa, por isso **Ctrl+Z** devolve o documento exatamente como estava.

Aceitar uma exclusão remove o texto definitivamente. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que ele era novo. Rejeitar uma exclusão mantém o texto e remove o tachado. Rejeitar uma mudança de aparência restaura a aparência que o texto tinha. O texto movido é um par de alterações: uma onde saiu e outra onde chegou. Quando o par está marcado com o mesmo nome, como outros programas o salvam, aceitar ou rejeitar uma das metades faz o mesmo com a outra, e o texto fica movido definitivamente ou volta ao lugar original.

## O Painel de Revisão

Clique em **Revisão > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem as fez, quando e as palavras. Um clique em uma delas a mostra no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Próxima** abaixo da lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## O que ainda não é controlado

Alterações em tabelas (linhas, células, mesclagem), em numeração e listas, em seções e configuração de página, e em texto de cabeçalhos, rodapés e notas são feitas normalmente, sem registro. As alterações registradas por outros programas nesses pontos são mantidas e salvas como estavam.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e as mostra como alterações controladas.

1. Abra a versão mais antiga. Ela é o **original**.
2. Clique em **Comparar** no grupo **Comparar**, na guia **Revisão**.
3. Clique em **Procurar** e escolha a versão mais recente. Ela é a versão **revisada**.
4. Digite o nome com que as alterações serão assinadas. Escolha as opções que quiser, como ignorar diferenças em letras maiúsculas.
5. Clique em **OK**.

Uma nova janela se abre com o texto original e as alterações que o transformam no texto revisado. Os seus dois arquivos não são alterados. Use **Aceitar** e **Rejeitar** como faria com qualquer alteração controlada. Se aceitar todas as alterações, você obtém o texto revisado. Se rejeitar todas, obtém o texto original.

A comparação examina o texto do corpo do documento. Não examina tabelas, cabeçalhos, rodapés, notas de rodapé nem a aparência do texto. Uma alteração apenas de aparência não é encontrada.
