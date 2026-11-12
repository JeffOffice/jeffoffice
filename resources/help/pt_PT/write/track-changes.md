# Registar alterações e aceitar ou rejeitar

<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, controlo de alterações, alterações registadas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, apresentar para revisão, marcação simples, toda a marcação, sem marcação, original, mostrar marcação, balões, painel de revisão, barra de alteração, inserção, eliminação, ctrl+shift+e, autor, revisor, nome de utilizador, edições de outra pessoa -->

O **Controlar Alterações** guarda um registo de cada edição que faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se a mantém. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai rever depois de si.

## Ativar e desativar o Controlar Alterações

Clique em **Rever > Controlar Alterações**, ou prima **Ctrl+Shift+E**. A barra de estado, na parte inferior da janela, indica se está ativado. Um clique nessas palavras inverte a definição. A definição fica guardada no documento, por isso continua ativada quando o documento regressa de outra pessoa.

Enquanto estiver ativado:

- **O que escrever** é marcado como inserção.
- **O que eliminar** não é removido. Fica no texto, riscado, para que possa ser reposto. A única exceção é o texto que escreveu enquanto o controlo estava ativado: eliminá-lo remove-o simplesmente, porque ninguém mais o chegou a ver.
- **Uma quebra de parágrafo que faz** (Enter) é marcada como inserida, e uma que elimina (Backspace no início de um parágrafo) é marcada como eliminada.
- **Uma alteração de aspeto** (negrito, tipo de letra, tamanho, cor, alinhamento, avanços ou estilo) fica registada com o aspeto que o texto tinha antes.

Cada alteração é guardada com o nome de quem a fez e a data e hora. O nome é o que está em **Ficheiro > Opções**, em **Nome de utilizador**. Altere-o aí se estiver errado.

Cada edição é um passo, por isso **Ctrl+Z** a anula, como faz com qualquer outra edição.

## Aspeto das alterações

Por predefinição, as alterações de cada pessoa têm uma cor própria. À primeira pessoa cujas alterações o documento mostra é atribuída a primeira cor, à seguinte a segunda, e assim por diante; cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido é sublinhado na cor da pessoa, e o texto eliminado é riscado. O texto que foi movido é verde: duplamente riscado onde saiu, e duplamente sublinhado onde chegou. O texto cuja formatação foi alterada fica a negrito. Uma barra na margem esquerda assinala cada linha que tem uma alteração.

Pode alterar tudo isto. Clique em **Rever > Controlo > Opções de Controlo** para abrir a caixa **Opções de Controlo de Alterações**. Em **Marcação**, cada tipo de alteração tem um **Aspeto** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, rasurado, negrito, itálico, apenas cor, ou (nenhum).
- **Eliminações**: rasurado, rasurado duplo, oculto, cursor, ou apenas cor. Com **Oculto**, as palavras eliminadas são retiradas da linha. Com **Cursor**, também são retiradas, e um pequeno ^ mostra onde estavam. O texto movido para outro local também é retirado quando escolhe **Balões** para as eliminações, mas não em caso contrário.
- **Movido de** e **Movido para**: os aspetos e as cores do texto movido. Estão disponíveis enquanto **Controlar movimentos** estiver marcado. Se o desmarcar, o texto movido aparece como uma eliminação e uma inserção comuns.
- **Formatação**: negrito, apenas cor, ou (nenhum). Se desmarcar **Controlar formatação**, as alterações de formatação não são marcadas no texto.
- **Barras de alteração**: limite esquerdo, limite direito, limite exterior (o lado direito de uma página ímpar e o lado esquerdo de uma página par), ou (nenhum), e a cor da barra.

Cada caixa de **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma só cor para todas as pessoas. Estas escolhas são suas. Ficam guardadas neste computador, não no documento, aplicam-se a todos os documentos e a todas as janelas abertas, e cada janela é atualizada assim que clicar em **OK**. As barras são desenhadas da mesma forma no ecrã, numa impressão e num PDF sempre que a marcação estiver visível.

## Escolher o que vê: Apresentar para Revisão

Clique em **Rever > Apresentar para Revisão** e escolha uma opção:

- **Toda a Marcação** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceites, e uma barra na margem junto a cada linha que tem uma alteração.
- **Sem Marcação** mostra o documento como ficará com todas as alterações aceites, e nada mais.
- **Original** mostra o documento como era antes de qualquer alteração: o texto eliminado volta, e o texto inserido desaparece.

Estas opções alteram apenas o que vê. Nunca alteram o documento, e não são guardadas com ele. Um documento que tenha alterações abre-se com **Toda a Marcação**.

**Rever > Mostrar Marcação** escolhe quais os tipos de alteração apresentados: **Inserções e Eliminações**, **Formatação** e **Comentários**. Abaixo deles, escolha como é mostrado o texto eliminado. **Mostrar Todas as Revisões no Texto** mantém-no no texto, riscado. **Mostrar Revisões em Balões** retira o texto eliminado da linha e coloca-o num balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e as eliminações no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la de forma definitiva. Rejeitá-la é desfazê-la.

- **Rever > Aceitar** aceita a alteração onde está o cursor (ou todas as alterações do texto selecionado) e passa para a seguinte. **Rever > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta por baixo de **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controlo**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcação** tem ativados (e, no **Painel de Revisão**, o revisor que escolheu).
- **Anterior** e **Seguinte**, no grupo **Alterações**, passam de uma alteração para a seguinte sem alterar nada.

Cada uma destas ações é um passo, por isso **Ctrl+Z** repõe o documento exatamente como estava.

Aceitar uma eliminação remove o texto de forma definitiva. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e retira a marca de que era novo. Rejeitar uma eliminação mantém o texto e retira o rasurado. Rejeitar uma alteração de aspeto repõe o aspeto que o texto tinha. O texto que foi movido é um par de alterações, uma no sítio de onde saiu e outra no sítio para onde foi. Quando o par está marcado com o mesmo nome, como outros programas o guardam, aceitar ou rejeitar uma das metades faz o mesmo à outra, pelo que o texto fica movido de forma definitiva ou volta ao sítio original.

## O Painel de Revisão

Clique em **Rever > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem as fez, quando, e as palavras. Um clique numa delas mostra-a no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Seguinte** por baixo da lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## O que ainda não é registado

As alterações a tabelas (linhas, células, unir células), à numeração e às listas, às secções e à configuração da página, e ao texto em cabeçalhos, rodapés e notas são feitas como habitualmente, sem registo. As alterações registadas por outros programas nesses locais são mantidas e guardadas como estavam.

## Comparar duas versões

A **Comparação** encontra as diferenças entre duas versões de um documento e mostra-as como alterações registadas.

1. Abra a versão mais antiga. É o **original**.
2. Clique em **Comparar** no grupo **Comparar** do separador **Rever**.
3. Clique em **Procurar** e escolha a versão mais recente. É a versão **revista**.
4. Escreva o nome com que assinar as alterações. Escolha as opções que pretender, como ignorar diferenças em maiúsculas.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisto. Os seus dois ficheiros não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração registada. Se aceitar todas as alterações, obtém o texto revisto. Se rejeitar todas, obtém o texto original.

A Comparação analisa apenas o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés, notas de rodapé nem o aspeto do texto. Uma alteração apenas de aspeto não é encontrada.
