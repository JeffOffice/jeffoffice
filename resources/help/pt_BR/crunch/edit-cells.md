# Digitar em células
<!-- keywords: símbolo, fonte de símbolos, caracteres especiais, tecla de atalho para símbolo, autocorreção, aspas inteligentes, travessão, fração, link, endereço, palavra em negrito, parte de uma célula, texto formatado, formatar parte do texto, colorir uma palavra, digitar, digitação, inserir, editar, alterar, célula, entrada, barra de fórmulas, F2, clicar duas vezes, Tab, Esc, desistir, desfazer, refazer, excluir, limpar, porcentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, igual, substituir, limite, caracteres -->

Para colocar algo em uma célula, selecione a célula e comece a digitar. O que você digitar substitui o que a célula continha. Pressione **Enter** para confirmar, ou **Esc** para desistir e deixar a célula como estava.

## Três formas de começar

- **Digitar sobre a célula.** Selecione uma célula e digite. O primeiro caractere que você digitar inicia a entrada, e a célula mostra o que você digita à medida que escreve.
- **Editar o conteúdo da célula.** Pressione **F2** ou clique duas vezes na célula. A célula abre com o seu conteúdo, e o cursor fica no final. Use as setas do teclado para se mover no texto.
- **Usar a barra de fórmulas.** Clique na barra de fórmulas, que fica ao lado da Caixa de Nome, e digite ali. A célula também mostra o que você digita.

Ao digitar sobre uma célula, uma tecla de seta conclui a entrada e move a seleção. Ao editar o conteúdo da célula, as setas movem o cursor no texto. Pressione **F2** para alternar entre um e outro.

## Concluir a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Confirma a entrada e desce (ou para onde **Arquivo > Opções > Edição** indicar) |
| Shift+Enter | Confirma a entrada e move no sentido contrário |
| Ctrl+Enter | Coloca a entrada em todas as células selecionadas e permanece onde está |
| Tab | Confirma a entrada e move para a direita |
| Shift+Tab | Confirma a entrada e move para a esquerda |
| Teclas de seta | Confirma a entrada e move uma célula (somente quando você digitou sobre a célula) |
| Esc | Desiste da entrada |
| Alt+Enter | Inicia uma nova linha dentro da célula |
| Ctrl+; | Insere a data de hoje (na entrada, quando você estiver digitando uma) |
| Ctrl+Shift+; | Insere a hora atual |
| Alt+Seta para baixo | Lista as palavras já existentes na coluna e permite escolher uma |

**Ctrl+Enter** com várias células selecionadas coloca o que você digitou em todas elas. Uma fórmula é escrita para a célula em que você digitou, e nas demais ela se comporta como se fosse uma cópia: digitar **=A1** com o bloco E6:F7 selecionado resulta em **=B2** em F7. É uma única etapa do Desfazer.

**Alt+Seta para baixo** em uma célula lista, na ordem e uma única vez cada, as palavras das células acima e abaixo dela que não estejam separadas por uma célula vazia. Escolher uma delas a digita. Uma célula com lista própria mostra essa lista.

Em **Arquivo > Opções > Edição**, escolha se **Enter** move para baixo, para a direita, para cima, para a esquerda ou não move.

Se você digitar em várias células de uma linha com **Tab** e depois pressionar **Enter**, a seleção desce para a linha seguinte, na coluna em que você começou. Se você selecionar um bloco de células antes, **Enter** e **Tab** percorrem o bloco.

## Autocorreção ao digitar

Quando você digita um espaço ou um sinal como um ponto depois de uma palavra, o JeffCrunch pode corrigir a palavra assim que você a termina. Ele corrige erros comuns de digitação (como "teh" em vez de "the"), duas letras maiúsculas no início de uma palavra, uma letra minúscula que inicia uma frase ou um dia da semana, e aspas retas (que se tornam aspas curvas). Também troca dois hífens entre palavras por um travessão, e 1/2, 1/4 e 3/4 por ½, ¼ e ¾ quando a entrada tem palavras. Um número como 1/2 digitado sozinho continua sendo uma data ou uma fração, como sempre.

Cada correção é uma etapa própria. Pressione **{key:Undo}** logo após uma correção para desfazer somente essa correção e manter o que você digitou. Nada é alterado em uma fórmula: uma entrada que começa com sinal de igual, ou com sinal de mais ou de menos seguido de uma fórmula, permanece como você digitou. Um endereço na web (como https://example.org) ou um endereço de email que seja a entrada inteira se torna um link em sua célula ao concluir a entrada, se **Transformar endereços da web em links conforme você digita** estiver ativado.

Ative ou desative essas opções em **Arquivo > Opções > Revisão de Texto**. Uma palavra com erro de ortografia é marcada enquanto você digita: veja [Ortografia](notes-links).

## Formatar parte do texto

Algumas palavras em uma célula podem ter aparência diferente do restante. Por exemplo, uma palavra pode estar em negrito ou em vermelho.

1. Pressione F2 para editar a célula (ou clique duas vezes nela).
2. Selecione as palavras que deseja alterar, com Shift e as teclas de seta ou com o mouse.
3. Use **Negrito**, **Itálico**, **Sublinhado**, **Tachado**, **Fonte**, **Tamanho da Fonte** ou **Cor da Fonte** na guia **Página Inicial**, ou use Ctrl+N, Ctrl+I e Ctrl+S.
4. Pressione Enter.

Somente as palavras selecionadas mudam. O restante da célula mantém a aparência própria da célula. A caixa de entrada mostra as palavras com uma aparência única enquanto você edita, e a célula mostra cada parte com sua própria aparência quando você pressiona Enter. Se você digitar ou apagar palavras depois, a formatação acompanha as palavras. Escolher uma fonte ou um tamanho sem palavras selecionadas altera a célula inteira, como sempre.

## A minibarra de ferramentas

Uma pequena barra com a formatação que você mais usa aparece flutuando onde você trabalha, para que você não precise subir até a faixa de opções. Ela nunca tira o teclado da célula: o que você estava digitando continua onde estava.

- **Texto selecionado em uma célula:** selecione palavras com o mouse enquanto edita uma célula, e a barra aparece acima delas. Ela tem as caixas de **fonte** e de **tamanho**, **Negrito**, **Itálico**, **Sublinhado**, **Aumentar Tamanho da Fonte**, **Diminuir Tamanho da Fonte** e **Cor da Fonte**. Elas alteram apenas as palavras selecionadas, do mesmo modo que na faixa de opções.
- **Células selecionadas:** clique com o botão direito nas células, e a barra fica no topo do menu de atalho. Ela também tem **Cor de Preenchimento**, **Bordas**, **Moeda**, **Estilo de Porcentagem**, **Estilo de Vírgula**, **Aumentar Casas Decimais**, **Diminuir Casas Decimais**, **Mesclar e Centralizar** e **Pincel de Formatação**. São os próprios comandos da faixa de opções, e cada um é uma única etapa que pode ser desfeita.

A barra sobre o texto selecionado se apaga à medida que você afasta o ponteiro, e some quando ele está longe. Ela também some quando você digita, pressiona Esc ou rola a tela. Para desativar a barra, desmarque **Mostrar Minibarra de Ferramentas na seleção** na página **Geral** de **Arquivo > Opções**; os demais aplicativos do JeffOffice seguem essa escolha também. O botão Análise Rápida, que aparece sob um intervalo selecionado, não está nesta versão.

Células que têm formatação dentro do texto vindo de outro programa mostram essa formatação, mantêm e a gravam de volta. Uma célula que você altera mantém a sua formatação. A formatação dentro de um texto se aplica ao texto, não a números ou fórmulas. Uma cópia salva como arquivo .xls antigo ou como .ods mantém as palavras, e o salvamento informa que a formatação dentro delas não é mantida.

## O que você digita se transforma em

O JeffCrunch interpreta o que você digita de acordo com as configurações do seu computador. O separador decimal, o separador de milhares, o símbolo de moeda e a ordem do dia e do mês em uma data vêm dessas configurações. Um número digitado em uma célula sem formato próprio recebe o formato que o mostra do jeito que você digitou.

| Você digita | A célula contém | E mostra |
| --- | --- | --- |
| 1234,5 | o número 1234,5 | 1234,5 |
| 5% | o número 0,05 | 5% |
| R$ 12,50 | o número 12,5 | R$ 12,50 |
| 1 1/2 | o número 1,5 | 1 1/2 |
| 4/3/2025 | a data, como número de dias | 04/03/2025 |
| 22:30 | a hora, como parte de um dia | 22:30 |
| VERDADEIRO | o valor lógico verdadeiro | VERDADEIRO |
| #N/D | o erro #N/D | #N/D |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tem um formato próprio, como porcentagem ou data, mantém esse formato. Uma célula cujo formato é **Texto** mantém tudo o que você digitar como texto, exceto uma fórmula.

Para manter como texto algo que parece número, como o código **007**, digite um apóstrofo antes: **'007**. O apóstrofo não faz parte do texto. Ao editar a célula, ele volta a aparecer.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** dobra o número em B2. Digite os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch as escreve em maiúsculas.

Os nomes das funções são sempre os do inglês, em todos os idiomas: digite **SUM**, e não o nome que seu idioma lhe dá. Os sinais entre eles seguem as configurações do seu computador. Onde o separador decimal é a vírgula, como no alemão e no francês, você separa os argumentos de uma função com ponto e vírgula e escreve vírgula em um número: **=SUM(A1;A2)*0,5**. A barra de fórmulas e o **F2** mostram as fórmulas do mesmo modo. Onde o separador decimal é o ponto, você separa os argumentos com vírgula: **=SUM(A1,A2)*0.5**. O arquivo sempre guarda a forma que todos os programas leem, com vírgulas entre argumentos e pontos nos números, para que abra igual em qualquer lugar.

Se uma fórmula tiver um erro, como um parêntese que nunca é fechado, o JeffCrunch informa o problema, e a entrada permanece como você digitou, com o cursor perto do erro, para que você possa corrigi-la. Nada é alterado até que a fórmula esteja correta.

Uma célula contém até 32.767 caracteres, e uma fórmula, até 8.192. O JeffCrunch avisa quando uma entrada é maior que isso.

## Limpar células

Selecione as células e pressione **Delete**. Isso limpa o que as células contêm e mantém a formatação delas, e é uma etapa que você pode desfazer. O JeffCrunch limpa até 100.000 células de uma vez.

Na guia **Página Inicial**, o botão **Limpar** tem um menu. **Limpar Tudo** esvazia as células, a formatação delas, as anotações, os comentários (as discussões com respostas) e os links. Uma discussão com mais de 500 comentários é mantida, e o JeffCrunch avisa, porque um Desfazer não poderia trazer de volta os comentários que nunca leu. **Limpar Formatos** mantém o que as células contêm e restaura o formato simples. **Limpar Conteúdo** é o mesmo que **Delete**. **Limpar Comentários e Anotações** retira somente as anotações e os comentários das células selecionadas. **Limpar Hiperlinks** retira somente os links, e as células mantêm o texto. Uma opção aparece desativada quando as células selecionadas não têm nada que ela possa limpar.

## Desfazer e Refazer

**{key:Undo}** desfaz a última alteração, e **{key:Redo}** a refaz. Esses comandos também estão na barra de ferramentas de acesso rápido, no topo. Cada célula em que você digita é uma etapa, e cada limpeza também é. Desfazer devolve a célula ao estado anterior: seu valor, sua fórmula, seu formato e as fórmulas que dependem dela. O JeffCrunch guarda até 1.000 etapas, menos quando elas alteraram um número muito grande de células, para nunca usar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando você altera uma célula, o JeffCrunch recalcula em segundo plano as fórmulas que dependem dela. A célula alterada mostra o novo conteúdo de imediato, e as demais células são atualizadas um instante depois; a barra de status mostra **Calculando** até que terminem. Enquanto isso, você pode continuar digitando.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula de matriz** ou de uma tabela de dados não pode ser alterada sozinha. Para criar uma fórmula de matriz, selecione as células que ela deve preencher, digite a fórmula e pressione **Ctrl+Shift+Enter**. Uma única fórmula preenche então todas as células, e a barra de fórmulas a mostra entre chaves, como **{=A1:A3*2}**. Para alterá-la, selecione qualquer célula da matriz, pressione **F2**, altere a fórmula e pressione **Ctrl+Shift+Enter** de novo. Para excluí-la, selecione todas as suas células e pressione **Delete**. As duas ações valem para a matriz inteira, em uma única etapa do Desfazer. O F2 em qualquer célula de uma matriz abre a fórmula da matriz, e uma planilha protegida ou um intervalo com senha recusa ou pede senha, como faz para qualquer célula. Copiar uma matriz, ou parte dela, e preencher a partir de uma de suas células, dão os valores que ela mostra, e não uma fórmula.
- Em uma **planilha protegida**, uma célula bloqueada não pode ser alterada. Todas as células são bloqueadas, a menos que a pasta de trabalho indique o contrário. O JeffCrunch avisa quando você tenta. Para alterar a célula, escolha Desproteger Planilha na guia Revisão.
- A primeira linha de uma **tabela** contém os nomes das suas colunas. Digite um novo nome em uma dessas células para renomear a coluna: as fórmulas que usam o nome antigo acompanham a mudança, e a tabela no arquivo continua válida. Um nome que outra coluna já tem recebe um número depois dele, um nome não pode ficar vazio, e **Delete** não limpa essa linha.
- Uma pasta de trabalho que não pôde ser lida por inteiro pode ser consultada, mas não alterada nem salva.
- Uma função que o JeffCrunch ainda não tem deixa uma nova fórmula sem valor na célula; a pasta de trabalho pede ao programa que a abre em seguida que calcule a fórmula.
- Formatação, classificação e localizar e substituir virão em uma versão posterior. Inserir e excluir, recortar e colar, preencher e os tamanhos de linhas e colunas estão em [Inserir e excluir linhas, colunas e células](insert-delete), [Recortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).

## Inserir um símbolo

Clique em **Inserir > Símbolos > Símbolo** para abrir a caixa Símbolo. Escolha um caractere e clique em **Inserir**, ou clique duas vezes nele. Ele é inserido no cursor da entrada que você está digitando. Se você não estiver digitando, a própria entrada da célula se abre com o símbolo acrescentado ao final, e **Enter** o coloca na célula. Exemplo: com uma célula contendo a palavra *Total*, escolher o sinal de euro torna a entrada *Total€*. Os símbolos usados por último aparecem primeiro na lista.

**Pegue o caractere de uma fonte.** Escolha uma fonte na caixa **Fonte** e depois o caractere. O caractere é inserido em um trecho próprio, escrito nessa fonte, e o restante do texto da célula mantém a fonte da célula. Exemplo: em uma célula com *Total*, um sinal de visto escolhido de uma fonte de símbolos resulta em *Total✓*, com somente o sinal de visto nessa fonte. Com **(texto normal)** escolhido, a fonte da célula desenha o símbolo, de modo que um símbolo que a fonte não possui aparece como um quadrado. Em uma fórmula, somente o caractere é inserido.

**Caracteres especiais.** A guia **Caracteres Especiais** lista o espaço e o hífen não separáveis, o hífen opcional, o travessão curto e o longo, os espaços curto e longo, as aspas e a quebra opcional sem largura, cada um com seu nome. Clique duas vezes em um deles para inseri-lo.

**Dê uma tecla de atalho a um símbolo.** Escolha um símbolo ou um caractere especial, clique em **Tecla de Atalho**, pressione as teclas desejadas, como **Ctrl+Alt+Shift+J**, e clique em **OK**. As teclas passam então a inserir esse símbolo, do mesmo modo que a caixa. Deixe as teclas vazias e clique em **OK** para removê-las. O JeffCrunch avisa quando outro comando já usa essas teclas. O símbolo passa a ser um comando, e por isso também aparece em **Arquivo > Opções > Personalizar > Teclado**, onde você pode alterar suas teclas (veja [Atalhos de teclado](keyboard)).

## Novas pastas de trabalho

Em **Arquivo > Opções > Novas Pastas de Trabalho**, escolha a fonte, o tamanho da fonte e o número de planilhas com que toda nova pasta de trabalho começa. As pastas de trabalho que você já tem não são alteradas.
