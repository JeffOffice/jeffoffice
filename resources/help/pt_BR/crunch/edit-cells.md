# Digitar em células
<!-- keywords: digitar, digitação, inserir, entrar, editar, alterar, célula, entrada, barra de fórmulas, F2, clique duplo, Tab, Esc, cancelar, desfazer, refazer, excluir, apagar, limpar, porcentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, sinal de igual, substituir, limite, caracteres -->

Para colocar algo em uma célula, selecione a célula e comece a digitar. O que você digita substitui o que a célula continha. Pressione **Enter** para manter a entrada ou **Esc** para cancelá-la e deixar a célula como estava.

## Três maneiras de começar

- **Digitar sobre a célula.** Selecione uma célula e digite. O primeiro caractere digitado inicia a entrada, e a célula mostra o que você digita à medida que digita.
- **Editar o conteúdo da célula.** Pressione **F2** ou clique duas vezes na célula. A célula é aberta com o conteúdo, e o cursor fica no final. Use as teclas de direção para se mover no texto.
- **Usar a barra de fórmulas.** Clique na barra de fórmulas, que fica ao lado da Caixa de nome, e digite ali. A célula também mostra o que você digita.

Quando você digita sobre uma célula, uma tecla de direção conclui a entrada e move a seleção. Quando você edita o conteúdo da célula, as teclas de direção se movem no texto. Pressione **F2** para alternar entre um modo e outro.

## Concluir a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Mantém a entrada e move para baixo (ou conforme definido em **Arquivo > Opções > Edição**) |
| Shift+Enter | Mantém a entrada e move no sentido contrário |
| Ctrl+Enter | Coloca a entrada em todas as células selecionadas e permanece no mesmo lugar |
| Tab | Mantém a entrada e move para a direita |
| Shift+Tab | Mantém a entrada e move para a esquerda |
| Teclas de direção | Mantêm a entrada e movem uma célula (somente quando você digitou sobre a célula) |
| Esc | Cancela a entrada |
| Alt+Enter | Inicia uma nova linha dentro da célula |
| Ctrl+; | Insere a data de hoje (na entrada, se você estiver digitando uma) |
| Ctrl+Shift+; | Insere a hora atual |
| Alt+Seta para Baixo | Lista as palavras já existentes na coluna para você escolher uma |

**Ctrl+Enter** com várias células selecionadas coloca o que você digitou em todas elas. Uma fórmula é escrita para a célula em que você digitou e, em cada uma das outras, fica como ficaria uma cópia dela: digitar **=A1** com o bloco E6:F7 selecionado resulta em **=B2** em F7. Isso conta como uma única etapa de Desfazer.

**Alt+Seta para Baixo** em uma célula lista, em ordem e uma vez cada, as palavras das células acima e abaixo dela que não estão separadas por uma célula vazia. Escolher uma delas a digita. Uma célula com uma lista própria mostra essa lista.

Em **Arquivo > Opções > Edição**, escolha se **Enter** move para baixo, para a direita, para cima, para a esquerda ou não move.

Se você digitar em várias células ao longo de uma linha usando **Tab** e depois pressionar **Enter**, a seleção desce para a próxima linha, na coluna em que você começou. Se você selecionar um bloco de células antes, **Enter** e **Tab** se movem dentro do bloco.

## O que o que você digita se torna

O JeffCrunch lê o que você digita de acordo com as configurações do seu computador. O separador decimal, o separador de milhar, o símbolo de moeda e a ordem do dia e do mês em uma data vêm dessas configurações. Um número digitado em uma célula sem formato próprio recebe o formato que o mostra da maneira como você o digitou.

| Você digita | A célula contém | E mostra |
| --- | --- | --- |
| 1234.5 | o número 1234,5 | 1234.5 |
| 5% | o número 0,05 | 5% |
| $12.50 | o número 12,5 | $12.50 |
| 1 1/2 | o número 1,5 | 1 1/2 |
| 3/4/2025 | a data, como um número de dias | 3/4/2025 |
| 10:30 PM | a hora, como uma fração de um dia | 10:30 PM |
| TRUE | o valor lógico verdadeiro | TRUE |
| #N/A | o erro #N/A | #N/A |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tem formato próprio, como porcentagem ou data, o mantém. Uma célula com o formato **Texto** mantém tudo o que você digita como texto, exceto uma fórmula.

Para manter como texto algo que parece um número, como o código **007**, digite um apóstrofo antes: **'007**. O apóstrofo não faz parte do texto. Quando você edita a célula, ele aparece novamente.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** dobra o número em B2. Digite os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch os escreve em maiúsculas.

Os nomes das funções são sempre os em inglês, em qualquer idioma: digite **SUM**, e não o nome que seu idioma dá a ela. Os sinais entre eles seguem as configurações do seu computador. Onde o separador decimal é a vírgula, como em alemão e francês, você separa os argumentos de uma função com ponto e vírgula e escreve uma vírgula nos números: **=SUM(A1;A2)*0,5**. A barra de fórmulas e **F2** mostram as fórmulas da mesma forma. Onde o separador decimal é o ponto, você separa os argumentos com vírgula: **=SUM(A1,A2)*0.5**. O arquivo sempre guarda a forma que todos os programas leem, com vírgulas entre os argumentos e pontos nos números, para que abra da mesma forma em qualquer lugar.

Se uma fórmula tiver um erro, como um parêntese que nunca é fechado, o JeffCrunch informa o que está errado, e a entrada permanece como você a digitou, com o cursor perto do erro, para que você possa corrigi-lo. Nada é alterado até que a fórmula esteja correta.

Uma célula comporta até 32.767 caracteres, e uma fórmula até 8.192. O JeffCrunch avisa quando uma entrada é mais longa.

## Limpar células

Selecione as células e pressione **Delete**. Isso limpa o conteúdo das células e mantém a formatação, e é uma etapa que pode ser desfeita. O JeffCrunch limpa até 100.000 células de uma vez.

## Desfazer e Refazer

**{key:Undo}** desfaz a última alteração, e **{key:Redo}** a refaz. Esses comandos também estão na Barra de Ferramentas de Acesso Rápido, na parte superior. Cada célula em que você digita é uma etapa, assim como cada limpeza. Desfazer deixa a célula como estava: o valor, a fórmula, o formato e as fórmulas que dependem dela. O JeffCrunch lembra até 1.000 etapas, menos quando elas alteraram muitas células, para nunca ocupar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando você altera uma célula, o JeffCrunch calcula em segundo plano as fórmulas que dependem dela. A célula alterada mostra o novo conteúdo imediatamente, e as outras células são atualizadas logo depois; a barra de status mostra **Calculando** até que terminem. Enquanto isso, você pode continuar digitando.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula de matriz** ou de uma tabela de dados não pode ser alterada isoladamente.
- Em uma **planilha protegida**, uma célula bloqueada não pode ser alterada. Todas as células são bloqueadas, a menos que a pasta de trabalho indique o contrário. O JeffCrunch avisa quando você tenta. Para alterar a célula, escolha Desproteger Planilha na guia Revisão.
- A primeira linha de uma **tabela** contém os nomes das colunas. Digite um novo nome em uma dessas células para renomear a coluna: as fórmulas que usam o nome antigo acompanham a mudança, e a tabela no arquivo continua válida. Um nome que outra coluna já tem recebe um número depois dele, um nome não pode ficar vazio, e **Delete** não limpa essa linha.
- Uma pasta de trabalho que não pôde ser lida por inteiro pode ser visualizada, mas não alterada nem salva.
- Uma função que o JeffCrunch ainda não tem deixa uma nova fórmula sem valor na célula; a pasta de trabalho pede ao próximo programa que a abrir que calcule a fórmula.
- Formatação, classificação e localizar e substituir virão em uma versão futura. Inserção e exclusão, recortar e colar, preenchimento e os tamanhos de linhas e colunas estão em [Inserir e excluir linhas, colunas e células](insert-delete), [Recortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).
