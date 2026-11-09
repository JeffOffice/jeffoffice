# Escrever nas células

<!-- keywords: correção automática, aspas inteligentes, travessão, fração, ligação, endereço, palavra a negrito, parte de uma célula, texto formatado, formatar parte do texto, colorir uma palavra, execuções, escrever, introduzir, entrada, editar, alterar, célula, entrada, barra de fórmulas, F2, duplo clique, Tab, Esc, desistir, anular, refazer, eliminar, limpar, percentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, igual, substituir, limite, carateres -->

Para colocar algo numa célula, selecione a célula e comece a escrever. O que escrever substitui o conteúdo que a célula tinha. Prima **Enter** para o manter, ou **Esc** para desistir da entrada e deixar a célula como estava.

## Três formas de começar

- **Escrever por cima da célula.** Selecione uma célula e escreva. O primeiro carácter que escrever inicia a entrada, e a célula mostra o que escreve à medida que avança.
- **Editar o conteúdo da célula.** Prima **F2** ou faça duplo clique na célula. A célula abre-se com o seu conteúdo, e o cursor fica no fim. Use as teclas de seta para se deslocar no texto.
- **Usar a barra de fórmulas.** Clique na barra de fórmulas, que fica ao lado da Caixa de nome, e escreva aí. A célula também mostra o que escreve.

Ao escrever por cima de uma célula, uma tecla de seta termina a entrada e move a seleção. Ao editar o conteúdo da célula, as teclas de seta deslocam-se no texto. Prima **F2** para passar de um modo para o outro.

## Terminar a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Mantém a entrada e desce (ou para onde indicar **Ficheiro > Opções > Edição**) |
| Shift+Enter | Mantém a entrada e move no sentido contrário |
| Ctrl+Enter | Coloca a entrada em todas as células selecionadas e mantém-se onde está |
| Tab | Mantém a entrada e move para a direita |
| Shift+Tab | Mantém a entrada e move para a esquerda |
| Teclas de seta | Mantêm a entrada e movem uma célula (só quando escreveu por cima da célula) |
| Esc | Desiste da entrada |
| Alt+Enter | Inicia uma nova linha dentro da célula |
| Ctrl+; | Coloca a data de hoje (na entrada, quando a estiver a escrever) |
| Ctrl+Shift+; | Coloca a hora atual |
| Alt+Seta para baixo | Lista as palavras que já existem na coluna, e escolha uma |

**Ctrl+Enter** com várias células selecionadas coloca o que escreveu em todas elas. Uma fórmula é escrita para a célula onde escreveu, e nas restantes lê-se como uma cópia dela leria: se escrever **=A1** com o bloco E6:F7 selecionado, obtém **=B2** em F7. É um único passo de **Anular**.

**Alt+Seta para baixo** numa célula lista, por ordem e uma só vez, as palavras das células acima e abaixo dela que não estejam separadas por uma célula vazia. Escolher uma delas escreve-a. Uma célula com lista própria mostra essa lista.

Em **Ficheiro > Opções > Edição**, escolha se **Enter** desce, vai para a direita, sobe, para a esquerda, ou não se desloca.

Se escrever em várias células de uma linha com **Tab** e depois premir **Enter**, a seleção desce para a linha seguinte, na coluna onde começou. Se selecionar primeiro um bloco de células, **Enter** e **Tab** percorrem o bloco.

## Correção automática ao escrever

Quando escreve um espaço ou um sinal como um ponto depois de uma palavra, o JeffCrunch pode corrigir a palavra, à medida que a termina. Corrige erros de escrita comuns (como "teh" em vez de "the"), duas maiúsculas no início de uma palavra, uma minúscula que inicia uma frase ou um dia da semana, e aspas retas (que passam a curvas). Também troca dois hífenes entre palavras por um travessão, e 1/2, 1/4 e 3/4 por ½, ¼ e ¾ quando a entrada tem palavras. Um número como 1/2 escrito sozinho continua a ser uma data ou uma fração, como sempre.

Cada correção é um passo próprio. Prima **{key:Undo}** logo após uma correção para desfazer apenas essa correção e manter o que escreveu. Nada é alterado numa fórmula: uma entrada que comece por um sinal de igual, ou por um sinal de mais ou de menos seguido de uma fórmula, fica como escreveu. Um endereço na Web (como https://example.org) ou um endereço de correio eletrónico que seja a entrada inteira torna-se uma ligação na sua célula quando termina a entrada, se **Converter endereços Web em ligações à medida que escreve** estiver ativo.

Ative ou desative estas opções em **Ficheiro > Opções > Verificação**. Uma palavra com erro ortográfico é assinalada enquanto escreve: veja [Ortografia](notes-links).

## Formatar parte do texto

Algumas palavras numa célula podem ter um aspeto diferente do resto. Por exemplo, uma palavra pode estar a negrito ou a vermelho.

1. Prima F2 para editar a célula (ou faça duplo clique nela).
2. Selecione as palavras que quer alterar, com Shift e as teclas de seta, ou com o rato.
3. Use **Negrito**, **Itálico**, **Sublinhado**, **Rasurado**, **Tipo de Letra**, **Tamanho do Tipo de Letra** ou **Cor do Tipo de Letra** no separador **Base**, ou Ctrl+B, Ctrl+I e Ctrl+U.
4. Prima Enter.

Só mudam as palavras que selecionou. O resto da célula mantém o aspeto próprio da célula. A caixa de entrada mostra as palavras com um aspeto único enquanto edita, e a célula mostra cada parte com o seu próprio aspeto quando prime Enter. Se escrever ou eliminar palavras mais tarde, a formatação acompanha as palavras. Escolher um tipo de letra ou um tamanho sem palavras selecionadas altera a célula inteira, como sempre.

As células que têm formatação dentro do texto vinda de outro programa mostram-na, mantêm-na e guardam-na de novo. Uma célula que altere mantém a respetiva formatação. A formatação dentro de um texto aplica-se ao texto, não aos números nem às fórmulas. Uma cópia guardada como ficheiro .xls mais antigo ou como .ods mantém as palavras, e a gravação indica que a formatação dentro delas não é mantida.

## O que escreve torna-se

O JeffCrunch interpreta o que escreve de acordo com as definições do computador. O separador decimal, o separador de milhares, o símbolo monetário e a ordem do dia e do mês numa data vêm dessas definições. Um número que escreva numa célula sem formato próprio recebe o formato que o mostra da forma como o escreveu.

| Escreve | A célula contém | E mostra |
| --- | --- | --- |
| 1234,5 | o número 1234,5 | 1234,5 |
| 5% | o número 0,05 | 5% |
| 12,50 € | o número 12,5 | 12,50 € |
| 1 1/2 | o número 1,5 | 1 1/2 |
| 4/3/2025 | a data, como número de dias | 04/03/2025 |
| 22:30 | a hora, como fração do dia | 22:30 |
| VERDADEIRO | o valor lógico verdadeiro | VERDADEIRO |
| #N/A | o erro #N/A | #N/A |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tenha um formato próprio, como uma percentagem ou uma data, mantém-no. Uma célula cujo formato seja **Texto** mantém tudo o que escreve como texto, exceto uma fórmula.

Para manter como texto algo que parece um número, como um código **007**, escreva primeiro um apóstrofo: **'007**. O apóstrofo não faz parte do texto. Quando editar a célula, volta a vê-lo.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** duplica o número em B2. Escreva os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch escreve-os em maiúsculas.

Os nomes das funções são sempre os ingleses, em todas as línguas: escreva **SUM**, e não o nome que a sua língua lhe dá. Os sinais entre eles seguem as definições do computador. Onde o separador decimal é a vírgula, como em alemão e francês, separa os argumentos de uma função com ponto e vírgula e escreve uma vírgula num número: **=SUM(A1;A2)*0,5**. A barra de fórmulas e o **F2** mostram as fórmulas da mesma forma. Onde o separador decimal é o ponto, separa os argumentos com vírgula: **=SUM(A1,A2)*0.5**. O ficheiro guarda sempre a forma que todos os programas leem, com vírgulas entre argumentos e pontos nos números, para que abra da mesma maneira em qualquer lado.

Se uma fórmula tiver um erro, como um parêntese que nunca é fechado, o JeffCrunch indica o que está errado, e a entrada fica como escreveu, com o cursor perto do erro, para que a possa corrigir. Nada é alterado até a fórmula estar correta.

Uma célula contém até 32 767 carateres, e uma fórmula até 8 192. O JeffCrunch avisa quando uma entrada é mais longa.

## Limpar células

Selecione as células e prima **Delete**. Isto limpa o conteúdo das células e mantém os respetivos formatos, e é um passo que pode anular. O JeffCrunch limpa até 100 000 células de cada vez.

No separador **Base**, o botão **Limpar** tem um menu. **Limpar Tudo** esvazia as células, os respetivos formatos, notas, comentários (as conversas com respostas) e ligações. Uma conversa com mais de 500 comentários é mantida, e o JeffCrunch indica-o, porque uma anulação não poderia repor os comentários que não leu. **Limpar Formatos** mantém o conteúdo das células e repõe o formato simples. **Limpar Conteúdo** é o mesmo que **Delete**. **Limpar Comentários e Notas** retira apenas as notas e os comentários das células selecionadas. **Limpar Hiperligações** retira apenas as ligações, e as células mantêm o texto. Uma opção aparece a cinzento quando as células selecionadas não têm nada para limpar com ela.

## Anular e Refazer

**{key:Undo}** anula a última alteração, e **{key:Redo}** volta a fazê-la. Estão também na barra de ferramentas rápida, no topo. Cada célula em que escreve é um passo, e cada limpeza também. Anular repõe a célula como estava: o seu valor, a sua fórmula, o seu formato e as fórmulas que dependem dela. O JeffCrunch guarda até 1 000 passos, menos quando estes alteraram muitíssimas células, para nunca ocupar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando altera uma célula, o JeffCrunch calcula em segundo plano as fórmulas que dependem dela. A célula que alterou mostra logo o novo conteúdo, e as restantes células atualizam-se um momento depois; a barra de estado indica **A calcular** até terminar. Entretanto, pode continuar a escrever.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula matricial** ou de uma tabela de dados não pode ser alterada isoladamente.
- Numa **folha protegida**, uma célula bloqueada não pode ser alterada. Todas as células estão bloqueadas, salvo se o livro indicar o contrário. O JeffCrunch avisa quando tenta. Para alterar a célula, escolha Desproteger Folha no separador Revisão.
- A primeira linha de uma **tabela** contém os nomes das suas colunas. Escreva um novo nome numa dessas células para mudar o nome da coluna: as fórmulas que usam o nome antigo acompanham a alteração, e a tabela no ficheiro mantém-se válida. Um nome que outra coluna já tenha recebe um número no fim, um nome não pode estar vazio, e **Delete** não limpa essa linha.
- Um livro que não pôde ser lido por inteiro pode ser consultado, mas não pode ser alterado nem guardado.
- Uma função que o JeffCrunch ainda não tem deixa uma nova fórmula sem valor na célula; o livro pede ao programa que a abre a seguir que calcule a fórmula.
- A formatação, a ordenação e a procura e substituição chegam numa versão posterior. Inserir e eliminar, cortar e colar, preencher e os tamanhos de linhas e colunas estão em [Inserir e eliminar linhas, colunas e células](insert-delete), [Cortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).

## Livros novos

Em **Ficheiro > Opções > Livros Novos**, escolha o tipo de letra, o tamanho do tipo de letra e o número de folhas com que todos os livros novos começam. Os livros que já tem não são alterados.
