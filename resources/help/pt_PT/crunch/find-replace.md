# Localizar, substituir e ir para

<!-- keywords: localizar, procurar, pesquisar, substituir, alterar, localizar o quê, substituir por, substituir tudo, localizar seguinte, alterar para, alterar tudo, mesmas maiúsculas, caráter universal, asterisco, ponto de interrogação, til, maiúsculas e minúsculas, célula inteira, localizar por formato, formato, substituir em notas, ir para, referência, nome, selecionar por tipo, espaços em branco, constantes, fórmulas, erros, região atual, matriz atual, diferenças de linha, diferenças de coluna, última célula, células visíveis, formatação condicional, validação, Ctrl+F, Ctrl+H, F5, notas -->

## Localizar

Prima **Ctrl+F** (ou escolha **Localizar** no separador Base e, depois, **Localizar** novamente). Escreva o texto em **Localizar o quê** e prima **Enter** para ir à célula seguinte que o contém, ou escolha **Listar Tudo** para obter uma lista de todas as células que correspondem. Clique numa linha da lista para ir para essa célula.

| Opção | O que faz |
| --- | --- |
| **Onde** | Pesquisa nesta folha ou em todas as folhas visíveis. Com mais de uma célula selecionada, uma pesquisa de uma folha procura apenas nelas. |
| **Ordem** | Visita as células por linhas (da esquerda para a direita, depois para baixo) ou por colunas (de cima para baixo, depois para a direita). **Localizar Seguinte** para na primeira correspondência nessa ordem. |
| **Procurar em** | **Fórmulas** procura no que está escrito em cada célula, **Valores** no que ela apresenta, e **Notas** nas notas das células. |
| **Formato…** | Localiza apenas as células que têm um formato. **Formato…** abre a caixa de diálogo Formatar Células, iniciada a partir da célula com o cursor: o que alterar aí é o que se procura (um tipo de letra a negrito, uma cor de preenchimento, um formato de número). **Formatar a Partir da Célula** toma todas as partes do formato da célula com o cursor, e **Limpar Formato** volta a procurar apenas o texto. A caixa de diálogo indica o que se procura, e avisa se o formato escolhido não tem nada. O aspeto que uma formatação condicional dá a uma célula não é procurado, apenas o formato que a própria célula tem. Sem texto em **Localizar o quê**, **Listar Tudo** e **Localizar Seguinte** encontram as células que têm o formato; com texto, uma célula tem de ter ambos. |
| **Maiúsculas e minúsculas** | As letras maiúsculas e minúsculas têm de ser iguais. |
| **Célula inteira** | A célula inteira tem de ser o texto, e não apenas parte dele. |

No texto que escreve em **Localizar o quê**, **?** representa qualquer carácter e **\*** qualquer sequência de carateres (incluindo nenhum). Escreva **~** antes de ?, \* ou ~ para procurar o próprio sinal: **a~\*b** corresponde a a\*b.

A pesquisa decorre aos poucos, para que possa continuar a trabalhar. **Esc** (ou **Cancelar**) interrompe-a, e o mesmo acontece ao alterar uma célula. Um texto muito longo custa mais em cada passo do que um texto curto, pelo que uma folha com textos longos continua a deixar a janela livre.

As notas das células acompanham as células: após uma ordenação, ou após a inserção ou eliminação de linhas ou colunas, uma pesquisa de notas encontra cada nota na célula onde se encontra agora. Uma nota numa célula que foi eliminada deixa de estar na pesquisa (e volta com Anular). Se uma pasta de trabalho tiver sido alterada de tantas formas que não seja possível acompanhar as notas em todas elas, é-lhe pedido que a guarde primeiro; uma pasta aberta a partir de um tipo de ficheiro mais antigo ou de um ficheiro de texto tem as notas numa forma que o JeffCrunch não pesquisa, e é-lhe indicado isso.

## Substituir

Prima **Ctrl+H**, escreva o novo texto em **Substituir por** e escolha um botão. **Substituir** coloca o novo texto na célula com o cursor, se nela existir o texto, e passa à seguinte. **Substituir Tudo** faz a alteração em todas as células que o contêm, como um único passo que pode anular; uma alteração grande é aplicada aos poucos. O novo texto é introduzido como se o tivesse escrito: um número torna-se um número, e uma fórmula alterada continua a ser uma fórmula.

Para substituir em notas, escolha **Notas** em **Procurar em** no separador Substituir. **Substituir Tudo** altera então as palavras de todas as notas que contêm o texto, nesta folha ou em todas as folhas, como um único passo que pode anular, e **Substituir** altera a nota da célula com o cursor e passa à seguinte. Uma nota que é uma conversa de comentários fica como está (altere-os no separador Rever), e é-lhe indicado quantos ficaram por alterar. Nada é alterado, e é-lhe indicado o motivo, se uma folha com uma nota a alterar estiver protegida, ou se uma nota ficasse sem palavras ou com mais de 32 767 carateres. Uma nota cujas palavras são substituídas perde a data em que foi escrita, tal como acontece ao editar as suas palavras. As notas são guardadas com a pasta de trabalho, em todas as formas que têm notas. **Valores** podem ser pesquisados, mas não substituídos, porque um valor é o que uma fórmula calcula.

Nada é alterado, e é-lhe indicado o motivo, se uma célula que seria alterada estiver bloqueada numa folha protegida, fizer parte de uma matriz ou de uma tabela de dados, estiver na primeira linha de uma tabela, ou deixasse de ser uma fórmula.

## Ir para

Prima **F5** (ou **Ctrl+G**), ou escreva na Caixa de nome. Pode indicar uma célula como B7, um intervalo como B3:D9, colunas ou linhas inteiras (C:E, 4:9), uma célula de outra folha (Sheet2!A1, ou 'A minha folha'!A1), ou o nome de um intervalo. Um nome que representa as mesmas células num conjunto de folhas, como Jan:Dez!B2, leva-o à primeira folha do conjunto.

## Selecionar por tipo

Escolha **Selecionar por Tipo** no menu **Localizar** para selecionar células pelo que são:

- **Células vazias**, **Constantes** (escritas), **Fórmulas** e **Células que mostram um erro**. Para constantes e fórmulas pode deixar de fora números, texto, VERDADEIRO e FALSO, ou erros.
- **O bloco à volta da célula com o cursor** e **A última célula utilizada**.
- **A fórmula matricial onde está o cursor** seleciona todas as células dessa fórmula matricial.
- **Células diferentes da célula do cursor na sua linha** e **na sua coluna** encontram as células que não são iguais à célula com o cursor. Selecione primeiro um bloco e coloque o cursor na célula com a qual as outras devem coincidir. Em cada linha (ou coluna) do bloco, é escolhida uma célula quando tem um número ou texto diferente, ou quando tem uma fórmula que não é igual depois de os nomes das suas células serem deslocados para a posição dessa célula. Uma célula que tem uma fórmula também é escolhida quando a célula do cursor não tem nenhuma. Se selecionar apenas uma célula, o JeffCrunch analisa o bloco à volta dela.
- **Células que as fórmulas da seleção utilizam** e **Células com fórmulas que utilizam a seleção** selecionam as células desta folha que as fórmulas das células selecionadas usam, ou as células cujas fórmulas usam as células selecionadas. Com uma célula selecionada, partem dessa célula. Marque **seguir em todos os níveis** para continuar a partir de cada célula encontrada (as células que estas lêem, e assim por diante); sem essa opção, apenas o primeiro nível é selecionado. Partem de no máximo 1000 das células selecionadas, e a barra de estado indica quando uma pesquisa parou num limite.
- **Células que mostram** (não em linhas ou colunas ocultas), **Células com formatação condicional** e **Células com uma regra para o que pode ser introduzido** (todas, ou apenas as que têm a mesma regra da célula com o cursor).

O primeiro bloco encontrado fica selecionado e os restantes são realçados; o realce desaparece ao clicar numa célula.
