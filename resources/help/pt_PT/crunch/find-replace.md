# Buscar, substituir e ir para

<!-- keywords: localizar, substituir, alterar, procurar, localizar o quê, substituir por, substituir tudo, localizar seguinte, alterar para, alterar tudo, mesmas maiúsculas, carácter universal, asterisco, ponto de interrogação, til, maiúsculas/minúsculas, célula inteira, ir para, referência, nome, selecionar por tipo, vazias, constantes, fórmulas, erros, região atual, matriz atual, diferenças de linha, diferenças de coluna, última célula, células visíveis, formatação condicional, validação, Ctrl+F, Ctrl+H, F5, notas -->

## Localizar

Prima **Ctrl+F** (ou escolha **Localizar** no separador Base e, depois, **Localizar** novamente). Escreva o texto em **Localizar o quê** e prima **Enter** para ir à célula seguinte que o contenha, ou escolha **Listar Tudo** para obter uma lista de todas as células correspondentes. Clique numa linha da lista para ir para essa célula.

| Opção | O que faz |
| --- | --- |
| **Onde** | Procura nesta folha ou em todas as folhas visíveis. Com mais do que uma célula selecionada, uma pesquisa numa folha procura apenas nelas. |
| **Ordem** | Visita as células por linhas (da esquerda para a direita e, depois, para baixo) ou por colunas (de cima para baixo e, depois, para a direita). **Localizar Seguinte** para na primeira correspondência nessa ordem. |
| **Procurar em** | **Fórmulas** procura no que está escrito em cada célula, **Valores** no que ela mostra, e **Notas** nas notas das células. |
| **Corresponder maiúsculas/minúsculas** | As letras maiúsculas e minúsculas têm de ser iguais. |
| **Corresponder à célula inteira** | A célula inteira tem de ser o texto, e não apenas uma parte dele. |

No texto que escreve em **Localizar o quê**, **?** representa qualquer carácter e **\*** qualquer sequência de carateres (incluindo nenhum). Escreva **~** antes de ?, \* ou ~ para procurar o próprio sinal: **a~\*b** corresponde a a\*b.

A pesquisa é executada aos poucos, para que possa continuar a trabalhar. **Esc** (ou **Cancelar**) interrompe-a, e o mesmo acontece quando altera uma célula. Um texto muito longo custa mais a cada passo do que um texto curto, pelo que uma folha com textos longos continua a deixar a janela livre.

As notas das células acompanham as células: depois de ordenar, ou depois de inserir ou eliminar linhas ou colunas, uma pesquisa em notas encontra cada nota na célula onde se encontra agora. Uma nota numa célula que foi eliminada deixa de ser pesquisada (e volta a ser pesquisada com Anular). Se um livro tiver sido alterado de tantas formas que não seja possível acompanhar as notas em todas elas, é-lhe pedido que o guarde primeiro. Um livro aberto a partir de um tipo de ficheiro mais antigo ou de um ficheiro de texto tem as suas notas numa forma que o JeffCrunch não pesquisa, e é informado disso.

## Substituir

Prima **Ctrl+H**, escreva o novo texto em **Substituir por** e escolha um botão. **Substituir** coloca o novo texto na célula com o cursor, se ela o contiver, e passa à seguinte. **Substituir Tudo** faz a substituição em todas as células que o contenham, num único passo que pode ser anulado; uma alteração grande é aplicada aos poucos. O novo texto é introduzido como se o tivesse escrito: um número torna-se um número, e uma fórmula alterada continua a ser uma fórmula.

Nada é alterado, e é-lhe indicado o motivo, se uma célula que seria alterada estiver bloqueada numa folha protegida, fizer parte de uma matriz ou de uma tabela de dados, estiver na primeira linha de uma tabela ou deixar de ser uma fórmula.

## Ir para

Prima **F5** (ou **Ctrl+G**), ou escreva na Caixa de nomes. Pode indicar uma célula, como B7, um intervalo, como B3:D9, colunas ou linhas inteiras (C:E, 4:9), uma célula de outra folha (Folha2!A1, ou 'A minha folha'!A1), ou o nome de um intervalo.

## Selecionar por tipo

Escolha **Selecionar por Tipo** no menu **Localizar** para selecionar células de acordo com o seu tipo:

- **Células em branco**, **Constantes** (introduzidas), **Fórmulas** e **Células que mostram um erro**. Para constantes e fórmulas, pode excluir números, texto, VERDADEIRO e FALSO, ou erros.
- **O bloco à volta da célula com o cursor** e **A última célula utilizada**.
- **A fórmula matricial onde está o cursor** seleciona todas as células dessa fórmula matricial.
- **Células diferentes da célula do cursor na sua linha** e **na sua coluna** encontram as células que não são iguais à célula com o cursor. Selecione primeiro um bloco e coloque o cursor na célula com que as outras devem coincidir. Em cada linha (ou coluna) do bloco, é escolhida uma célula quando contém um número ou texto diferente, ou quando contém uma fórmula que deixa de ser igual depois de os nomes de células da fórmula serem deslocados para a posição dessa célula. Uma célula com fórmula também é escolhida quando a célula do cursor não tem nenhuma. Se selecionar apenas uma célula, o JeffCrunch analisa o bloco à sua volta.
- **Células que mostram** (sem linhas ou colunas ocultas), **Células com formatação condicional** e **Células com uma regra para o que pode ser introduzido** (todas, ou apenas as que têm a mesma regra da célula com o cursor).

O primeiro bloco encontrado fica selecionado e os restantes são realçados; o realce desaparece quando clica numa célula.
