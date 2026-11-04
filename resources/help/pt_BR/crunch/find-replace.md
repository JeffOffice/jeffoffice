# Localizar, substituir e ir para

<!-- keywords: localizar, substituir, alterar, pesquisar, curinga, asterisco, ponto de interrogação, til, mesmas maiúsculas, célula inteira, ir para, referência, nome, selecionar por tipo, em branco, constantes, fórmulas, erros, região atual, matriz atual, diferenças em linhas, diferenças em colunas, última célula, células visíveis, formatação condicional, validação, Ctrl+F, Ctrl+H, F5, anotações -->

## Localizar

Pressione **Ctrl+F** (ou escolha **Localizar** na guia Página Inicial e, em seguida, **Localizar** novamente). Digite o texto e pressione **Enter** para ir à próxima célula que o contém, ou escolha **Listar Todas** para obter uma lista de todas as células correspondentes. Clique em uma linha da lista para ir até essa célula.

| Opção | O que faz |
| --- | --- |
| **Onde** | Pesquisa nesta planilha ou em todas as planilhas visíveis. Com mais de uma célula selecionada, uma pesquisa em uma planilha procura somente nelas. |
| **Ordem** | Percorre as células por linhas (da esquerda para a direita, depois para baixo) ou por colunas (de cima para baixo, depois para a direita). **Próximo** para na primeira correspondência nessa ordem. |
| **Pesquisar em** | **Fórmulas** procura no que está digitado em cada célula, **Valores** no que ela exibe, e **Anotações** nas anotações das células. |
| **Diferenciar maiúsculas de minúsculas** | Letras maiúsculas e minúsculas devem ser iguais. |
| **Células inteiras** | A célula inteira deve conter o texto, não apenas uma parte dele. |

No texto que você procura, **?** representa qualquer caractere e **\*** representa qualquer sequência de caracteres (inclusive nenhum). Digite **~** antes de ?, \* ou ~ para procurar o próprio sinal: **a~\*b** encontra a\*b.

A pesquisa é executada aos poucos, para que você possa continuar trabalhando. **Esc** (ou **Cancelar**) interrompe a pesquisa, e o mesmo acontece ao alterar uma célula. Um texto muito longo exige mais de cada pequena etapa do que um texto curto, portanto uma planilha com textos longos ainda deixa a janela livre.

Anotações das células acompanham as células: depois de uma classificação, ou depois de linhas ou colunas serem inseridas ou excluídas, uma pesquisa em anotações encontra cada anotação na célula em que ela está agora. Uma anotação de uma célula excluída deixa de aparecer na pesquisa (e volta com Desfazer). Se uma pasta de trabalho foi alterada de tantas maneiras que as anotações não podem ser acompanhadas por todas elas, você será solicitado a salvá-la primeiro. Uma pasta de trabalho aberta de um tipo de arquivo mais antigo ou de um arquivo de texto tem suas anotações em um formato que o JeffCrunch não pesquisa, e você será avisado disso.

## Substituir

Pressione **Ctrl+H**. **Alterar** coloca o novo texto na célula com o cursor, se ela contiver o texto, e passa para a próxima. **Alterar Tudo** faz a substituição em todas as células que o contêm, como uma única etapa que você pode desfazer; uma alteração grande é aplicada aos poucos. O novo texto é inserido como se você o tivesse digitado: um número se torna número, e uma fórmula alterada continua sendo fórmula.

Nada é alterado, e você é informado do motivo, se uma célula que seria alterada estiver bloqueada em uma planilha protegida, fizer parte de uma matriz ou de uma tabela de dados, estiver na primeira linha de uma tabela ou deixar de ser uma fórmula.

## Ir para

Pressione **F5** (ou **Ctrl+G**), ou digite na Caixa de Nome. Você pode indicar uma célula, como B7, um intervalo, como B3:D9, colunas ou linhas inteiras (C:E, 4:9), uma célula de outra planilha (Planilha2!A1, ou 'Minha planilha'!A1), ou o nome de um intervalo.

## Selecionar por tipo

Escolha **Selecionar por Tipo** no menu **Localizar** para selecionar células pelo que elas são:

- **Células em branco**, **Constantes** (digitadas), **Fórmulas** e **Células que mostram um erro**. Para constantes e fórmulas, você pode deixar de fora números, texto, VERDADEIRO e FALSO, ou erros.
- **O bloco ao redor da célula com o cursor** e **A última célula usada**.
- **A fórmula de matriz em que está o cursor** seleciona todas as células dessa fórmula de matriz.
- **Células diferentes da célula do cursor na sua linha** e **na sua coluna** encontram as células que não são iguais à célula com o cursor. Selecione primeiro um bloco e coloque o cursor na célula com a qual as outras devem coincidir. Em cada linha (ou coluna) do bloco, uma célula é escolhida quando contém um número ou texto diferente, ou quando contém uma fórmula que não é igual depois que seus nomes de célula são deslocados para a posição dessa célula. Uma célula com fórmula também é escolhida quando a célula do cursor não tem nenhuma. Se você selecionar apenas uma célula, o JeffCrunch analisa o bloco ao redor dela.
- **Células que mostram** (não em linhas ou colunas ocultas), **Células com formatação condicional** e **Células com uma regra para o que pode ser digitado** (todas elas, ou somente as que têm a mesma regra da célula com o cursor).

O primeiro bloco encontrado é selecionado, e os demais são destacados com uma cor de fundo; essa cor desaparece quando você clica em uma célula.
