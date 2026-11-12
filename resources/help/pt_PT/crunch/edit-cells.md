# Escrever nas células

<!-- keywords: símbolo, tipo de letra de símbolos, carateres especiais, tecla de atalho para um símbolo, correção automática, aspas inteligentes, travessão, fração, ligação, endereço, palavra a negrito, parte de uma célula, texto formatado, formatar parte do texto, colorir uma palavra, escrever, introduzir, entrada, editar, alterar, célula, entrada, barra de fórmulas, F2, duplo clique, Tab, Esc, desistir, anular, refazer, eliminar, limpar, percentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, igual, substituir, limite, carateres -->

Para introduzir algo numa célula, selecione a célula e comece a escrever. O que escrever substitui o que a célula tinha. Prima **Enter** para o manter, ou **Esc** para desistir da entrada e deixar a célula como estava.

## Três formas de começar

- **Escrever por cima da célula.** Selecione uma célula e escreva. O primeiro carácter que escrever inicia a entrada, e a célula mostra o que escreve à medida que o faz.
- **Editar o que está na célula.** Prima **F2**, ou faça duplo clique na célula. A célula abre com o seu conteúdo, e o cursor fica no fim. Use as teclas de seta para se deslocar no texto.
- **Usar a barra de fórmulas.** Clique na barra de fórmulas, que fica ao lado da Caixa de nome, e escreva aí. A célula mostra também o que escreve.

Ao escrever por cima de uma célula, uma tecla de seta termina a entrada e move a seleção. Ao editar o que está na célula, as teclas de seta deslocam-se no texto. Prima **F2** para passar de um modo para o outro.

## Terminar a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Mantém a entrada e desce (ou para onde **Ficheiro > Opções > Edição** indicar) |
| Shift+Enter | Mantém a entrada e move no sentido contrário |
| Ctrl+Enter | Coloca a entrada em todas as células selecionadas, e fica onde está |
| Tab | Mantém a entrada e move para a direita |
| Shift+Tab | Mantém a entrada e move para a esquerda |
| Teclas de seta | Mantêm a entrada e movem uma célula (só quando escreveu por cima da célula) |
| Esc | Desiste da entrada |
| Alt+Enter | Inicia uma nova linha dentro da célula |
| Ctrl+; | Introduz a data de hoje (na entrada, quando está a escrever uma) |
| Ctrl+Shift+; | Introduz a hora atual |
| Alt+Seta para baixo | Lista as palavras já existentes na coluna, e permite escolher uma |

**Ctrl+Enter** com várias células selecionadas coloca o que escreveu em todas elas. Uma fórmula é escrita para a célula onde escreveu e, nas restantes, é lida como uma cópia da mesma: escrever **=A1** com o bloco E6:F7 selecionado dá **=B2** em F7. É um passo de Anular.

**Alt+Seta para baixo** numa célula lista, por ordem e uma só vez, as palavras das células acima e abaixo dela que não estejam separadas por uma célula vazia. Escolher uma palavra escreve-a. Uma célula com uma lista própria mostra antes essa lista.

Em **Ficheiro > Opções > Edição**, escolha se **Enter** desce, vai para a direita, sobe ou para a esquerda, ou não se desloca.

Se escrever em várias células numa linha com **Tab** e depois premir **Enter**, a seleção desce para a linha seguinte, na coluna onde começou. Se selecionar primeiro um bloco de células, **Enter** e **Tab** percorrem o bloco.

## Correção automática ao escrever

Quando escreve um espaço ou um sinal, como um ponto, depois de uma palavra, o JeffCrunch pode corrigir a palavra à medida que a termina. Corrige erros comuns de escrita (como "teh" em vez de "the"), duas maiúsculas no início de uma palavra, uma minúscula que inicia uma frase ou um dia da semana, e aspas retas (passam a curvas). Também troca dois hífenes entre palavras por um travessão, e 1/2, 1/4 e 3/4 por ½, ¼ e ¾ quando a entrada tem palavras. Um número como 1/2 escrito sozinho continua a ser uma data ou uma fração, como sempre.

Cada correção é um passo próprio. Prima **{key:Undo}** logo a seguir a uma correção para anular apenas essa correção e manter o que escreveu. Nada é alterado numa fórmula: uma entrada que começa por um sinal de igual, ou por um sinal de mais ou de menos seguido de uma fórmula, mantém-se como escreveu. Um endereço na Web (como https://example.org) ou um endereço de correio eletrónico que seja a entrada inteira torna-se uma ligação na célula quando termina a entrada, se **Transformar endereços Web em hiperligações à medida que escreve** estiver ativado.

Ative ou desative estas opções em **Ficheiro > Opções > Verificação**. Uma palavra com erro ortográfico é assinalada enquanto escreve: veja [Ortografia](notes-links).

## Formatar parte do texto

Algumas palavras numa célula podem ter um aspeto diferente do resto. Por exemplo, uma palavra pode estar a negrito ou a vermelho.

1. Prima F2 para editar a célula (ou faça duplo clique nela).
2. Selecione as palavras que quer alterar, com Shift e as teclas de seta, ou com o rato.
3. Use **Negrito**, **Itálico**, **Sublinhado**, **Rasurado**, **Tipo de Letra**, **Tamanho do Tipo de Letra** ou **Cor do Tipo de Letra** no separador **Base**, ou Ctrl+B, Ctrl+I e Ctrl+U.
4. Prima Enter.

Só mudam as palavras selecionadas. O resto da célula mantém o aspeto próprio da célula. A caixa de entrada mostra as palavras com um aspeto único enquanto edita, e a célula mostra cada parte com o seu próprio aspeto quando prime Enter. Se mais tarde escrever ou eliminar palavras, a formatação acompanha as palavras. Escolher um tipo de letra ou um tamanho sem palavras selecionadas altera a célula inteira, como sempre.

As células que têm formatação dentro do texto vinda de outro programa mostram-na, mantêm-na e voltam a guardá-la. Uma célula que altera mantém a sua formatação. A formatação dentro de um texto aplica-se ao texto, não a números ou fórmulas. Uma cópia guardada como ficheiro .xls mais antigo ou como .ods mantém as palavras, e a gravação indica que a formatação dentro delas não é mantida.

## O que escreve torna-se

O JeffCrunch lê o que escreve de acordo com as definições do seu computador. O separador decimal, o separador de milhares, o símbolo monetário e a ordem do dia e do mês numa data vêm dessas definições. Um número escrito numa célula que não tem formato próprio recebe o formato que o mostra da forma como o escreveu.

| Escreve | A célula contém | E mostra |
| --- | --- | --- |
| 1234,5 | o número 1234,5 | 1234,5 |
| 5% | o número 0,05 | 5% |
| 12,50 € | o número 12,5 | 12,50 € |
| 1 1/2 | o número 1,5 | 1 1/2 |
| 4/3/2025 | a data, como número de dias | 4/3/2025 |
| 22:30 | a hora, como parte de um dia | 22:30 |
| VERDADEIRO | o valor lógico verdadeiro | VERDADEIRO |
| #N/D | o erro #N/D | #N/D |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tem um formato próprio, como uma percentagem ou uma data, mantém-no. Uma célula cujo formato é **Texto** mantém tudo o que escreve como texto, exceto uma fórmula.

Para manter como texto algo que parece um número, como um código **007**, escreva primeiro um apóstrofo: **'007**. O apóstrofo não faz parte do texto. Quando editar a célula, volta a vê-lo.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** duplica o número em B2. Escreva os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch escreve-os em maiúsculas.

Os nomes das funções são sempre os ingleses, em todas as línguas: escreva **SUM**, e não o nome que a sua língua lhe dá. Os sinais entre eles seguem as definições do seu computador. Quando o separador decimal é uma vírgula, como em português, separa os argumentos de uma função com ponto e vírgula e escreve a vírgula num número: **=SOMA(A1;A2)*0,5**. A barra de fórmulas e o **F2** mostram as fórmulas da mesma forma. Quando o separador decimal é um ponto, separa os argumentos com uma vírgula: **=SUM(A1,A2)*0.5**. O ficheiro guarda sempre a forma que todos os programas leem, com vírgulas entre argumentos e pontos nos números, para que abra da mesma forma em qualquer lado.

Se uma fórmula tiver um erro, como um parêntese que nunca é fechado, o JeffCrunch indica o que está errado, e a entrada mantém-se como escreveu, com o cursor perto do erro, para que a possa corrigir. Nada é alterado até a fórmula estar correta.

Uma célula contém até 32 767 carateres, e uma fórmula até 8192. O JeffCrunch avisa quando uma entrada é mais longa.

## Limpar células

Selecione as células e prima **Delete**. Isto limpa o que as células contêm e mantém os seus formatos, e é um passo que pode anular. O JeffCrunch limpa até 100 000 células de cada vez.

No separador **Base**, o botão **Limpar** tem um menu. **Limpar Tudo** esvazia as células, os seus formatos, as suas notas, os seus comentários (as conversas com respostas) e as suas ligações. Uma conversa com mais de 500 comentários é mantida, e o JeffCrunch indica-o, porque uma Anulação não conseguiria repor os comentários que não leu. **Limpar Formatos** mantém o que as células contêm e repõe o formato simples. **Limpar Conteúdo** é igual a **Delete**. **Limpar Comentários e Notas** retira apenas as notas e os comentários das células selecionadas. **Limpar Hiperligações** retira apenas as ligações, e as células mantêm o seu texto. Uma opção fica esbatida quando as células selecionadas não têm nada para limpar.

## Anular e refazer

**{key:Undo}** anula a última alteração, e **{key:Redo}** volta a fazê-la. Estão também na barra de ferramentas rápida, no topo. Cada célula em que escreve é um passo, e cada limpeza também. Anular repõe a célula como estava: o seu valor, a sua fórmula, o seu formato e as fórmulas que dependem dela. O JeffCrunch guarda até 1000 passos, menos quando estes alteraram muitíssimas células, para nunca ocupar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando altera uma célula, o JeffCrunch calcula em segundo plano as fórmulas que dependem dela. A célula que alterou mostra logo o novo conteúdo, e as outras células atualizam-se um momento depois; a barra de estado indica **A calcular** até terminar. Entretanto, pode continuar a escrever.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula matricial** ou de uma tabela de dados não pode ser alterada isoladamente.
- Numa **folha protegida**, uma célula bloqueada não pode ser alterada. Todas as células estão bloqueadas, salvo se o livro indicar o contrário. O JeffCrunch avisa quando tenta fazê-lo. Para alterar a célula, escolha Desproteger Folha no separador Rever.
- A primeira linha de uma **tabela** contém os nomes das suas colunas. Escreva um novo nome numa dessas células para mudar o nome da coluna: as fórmulas que usam o nome antigo acompanham a mudança, e a tabela no ficheiro continua válida. Um nome que outra coluna já tenha recebe um número a seguir, um nome não pode ficar vazio, e **Delete** não limpa essa linha.
- Um livro que não pôde ser lido por inteiro pode ser consultado, mas não pode ser alterado nem guardado.
- Uma função que o JeffCrunch ainda não tem deixa uma fórmula nova sem valor na célula; o livro pede ao programa que o abre a seguir para calcular a fórmula.
- A formatação, a ordenação e a procura e substituição chegam numa versão posterior. Inserir e eliminar, cortar e colar, preencher e os tamanhos das linhas e colunas estão em [Inserir e eliminar linhas, colunas e células](insert-delete), [Cortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).

## Inserir um símbolo

Clique em **Inserir > Símbolos > Símbolo** para abrir a caixa Símbolo. Escolha um carácter e clique em **Inserir**, ou faça duplo clique nele. Fica inserido no cursor da entrada que está a escrever. Se não estiver a escrever, a entrada da própria célula abre com o símbolo acrescentado no fim, e **Enter** coloca-o na célula. Exemplo: com uma célula que contém a palavra *Total*, escolher o sinal do euro dá a entrada *Total€*. Os símbolos que usou por último aparecem primeiro na lista.

**Retirar o carácter de um tipo de letra.** Escolha um tipo de letra na caixa **Tipo de Letra** e, depois, o carácter. O carácter fica num trecho próprio, composto nesse tipo de letra, e o resto do texto da célula mantém o tipo de letra da célula. Exemplo: numa célula com *Total*, um visto escolhido num tipo de letra de símbolos dá *Total✓*, com só o visto nesse tipo de letra. Com **(texto normal)** escolhido, é o tipo de letra da célula que desenha o símbolo, pelo que um símbolo que o tipo de letra não tenha aparece como um quadrado. Numa fórmula, entra apenas o carácter.

**Carateres especiais.** O separador **Carateres Especiais** lista o espaço inseparável e o hífen inseparável, o hífen opcional, o travessão curto e o longo, os espaços curto e longo, as aspas e a quebra opcional de largura nula, cada um com o seu nome. Faça duplo clique num deles para o inserir.

**Atribuir uma tecla de atalho a um símbolo.** Escolha um símbolo ou um carácter especial, clique em **Tecla de Atalho**, prima as teclas que quer, como **Ctrl+Alt+Shift+J**, e clique em **OK**. As teclas passam então a inserir esse símbolo, da mesma forma que a caixa. Deixe as teclas vazias e clique em **OK** para as retirar. O JeffCrunch avisa quando outro comando já usa essas teclas. O símbolo torna-se um comando, pelo que também aparece em **Ficheiro > Opções > Personalizar > Teclado**, onde pode alterar as suas teclas (veja [Atalhos de teclado](keyboard)).

## Livros novos

Em **Ficheiro > Opções > Novos Livros**, escolha o tipo de letra, o tamanho do tipo de letra e o número de folhas com que todos os livros novos começam. Os livros que já tem não são alterados.
