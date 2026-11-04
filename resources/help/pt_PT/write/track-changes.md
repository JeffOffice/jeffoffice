# Controlar alterações e aceitar ou rejeitar

<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlo de alterações, alterações registadas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, apresentar para revisão, marcação simples, toda a marcação, sem marcação, original, mostrar marcação, balões, painel de revisão, barra de alterações, inserção, eliminação, ctrl+shift+e, autor, revisor, nome de utilizador, edições de outra pessoa -->

O **Controlo de Alterações** guarda um registo de cada edição que faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se a quer manter. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai rever depois de si.

## Ligar e desligar o Controlo de Alterações

Clique em **Rever > Controlar Alterações**, ou prima **Ctrl+Shift+E**. A barra de estado, na parte inferior da janela, indica se está ativado. Um clique nessas palavras inverte a definição. A definição fica guardada no documento, por isso continua ativada quando o documento regressa de outra pessoa.

Enquanto estiver ativado:

- **O que escrever** fica marcado como inserção.
- **O que eliminar** não é removido. Continua no texto, riscado, para que possa ser reposto. A única exceção é o texto que você próprio escreveu com o controlo ativado: ao eliminá-lo, é simplesmente removido, porque ninguém mais o chegou a ver.
- **Uma quebra de parágrafo que fizer** (Enter) fica marcada como inserida, e uma que eliminar (Backspace no início de um parágrafo) fica marcada como eliminada.
- **Uma alteração de aspeto** (negrito, tipo de letra, tamanho, cor, alinhamento, avanços ou estilo) é registada com o aspeto que o texto tinha antes.

Cada alteração é guardada com o nome de quem a fez e a data e hora. O nome é o indicado em **Ficheiro > Opções**, em **Nome de utilizador**. Altere-o aí se estiver errado.

Cada edição é um passo, por isso **Ctrl+Z** a anula, como faz com qualquer outra edição.

## Aspeto de uma alteração

As alterações de cada pessoa têm uma cor própria. O texto inserido é sublinhado na cor da pessoa, e o texto eliminado é riscado. Uma barra na margem esquerda assinala cada linha que tem uma alteração.

## Escolher o que vê: Apresentar para Revisão

Clique em **Rever > Apresentar para Revisão** e escolha uma opção:

- **Toda a Marcação** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento tal como ficará com as alterações aceites, e uma barra na margem junto a cada linha que tenha uma alteração.
- **Sem Marcação** mostra o documento tal como ficará com todas as alterações aceites, sem mais nada.
- **Original** mostra o documento tal como era antes de qualquer alteração: o texto eliminado volta a aparecer, e o texto inserido desaparece.

Estas opções alteram apenas o que vê. Nunca alteram o documento e não são guardadas com ele. Um documento que tenha alterações abre com **Toda a Marcação**.

**Rever > Mostrar Marcação** escolhe que tipos de alteração são mostrados: **Inserções e Eliminações**, **Formatação** e **Comentários**. Por baixo, escolha como é mostrado o texto eliminado. **Mostrar Todas as Revisões Inseridas no Texto** mantém-no no texto, riscado. **Mostrar Revisões em Balões** tira o texto eliminado da linha e coloca-o num balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e as eliminações no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la de forma definitiva. Rejeitá-la é desfazê-la.

- **Rever > Aceitar** aceita a alteração em que está o cursor (ou todas as alterações do texto que selecionou) e passa para a seguinte. **Rever > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta por baixo de **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controlo**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcação** tem ativados (e, no painel **Revisão**, o revisor que escolheu).
- **Anterior** e **Seguinte**, no grupo **Alterações**, passam de uma alteração para a seguinte sem alterar nada.

Cada uma destas ações é um passo, por isso **Ctrl+Z** repõe o documento exatamente como estava.

Aceitar uma eliminação remove o texto de forma definitiva. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que era novo. Rejeitar uma eliminação mantém o texto e remove o riscado. Rejeitar uma alteração de aspeto repõe o aspeto que o texto tinha.

## O painel Revisão

Clique em **Rever > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem as fez, quando, e as palavras. Um clique numa delas mostra-a no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Seguinte** por baixo da lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## O que ainda não é registado

As alterações a tabelas (linhas, células, uniões), a numeração e listas, a secções e configuração da página, e ao texto de cabeçalhos, rodapés e notas são feitas como habitualmente, sem registo. As alterações registadas por outros programas nesses locais são mantidas e guardadas tal como estavam.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e mostra-as como alterações registadas.

1. Abra a versão mais antiga. É o **original**.
2. Clique em **Comparar** no grupo **Comparar** do separador **Rever**.
3. Clique em **Procurar** e escolha a versão mais recente. É a versão **revista**.
4. Escreva o nome com que vai assinar as alterações. Escolha as opções que desejar, como ignorar maiúsculas e minúsculas.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisto. Os seus dois ficheiros não são alterados. Use **Aceitar** e **Rejeitar** como faz para qualquer alteração registada. Se aceitar todas as alterações, obtém o texto revisto. Se rejeitar todas, obtém o texto original.

A comparação analisa o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés, notas de rodapé nem o aspeto do texto. Uma alteração apenas de aspeto não é encontrada.
