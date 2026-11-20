# Inserir e eliminar linhas, colunas e células
<!-- keywords: inserir células recortadas, inserir células copiadas, deslocar para a direita, deslocar para baixo, inserir, eliminar, remover, linha, coluna, célula, deslocar, mover, adicionar, em branco, Ctrl+mais, Ctrl+menos, clique com o botão direito, cabeçalho, anular, referência, #REF!, fórmula, nome, unida, unidas, tabela, matriz, margem, limite -->

Pode adicionar linhas, colunas ou células vazias no meio dos seus dados, e retirar linhas, colunas ou células. Tudo o que está abaixo ou ao lado desloca-se para abrir espaço ou para fechar a lacuna, e as fórmulas que apontam para as células que se deslocaram continuam a apontar para elas.

## Inserir

1. Selecione os números das linhas, as letras das colunas ou as células onde pretende as novas. Para adicionar três linhas, selecione três linhas.
2. Prima **Ctrl e a tecla de mais** (**Ctrl e a tecla de menos** elimina), ou use **Base > Inserir**, ou clique com o botão direito na seleção e escolha **Inserir**.

Com linhas ou colunas inteiras selecionadas, as novas entram de uma só vez, acima das linhas ou à esquerda das colunas. Com células selecionadas, abre-se uma pequena janela que pergunta como devem deslocar-se as células seguintes:

- **Deslocar células para a direita** ou **Deslocar células para baixo** move apenas as células das colunas (ou linhas) da sua seleção, e deixa o resto da folha onde está.
- **Linha inteira** ou **Coluna inteira** adiciona linhas ou colunas inteiras.

## Inserir células recortadas ou copiadas

Para colocar células que recortou ou copiou entre outras células, em vez de as colocar por cima, recorte-as ou copie-as primeiro (**Ctrl+X** ou **Ctrl+C**). Depois selecione a célula onde devem ficar e use **Base > Inserir > Inserir Células Recortadas ou Copiadas** (o comando mostra **Inserir Células Recortadas** ou **Inserir Células Copiadas** quando houver algo para colocar), ou clique com o botão direito na célula e escolha esse comando. Abre-se uma pequena janela que pergunta se as células que lá estão devem **deslocar-se para a direita** ou **deslocar-se para baixo** para abrir espaço.

O botão **Inserir** e **Ctrl e a tecla de mais** (também **Ctrl+Shift e a tecla de mais**) fazem o mesmo, mas apenas quando for claro o que pretende:

- quando as células estão **recortadas** e o recorte ainda está pendente (a moldura tracejada aparece) na folha de onde vieram, e
- quando as células estão **copiadas** e não alterou nada desde então (sem escrita, sem outra edição, sem Anular ou Refazer).

Em todos os outros casos, o botão e as teclas inserem células em branco, como sempre fizeram. Depois de alterar algo, a cópia continua disponível para **Colar** e para o comando do menu. **Esc** termina um corte ou uma cópia: a moldura desaparece, e o botão e as teclas voltam a inserir células em branco. Se recortar células numa folha e premir as teclas noutra folha, são inseridas células em branco; o comando do menu indica que as células recortadas só podem ser colocadas entre outras na folha de onde vieram. Com linhas ou colunas inteiras selecionadas, o comando fica indisponível, e Inserir adiciona linhas ou colunas como antes.

- As células recortadas **são movidas**: saem do sítio original, e o que aponta para elas acompanha-as. É o mesmo que manter **Shift** premido ao arrastar a moldura da seleção, e é tudo ou nada: se alguma parte for recusada, nada muda.
- As células copiadas **são inseridas** como cópia, tal como o **Colar** as colocaria (conforme indicado em **Definir Colagem Predefinida**, e com as fórmulas alteradas para o novo lugar), e a cópia continua pronta, para a poder inserir novamente noutro sítio.
- As células são inseridas uma só vez, na célula superior esquerda do que selecionou, mesmo que tenha selecionado mais células do que as que copiou.
- Se a lista estiver filtrada, as células entram nas linhas visíveis, como acontece numa colagem, e o espaço criado inclui as linhas ocultas pelo filtro que ficam pelo meio. Uma linha que ocultou manualmente conta como qualquer outra linha.
- Cada uma destas ações é um passo que pode anular. As células recortadas só podem ser colocadas entre outras na folha de onde vieram; para as mover para outra folha, use **Colar**.
- Uma célula não pode ser inserida dentro de si própria (células recortadas de A3:A4 não podem ir entre A3 e A4), e uma folha protegida recusa a operação. O JeffCrunch indica o motivo e não altera nada.

## Eliminar

Selecione o que pretende retirar e prima **Ctrl e a tecla de menos**, ou use **Base > Eliminar**. Com células selecionadas, a janela pergunta se pretende **deslocar células para a esquerda**, **deslocar células para cima**, ou eliminar a **linha inteira** ou a **coluna inteira**. Eliminar é um único passo, e **Anular** repõe tudo o que foi retirado, incluindo as fórmulas.

## O que acompanha

Uma fórmula que aponta para células que se deslocam passa a apontar para elas nas suas novas posições. Exemplo: em B1 tem **=SOMA(A1:A5)**. Se inserir uma linha acima da linha 3, a fórmula passa a ser **=SOMA(A1:A6)**, porque o intervalo cresceu para incluir a nova linha. Se eliminar a linha 3, passa a ser **=SOMA(A1:A4)**. Uma fórmula que apontava para uma célula eliminada mostra **#REF!**, o que significa que a célula de que precisa já não existe. **Anular** repõe a célula e a fórmula.

Também acompanham as células: as fórmulas de todas as folhas, os nomes (como áreas de impressão e títulos de impressão), as células unidas, as células a que se aplicam regras de validação e formatações condicionais, tabelas, gráficos, imagens, notas, hiperligações e quebras de página.

## O que não é possível fazer

- Não é possível inserir ou eliminar células numa parte de uma **fórmula matricial** ou de uma tabela de dados. Mova-as por inteiro, ou deixe-as como estão. Também não pode eliminar uma célula que uma tabela de dados usa como entrada.
- Uma **tabela dinâmica** ainda não pode ser movida nem cortada: não é possível inserir ou eliminar linhas, colunas ou células onde isso a moveria ou alteraria, nem recortar células dela ou para ela. O resto da folha pode ser alterado como de costume. Uma tabela dinâmica altera-se com os comandos do separador **Tabela Dinâmica** (atualizar, alterar a origem dos dados, campos e esquema), e não escrevendo nas suas células.
- Não é possível cortar uma **tabela** ao meio: não pode inserir uma coluna no interior dela, nem eliminar a linha de cabeçalho ou todas as suas linhas. Insira ou elimine linhas no interior dela, ou altere as células à sua volta.
- Numa **folha protegida** não é possível inserir ou eliminar linhas, colunas ou células. Para o fazer, escolha Desproteger Folha no separador Rever.
- As células no fim da folha que contenham algo não podem ser empurradas para fora da extremidade da folha. Elimine-as primeiro.
- Depois de guardar, **Anular** continua a recuar pelo que fez antes, incluindo as linhas e colunas que inseriu (na gravação seguinte voltam a ser retiradas). A única coisa que não pode ser reposta são as linhas, colunas ou células que **eliminou**: o ficheiro guardado perdeu o que estava nelas, por isso **Anular** para na última eliminação guardada, e a linha em baixo da janela indica-o quando guarda.
- Um passo que desloque ou elimine tantas células que o **Anular** não consiga guardar (mais de cerca de 128 MB) pede confirmação primeiro, como fazem os programas de folhas de cálculo conhecidos: "Isto não pode ser anulado." Se continuar, a operação é feita, e o **Anular** não recua por ela nem pelo que a antecedeu.
