# Localizar, substituir e ir para
<!-- keywords: localizar, procurar, pesquisar, substituir, alterar, carácter universal, caráter universal, asterisco, ponto de interrogação, til, maiúsculas e minúsculas, célula inteira, ir para, referência, nome, selecionar por tipo, células em branco, constantes, fórmulas, erros, região atual, última célula, células visíveis, formatação condicional, validação de dados, Ctrl+F, Ctrl+H, F5, notas -->

## Localizar

Prima **Ctrl+F** (ou escolha **Localizar** no separador Base e, em seguida, **Localizar** novamente). Escreva o texto e prima **Enter** para ir para a célula seguinte que o contém, ou escolha **Listar Tudo** para obter uma lista de todas as células correspondentes. Clique numa linha da lista para ir para essa célula.

| Opção | O que faz |
| --- | --- |
| **Em** | Procura nesta folha ou em todas as folhas visíveis. Com mais do que uma célula selecionada, a procura numa folha incide apenas nessas células. |
| **Ordem** | Percorre as células por linhas (da esquerda para a direita e depois para baixo) ou por colunas (de cima para baixo e depois para a direita). **Seguinte** para na primeira correspondência por essa ordem. |
| **Procurar em** | **Fórmulas** procura no que está escrito em cada célula, **Valores** no que a célula mostra e **Notas** nas notas das células. |
| **Corresponder maiúsculas e minúsculas** | As letras maiúsculas e minúsculas têm de ser iguais. |
| **Corresponder à célula inteira** | A célula inteira tem de corresponder ao texto, e não apenas uma parte dela. |

No texto a procurar, **?** representa um carácter qualquer e **\*** qualquer sequência de carateres (incluindo nenhum). Escreva **~** antes de ?, \* ou ~ para procurar o próprio sinal: **a~\*b** corresponde a a\*b.

A procura é efetuada aos poucos, para que possa continuar a trabalhar. **Esc** (ou **Cancelar**) interrompe-a, tal como a alteração de uma célula. Um texto muito longo ocupa mais de cada pequeno passo do que um texto curto, pelo que uma folha com textos longos continua a deixar a janela disponível.

As notas acompanham as células: depois de uma ordenação, ou depois de inserir ou eliminar linhas ou colunas, a procura nas notas encontra cada nota na célula onde está agora. Uma nota numa célula eliminada deixa de ser encontrada (e volta depois de Anular). Se um livro tiver sido alterado de tantas formas que não seja possível acompanhar as notas em todas elas, ser-lhe-á pedido que o guarde primeiro; um livro aberto a partir de um tipo de ficheiro mais antigo ou de um ficheiro de texto tem as notas num formato em que o JeffCrunch não procura, e será informado disso.

## Substituir

Prima **Ctrl+H**. **Substituir** coloca o novo texto na célula com o cursor, se esta contiver o texto, e avança para a seguinte. **Substituir Tudo** faz o mesmo em todas as células que contêm o texto, num único passo que pode anular; uma substituição extensa é efetuada aos poucos. O novo texto é introduzido como se o tivesse escrito: um número torna-se um número e uma fórmula alterada continua a ser uma fórmula.

Nada é alterado, e é informado do motivo, se uma célula a alterar estiver bloqueada numa folha protegida, fizer parte de uma matriz ou de uma tabela de dados, estiver na primeira linha de uma tabela ou deixar de ser uma fórmula.

## Ir para

Prima **F5** (ou **Ctrl+G**), ou escreva na Caixa de nome. Pode indicar uma célula como B7, um intervalo como B3:D9, colunas ou linhas inteiras (C:E, 4:9), uma célula de outra folha (Folha2!A1 ou 'A minha folha'!A1) ou o nome de um intervalo.

## Selecionar por tipo

Escolha **Selecionar por Tipo** no menu **Localizar** para selecionar células consoante o que são:

- **Células em branco**, **Constantes (introduzidas, não calculadas)**, **Fórmulas** e **Células que mostram um erro**. Para constantes e fórmulas, pode omitir números, texto, VERDADEIRO e FALSO, ou erros.
- **O bloco à volta da célula com o cursor** e **A última célula utilizada**.
- **Células que são mostradas** (fora de linhas ou colunas ocultas), **Células com formatação condicional** e **Células com uma regra para o que pode ser introduzido** (todas, ou apenas as que têm a mesma regra que a célula com o cursor).

O primeiro bloco encontrado fica selecionado e os restantes ficam sombreados; o sombreado desaparece quando clica numa célula.
