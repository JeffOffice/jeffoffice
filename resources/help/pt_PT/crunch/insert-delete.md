# Inserir e eliminar linhas, colunas e células
<!-- keywords: inserir, eliminar, apagar, remover, linha, coluna, célula, deslocar, mover, adicionar, acrescentar, em branco, vazia, Ctrl+mais, Ctrl+menos, clique com o botão direito, cabeçalho, anular, referência, #REF!, fórmula, nome, unidas, tabela, matriz, limite -->

Pode adicionar linhas, colunas ou células vazias no meio dos dados, bem como retirar linhas, colunas ou células. Tudo o que está a seguir desloca-se para abrir espaço ou para fechar o intervalo, e as fórmulas que apontam para as células que se moveram continuam a apontar para elas.

## Inserir

1. Selecione os números de linha, as letras de coluna ou as células onde pretende as novas. Para adicionar três linhas, selecione três linhas.
2. Prima **Ctrl e a tecla mais** (**Ctrl e a tecla menos** elimina), utilize **Base > Inserir** ou clique com o botão direito do rato na seleção e escolha **Inserir**.

Com linhas ou colunas inteiras selecionadas, as novas são inseridas de imediato, acima das linhas ou à esquerda das colunas. Com células selecionadas, uma pequena janela pergunta como se devem deslocar as células que se seguem:

- **Deslocar células para a direita** ou **Deslocar células para baixo** move apenas as células nas colunas (ou linhas) da seleção e deixa o resto da folha onde está.
- **Linha inteira** ou **Coluna inteira** adiciona linhas ou colunas inteiras.

## Eliminar

Selecione o que pretende retirar e prima **Ctrl e a tecla menos** ou utilize **Base > Eliminar**. Com células selecionadas, a janela pergunta se pretende **deslocar células para a esquerda**, **deslocar células para cima** ou eliminar a **linha inteira** ou a **coluna inteira**. A eliminação é um único passo e **Anular** repõe tudo o que foi retirado, incluindo as fórmulas.

## O que acontece a seguir

Uma fórmula que aponta para células que se movem passa a apontar para elas nas novas posições. Exemplo: em B1 tem **=SUM(A1:A5)**. Insira uma linha acima da linha 3 e a fórmula passa a **=SUM(A1:A6)**, porque o intervalo aumentou para incluir a nova linha. Se eliminar a linha 3, passa a **=SUM(A1:A4)**. Uma fórmula que apontava para uma célula eliminada mostra **#REF!**, o que significa que a célula de que precisa já não existe. **Anular** repõe a célula e a fórmula.

Também acompanham as células: as fórmulas em todas as folhas, os nomes (como áreas de impressão e títulos de impressão), as células unidas, as células a que se aplicam regras de validação e formatação condicional, as tabelas, os gráficos, as imagens, as notas, as hiperligações e as quebras de página.

## O que não é possível fazer

- Não é possível inserir nem eliminar células através de parte de uma **fórmula de matriz** ou de uma tabela de dados. Mova-as por inteiro ou não lhes toque. Também não é possível eliminar uma célula que uma tabela de dados lê como entrada.
- Ainda não é possível mover nem cortar uma **tabela dinâmica**: não é possível inserir nem eliminar linhas, colunas ou células onde isso a moveria ou alteraria, nem cortar células dela ou para ela. O resto da folha pode ser alterado normalmente.
- Não é possível cortar uma **tabela** ao meio: não pode inserir uma coluna dentro dela nem eliminar a respetiva linha de cabeçalho ou todas as suas linhas. Insira ou elimine linhas dentro dela ou altere as células à volta.
- Numa **folha protegida**, não é possível inserir nem eliminar linhas, colunas ou células. Para o fazer, escolha Desproteger Folha no separador Rever.
- As células no fim da folha que tenham conteúdo não podem ser empurradas para além do limite. Elimine-as primeiro.
- Depois de guardar, **Anular** continua a recuar sobre o que fez antes, incluindo as linhas e colunas que inseriu (na gravação seguinte, estas voltam a ser retiradas). A única coisa que não consegue repor são as linhas, colunas ou células que **eliminou**: o ficheiro guardado perdeu o que continham, pelo que **Anular** para na última eliminação guardada, e a linha na parte inferior da janela indica-o quando guarda.
- Um passo que move ou elimina tantas células que **Anular** não o conseguiria guardar (mais de cerca de 128 MB) pede confirmação primeiro, tal como fazem as folhas de cálculo que conhece: "Esta ação não pode ser anulada." Se continuar, a ação é executada e **Anular** não recua sobre ela nem sobre o que veio antes.
