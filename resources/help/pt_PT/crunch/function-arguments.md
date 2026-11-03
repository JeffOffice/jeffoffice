# Introduzir uma função com a caixa de diálogo Argumentos de Função
<!-- keywords: argumentos de função, argumento, fx, resultado da fórmula, Shift+F3, selecionar células, intervalo, opcional, repetir, editar uma função, função aninhada, erro -->

A caixa de diálogo **Argumentos de Função** ajuda-o a preencher uma função um argumento de cada vez e mostra o resultado da fórmula antes de a colocar na célula.

## Abrir a caixa de diálogo

- Escolha **Escolher uma Função** no separador Fórmulas (**Shift+F3**) ou clique em **fx** junto à barra de fórmulas e, em seguida, escolha uma função. A caixa de diálogo abre-se para essa função.
- Para alterar uma função que já está numa fórmula, coloque o cursor dentro dos respetivos parênteses (na célula ou na barra de fórmulas) e prima **Shift+F3** ou clique em **fx**. A caixa de diálogo abre-se para essa função, com os respetivos argumentos nas caixas. Numa fórmula como =SOMA(A1; SE(B1>2; 5; 6)), com o cursor dentro do SE altera o SE e, com o cursor dentro do SOMA mas fora do SE, altera o SOMA.

## Preencher os argumentos

Cada argumento tem uma linha: o nome, uma caixa onde o escrever e, à direita, o valor que tem no momento, como 12,5, "texto", VERDADEIRO ou {1;2;3} para um bloco de células. Um nome a negrito tem de ser preenchido; um nome que não está a negrito é opcional e pode ficar vazio.

- **Tab** passa para a caixa seguinte e **Shift+Tab** para a anterior.
- A linha por baixo das caixas indica o que a função faz e a linha seguinte indica o que é o argumento em que se encontra.
- Funções como SOMA aceitam tantos argumentos quantos precisar: quando escreve na última caixa, aparece uma nova.
- O botão à direita de uma caixa permite apontar para células. A caixa de diálogo dá lugar a uma pequena faixa e pode selecionar células na folha com o rato (a caixa mostra o endereço das células à medida que arrasta). Clique no botão da faixa ou prima **Enter** para voltar.

## O resultado

**Resultado da fórmula =** mostra o resultado da função completa, pouco depois de parar de escrever. Nada é escrito na folha até premir **OK**. Se algo estiver errado, uma linha por baixo do resultado explica o problema por palavras: por exemplo, que um valor não foi encontrado, que algo é dividido por zero ou que um número não é permitido. Uma fórmula que demoraria demasiado tempo a calcular aqui não é mostrada; prima **OK** e a folha calcula-a.

**Ajuda sobre esta função** abre esta página.

## Concluir

Prima **Enter** ou clique em **OK** para inserir a função. Numa célula sem nenhuma introdução em curso, a fórmula é escrita na célula. Numa fórmula que esteja a escrever, a função é inserida na posição do cursor, ou substitui a que alterou, e pode continuar a escrever. Prima **Esc** ou clique em **Cancelar** e nada é alterado.
