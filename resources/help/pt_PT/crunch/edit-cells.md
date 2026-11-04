# Escrever em células
<!-- keywords: escrever, escrita, introduzir, editar, alterar, célula, entrada, barra de fórmulas, F2, duplo clique, Tab, Esc, desistir, anular, refazer, eliminar, limpar, percentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, igual, substituir, limite, carateres -->

Para colocar algo numa célula, selecione a célula e comece a escrever. O que escrever substitui o que a célula continha. Prima **Enter** para o manter, ou **Esc** para desistir e deixar a célula como estava.

## Três formas de começar

- **Escrever por cima da célula.** Selecione uma célula e escreva. O primeiro carácter que escrever inicia a entrada, e a célula mostra o que escreve à medida que avança.
- **Editar o conteúdo da célula.** Prima **F2** ou faça duplo clique na célula. A célula abre-se com o seu conteúdo, e o cursor fica no fim. Use as teclas de seta para se deslocar no texto.
- **Usar a barra de fórmulas.** Clique na barra de fórmulas, que fica ao lado da Caixa de nome, e escreva aí. A célula mostra também o que escreve.

Quando escreve por cima de uma célula, uma tecla de seta termina a entrada e move a seleção. Quando edita o conteúdo da célula, as teclas de seta deslocam-se no texto. Prima **F2** para passar de um modo para o outro.

## Terminar a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Mantém a entrada e desce (ou segue o que indica **Ficheiro > Opções > Edição**) |
| Shift+Enter | Mantém a entrada e move no sentido contrário |
| Ctrl+Enter | Coloca a entrada em todas as células selecionadas e mantém-se onde está |
| Tab | Mantém a entrada e move para a direita |
| Shift+Tab | Mantém a entrada e move para a esquerda |
| Teclas de seta | Mantêm a entrada e movem uma célula (só quando escreveu por cima da célula) |
| Esc | Desiste da entrada |
| Alt+Enter | Inicia uma nova linha dentro da célula |
| Ctrl+; | Introduz a data de hoje (na entrada, quando está a escrever uma) |
| Ctrl+Shift+; | Introduz a hora atual |
| Alt+Seta para baixo | Lista as palavras já existentes na coluna, e escolha uma |

**Ctrl+Enter** com várias células selecionadas coloca o que escreveu em todas elas. Uma fórmula é escrita para a célula onde escreveu, e nas outras lê-se como se fosse uma cópia dela: se escrever **=A1** com o bloco E6:F7 selecionado, obtém **=B2** em F7. É um passo de Anular.

**Alt+Seta para baixo** numa célula lista, por ordem e uma só vez, as palavras das células acima e abaixo dela que não estejam separadas por uma célula vazia. Ao escolher uma, essa palavra é escrita. Uma célula que tenha lista própria mostra essa lista.

Em **Ficheiro > Opções > Edição**, escolha se **Enter** desce, vai para a direita, sobe ou para a esquerda, ou se não move a seleção.

Se escrever em várias células de uma linha com **Tab** e depois premir **Enter**, a seleção desce para a linha seguinte, na coluna onde começou. Se selecionar primeiro um bloco de células, **Enter** e **Tab** percorrem o bloco.

## O que escreve passa a ser

O JeffCrunch interpreta o que escreve de acordo com as definições do seu computador. O separador decimal, o separador de milhares, o símbolo monetário e a ordem do dia e do mês numa data vêm dessas definições. Um número escrito numa célula que não tem formato próprio recebe o formato que o mostra tal como o escreveu.

| Escreve | A célula contém | E mostra |
| --- | --- | --- |
| 1234,5 | o número 1234,5 | 1234,5 |
| 5% | o número 0,05 | 5% |
| 12,50 € | o número 12,5 | 12,50 € |
| 1 1/2 | o número 1,5 | 1 1/2 |
| 3/4/2025 | a data, como número de dias | 3/4/2025 |
| 10:30 PM | a hora, como fração de um dia | 10:30 PM |
| VERDADEIRO | o valor lógico verdadeiro | VERDADEIRO |
| #N/A | o erro #N/A | #N/A |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tem formato próprio, como percentagem ou data, mantém-no. Uma célula cujo formato é **Texto** mantém tudo o que escrever como texto, exceto uma fórmula.

Para manter como texto algo que parece um número, como um código **007**, escreva primeiro um apóstrofo: **'007**. O apóstrofo não faz parte do texto. Quando editar a célula, volta a vê-lo.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** duplica o número em B2. Escreva os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch escreve-os em maiúsculas.

Os nomes das funções são sempre os ingleses, em todas as línguas: escreva **SUM**, e não o nome que a sua língua lhe dá. Os sinais entre eles seguem as definições do seu computador. Onde o separador decimal é a vírgula, como em alemão e francês, separa os argumentos de uma função com ponto e vírgula, e escreve a vírgula num número: **=SOMA(A1;A2)*0,5**. A barra de fórmulas e o **F2** mostram as fórmulas da mesma forma. Onde o separador decimal é o ponto, separa os argumentos com vírgula: **=SOMA(A1,A2)*0.5**. O ficheiro guarda sempre a forma que todos os programas leem, com vírgulas entre argumentos e pontos nos números, para que abra igual em todo o lado.

Se uma fórmula tiver um erro, como um parêntese que nunca é fechado, o JeffCrunch indica o que está errado, e a entrada fica como a escreveu, com o cursor junto do erro, para que a possa corrigir. Nada é alterado até a fórmula estar correta.

Uma célula contém até 32 767 carateres, e uma fórmula até 8192. O JeffCrunch avisa quando uma entrada é mais longa.

## Limpar células

Selecione as células e prima **Delete**. Isto limpa o que as células contêm e mantém os seus formatos, e é um passo que pode anular. O JeffCrunch limpa até 100 000 células de cada vez.

No separador **Base**, o botão **Limpar** tem um menu. **Limpar Tudo** esvazia as células, os seus formatos, as suas notas e as suas ligações. **Limpar Formatos** mantém o que as células contêm e repõe o formato simples. **Limpar Conteúdo** é o mesmo que **Delete**. **Limpar Comentários e Notas** retira apenas as notas das células selecionadas. **Limpar Hiperligações** retira apenas as ligações, e as células mantêm o seu texto. Uma opção aparece a cinzento quando as células selecionadas não têm nada para limpar.

## Anular e Refazer

**{key:Undo}** anula a última alteração, e **{key:Redo}** volta a fazê-la. Estão também na barra de ferramentas rápida, no topo. Cada célula em que escreve é um passo, e também cada limpeza. Anular repõe a célula como estava: o seu valor, a sua fórmula, o seu formato e as fórmulas que dependem dela. O JeffCrunch guarda até 1000 passos, menos quando as alterações abrangem muitas células, para nunca ocupar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando altera uma célula, o JeffCrunch calcula em segundo plano as fórmulas que dependem dela. A célula alterada mostra logo o novo conteúdo, e as outras células atualizam-se um momento depois; a barra de estado indica **A calcular** até terminarem. Pode continuar a escrever entretanto.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula de matriz** ou de uma tabela de dados não pode ser alterada isoladamente.
- Numa **folha protegida**, uma célula bloqueada não pode ser alterada. Todas as células estão bloqueadas, salvo se o livro indicar o contrário. O JeffCrunch avisa quando tenta fazê-lo. Para alterar a célula, escolha Desproteger Folha no separador Rever.
- A primeira linha de uma **tabela** contém os nomes das suas colunas. Escreva um novo nome numa dessas células para mudar o nome da coluna: as fórmulas que usam o nome antigo acompanham a alteração, e a tabela no ficheiro continua válida. Um nome que outra coluna já tenha recebe um número a seguir, um nome não pode ficar vazio, e **Delete** não limpa essa linha.
- Um livro que não pôde ser lido por inteiro pode ser consultado, mas não pode ser alterado nem guardado.
- Uma função que o JeffCrunch ainda não tem deixa uma nova fórmula sem valor na célula; o livro pede ao programa que o abrir a seguir que calcule a fórmula.
- A formatação, a ordenação e o localizar e substituir chegam numa versão posterior. Inserir e eliminar, cortar e colar, preencher e os tamanhos de linhas e colunas estão em [Inserir e eliminar linhas, colunas e células](insert-delete), [Cortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).
