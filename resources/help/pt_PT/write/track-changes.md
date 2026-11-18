# Controlar alterações e aceitá-las ou rejeitá-las
<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações controladas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, apresentar para revisão, marcação simples, toda a marcação, sem marcação, original, mostrar marcação, balões, painel de revisão, barra de alteração, inserção, eliminação, ctrl+shift+e, autor, revisor, nome de utilizador, edições de outra pessoa -->

**Controlar Alterações** regista cada edição que fizer, para que você (ou outra pessoa) possa ver o que mudou e decidir se a quer manter. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai rever depois de si.

## Ativar e desativar o controlo de alterações

Clique em **Rever > Controlar Alterações**, ou prima **Ctrl+Shift+E**. A barra de estado, na parte inferior da janela, indica se está ativado. Um clique nesse texto alterna o estado. A definição é guardada no documento, por isso continua ativada quando o documento regressa de outra pessoa.

Enquanto estiver ativado:

- **O que escreve** fica marcado como inserção.
- **O que elimina** não é removido. Mantém-se no texto, riscado, para que possa ser reposto. A única exceção é o texto que escreveu a si próprio enquanto o controlo estava ativado: eliminá-lo remove-o simplesmente, porque ninguém mais o chegou a ver.
- **Uma quebra de parágrafo que faz** (Enter) fica marcada como inserida, e uma que elimina (Backspace no início de um parágrafo) fica marcada como eliminada.
- **Uma alteração de aspeto** (negrito, tipo de letra, tamanho, cor, alinhamento, avanços ou estilo) fica registada com o aspeto que o texto tinha antes.

Cada alteração é guardada com o nome de quem a fez e a data e hora. O nome é o indicado em **Ficheiro > Opções**, em **Nome de utilizador**. Altere-o aí se estiver errado.

Cada edição é um passo, por isso **Ctrl+Z** a anula como qualquer outra edição.

## Como é uma alteração

Por predefinição, as alterações de cada pessoa têm uma cor própria. A primeira pessoa cuja alteração o documento mostra recebe a primeira cor, a seguinte a segunda cor, e assim sucessivamente, e cada pessoa mantém a mesma cor enquanto o documento estiver aberto. O texto inserido fica sublinhado na cor da pessoa, e o texto eliminado fica riscado. O texto que foi movido aparece a verde: com risco duplo onde saiu, e com sublinhado duplo onde chegou. O texto cuja formatação foi alterada fica a negrito. Uma barra na margem esquerda assinala cada linha que tem uma alteração.

Pode alterar tudo isto. Clique em **Rever > Controlo > Opções de Controlo** para abrir a caixa **Opções de Controlo de Alterações**. Em **Marcação**, cada tipo de alteração tem um **Aspeto** e uma **Cor**:

- **Inserções**: sublinhado, sublinhado duplo, riscado, negrito, itálico, apenas cor, ou (nenhum).
- **Eliminações**: riscado, riscado duplo, oculto, circunflexo, ou apenas cor. Com **Oculto**, as palavras eliminadas são retiradas da linha. Com **Circunflexo**, também são retiradas, e um pequeno ^ indica onde estavam. O texto movido para outro local também é retirado quando escolhe **Balões** para as eliminações, mas não de outra forma.
- **Movido de** e **Movido para**: os aspetos e cores do texto movido. Estão disponíveis enquanto **Controlar movimentos** estiver assinalado. Se o desmarcar, o texto movido aparece como uma eliminação comum e uma inserção comum.
- **Formatação**: negrito, apenas cor, ou (nenhum). Se desmarcar **Controlar formatação**, as alterações de formatação não são assinaladas no texto.
- **Barras de alteração**: limite esquerdo, limite direito, limite exterior (o lado direito de uma página ímpar e o lado esquerdo de uma par), ou (nenhum), e a cor da barra.

Cada caixa **Cor** oferece **Por autor** (a cor própria de cada pessoa) ou uma só cor para todas as pessoas. Estas escolhas são suas. Ficam guardadas neste computador, e não no documento, aplicam-se a todos os documentos e a todas as janelas abertas, e cada janela é atualizada assim que clicar em **OK**. As barras são desenhadas da mesma forma no ecrã, numa impressão e num PDF sempre que a marcação estiver visível.

## Escolher o que vê: Apresentar para Revisão

Clique em **Rever > Apresentar para Revisão** e escolha uma opção:

- **Toda a Marcação** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento tal como ficará com as alterações aceites, e uma barra na margem junto a cada linha que tem uma alteração.
- **Sem Marcação** mostra o documento tal como ficará com todas as alterações aceites, e mais nada.
- **Original** mostra o documento tal como era antes de qualquer das alterações: o texto eliminado volta e o texto inserido desaparece.

Estas opções alteram apenas o que vê. Nunca alteram o documento, e não são guardadas com ele. Um documento que tenha alterações abre-se com **Toda a Marcação**.

**Rever > Mostrar Marcação** escolhe que tipos de alteração são mostrados: **Inserções e Eliminações**, **Formatação** e **Comentários**. Abaixo deles, escolha como é mostrado o texto eliminado. **Mostrar Todas as Revisões Inseridas no Texto** mantém-no no texto, riscado. **Mostrar Revisões em Balões** retira o texto eliminado da linha e coloca-o num balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e eliminações no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la definitivamente. Rejeitá-la é desfazê-la.

- **Rever > Aceitar** aceita a alteração onde está o cursor (ou todas as alterações do texto selecionado) e avança para a seguinte. **Rever > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta por baixo de **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controlo**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcação** tiver ativado (e, no painel **Revisão**, o revisor escolhido).
- **Anterior** e **Seguinte**, no grupo **Alterações**, passam de uma alteração para a seguinte sem alterar nada.

Cada uma destas ações é um passo, por isso **Ctrl+Z** repõe o documento exatamente como estava.

Aceitar uma eliminação remove o texto definitivamente. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que era novo. Rejeitar uma eliminação mantém o texto e remove o risco. Rejeitar uma alteração de aspeto repõe o aspeto que o texto tinha. O texto movido é um par de alterações, uma onde saiu e outra onde chegou. Quando o par está marcado com o mesmo nome, como outros programas o guardam, aceitar ou rejeitar qualquer uma das metades faz o mesmo à outra, por isso o texto fica movido definitivamente ou é reposto.

## O painel Revisão

Clique em **Rever > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem a fez, quando e as palavras. Um clique numa delas mostra-a no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Seguinte** por baixo da lista, ou escolha um revisor na caixa por cima dela para ver apenas as alterações dessa pessoa.

## O que ainda não é controlado

As alterações a tabelas (linhas, células, união de células), à numeração e às listas, às secções e à configuração da página, e ao texto de cabeçalhos, rodapés e notas são feitas como de costume, sem registo. As alterações que outros programas registaram nesses locais são mantidas e guardadas tal como estavam.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e mostra-as como alterações controladas.

1. Abra a versão mais antiga. É o **original**.
2. Clique em **Comparar** no grupo **Comparar** do separador **Rever**.
3. Clique em **Procurar** e escolha a versão mais recente. É a versão **revista**.
4. Escreva o nome com que quer assinar as alterações. Escolha as opções que pretender, como ignorar maiúsculas e minúsculas. **Localizar texto que foi movido** e **Localizar alterações de formatação** estão ativados, a menos que os desative.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisto. Os seus dois ficheiros não são alterados. Use **Aceitar** e **Rejeitar** como faz com qualquer alteração controlada. Se aceitar todas as alterações, obtém o texto revisto. Se rejeitar todas, obtém o texto original.

A comparação analisa o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés nem notas de rodapé.

Um parágrafo de 12 palavras ou mais que foi cortado de um sítio e colocado noutro é mostrado como um **movimento**: um registo onde o texto saiu e outro onde chegou. Aceitar ou rejeitar um deles aplica-se aos dois. O parágrafo tem de ser igual, ou pelo menos 9 em cada 10 palavras iguais e na maioria na mesma ordem; caso contrário, é mostrado como uma eliminação e uma inserção. As palavras que diferem dentro de um parágrafo movido são mostradas como inserções e eliminações dentro do movimento.

O texto que é igual nas duas versões mas tem aspeto diferente é mostrado como uma **alteração de formatação**. A comparação verifica negrito, itálico, sublinhado, tamanho, tipo de letra, cor, realce, e o estilo, alinhamento, avanços e espaçamento de um parágrafo. Rejeitar a alteração repõe o aspeto antigo.

Se os dois documentos tiverem alterações em excesso, a comparação indica-o. Nesse caso, mostra alguns parágrafos como substituídos por inteiro, e pode não procurar movimentos ou alterações de formatação em todos eles.
