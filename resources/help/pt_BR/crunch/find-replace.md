# Localizar, substituir e ir para
<!-- keywords: localizar, substituir, alterar, pesquisar, procurar, buscar, curinga, asterisco, ponto de interrogação, til, diferenciar maiúsculas de minúsculas, célula inteira, ir para, referência, nome, selecionar por tipo, células em branco, constantes, fórmulas, erros, região atual, última célula, células visíveis, formatação condicional, validação, Ctrl+F, Ctrl+H, F5, anotações -->

## Localizar

Pressione **Ctrl+F** (ou escolha **Localizar** na guia Página Inicial e depois **Localizar** novamente). Digite o texto e pressione **Enter** para ir à próxima célula que o contém, ou escolha **Listar Todas** para obter uma lista de todas as células correspondentes. Clique em uma linha da lista para ir até essa célula.

| Opção | O que faz |
| --- | --- |
| **Local** | Pesquisa nesta planilha ou em todas as planilhas visíveis. Com mais de uma célula selecionada, a pesquisa em uma planilha procura somente nessas células. |
| **Ordem** | Percorre as células por linhas (da esquerda para a direita e depois para baixo) ou por colunas (de cima para baixo e depois para a direita). **Próxima** para na primeira correspondência nessa ordem. |
| **Pesquisar em** | **Fórmulas** procura no que está digitado em cada célula, **Valores** no que ela mostra e **Anotações** nas anotações das células. |
| **Diferenciar maiúsculas de minúsculas** | As letras maiúsculas e minúsculas precisam ser iguais. |
| **Somente célula inteira** | A célula inteira precisa corresponder ao texto, não apenas uma parte dela. |

No texto procurado, **?** representa qualquer caractere único e **\*** qualquer sequência de caracteres (inclusive nenhum). Digite **~** antes de ?, \* ou ~ para procurar o próprio sinal: **a~\*b** corresponde a a\*b.

A pesquisa é feita aos poucos, para que você possa continuar trabalhando. **Esc** (ou **Cancelar**) a interrompe, assim como alterar uma célula. Um texto muito longo ocupa mais de cada pequena etapa do que um texto curto, de modo que uma planilha com textos longos ainda deixa a janela livre.

As anotações acompanham as células: depois de uma classificação, ou depois de inserir ou excluir linhas ou colunas, a pesquisa nas anotações encontra cada anotação na célula em que ela está agora. Uma anotação em uma célula excluída sai da pesquisa (e volta depois de Desfazer). Se uma pasta de trabalho tiver sido alterada de tantas maneiras que não seja possível acompanhar as anotações por todas elas, você será solicitado a salvá-la primeiro; uma pasta de trabalho aberta de um tipo de arquivo mais antigo ou de um arquivo de texto tem as anotações em um formato que o JeffCrunch não pesquisa, e você será avisado disso.

## Substituir

Pressione **Ctrl+H**. **Alterar** coloca o novo texto na célula com o cursor, se ela contiver o texto, e passa para a próxima. **Alterar Tudo** faz isso em todas as células que contêm o texto, como uma única etapa que pode ser desfeita; uma alteração grande é feita aos poucos. O novo texto é inserido como se você o tivesse digitado: um número se torna um número, e uma fórmula alterada continua sendo uma fórmula.

Nada é alterado, e você é informado do motivo, se uma célula que seria alterada estiver bloqueada em uma planilha protegida, fizer parte de uma matriz ou de uma tabela de dados, estiver na primeira linha de uma tabela ou deixar de ser uma fórmula.

## Ir para

Pressione **F5** (ou **Ctrl+G**) ou digite na caixa de nome. Você pode informar uma célula, como B7, um intervalo, como B3:D9, colunas ou linhas inteiras (C:E, 4:9), uma célula de outra planilha (Planilha2!A1 ou 'Minha planilha'!A1) ou o nome de um intervalo.

## Selecionar por tipo

Escolha **Selecionar por Tipo** no menu **Localizar** para selecionar células de acordo com o que elas são:

- **Células em branco**, **Constantes (digitadas, não calculadas)**, **Fórmulas** e **Células que mostram um erro**. Para constantes e fórmulas, você pode excluir números, texto, VERDADEIRO e FALSO ou erros.
- **O bloco ao redor da célula com o cursor** e **A última célula usada**.
- **Células visíveis** (fora de linhas ou colunas ocultas), **Células com formatação condicional** e **Células com uma regra para o que pode ser inserido** (todas elas ou somente as que têm a mesma regra da célula com o cursor).

O primeiro bloco encontrado é selecionado e os demais ficam destacados com uma tonalidade; a tonalidade desaparece quando você clica em uma célula.
