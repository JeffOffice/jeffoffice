# Escrever em células
<!-- keywords: escrever, introduzir, inserir, editar, alterar, modificar, célula, entrada, barra de fórmulas, F2, duplo clique, Tab, Esc, cancelar, anular, refazer, eliminar, limpar, apagar, percentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, sinal de igual, substituir, limite, carateres -->

Para colocar algo numa célula, selecione a célula e comece a escrever. O que escrever substitui o conteúdo da célula. Prima **Enter** para o manter ou **Esc** para desistir e deixar a célula como estava.

## Três formas de começar

- **Escrever por cima da célula.** Selecione uma célula e escreva. O primeiro carácter que escrever inicia a entrada e a célula mostra o que vai escrevendo.
- **Editar o conteúdo da célula.** Prima **F2** ou faça duplo clique na célula. A célula abre com o respetivo conteúdo e o cursor fica no fim. Utilize as teclas de seta para se deslocar no texto.
- **Utilizar a barra de fórmulas.** Clique na barra de fórmulas, que fica ao lado da Caixa de nome, e escreva aí. A célula também mostra o que escrever.

Quando escreve por cima de uma célula, uma tecla de seta conclui a entrada e move a seleção. Quando edita o conteúdo da célula, as teclas de seta deslocam-se no texto. Prima **F2** para alternar entre os dois modos.

## Concluir a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Mantém a entrada e move para baixo (ou para onde indicar **Ficheiro > Opções > Edição**) |
| Shift+Enter | Mantém a entrada e move no sentido contrário |
| Ctrl+Enter | Coloca a entrada em todas as células selecionadas e fica onde está |
| Tab | Mantém a entrada e move para a direita |
| Shift+Tab | Mantém a entrada e move para a esquerda |
| Teclas de seta | Mantêm a entrada e movem uma célula (apenas quando escreveu por cima da célula) |
| Esc | Desiste da entrada |
| Alt+Enter | Inicia uma nova linha dentro da célula |
| Ctrl+; | Insere a data de hoje (na entrada, quando está a escrever uma) |
| Ctrl+Shift+; | Insere a hora atual |
| Alt+Seta para baixo | Lista as palavras que já existem na coluna e permite escolher uma |

**Ctrl+Enter** com várias células selecionadas coloca o que escreveu em todas elas. Uma fórmula é escrita para a célula onde escreveu e, em cada uma das outras, lê-se como se fosse uma cópia dela: escrever **=A1** com o bloco E6:F7 selecionado dá **=B2** em F7. É um único passo de Anular.

**Alt+Seta para baixo** numa célula lista, por ordem e uma vez cada, as palavras das células acima e abaixo dela que não estejam separadas por uma célula vazia. Escolher uma escreve-a. Uma célula com uma lista própria mostra essa lista.

Em **Ficheiro > Opções > Edição**, escolha se **Enter** move para baixo, para a direita, para cima ou para a esquerda, ou se não move.

Se escrever em várias células ao longo de uma linha com **Tab** e depois premir **Enter**, a seleção desce para a linha seguinte, na coluna onde começou. Se selecionar primeiro um bloco de células, **Enter** e **Tab** movem-se dentro do bloco.

## Em que se torna o que escreve

O JeffCrunch lê o que escreve conforme indicam as definições do computador. O separador decimal, o separador de milhares, o símbolo monetário e a ordem do dia e do mês numa data vêm todos dessas definições. Um número escrito numa célula sem formato próprio recebe o formato que o mostra tal como o escreveu.

| Escreve | A célula contém | E mostra |
| --- | --- | --- |
| 1234.5 | o número 1234,5 | 1234.5 |
| 5% | o número 0,05 | 5% |
| $12.50 | o número 12,5 | $12.50 |
| 1 1/2 | o número 1,5 | 1 1/2 |
| 3/4/2025 | a data, como número de dias | 3/4/2025 |
| 10:30 PM | a hora, como parte de um dia | 10:30 PM |
| TRUE | o valor lógico verdadeiro | TRUE |
| #N/A | o erro #N/A | #N/A |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tem um formato próprio, como percentagem ou data, mantém-no. Uma célula com o formato **Texto** guarda tudo o que escrever como texto, exceto uma fórmula.

Para manter como texto algo que parece um número, como o código **007**, escreva primeiro um apóstrofo: **'007**. O apóstrofo não faz parte do texto. Quando editar a célula, volta a vê-lo.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** duplica o número em B2. Escreva os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch escreve-os em maiúsculas.

Os nomes das funções são sempre os ingleses, em todos os idiomas: escreva **SUM**, e não o nome que o seu idioma lhe dá. Os sinais entre eles seguem as definições do computador. Onde o separador decimal é a vírgula, como em alemão e francês, separa os argumentos de uma função com ponto e vírgula e escreve uma vírgula num número: **=SUM(A1;A2)*0,5**. A barra de fórmulas e **F2** mostram as fórmulas da mesma forma. Onde o separador decimal é o ponto, separa os argumentos com vírgula: **=SUM(A1,A2)*0.5**. O ficheiro guarda sempre a forma que todos os programas leem, com vírgulas entre argumentos e pontos nos números, para que abra da mesma forma em qualquer lado.

Se uma fórmula tiver um erro, como um parêntese que nunca é fechado, o JeffCrunch indica o que está errado e a entrada fica tal como a escreveu, com o cursor perto do erro, para que o possa corrigir. Nada é alterado até a fórmula estar correta.

Uma célula pode conter até 32 767 carateres e uma fórmula até 8192. O JeffCrunch avisa quando uma entrada é mais longa.

## Limpar células

Selecione as células e prima **Delete**. Isto limpa o conteúdo das células e mantém os formatos, e é um único passo que pode anular. O JeffCrunch limpa até 100 000 células de cada vez.

## Anular e Refazer

**{key:Undo}** anula a última alteração e **{key:Redo}** volta a efetuá-la. Também estão na barra de acesso rápido, na parte superior. Cada célula em que escreve é um passo, tal como cada limpeza. Anular repõe a célula como estava: o valor, a fórmula, o formato e as fórmulas que dependem dela. O JeffCrunch memoriza até 1000 passos, menos quando alteraram muitíssimas células, para nunca ocupar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando altera uma célula, o JeffCrunch calcula em segundo plano as fórmulas que dependem dela. A célula alterada mostra logo o novo conteúdo e as outras células são atualizadas um momento depois; a barra de estado indica **A calcular** até estarem atualizadas. Entretanto, pode continuar a escrever.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula de matriz** ou de uma tabela de dados não pode ser alterada isoladamente.
- Numa **folha protegida**, uma célula bloqueada não pode ser alterada. Todas as células estão bloqueadas, salvo indicação em contrário no livro. O JeffCrunch avisa quando tenta. Para alterar a célula, escolha Desproteger Folha no separador Rever.
- A primeira linha de uma **tabela** contém os nomes das colunas. Escreva um novo nome numa dessas células para mudar o nome da coluna: as fórmulas que utilizam o nome antigo acompanham a alteração e a tabela no ficheiro continua válida. Um nome que outra coluna já tenha recebe um número a seguir, um nome não pode estar vazio e **Delete** não limpa essa linha.
- Um livro que não pôde ser lido na íntegra pode ser consultado, mas não alterado nem guardado.
- Uma função que o JeffCrunch ainda não tem deixa uma nova fórmula sem valor na célula; o livro pede ao programa que o abrir a seguir que calcule a fórmula.
- A formatação, a ordenação e a funcionalidade localizar e substituir chegam numa versão posterior. Inserir e eliminar, cortar e colar, preencher e o tamanho das linhas e colunas são explicados em [Inserir e eliminar linhas, colunas e células](insert-delete), [Cortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).
