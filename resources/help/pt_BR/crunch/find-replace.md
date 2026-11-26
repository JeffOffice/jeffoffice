# Localizar, substituir e ir para

<!-- keywords: localizar, encontrar, procurar, substituir, trocar, localizar o quê, substituir por, substituir tudo, localizar próximo, procurar por, alterar para, alterar tudo, mesmas maiúsculas, curinga, asterisco, ponto de interrogação, til, diferenciar maiúsculas, célula inteira, localizar por formato, formato, substituir em anotações, ir para, referência, nome, selecionar por tipo, espaços em branco, constantes, fórmulas, erros, região atual, matriz atual, diferenças em linhas, diferenças em colunas, última célula, células visíveis, formatação condicional, validação, Ctrl+F, Ctrl+H, F5, anotações -->

## Localizar

Pressione **Ctrl+F** (ou escolha **Localizar** na guia Página Inicial e, em seguida, **Localizar** novamente). Digite o texto em **Localizar o quê** e pressione **Enter** para ir à próxima célula que o contenha, ou escolha **Localizar Tudo** para obter uma lista de todas as células correspondentes. Clique em uma linha da lista para ir até essa célula.

| Opção | O que faz |
| --- | --- |
| **Onde** | Pesquisa nesta planilha ou em todas as planilhas visíveis. Com mais de uma célula selecionada, a pesquisa em uma planilha procura apenas nelas. |
| **Ordem** | Percorre as células por linhas (da esquerda para a direita e depois para baixo) ou por colunas (de cima para baixo e depois para a direita). **Localizar Próximo** para na primeira correspondência nessa ordem. |
| **Pesquisar em** | **Fórmulas** procura no que é digitado em cada célula, **Valores** no que ela exibe, e **Anotações** nas anotações das células. |
| **Formato…** | Localiza apenas as células que têm um formato. **Formato…** abre a caixa de diálogo Formatar Células, iniciada a partir da célula com o cursor: o que você alterar ali é o que será procurado (uma fonte em negrito, uma cor de preenchimento, um formato de número). **Formatar a Partir da Célula** usa todas as partes da formatação da célula com o cursor, e **Limpar Formatação** volta a procurar somente o texto. A caixa de diálogo informa o que está sendo procurado, e avisa se o formato escolhido não tem nenhuma configuração. A aparência que uma formatação condicional dá a uma célula não é pesquisada, apenas o formato que a própria célula possui. Sem texto em **Localizar o quê**, **Localizar Tudo** e **Localizar Próximo** encontram as células que têm o formato; com texto, a célula deve ter os dois. |
| **Diferenciar maiúsculas de minúsculas** | Letras maiúsculas e minúsculas devem ser iguais. |
| **Células inteiras** | A célula inteira deve conter o texto, e não apenas uma parte dele. |

No texto que você digita em **Localizar o quê**, **?** representa qualquer caractere e **\*** qualquer sequência de caracteres (inclusive nenhum). Digite **~** antes de ?, \* ou ~ para procurar o próprio sinal: **a~\*b** encontra a\*b.

A pesquisa é executada aos poucos, para que você possa continuar trabalhando. **Esc** (ou **Cancelar**) a interrompe, e alterar uma célula também. Um texto muito longo exige mais de cada pequena etapa do que um texto curto, portanto uma planilha com textos longos ainda deixa a janela livre.

As anotações das células acompanham as células: após uma classificação, ou depois de inserir ou excluir linhas ou colunas, uma pesquisa em anotações encontra cada anotação na célula em que ela está agora. Uma anotação de uma célula que foi excluída deixa de aparecer na pesquisa (e volta com Desfazer). Se uma pasta de trabalho foi alterada de tantas maneiras que as anotações não podem ser acompanhadas em todas elas, você é solicitado a salvá-la antes. Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo ou de um arquivo de texto tem suas anotações em um formato que o JeffCrunch não pesquisa, e você é avisado disso.

## Substituir

Pressione **Ctrl+H**, digite o novo texto em **Substituir por** e escolha um botão. **Substituir** coloca o novo texto na célula com o cursor, se ela contiver o texto, e passa para a próxima. **Substituir Tudo** faz isso em todas as células que o contêm, como uma única etapa que pode ser desfeita; uma alteração grande é aplicada aos poucos. O novo texto é inserido como se você o tivesse digitado: um número se torna número, e uma fórmula alterada continua sendo uma fórmula.

Para substituir em anotações, escolha **Anotações** em **Pesquisar em** na guia Substituir. **Substituir Tudo** então altera as palavras de todas as anotações que contêm o texto, nesta planilha ou em todas as planilhas, como uma única etapa que pode ser desfeita, e **Substituir** altera a anotação da célula com o cursor e passa para a próxima. Uma anotação que é uma conversa de comentários é mantida como está (altere-as na guia Revisão), e você é informado de quantas foram deixadas de lado. Nada é alterado, e você é informado do motivo, se uma planilha com uma anotação a alterar estiver protegida, ou se uma anotação ficaria sem palavras ou com mais de 32.767 caracteres. Uma anotação cujas palavras são substituídas perde a data em que foi escrita, como acontece ao editar suas palavras. As anotações são salvas com a pasta de trabalho, em todas as formas que possuem anotações. **Valores** podem ser pesquisados, mas não substituídos, porque um valor é o resultado de uma fórmula.

Nada é alterado, e você é informado do motivo, se uma célula que seria alterada estiver bloqueada em uma planilha protegida, fizer parte de uma matriz ou de uma tabela de dados, estiver na primeira linha de uma tabela ou deixar de ser uma fórmula.

## Ir para

Pressione **F5** (ou **Ctrl+G**), ou digite na Caixa de Nome. Você pode informar uma célula, como B7, um intervalo, como B3:D9, colunas ou linhas inteiras (C:E, 4:9), uma célula de outra planilha (Planilha2!A1, ou 'Minha planilha'!A1), ou o nome de um intervalo. Um nome que representa as mesmas células em um conjunto de planilhas, como Jan:Dez!B2, leva à primeira planilha do conjunto.

## Selecionar por tipo

Escolha **Selecionar por Tipo** no menu **Localizar** para selecionar células pelo que elas são:

- **Células em branco**, **Constantes** (digitadas), **Fórmulas** e **Células que mostram um erro**. Para constantes e fórmulas, você pode deixar de fora números, texto, VERDADEIRO e FALSO, ou erros.
- **O bloco ao redor da célula com o cursor** e **A última célula usada**.
- **A fórmula de matriz em que o cursor está** seleciona todas as células dessa fórmula de matriz.
- **Células diferentes da célula com o cursor na sua linha** e **na sua coluna** encontram as células que não são iguais à célula com o cursor. Selecione primeiro um bloco e coloque o cursor na célula com a qual as outras devem coincidir. Em cada linha (ou coluna) do bloco, uma célula é escolhida quando contém um número ou texto diferente, ou quando contém uma fórmula que não é a mesma depois que os nomes de suas células são deslocados para a posição dessa célula. Uma célula que tem uma fórmula também é escolhida quando a célula com o cursor não tem nenhuma. Se você selecionar apenas uma célula, o JeffCrunch analisa o bloco ao redor dela.
- **Células que as fórmulas da seleção leem** e **Células com fórmulas que leem a seleção** selecionam as células desta planilha que as fórmulas das células selecionadas usam, ou as células cujas fórmulas usam as células selecionadas. Com uma célula selecionada, a busca parte dela. Marque **acompanhar em todos os níveis** para continuar a partir de cada célula encontrada (as células que estas leem, e assim por diante); sem essa opção, somente o primeiro nível é selecionado. A busca parte de no máximo 1.000 das células selecionadas, e a barra de status informa quando uma busca parou em um limite.
- **Células que mostram** (não em linhas ou colunas ocultas), **Células com formatação condicional** e **Células com uma regra para o que pode ser digitado** (todas elas, ou somente as que têm a mesma regra da célula com o cursor).

O primeiro bloco encontrado é selecionado e os demais recebem uma cor de realce; a cor desaparece quando você clica em uma célula.
