# Controlar alterações, e aceitar ou rejeitar

<!-- keywords: comparar, comparar documentos, duas versões, o que mudou, controlar alterações, alterações controladas, revisões, marcações, aceitar, rejeitar, aceitar tudo, rejeitar tudo, exibir para revisão, marcação simples, todas as marcações, sem marcação, original, mostrar marcações, balões, painel de revisão, barra de alterações, inserção, exclusão, ctrl+shift+e, autor, revisor, nome de usuário, edições de outra pessoa -->

O **Controlar Alterações** registra cada edição que você faz, para que você (ou outra pessoa) veja o que mudou e decida se deseja mantê-la. Use-o quando editar um documento que pertence a outra pessoa, ou que alguém vai revisar depois de você.

## Ativar e desativar o Controlar Alterações

Clique em **Revisão > Controlar Alterações**, ou pressione **Ctrl+Shift+E**. A barra de status, na parte inferior da janela, mostra se ele está ativado. Um clique nessas palavras alterna o estado. A configuração é salva no documento, então continua ativada quando o documento volta de outra pessoa.

Enquanto estiver ativado:

- **O que você digita** é marcado como inserção.
- **O que você exclui** não é removido. Ele continua no texto, riscado, para que possa ser restaurado. A única exceção é o texto que você mesmo digitou com o controle ativado: excluí-lo apenas o remove, porque ninguém mais chegou a vê-lo.
- **Uma quebra de parágrafo que você faz** (Enter) é marcada como inserida, e uma que você exclui (Backspace no início de um parágrafo) é marcada como excluída.
- **Uma mudança de aparência** (negrito, fonte, tamanho, cor, alinhamento, recuos ou estilo) é registrada com a aparência que o texto tinha antes.

Cada alteração é salva com quem a fez e quando. O nome é o que está em **Arquivo > Opções**, em **Nome de usuário**. Altere-o ali se estiver errado.

Cada edição é uma etapa, então **Ctrl+Z** a desfaz como qualquer outra edição.

## Como é uma alteração

As alterações de cada pessoa têm uma cor própria. O texto inserido é sublinhado na cor da pessoa, e o texto excluído é riscado. Uma barra na margem esquerda marca cada linha que tem alteração.

## Escolher o que você vê: Exibir para Revisão

Clique em **Revisão > Exibir para Revisão** e escolha uma opção:

- **Todas as Marcações** mostra todas as alterações no texto, com as barras na margem.
- **Marcação Simples** mostra o documento como ficará com as alterações aceitas, e uma barra na margem ao lado de cada linha que tem alteração.
- **Sem Marcação** mostra o documento como ficará com todas as alterações aceitas, e nada além disso.
- **Original** mostra o documento como era antes de qualquer alteração: o texto excluído volta, e o texto inserido desaparece.

Essas opções mudam apenas o que você vê. Elas nunca alteram o documento e não são salvas com ele. Um documento que tenha alterações é aberto em **Todas as Marcações**.

**Revisão > Mostrar Marcações** escolhe quais tipos de alteração são exibidos: **Inserções e Exclusões**, **Formatação** e **Comentários**. Abaixo deles, escolha como o texto excluído é exibido. **Mostrar Todas as Revisões Embutidas** mantém o texto excluído no próprio texto, riscado. **Mostrar Revisões em Balões** tira o texto excluído da linha e o coloca em um balão na margem. **Mostrar Somente Comentários e Formatação em Balões** mantém inserções e exclusões no texto.

## Aceitar ou rejeitar alterações

Aceitar uma alteração é mantê-la definitivamente. Rejeitá-la é desfazê-la.

- **Revisão > Aceitar** aceita a alteração em que está o cursor (ou todas as alterações do texto selecionado) e depois passa para a próxima. **Revisão > Rejeitar** faz o mesmo, desfazendo a alteração.
- Clique na seta abaixo de **Aceitar** ou **Rejeitar** para mais opções: **Aceitar Todas as Alterações Mostradas**, **Aceitar Todas as Alterações** e **Aceitar Todas as Alterações e Parar o Controle**; e **Rejeitar Todas as Alterações Mostradas** e **Rejeitar Todas as Alterações**. "Mostradas" significa as alterações dos tipos que **Mostrar Marcações** deixou ativados (e, no painel **Revisão**, do revisor que você escolheu).
- **Anterior** e **Próxima**, no grupo **Alterações**, passam de uma alteração para a seguinte sem alterar nada.

Cada uma dessas ações é uma etapa, então **Ctrl+Z** devolve o documento exatamente como estava.

Aceitar uma exclusão remove o texto definitivamente. Rejeitar uma inserção também remove o texto. Aceitar uma inserção mantém o texto e remove a marca de que ele era novo. Rejeitar uma exclusão mantém o texto e remove o risco. Rejeitar uma mudança de aparência restaura a aparência que o texto tinha.

## O painel Revisão

Clique em **Revisão > Painel de Revisão** para abrir uma lista das alterações do documento: o tipo, quem as fez, quando e as palavras. Um clique em uma delas a mostra no texto. Use **Aceitar**, **Rejeitar**, **Anterior** e **Próxima** abaixo da lista, ou escolha um revisor na caixa acima dela para ver apenas as alterações dessa pessoa.

## O que ainda não é controlado

Alterações em tabelas (linhas, células, mesclagem), em numeração e listas, em seções e configuração de página, e em textos de cabeçalhos, rodapés e notas são feitas normalmente, sem registro. Alterações que outros programas registraram nesses locais são mantidas e salvas como estavam.

## Comparar duas versões

Comparar encontra as diferenças entre duas versões de um documento e as mostra como alterações controladas.

1. Abra a versão mais antiga. Ela é o **original**.
2. Clique em **Comparar** no grupo **Comparar**, na guia **Revisão**.
3. Clique em **Procurar** e escolha a versão mais recente. Ela é a versão **revisada**.
4. Digite o nome com o qual as alterações serão assinadas. Escolha as opções que desejar, como ignorar maiúsculas e minúsculas.
5. Clique em **OK**.

Abre-se uma nova janela com o texto original e as alterações que o transformam no texto revisado. Os dois arquivos não são alterados. Use **Aceitar** e **Rejeitar** como faria com qualquer alteração controlada. Se aceitar todas as alterações, você obtém o texto revisado. Se rejeitar todas, obtém o texto original.

Comparar analisa o texto do corpo do documento. Não analisa tabelas, cabeçalhos, rodapés, notas de rodapé nem a aparência do texto. Uma mudança apenas de aparência não é encontrada.
