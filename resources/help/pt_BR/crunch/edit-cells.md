# Digitar em células

<!-- keywords: correção automática, aspas inteligentes, travessão, fração, link, endereço, palavra em negrito, parte de uma célula, texto formatado, formatar parte do texto, colorir uma palavra, digitar, digitação, entrar, editar, alterar, célula, entrada, barra de fórmulas, F2, clique duplo, Tab, Esc, desistir, desfazer, refazer, excluir, limpar, porcentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, igual, substituir, limite, caracteres -->

Para colocar algo em uma célula, selecione a célula e comece a digitar. O que você digitar substitui o que a célula continha. Pressione **Enter** para confirmar, ou **Esc** para desistir e deixar a célula como estava.

## Três formas de começar

- **Digitar sobre a célula.** Selecione uma célula e digite. O primeiro caractere que você digitar inicia a entrada, e a célula mostra o que você digita enquanto escreve.
- **Editar o conteúdo da célula.** Pressione **F2** ou clique duas vezes na célula. A célula abre com o seu conteúdo, e o cursor fica no final. Use as setas do teclado para se mover no texto.
- **Usar a barra de fórmulas.** Clique na barra de fórmulas, que fica ao lado da Caixa de Nome, e digite ali. A célula também mostra o que você digita.

Ao digitar sobre uma célula, uma tecla de seta conclui a entrada e move a seleção. Ao editar o conteúdo da célula, as setas movem o cursor no texto. Pressione **F2** para alternar de um modo para o outro.

## Concluir a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Confirma a entrada e move para baixo (ou para onde **Arquivo > Opções > Edição** indicar) |
| Shift+Enter | Confirma a entrada e move no sentido contrário |
| Ctrl+Enter | Coloca a entrada em todas as células selecionadas e permanece onde está |
| Tab | Confirma a entrada e move para a direita |
| Shift+Tab | Confirma a entrada e move para a esquerda |
| Setas | Confirma a entrada e move uma célula (somente quando você digitou sobre a célula) |
| Esc | Desiste da entrada |
| Alt+Enter | Inicia uma nova linha dentro da célula |
| Ctrl+; | Insere a data de hoje (na entrada, quando você estiver digitando uma) |
| Ctrl+Shift+; | Insere a hora atual |
| Alt+Seta para baixo | Lista as palavras já existentes na coluna e permite escolher uma |

**Ctrl+Enter** com várias células selecionadas coloca o que você digitou em todas elas. Uma fórmula é escrita para a célula em que você digitou, e nas outras ela se comporta como uma cópia dela se comportaria: digitar **=A1** com o intervalo E6:F7 selecionado resulta em **=B2** em F7. É uma etapa de Desfazer.

**Alt+Seta para baixo** em uma célula lista, em ordem e uma vez cada, as palavras das células acima e abaixo dela que não estão separadas por uma célula vazia. Escolher uma palavra a digita. Uma célula que tenha uma lista própria mostra essa lista.

Em **Arquivo > Opções > Edição**, escolha se **Enter** move para baixo, para a direita, para cima ou para a esquerda, ou se não move.

Se você digitar em várias células de uma linha com **Tab** e depois pressionar **Enter**, a seleção desce para a linha seguinte, na coluna em que você começou. Se você selecionar um intervalo de células antes, **Enter** e **Tab** percorrem o intervalo.

## Correção automática enquanto você digita

Quando você digita um espaço ou um sinal, como um ponto, depois de uma palavra, o JeffCrunch pode corrigi-la assim que você termina de escrevê-la. Ele corrige erros de digitação comuns (como "teh" no lugar de "the"), duas letras maiúsculas no início de uma palavra, uma letra minúscula que inicia uma frase ou um dia da semana, e aspas retas (que viram aspas curvas). Também troca dois hifens entre palavras por um travessão, e 1/2, 1/4 e 3/4 por ½, ¼ e ¾ quando a entrada tem palavras. Um número como 1/2 digitado sozinho continua sendo uma data ou uma fração, como sempre.

Cada correção é uma etapa separada. Pressione **{key:Undo}** logo após uma correção para desfazer apenas aquela correção e manter o que você digitou. Nada é alterado em uma fórmula: uma entrada que começa com sinal de igual, ou com sinal de mais ou de menos seguido de uma fórmula, permanece como você digitou. Um endereço na web (como https://example.org) ou um endereço de email que seja a entrada inteira se torna um link na célula ao confirmar a entrada, se **Converter endereços da web em links ao digitar** estiver ativado.

Ative ou desative essas opções em **Arquivo > Opções > Revisão de Texto**. Uma palavra com erro de ortografia é marcada enquanto você digita: veja [Ortografia](notes-links).

## Formatar parte do texto

Algumas palavras em uma célula podem aparecer diferentes do restante. Por exemplo, uma palavra pode estar em negrito ou em vermelho.

1. Pressione F2 para editar a célula (ou clique duas vezes nela).
2. Selecione as palavras que deseja alterar, com Shift e as setas do teclado ou com o mouse.
3. Use **Negrito**, **Itálico**, **Sublinhado**, **Tachado**, **Fonte**, **Tamanho da Fonte** ou **Cor da Fonte** na guia **Página Inicial**, ou Ctrl+N, Ctrl+I e Ctrl+S.
4. Pressione Enter.

Somente as palavras selecionadas mudam. O restante da célula mantém a aparência própria da célula. A caixa de entrada mostra as palavras com uma aparência única enquanto você edita, e a célula mostra cada parte com a sua própria aparência quando você pressiona Enter. Se você digitar ou apagar palavras depois, a formatação acompanha as palavras. Escolher uma fonte ou um tamanho sem palavras selecionadas altera a célula inteira, como sempre.

Células que têm formatação dentro do texto vinda de outro programa a exibem, mantêm e salvam de volta. Uma célula que você altera mantém a sua formatação. A formatação dentro de um texto se aplica ao texto, não a números nem a fórmulas. Uma cópia salva como arquivo .xls antigo ou como .ods mantém as palavras, e ao salvar o programa informa que a formatação dentro delas não é mantida.

## O que você digita se torna

O JeffCrunch interpreta o que você digita de acordo com as configurações do seu computador. O separador decimal, o separador de milhares, o símbolo monetário e a ordem do dia e do mês em uma data vêm dessas configurações. Um número digitado em uma célula que não tem formato próprio recebe o formato que o mostra da maneira como você o digitou.

| Você digita | A célula contém | E mostra |
| --- | --- | --- |
| 1234,5 | o número 1234,5 | 1234,5 |
| 5% | o número 0,05 | 5% |
| R$ 12,50 | o número 12,5 | R$ 12,50 |
| 1 1/2 | o número 1,5 | 1 1/2 |
| 4/3/2025 | a data, como um número de dias | 4/3/2025 |
| 22:30 | a hora, como uma parte de um dia | 22:30 |
| VERDADEIRO | o valor lógico verdadeiro | VERDADEIRO |
| #N/D | o erro #N/D | #N/D |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tem um formato próprio, como porcentagem ou data, o mantém. Uma célula cujo formato é **Texto** mantém tudo o que você digitar como texto, exceto uma fórmula.

Para manter como texto algo que parece um número, como um código **007**, digite primeiro um apóstrofo: **'007**. O apóstrofo não faz parte do texto. Ao editar a célula, você o vê novamente.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** dobra o número em B2. Digite os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch as escreve em maiúsculas.

Os nomes das funções são sempre os em inglês, em todos os idiomas: digite **SUM**, e não o nome que o seu idioma dá a ela. Os sinais entre eles seguem as configurações do seu computador. Onde o separador decimal é a vírgula, como em alemão e francês, você separa os argumentos de uma função com ponto e vírgula e escreve a vírgula em um número: **=SUM(A1;A2)*0,5**. A barra de fórmulas e a **F2** mostram as fórmulas da mesma forma. Onde o separador decimal é o ponto, você separa os argumentos com vírgula: **=SUM(A1,A2)*0.5**. O arquivo sempre guarda a forma que todos os programas leem, com vírgulas entre os argumentos e pontos nos números, para que abra igual em qualquer lugar.

Se uma fórmula tiver um erro, como um parêntese que nunca é fechado, o JeffCrunch informa o problema, e a entrada permanece como você digitou, com o cursor próximo ao erro, para que você possa corrigi-lo. Nada é alterado até que a fórmula esteja correta.

Uma célula contém até 32.767 caracteres, e uma fórmula, até 8.192. O JeffCrunch avisa quando uma entrada é maior que isso.

## Limpar células

Selecione as células e pressione **Delete**. Isso limpa o que as células contêm e mantém os seus formatos, e é uma etapa que você pode desfazer. O JeffCrunch limpa até 100.000 células de uma vez.

Na guia **Página Inicial**, o botão **Limpar** tem um menu. **Limpar Tudo** esvazia as células, seus formatos, suas anotações, seus comentários (as discussões com respostas) e seus links. Uma discussão com mais de 500 comentários é mantida, e o JeffCrunch informa isso, porque um Desfazer não conseguiria trazer de volta os comentários que ele não leu. **Limpar Formatos** mantém o que as células contêm e restaura o formato simples. **Limpar Conteúdo** é o mesmo que **Delete**. **Limpar Comentários e Anotações** remove somente as anotações e os comentários das células selecionadas. **Limpar Hiperlinks** remove somente os links, e as células mantêm o seu texto. Uma opção fica desativada quando as células selecionadas não têm nada para ela limpar.

## Desfazer e Refazer

**{key:Undo}** desfaz a última alteração, e **{key:Redo}** a refaz. Eles também estão na barra de ferramentas de acesso rápido, no alto da janela. Cada célula em que você digita é uma etapa, e cada limpeza também. Desfazer devolve a célula ao estado anterior: seu valor, sua fórmula, seu formato e as fórmulas que dependem dela. O JeffCrunch guarda até 1.000 etapas, menos quando elas alteraram um número muito grande de células, para nunca usar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando você altera uma célula, o JeffCrunch calcula em segundo plano as fórmulas que dependem dela. A célula que você alterou mostra o novo conteúdo imediatamente, e as outras células são atualizadas um instante depois; a barra de status mostra **Calculando** até que terminem. Você pode continuar digitando enquanto isso.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula de matriz** ou de uma tabela de dados não pode ser alterada isoladamente.
- Em uma **planilha protegida**, uma célula bloqueada não pode ser alterada. Todas as células são bloqueadas, a menos que a pasta de trabalho indique o contrário. O JeffCrunch informa quando você tenta. Para alterar a célula, escolha Desproteger Planilha na guia Revisão.
- A primeira linha de uma **tabela** contém os nomes das suas colunas. Digite um novo nome em uma dessas células para renomear a coluna: as fórmulas que usam o nome antigo são atualizadas, e a tabela no arquivo continua válida. Um nome que outra coluna já tem recebe um número no final, um nome não pode ficar vazio, e **Delete** não limpa essa linha.
- Uma pasta de trabalho que não pôde ser lida por completo pode ser vista, mas não alterada nem salva.
- Uma função que o JeffCrunch ainda não possui deixa uma nova fórmula sem valor na célula; a pasta de trabalho pede ao programa que a abre em seguida que calcule a fórmula.
- Formatação, classificação e localizar e substituir virão em uma versão futura. Inserir e excluir, recortar e colar, preencher e o tamanho de linhas e colunas estão em [Inserir e excluir linhas, colunas e células](insert-delete), [Recortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).

## Novas pastas de trabalho

Em **Arquivo > Opções > Novas Pastas de Trabalho**, escolha a fonte, o tamanho da fonte e o número de planilhas com que toda nova pasta de trabalho começa. As pastas de trabalho que você já tem não são alteradas.
