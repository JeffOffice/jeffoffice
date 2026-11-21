# Localizar, substituir e ir para

<!-- keywords: localizar, substituir, alterar, pesquisar, procurar, localizar o quê, substituir por, substituir tudo, localizar próximo, procurar por, alterar para, alterar tudo, mesmas maiúsculas, curinga, asterisco, ponto de interrogação, til, diferenciar maiúsculas de minúsculas, célula inteira, localizar por formato, formato, substituir em anotações, ir para, referência, nome, selecionar por tipo, vazias, constantes, fórmulas, erros, região atual, matriz atual, diferenças de linha, diferenças de coluna, última célula, células visíveis, formatação condicional, validação, Ctrl+F, Ctrl+H, F5, anotações -->

## Localizar

Pressione **Ctrl+F** (ou escolha **Localizar** na guia Página Inicial e, em seguida, **Localizar** novamente). Digite o texto em **Localizar o quê** e pressione **Enter** para ir à próxima célula que o contenha, ou escolha **Listar Tudo** para obter uma lista de todas as células correspondentes. Clique em uma linha da lista para ir até essa célula.

| Opção | O que faz |
| --- | --- |
| **Onde** | Pesquisa nesta planilha ou em todas as planilhas visíveis. Com mais de uma célula selecionada, uma pesquisa em uma planilha procura somente nelas. |
| **Ordem** | Percorre as células por linhas (da esquerda para a direita e, depois, para baixo) ou por colunas (de cima para baixo e, depois, para a direita). **Localizar Próxima** para na primeira correspondência nessa ordem. |
| **Pesquisar em** | **Fórmulas** procura no que está digitado em cada célula, **Valores** no que ela exibe, e **Anotações** nas anotações das células. |
| **Formato…** | Localiza somente as células que têm um formato. **Formato…** abre a caixa de diálogo Formatar Células, iniciada a partir da célula com o cursor: o que você alterar ali é o que será procurado (uma fonte em negrito, uma cor de preenchimento, um formato de número). **Formatar a Partir da Célula** usa todas as partes do formato da célula com o cursor, e **Limpar Formato** volta a procurar apenas o texto. A caixa de diálogo informa o que está sendo procurado e avisa se o formato escolhido não tem nenhuma configuração. A aparência que uma formatação condicional dá a uma célula não é pesquisada, apenas o formato que a própria célula tem. Sem texto em **Localizar o quê**, **Listar Tudo** e **Localizar Próxima** encontram as células que têm o formato; com texto, a célula precisa ter ambos. |
| **Diferenciar maiúsculas de minúsculas** | Letras maiúsculas e minúsculas devem ser iguais. |
| **Célula inteira** | A célula inteira deve conter o texto, e não apenas uma parte dele. |

No texto que você digita em **Localizar o quê**, **?** representa qualquer caractere e **\*** representa qualquer sequência de caracteres (inclusive nenhum). Digite **~** antes de ?, \* ou ~ para procurar o próprio sinal: **a~\*b** encontra a\*b.

A pesquisa é executada aos poucos, para que você possa continuar trabalhando. **Esc** (ou **Cancelar**) a interrompe, e alterar uma célula também. Um texto muito longo custa mais a cada pequena etapa do que um texto curto, de modo que uma planilha com textos longos ainda deixa a janela livre.

As anotações das células acompanham as células: após uma classificação, ou depois de inserir ou excluir linhas ou colunas, a pesquisa em anotações encontra cada anotação na célula em que ela está agora. Uma anotação de uma célula que foi excluída deixa de aparecer na pesquisa (e volta com Desfazer). Se uma pasta de trabalho foi alterada de tantas maneiras que as anotações não podem ser acompanhadas em todas elas, você é solicitado a salvá-la antes. Uma pasta de trabalho aberta de um tipo de arquivo mais antigo ou de um arquivo de texto tem suas anotações em um formato que o JeffCrunch não pesquisa, e você é informado disso.

## Substituir

Pressione **Ctrl+H**, digite o novo texto em **Substituir por** e escolha um botão. **Substituir** coloca o novo texto na célula com o cursor, se ela contiver o texto, e passa para a próxima. **Substituir Tudo** faz isso em todas as células que o contêm, como uma única etapa que você pode desfazer; uma alteração grande é feita aos poucos. O novo texto é inserido como se você o tivesse digitado: um número se torna número, e uma fórmula alterada continua sendo uma fórmula.

Para substituir em anotações, escolha **Anotações** em **Pesquisar em** na página Substituir. **Substituir Tudo** então altera as palavras de todas as anotações que contêm o texto, nesta planilha ou em todas as planilhas, como uma única etapa que você pode desfazer, e **Substituir** altera a anotação da célula com o cursor e passa para a próxima. Uma anotação que é uma conversa de comentários não é alterada (altere-as na guia Revisão), e você é informado de quantas ficaram de fora. Nada é alterado, e você é informado do motivo, se uma planilha protegida que tem uma anotação a alterar estiver protegida, ou se uma anotação ficaria sem palavras ou com mais de 32.767 caracteres. Uma anotação cujas palavras são substituídas perde a data em que foi escrita, como acontece ao editar suas palavras. As anotações são salvas com a pasta de trabalho, em todos os formatos que têm anotações. **Valores** podem ser pesquisados, mas não substituídos, porque um valor é o resultado de uma fórmula.

Nada é alterado, e você é informado do motivo, se uma célula que seria alterada estiver bloqueada em uma planilha protegida, fizer parte de uma matriz ou de uma tabela de dados, estiver na primeira linha de uma tabela ou deixar de ser uma fórmula.

## Ir para

Pressione **F5** (ou **Ctrl+G**), ou digite na Caixa de Nome. Você pode informar uma célula, como B7, um intervalo, como B3:D9, colunas ou linhas inteiras (C:E, 4:9), uma célula de outra planilha (Planilha2!A1, ou 'Minha planilha'!A1), ou o nome de um intervalo.

## Selecionar por tipo

Escolha **Selecionar por Tipo** no menu **Localizar** para selecionar células pelo que elas são:

- **Células vazias**, **Constantes** (digitadas), **Fórmulas** e **Células que mostram um erro**. Para constantes e fórmulas, você pode deixar de fora números, textos, VERDADEIRO e FALSO, ou erros.
- **O bloco ao redor da célula com o cursor** e **A última célula usada**.
- **A fórmula de matriz em que está o cursor** seleciona todas as células dessa fórmula de matriz.
- **Células diferentes da célula do cursor na mesma linha** e **na mesma coluna** encontram as células que não são iguais à célula com o cursor. Selecione primeiro um bloco e coloque o cursor na célula com a qual as demais devem coincidir. Em cada linha (ou coluna) do bloco, uma célula é escolhida quando contém um número ou texto diferente, ou quando contém uma fórmula que não é igual depois que seus nomes de célula são deslocados para a posição dessa célula. Uma célula que tem fórmula também é escolhida quando a célula do cursor não tem nenhuma. Se você selecionar apenas uma célula, o JeffCrunch analisa o bloco ao redor dela.
- **Células que mostram** (sem linhas ou colunas ocultas), **Células com formatação condicional** e **Células com regra de validação de dados** (todas, ou somente as que têm a mesma regra da célula com o cursor).

O primeiro bloco encontrado é selecionado, e os demais são destacados com uma cor de fundo; a cor desaparece quando você clica em uma célula.
