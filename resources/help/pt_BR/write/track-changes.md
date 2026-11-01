# Controlar alterações e aceitá-las ou rejeitá-las
<!-- keywords: controlar alterações, alterações controladas, revisões, marcações, aceitar, rejeitar, aceitar tudo, rejeitar tudo, exibir para revisão, marcação simples, todas as marcações, sem marcação, original, mostrar marcações, balões, painel de revisão, barra de alteração, inserção, exclusão, ctrl+shift+e, autor, revisor, nome de usuário, edições de outra pessoa -->

O recurso **Controlar Alterações** mantém um registro de cada edição que você faz, para que você (ou outra pessoa) possa ver o que mudou e decidir se deve mantê-la. Use-o ao editar um documento de outra pessoa ou que alguém vai revisar depois de você.

## Ativar e desativar Controlar Alterações

Clique em **Revisão > Controlar Alterações** ou pressione **Ctrl+Shift+E**. A barra de status, na parte inferior da janela, informa se o recurso está ativado. Um clique nessas palavras inverte o estado. A configuração é salva no documento, por isso continua ativada quando o documento volta de outra pessoa.

Enquanto ele estiver ativado:

- **O que você digita** é marcado como inserção.
- **O que você exclui** não é removido. Permanece no texto, tachado, para que possa ser restaurado. A única exceção é o texto que você mesmo digitou com o controle ativado: ao excluí-lo, ele é simplesmente removido, porque ninguém mais chegou a vê-lo.
- **Uma quebra de parágrafo que você cria** (Enter) é marcada como inserida, e uma que você exclui (Backspace no início de um parágrafo) é marcada como excluída.
- **Uma alteração de aparência** (negrito, fonte, tamanho, cor, alinhamento, recuos ou estilo) é registrada junto com a aparência anterior do texto.

Cada alteração é salva com o nome de quem a fez e quando. O nome é o que aparece em **Arquivo > Opções**, em **Nome de usuário**. Corrija-o ali se estiver errado.

Cada edição é uma etapa, então **Ctrl+Z** a desfaz como qualquer outra edição.

## Aparência de uma alteração

As alterações de cada pessoa têm uma cor própria. O texto inserido aparece sublinhado na cor da pessoa, e o texto excluído aparece tachado. Uma barra na margem esquerda marca cada linha que contém uma alteração.

## Escolher o que ver: Exibir para Revisão

Clique em **Revisão > Exibir para Revisão** e escolha uma opção:

- **Todas as Marcações** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceitas, com uma barra na margem ao lado de cada linha que tem uma alteração.
- **Sem Marcação** mostra o documento como ficará com todas as alterações aceitas, e nada mais.
- **Original** mostra o documento como era antes de todas as alterações: o texto excluído volta e o texto inserido desaparece.

Essas opções mudam apenas o que você vê. Nunca alteram o documento e não são salvas com ele. Um documento que contém alterações é aberto com **Todas as Marcações**.

**Revisão > Mostrar Marcações** escolhe quais tipos de alteração são mostrados: **Inserções e Exclusões**, **Formatação** e **Comentários**. Abaixo deles, escolha como o texto excluído é mostrado. **Mostrar Todas as Revisões Embutidas** o mantém no texto, tachado. **Mostrar Revisões em Balões** retira o texto excluído da linha e o coloca em um balão na margem. **Mostrar Somente Comentários e Formatação em Balões** mantém as inserções e exclusões no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la definitivamente. Rejeitá-la é desfazê-la.

- **Revisão > Aceitar** aceita a alteração em que está o cursor (ou todas as alterações do texto selecionado) e passa para a próxima. **Revisão > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta abaixo de **Aceitar** ou **Rejeitar** para ver mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controle**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos ativados em **Mostrar Marcações** (e, no **Painel de Revisão**, do revisor escolhido).
- **Anterior** e **Próximo**, no grupo **Alterações**, passam de uma alteração para outra sem alterar nada.

Cada uma dessas ações é uma etapa, então **Ctrl+Z** restaura o documento exatamente como estava.

Aceitar uma exclusão remove o texto definitivamente. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que ele era novo. Rejeitar uma exclusão mantém o texto e remove o tachado. Rejeitar uma alteração de aparência restaura a aparência que o texto tinha.

## O Painel de Revisão

Clique em **Revisão > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem fez, quando e as palavras. Um clique em uma delas a mostra no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Próximo** abaixo da lista, ou escolha um revisor na caixa acima dela para ver somente as alterações dessa pessoa.

## O que ainda não é controlado

Alterações em tabelas (linhas, células, mesclagem), em numeração e listas, em seções e na configuração da página, e em texto de cabeçalhos, rodapés e notas são feitas normalmente, sem registro. As alterações que outros programas registraram nesses locais são mantidas e salvas como estavam.
