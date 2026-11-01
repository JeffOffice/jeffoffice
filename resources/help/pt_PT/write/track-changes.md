# Controlar alterações e aceitá-las ou rejeitá-las
<!-- keywords: controlar alterações, registar alterações, alterações controladas, revisões, marcação, aceitar, rejeitar, aceitar tudo, rejeitar tudo, apresentar para revisão, marcação simples, toda a marcação, sem marcação, original, mostrar marcações, balões, painel de revisão, barra de alteração, inserção, eliminação, ctrl+shift+e, autor, revisor, nome de utilizador, edições de outra pessoa -->

**Controlar Alterações** mantém um registo de todas as edições que efetua, para que o utilizador (ou outra pessoa) possa ver o que mudou e decidir se pretende mantê-lo. Utilize esta funcionalidade quando editar um documento que pertence a outra pessoa ou que alguém irá rever depois de si.

## Ativar e desativar o controlo de alterações

Clique em **Rever > Controlar Alterações** ou prima **Ctrl+Shift+E**. A barra de estado na parte inferior da janela indica se está ativado. Um clique nessas palavras inverte o estado. A definição é guardada no documento, pelo que continua ativada quando o documento regressa de outra pessoa.

Enquanto estiver ativado:

- **O que escreve** é marcado como inserção.
- **O que elimina** não é removido. Permanece no texto, rasurado, para poder ser reposto. A única exceção é o texto que o próprio utilizador escreveu enquanto o controlo estava ativado: eliminá-lo simplesmente remove-o, porque mais ninguém o viu.
- **Uma quebra de parágrafo que cria** (Enter) é marcada como inserida e uma que elimina (Retrocesso no início de um parágrafo) é marcada como eliminada.
- **Uma alteração de aspeto** (negrito, tipo de letra, tamanho, cor, alinhamento, avanços ou estilo) é registada juntamente com o aspeto anterior do texto.

Cada alteração é guardada com o nome de quem a efetuou e a data e hora. O nome é o que está em **Ficheiro > Opções**, em **Nome de utilizador**. Altere-o aí se estiver errado.

Cada edição é um passo, pelo que **Ctrl+Z** a anula como qualquer outra edição.

## Aspeto de uma alteração

As alterações de cada pessoa têm uma cor própria. O texto inserido é sublinhado na cor da pessoa e o texto eliminado é rasurado. Uma barra na margem esquerda assinala cada linha que contém uma alteração.

## Escolher o que vê: Apresentar para Revisão

Clique em **Rever > Apresentar para Revisão** e escolha uma opção:

- **Toda a Marcação** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento tal como ficará com as alterações aceites e uma barra na margem junto a cada linha que contém uma alteração.
- **Sem Marcação** mostra o documento tal como ficará com todas as alterações aceites, e nada mais.
- **Original** mostra o documento tal como era antes de qualquer alteração: o texto eliminado regressa e o texto inserido desaparece.

Estas opções apenas alteram o que vê. Nunca alteram o documento e não são guardadas com ele. Um documento que contém alterações abre com **Toda a Marcação**.

**Rever > Mostrar Marcações** permite escolher os tipos de alteração apresentados: **Inserções e Eliminações**, **Formatação** e **Comentários**. Abaixo destes, escolha como o texto eliminado é apresentado. **Mostrar Todas as Revisões Inseridas no Texto** mantém-no no texto, rasurado. **Mostrar Revisões em Balões** retira o texto eliminado da linha e coloca-o num balão na margem. **Mostrar Apenas Comentários e Formatação em Balões** mantém as inserções e eliminações no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la definitivamente. Rejeitá-la é anulá-la.

- **Rever > Aceitar** aceita a alteração onde está o cursor (ou todas as alterações no texto selecionado) e avança para a seguinte. **Rever > Rejeitar** faz o mesmo, anulando a alteração.
- Clique na seta por baixo de **Aceitar** ou **Rejeitar** para ver mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controlo**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" refere-se às alterações dos tipos ativados em **Mostrar Marcações** (e, no **Painel de Revisão**, ao revisor escolhido).
- **Anterior** e **Seguinte** no grupo **Alterações** permitem passar de uma alteração para outra sem alterar nada.

Cada uma destas ações é um passo, pelo que **Ctrl+Z** repõe o documento exatamente como estava.

Aceitar uma eliminação remove o texto definitivamente. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que era novo. Rejeitar uma eliminação mantém o texto e remove o rasurado. Rejeitar uma alteração de aspeto repõe o aspeto anterior do texto.

## O Painel de Revisão

Clique em **Rever > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem a efetuou, quando e as palavras. Um clique numa delas mostra-a no texto. Utilize **Aceitar**, **Rejeitar**, **Anterior** e **Seguinte** por baixo da lista ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## O que ainda não é controlado

As alterações a tabelas (linhas, células, união), à numeração e listas, a secções e à configuração da página e ao texto de cabeçalhos, rodapés e notas são efetuadas normalmente, sem registo. As alterações registadas por outros programas nesses locais são mantidas e guardadas tal como estavam.
