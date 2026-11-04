# Digitar em células
<!-- keywords: digitar, digitação, entrar, editar, alterar, célula, entrada, barra de fórmulas, F2, clique duplo, Tab, Esc, desistir, desfazer, refazer, excluir, limpar, porcentagem, moeda, dinheiro, data, hora, número, texto, apóstrofo, fórmula, igual, substituir, limite, caracteres -->

Para colocar algo em uma célula, selecione a célula e comece a digitar. O que você digitar substitui o que a célula continha. Pressione **Enter** para confirmar, ou **Esc** para desistir e deixar a célula como estava.

## Três formas de começar

- **Digitar sobre a célula.** Selecione uma célula e digite. O primeiro caractere que você digitar inicia a entrada, e a célula mostra o que você digita à medida que escreve.
- **Editar o conteúdo da célula.** Pressione **F2** ou clique duas vezes na célula. A célula abre com o seu conteúdo, e o cursor fica no final. Use as teclas de seta para se mover no texto.
- **Usar a barra de fórmulas.** Clique na barra de fórmulas, que fica ao lado da Caixa de nome, e digite ali. A célula também mostra o que você digita.

Quando você digita sobre uma célula, uma tecla de seta finaliza a entrada e move a seleção. Quando você edita o conteúdo da célula, as teclas de seta movem o cursor no texto. Pressione **F2** para alternar entre um e outro.

## Finalizar a entrada

| Teclas | O que fazem |
| --- | --- |
| Enter | Confirma a entrada e desce (ou para onde indicar **Arquivo > Opções > Edição**) |
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

**Ctrl+Enter** com várias células selecionadas coloca o que você digitou em todas elas. Uma fórmula é escrita para a célula em que você digitou, e nas outras ela se comporta como uma cópia dela: digitar **=A1** com o bloco E6:F7 selecionado resulta em **=B2** na célula F7. Isso conta como uma etapa do Desfazer.

**Alt+Seta para baixo** em uma célula lista, na ordem e uma única vez cada, as palavras das células acima e abaixo dela que não estejam separadas por uma célula vazia. Escolher uma delas digita essa palavra. Uma célula que tenha uma lista própria mostra essa lista.

Em **Arquivo > Opções > Edição**, escolha se **Enter** desce, vai para a direita, sobe, vai para a esquerda ou não se move.

Se você digitar em várias células de uma linha com **Tab** e depois pressionar **Enter**, a seleção desce para a próxima linha, na coluna em que você começou. Se você selecionar primeiro um bloco de células, **Enter** e **Tab** percorrem o bloco.

## O que você digita vira

O JeffCrunch interpreta o que você digita de acordo com as configurações do seu computador. O separador decimal, o separador de milhares, o símbolo de moeda e a ordem do dia e do mês em uma data vêm dessas configurações. Um número digitado em uma célula que não tem formato próprio recebe o formato que o mostra do jeito que você digitou.

| Você digita | A célula contém | E mostra |
| --- | --- | --- |
| 1234,5 | o número 1234,5 | 1234,5 |
| 5% | o número 0,05 | 5% |
| R$ 12,50 | o número 12,5 | R$ 12,50 |
| 1 1/2 | o número 1,5 | 1 1/2 |
| 3/4/2025 | a data, como um número de dias | 3/4/2025 |
| 22:30 | a hora, como uma parte do dia | 22:30 |
| VERDADEIRO | o valor lógico verdadeiro | VERDADEIRO |
| #N/D | o erro #N/D | #N/D |
| =B2*2 | uma fórmula | o resultado da fórmula |
| qualquer outra coisa | texto | o texto |

Uma célula que já tenha formato próprio, como porcentagem ou data, mantém esse formato. Uma célula cujo formato é **Texto** mantém tudo o que você digitar como texto, exceto uma fórmula.

Para manter como texto algo que parece um número, como um código **007**, digite primeiro um apóstrofo: **'007**. O apóstrofo não faz parte do texto. Ao editar a célula, você o verá novamente.

## Fórmulas

Uma fórmula começa com um sinal de igual. Por exemplo, **=B2*2** dobra o número em B2. Digite os nomes das funções em maiúsculas ou minúsculas; o JeffCrunch as escreve em maiúsculas.

Os nomes das funções são sempre os nomes em inglês, em todos os idiomas: digite **SUM**, e não o nome que o seu idioma dá a ela. Os sinais entre eles seguem as configurações do seu computador. Quando o separador decimal é a vírgula, como em alemão e francês, você separa os argumentos de uma função com ponto e vírgula e escreve vírgula em um número: **=SOMA(A1;A2)*0,5**. A barra de fórmulas e a tecla **F2** mostram as fórmulas da mesma maneira. Quando o separador decimal é o ponto, você separa os argumentos com vírgula: **=SOMA(A1,A2)*0.5**. O arquivo sempre guarda a forma que todos os programas leem, com vírgulas entre argumentos e pontos nos números, para que ele abra igual em qualquer lugar.

Se uma fórmula tiver algum erro, como um parêntese que nunca é fechado, o JeffCrunch informa o problema, e a entrada permanece como você digitou, com o cursor perto do erro, para que você possa corrigi-la. Nada é alterado até que a fórmula esteja correta.

Uma célula armazena até 32.767 caracteres, e uma fórmula, até 8.192. O JeffCrunch avisa quando uma entrada é maior que isso.

## Limpar células

Selecione as células e pressione **Delete**. Isso apaga o que as células contêm e mantém os formatos delas, e é uma etapa que você pode desfazer. O JeffCrunch limpa até 100.000 células de uma vez.

Na guia **Página Inicial**, o botão **Limpar** tem um menu. **Limpar Tudo** esvazia as células, seus formatos, suas notas e seus links. **Limpar Formatos** mantém o que as células contêm e restaura o formato simples. **Limpar Conteúdo** é o mesmo que **Delete**. **Limpar Comentários e Anotações** remove apenas as notas das células selecionadas. **Limpar Hiperlinks** remove apenas os links, e as células mantêm o texto. Uma opção fica esmaecida quando as células selecionadas não têm nada para ela limpar.

## Desfazer e Refazer

**{key:Undo}** desfaz a última alteração, e **{key:Redo}** a refaz. Os dois também estão na barra de ferramentas de acesso rápido, no topo. Cada célula em que você digita é uma etapa, e cada limpeza também. Desfazer devolve a célula ao estado anterior: o valor, a fórmula, o formato e as fórmulas que dependem dela. O JeffCrunch guarda até 1.000 etapas, menos quando elas alteraram um número muito grande de células, para nunca usar muita memória.

## Enquanto o JeffCrunch calcula as fórmulas

Quando você altera uma célula, o JeffCrunch recalcula, em segundo plano, as fórmulas que dependem dela. A célula alterada mostra o novo conteúdo imediatamente, e as outras células são atualizadas um pouco depois; a barra de status mostra **Calculando** até que terminem. Você pode continuar digitando enquanto isso.

## O que não pode ser alterado

- Uma célula que faz parte de uma **fórmula de matriz** ou de uma tabela de dados não pode ser alterada isoladamente.
- Em uma **planilha protegida**, uma célula bloqueada não pode ser alterada. Todas as células são bloqueadas, a menos que a pasta de trabalho indique o contrário. O JeffCrunch avisa quando você tenta. Para alterar a célula, escolha Desproteger Planilha na guia **Revisão**.
- A primeira linha de uma **tabela** contém os nomes das colunas. Digite um novo nome em uma dessas células para renomear a coluna: as fórmulas que usam o nome antigo são atualizadas, e a tabela no arquivo continua válida. Um nome que outra coluna já tem recebe um número no final, um nome não pode ficar vazio, e **Delete** não limpa essa linha.
- Uma pasta de trabalho que não pôde ser lida por completo pode ser consultada, mas não pode ser alterada nem salva.
- Uma função que o JeffCrunch ainda não tem deixa a nova fórmula sem valor na célula; a pasta de trabalho pede ao programa que a abrir para calcular a fórmula.
- Formatação, classificação e localizar e substituir virão em uma versão futura. Inserir e excluir, recortar e colar, preencher e o tamanho de linhas e colunas estão em [Inserir e excluir linhas, colunas e células](insert-delete), [Recortar, copiar e colar](clipboard), [Preencher células e somá-las](fill-sum) e [Alterar a largura das colunas e a altura das linhas](sizes).
