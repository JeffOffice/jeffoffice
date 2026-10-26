# Digitar em células
<!-- keywords: digitar, digitação, inserir, entrada, editar, alterar, modificar, célula, conteúdo, barra de fórmulas, F2, clique duplo, Tab, Esc, cancelar, desfazer, refazer, excluir, apagar, limpar, porcentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, sinal de igual, substituir, limite, caracteres -->

Para inserir algo em uma célula, selecione a célula e comece a digitar. O que você digitar substitui o conteúdo da célula. Pressione **Enter** para manter a entrada ou **Esc** para cancelá-la e deixar a célula como estava.

## Três maneiras de começar

- **Digitar sobre a célula.** Selecione uma célula e digite. O primeiro caractere digitado inicia a entrada, e a célula mostra o que você digita à medida que digita.
- **Editar o conteúdo da célula.** Pressione **F2** ou clique duas vezes na célula. A célula abre com seu conteúdo e o cursor fica no final. Use as teclas de direção para se mover no texto.
- **Usar a barra de fórmulas.** Clique na barra de fórmulas, ao lado da Caixa de nome, e digite ali. A célula também mostra o que você digita.

Quando você digita sobre uma célula, uma tecla de direção conclui a entrada e move a seleção. Quando você edita o conteúdo da célula, as teclas de direção se movem no texto. Pressione **F2** para alternar entre um modo e outro.

## Concluir a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Mantém a entrada e move para baixo |
| Shift+Enter | Mantém a entrada e move para cima |
| Tab | Mantém a entrada e move para a direita |
| Shift+Tab | Mantém a entrada e move para a esquerda |
| Teclas de direção | Mantêm a entrada e movem uma célula (somente quando você digitou sobre a célula) |
| Esc | Cancela a entrada |
| Alt+Enter | Inicia uma nova linha dentro da célula |

Se você digitar em várias células ao longo de uma linha usando **Tab** e depois pressionar **Enter**, a seleção desce para a próxima linha, na coluna em que você começou. Se você selecionar um bloco de células primeiro, **Enter** e **Tab** percorrem o bloco.

## Como o que você digita é interpretado

O JeffCrunch interpreta o que você digita de acordo com as configurações do seu computador. O separador decimal, o separador de milhar, o símbolo de moeda e a ordem do dia e do mês em uma data vêm dessas configurações. Um número digitado em uma célula sem formato próprio recebe o formato que o mostra do jeito que você o digitou.

| Você digita | A célula contém | E mostra |
| --- | --- | --- |
| 1234,5 | o número 1234,5 | 1234,5 |
| 5% | o número 0,05 | 5% |
| R$ 12,50 | o número 12,5 | R$ 12,50 |
| 4/3/2025 | a data, como um número de dias | 4/3/2025 |
| 22:30 | a hora, como uma fração de um dia | 22:30 |
| VERDADEIRO | o valor lógico verdadeiro | VERDADEIRO |
| #N/D | o erro #N/D | #N/D |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tem um formato próprio, como porcentagem ou data, o mantém. Uma célula com o formato **Texto** mantém tudo o que você digita como texto, exceto uma fórmula.

Para manter como texto algo que parece um número, como o código **007**, digite um apóstrofo antes: **'007**. O apóstrofo não faz parte do texto. Ao editar a célula, você o vê novamente.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** dobra o número em B2. Digite os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch os escreve em maiúsculas.

Os nomes das funções são sempre os em inglês, em qualquer idioma: digite **SUM**, e não o nome que seu idioma dá a ela. Os separadores seguem as configurações do seu computador. Onde o separador decimal é a vírgula, como no Brasil, você separa os argumentos de uma função com ponto e vírgula e escreve vírgula nos números: **=SUM(A1;A2)*0,5**. A barra de fórmulas e **F2** mostram as fórmulas da mesma forma. Onde o separador decimal é o ponto, você separa os argumentos com vírgula: **=SUM(A1,A2)*0.5**. O arquivo sempre guarda a forma que todos os programas leem, com vírgulas entre os argumentos e pontos nos números, para que abra da mesma maneira em qualquer lugar.

Se uma fórmula tiver um erro, como um parêntese que nunca é fechado, o JeffCrunch informa o que está errado, e a entrada fica como você a digitou, com o cursor perto do erro, para que você possa corrigi-lo. Nada é alterado até que a fórmula esteja correta.

Uma célula comporta até 32.767 caracteres, e uma fórmula até 8.192. O JeffCrunch avisa quando uma entrada é mais longa.

## Limpar células

Selecione as células e pressione **Delete**. Isso limpa o conteúdo das células e mantém seus formatos, em uma única etapa que pode ser desfeita. O JeffCrunch limpa até 100.000 células de cada vez.

## Desfazer e Refazer

**{key:Undo}** desfaz a última alteração, e **{key:Redo}** a refaz. Esses comandos também estão na Barra de Ferramentas de Acesso Rápido, na parte superior. Cada célula em que você digita é uma etapa, assim como cada limpeza. Desfazer devolve a célula ao estado anterior: seu valor, sua fórmula, seu formato e as fórmulas que dependem dela. O JeffCrunch guarda até 1.000 etapas, menos quando elas alteraram muitas células, para nunca ocupar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando você altera uma célula, o JeffCrunch calcula em segundo plano as fórmulas que dependem dela. A célula alterada mostra o novo conteúdo imediatamente, e as outras células são atualizadas um momento depois; a barra de status mostra **Calculando** até que isso termine. Enquanto isso, você pode continuar digitando.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula de matriz** ou de uma tabela de dados não pode ser alterada isoladamente.
- Em uma **planilha protegida**, uma célula bloqueada não pode ser alterada. Todas as células são bloqueadas, a menos que a pasta de trabalho indique o contrário. O JeffCrunch avisa quando você tenta. Para alterar a célula, escolha Desproteger Planilha na guia Revisão.
- A primeira linha de uma **tabela** contém os nomes das colunas. Digite um novo nome em uma dessas células para renomear a coluna: as fórmulas que usam o nome antigo acompanham a mudança, e a tabela no arquivo continua válida. Um nome que outra coluna já tem recebe um número no final, um nome não pode ficar vazio e **Delete** não limpa essa linha.
- Uma pasta de trabalho que não pôde ser lida por inteiro pode ser visualizada, mas não alterada nem salva.
- Uma função que o JeffCrunch ainda não tem deixa uma nova fórmula sem valor na célula; a pasta de trabalho pede ao próximo programa que a abrir que calcule a fórmula.
- Formatação, classificação e localizar e substituir virão em uma versão futura. Inserir e excluir, recortar e colar, preencher e o tamanho de linhas e colunas são explicados em [Inserir e excluir linhas, colunas e células](insert-delete), [Recortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).
