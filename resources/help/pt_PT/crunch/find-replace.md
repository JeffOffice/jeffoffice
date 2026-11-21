# Localizar, substituir e ir para

<!-- keywords: localizar, procurar, substituir, alterar, encontrar, localizar o quê, substituir por, substituir tudo, localizar seguinte, alterar para, alterar tudo, mesmas maiúsculas, carácter universal, asterisco, ponto de interrogação, til, maiúsculas e minúsculas, célula inteira, localizar por formato, formato, substituir em notas, ir para, referência, nome, selecionar por tipo, espaços em branco, constantes, fórmulas, erros, região atual, matriz atual, diferenças de linha, diferenças de coluna, última célula, células visíveis, formatação condicional, validação, Ctrl+F, Ctrl+H, F5, notas -->

## Localizar

Prima **Ctrl+F** (ou escolha **Localizar** no separador Base e, em seguida, **Localizar** novamente). Escreva o texto em **Localizar o quê** e prima **Enter** para ir para a célula seguinte que o contém, ou escolha **Listar Tudo** para obter uma lista de todas as células correspondentes. Clique numa linha da lista para ir para essa célula.

| Opção | O que faz |
| --- | --- |
| **Onde** | Procura nesta folha ou em todas as folhas visíveis. Com mais de uma célula selecionada, uma pesquisa numa folha procura apenas nelas. |
| **Ordem** | Visita as células por linhas (da esquerda para a direita, e depois para baixo) ou por colunas (de cima para baixo, e depois para a direita). **Localizar Seguinte** para na primeira correspondência nessa ordem. |
| **Procurar em** | **Fórmulas** procura no que está escrito em cada célula, **Valores** no que ela mostra, e **Notas** nas notas das células. |
| **Formato…** | Localiza apenas as células que têm um formato. **Formato…** abre a caixa de diálogo Formatar Células, iniciada a partir da célula onde está o cursor: o que alterar aí é o que se procura (um tipo de letra a negrito, uma cor de preenchimento, um formato de número). **Formatar a Partir da Célula** usa todas as partes do formato da célula onde está o cursor, e **Limpar Formato** volta a procurar apenas o texto. A caixa de diálogo indica o que está a ser procurado, e avisa se o formato escolhido não tem nada. O aspeto que uma formatação condicional dá a uma célula não é pesquisado, apenas o formato que a própria célula tem. Sem texto em **Localizar o quê**, **Listar Tudo** e **Localizar Seguinte** encontram as células com o formato; com texto, uma célula tem de ter ambos. |
| **Corresponder maiúsculas e minúsculas** | As letras maiúsculas e minúsculas têm de coincidir. |
| **Corresponder à célula inteira** | A célula inteira tem de ser o texto, e não apenas uma parte dele. |

No texto que escreve em **Localizar o quê**, **?** representa qualquer carácter e **\*** qualquer sequência de carateres (incluindo nenhum). Escreva **~** antes de ?, \* ou ~ para procurar o próprio sinal: **a~\*b** corresponde a a\*b.

A pesquisa decorre aos poucos, para que possa continuar a trabalhar. **Esc** (ou **Cancelar**) interrompe-a, e o mesmo acontece se alterar uma célula. Um texto muito longo custa mais em cada pequeno passo do que um texto curto, pelo que uma folha com textos longos continua a deixar a janela livre.

As notas das células acompanham as células: depois de uma ordenação, ou depois de linhas ou colunas serem inseridas ou eliminadas, uma pesquisa de notas encontra cada nota na célula onde se encontra agora. Uma nota numa célula que foi eliminada deixa de estar na pesquisa (e volta a estar após Anular). Se um livro tiver sido alterado de tantas formas que as notas não possam ser acompanhadas em todas elas, é-lhe pedido que o guarde primeiro. Um livro aberto a partir de um tipo de ficheiro mais antigo ou de um ficheiro de texto tem as notas num formato que o JeffCrunch não pesquisa, e é informado disso.

## Substituir

Prima **Ctrl+H**, escreva o novo texto em **Substituir por** e escolha um botão. **Substituir** coloca o novo texto na célula onde está o cursor, se a contiver, e passa à seguinte. **Substituir Tudo** faz a substituição em todas as células que o contêm, como um único passo que pode anular; uma alteração grande é aplicada aos poucos. O novo texto é introduzido como se o tivesse escrito: um número torna-se um número, e uma fórmula que seja alterada continua a ser uma fórmula.

Para substituir em notas, escolha **Notas** em **Procurar em** no separador Substituir. **Substituir Tudo** altera então as palavras de todas as notas que contenham o texto, nesta folha ou em todas as folhas, como um único passo que pode anular; **Substituir** altera a nota da célula onde está o cursor e passa à seguinte. Uma nota que seja uma conversa de comentários fica como está (altere-os no separador Rever), e é-lhe indicado quantos ficaram por alterar. Nada é alterado, e é-lhe indicado o motivo, se uma folha com uma nota a alterar estiver protegida, ou se uma nota ficasse sem palavras ou com mais de 32 767 carateres. Uma nota cujas palavras sejam substituídas perde a data em que foi escrita, tal como acontece quando edita as suas palavras. As notas são guardadas com o livro, em todas as formas que têm notas. **Valores** podem ser pesquisados, mas não substituídos, porque um valor é o que uma fórmula calcula.

Nada é alterado, e é-lhe indicado o motivo, se uma célula a alterar estiver bloqueada numa folha protegida, fizer parte de uma matriz ou de uma tabela de dados, estiver na primeira linha de uma tabela, ou deixar de ser uma fórmula.

## Ir para

Prima **F5** (ou **Ctrl+G**), ou escreva na Caixa de nome. Pode indicar uma célula, como B7, um intervalo, como B3:D9, colunas ou linhas inteiras (C:E, 4:9), uma célula de outra folha (Folha2!A1, ou 'A minha folha'!A1), ou o nome de um intervalo.

## Selecionar por tipo

Escolha **Selecionar por Tipo** no menu **Localizar** para selecionar células pelo que são:

- **Células vazias**, **Constantes** (introduzidas), **Fórmulas** e **Células que mostram um erro**. Para constantes e fórmulas, pode excluir números, texto, VERDADEIRO e FALSO, ou erros.
- **O bloco à volta da célula onde está o cursor**, e **A última célula utilizada**.
- **A fórmula matricial onde está o cursor** seleciona todas as células dessa fórmula matricial.
- **Células diferentes da célula do cursor na sua linha** e **na sua coluna** encontram as células que não são iguais à célula onde está o cursor. Selecione primeiro um bloco e coloque o cursor na célula com a qual as outras devem coincidir. Em cada linha (ou coluna) do bloco, uma célula é escolhida quando contém um número ou texto diferente, ou quando contém uma fórmula que não é a mesma depois de os nomes de células se deslocarem para a posição dessa célula. Uma célula que tenha uma fórmula também é escolhida quando a célula do cursor não tem nenhuma. Se selecionar apenas uma célula, o JeffCrunch analisa o bloco à volta dela.
- **Células que mostram** (não em linhas ou colunas ocultas), **Células com formatação condicional**, e **Células com uma regra para o que pode ser introduzido** (todas, ou apenas as que têm a mesma regra da célula onde está o cursor).

O primeiro bloco encontrado é selecionado e os restantes ficam com uma cor de realce; a cor desaparece quando clicar numa célula.
