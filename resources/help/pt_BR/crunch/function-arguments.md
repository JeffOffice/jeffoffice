# Inserir uma função com a caixa de diálogo Argumentos da função
<!-- keywords: argumentos da função, argumento, fx, resultado da fórmula, Shift+F3, selecionar células, intervalo, opcional, repetir, editar uma função, função aninhada, erro -->

A caixa de diálogo **Argumentos da função** ajuda você a preencher uma função um argumento de cada vez e mostra o resultado da fórmula antes de você inseri-la na célula.

## Abrir a caixa de diálogo

- Escolha **Escolher uma Função** na guia Fórmulas (**Shift+F3**) ou clique em **fx** ao lado da barra de fórmulas e escolha uma função. A caixa de diálogo é aberta para essa função.
- Para alterar uma função que já está em uma fórmula, coloque o cursor dentro dos parênteses dela (na célula ou na barra de fórmulas) e pressione **Shift+F3** ou clique em **fx**. A caixa de diálogo é aberta para essa função, com os argumentos dela nas caixas. Em uma fórmula como =SOMA(A1; SE(B1>2; 5; 6)), um cursor dentro do SE altera o SE, e um cursor dentro do SOMA, mas fora do SE, altera o SOMA.

## Preencher os argumentos

Cada argumento tem uma linha: o nome dele, uma caixa para digitá-lo e, à direita, o valor atual dele, como 12,5, "texto", VERDADEIRO ou {1;2;3} para um intervalo de células. Um nome em negrito precisa ser preenchido; um nome que não está em negrito é opcional e pode ficar vazio.

- **Tab** passa para a próxima caixa, e **Shift+Tab**, para a anterior.
- A linha abaixo das caixas informa o que a função faz, e a linha seguinte informa o que é o argumento em que você está.
- Funções como SOMA aceitam quantos argumentos forem necessários: quando você digita na última caixa, uma nova caixa aparece.
- O botão à direita de uma caixa permite apontar para células. A caixa de diálogo é substituída por uma pequena faixa, e você seleciona as células na planilha com o mouse (a caixa mostra o endereço delas enquanto você arrasta). Clique no botão da faixa ou pressione **Enter** para voltar.

## O resultado

**Resultado da fórmula =** mostra o resultado da função inteira, um instante depois que você para de digitar. Nada é gravado na planilha até você pressionar **OK**. Se houver algum problema, uma linha abaixo do resultado o descreve em palavras: por exemplo, que um valor não foi encontrado, que algo está sendo dividido por zero ou que um número não é permitido. Uma fórmula que demoraria demais para ser calculada aqui não é mostrada; pressione **OK** e a planilha a calcula.

**Ajuda sobre esta função** abre esta página.

## Concluir

Pressione **Enter** ou clique em **OK** para inserir a função. Em uma célula sem nenhuma entrada em andamento, a fórmula é gravada na célula. Em uma fórmula que você está digitando, a função é inserida no cursor ou substitui a que você alterou, e você pode continuar digitando. Pressione **Esc** ou clique em **Cancelar** e nada é alterado.
