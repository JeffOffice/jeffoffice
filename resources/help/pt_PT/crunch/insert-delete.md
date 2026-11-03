# Inserir e eliminar linhas, colunas e células
<!-- keywords: inserir, eliminar, apagar, remover, linha, coluna, célula, deslocar, mover, adicionar, acrescentar, em branco, vazia, Ctrl+mais, Ctrl+menos, clique com o botão direito, cabeçalho, anular, desfazer, referência, #REF!, fórmula, nome, unidas, intercaladas, tabela, matriz, limite -->

Pode adicionar linhas, colunas ou células vazias no meio dos seus dados e retirar linhas, colunas ou células. Tudo o que está a seguir desloca-se para dar espaço ou para fechar o intervalo, e as fórmulas que apontam para as células deslocadas continuam a apontar para elas.

## Inserir

1. Selecione os números das linhas, as letras das colunas ou as células onde pretende as novas. Para adicionar três linhas, selecione três linhas.
2. Prima **Ctrl e a tecla de adição** (**Ctrl e a tecla de subtração** elimina), utilize **Base > Inserir** ou clique com o botão direito do rato na seleção e escolha **Inserir**.

Com linhas ou colunas inteiras selecionadas, as novas são inseridas de imediato, acima das linhas ou à esquerda das colunas. Com células selecionadas, uma pequena janela pergunta como devem deslocar-se as células seguintes:

- **Deslocar células para a direita** ou **Deslocar células para baixo** desloca apenas as células nas colunas (ou linhas) da seleção e deixa o resto da folha onde está.
- **Linha inteira** ou **Coluna inteira** adiciona linhas ou colunas inteiras.

## Eliminar

Selecione o que pretende retirar e prima **Ctrl e a tecla de subtração** ou utilize **Base > Eliminar**. Com células selecionadas, a janela pergunta se pretende **deslocar células para a esquerda**, **deslocar células para cima** ou eliminar a **linha inteira** ou a **coluna inteira**. A eliminação é um só passo e **Anular** repõe tudo o que foi retirado, incluindo as fórmulas.

## O que acontece a seguir

Uma fórmula que aponta para células que se deslocam passa a apontar para elas nas novas posições. Exemplo: em B1 tem **=SOMA(A1:A5)**. Insira uma linha acima da linha 3 e a fórmula passa a **=SOMA(A1:A6)**, porque o intervalo aumentou para incluir a nova linha. Se eliminar a linha 3, passa a **=SOMA(A1:A4)**. Uma fórmula que apontava para uma célula eliminada mostra **#REF!**, o que significa que a célula de que precisa já não existe. **Anular** repõe a célula e a fórmula.

Também acompanham as células: fórmulas em todas as folhas, nomes (como áreas de impressão e títulos de impressão), células unidas, as células a que se aplicam regras de validação e formatação condicional, tabelas, gráficos, imagens, notas, hiperligações e quebras de página.

## O que não é possível fazer

- Não é possível inserir nem eliminar células através de parte de uma **fórmula de matriz** ou de uma tabela de dados. Desloque-as por inteiro ou não lhes mexa. Também não é possível eliminar uma célula que uma tabela de dados lê como entrada.
- Ainda não é possível deslocar nem atravessar uma **tabela dinâmica**: não é possível inserir nem eliminar linhas, colunas ou células onde isso a deslocaria ou alteraria, nem cortar células dela ou para ela. O resto da folha pode ser alterado normalmente. Uma tabela dinâmica altera-se com os comandos do separador **Tabela Dinâmica** (atualizar, alterar origem de dados, campos e esquema), e não escrevendo nas respetivas células.
- Não é possível atravessar uma **tabela**: não pode inserir uma coluna no seu interior nem eliminar a linha de cabeçalho ou todas as suas linhas. Insira ou elimine linhas no seu interior, ou altere as células à volta.
- Numa **folha protegida** não é possível inserir nem eliminar linhas, colunas ou células. Para o fazer, escolha Desproteger Folha no separador Rever.
- Não é possível empurrar para fora do limite da folha células no fim da folha que tenham conteúdo. Elimine-as primeiro.
- Depois de guardar, **Anular** continua a recuar sobre o que fez antes, incluindo as linhas e colunas que inseriu (a gravação seguinte volta a retirá-las). A única coisa que não consegue repor são linhas, colunas ou células que **eliminou**: o ficheiro guardado perdeu o que continham, pelo que **Anular** para na última eliminação guardada, e a linha na parte inferior da janela indica-o quando guarda.
- Um passo que desloca ou elimina tantas células que **Anular** não o conseguiria guardar (mais de cerca de 128 MB) pergunta primeiro, tal como fazem os programas de folha de cálculo conhecidos: "Não é possível anular esta ação." Se continuar, a ação é executada e **Anular** não recua sobre ela nem sobre o que veio antes.
