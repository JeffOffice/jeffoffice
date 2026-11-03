# Inserir e excluir linhas, colunas e células
<!-- keywords: inserir, excluir, apagar, remover, linha, coluna, célula, deslocar, mover, adicionar, em branco, Ctrl+mais, Ctrl+menos, clique com o botão direito, cabeçalho, desfazer, referência, #REF!, fórmula, nome, mescladas, tabela, matriz, borda, limite -->

Você pode adicionar linhas, colunas ou células vazias no meio dos dados e também retirar linhas, colunas ou células. Tudo o que vem depois se desloca para abrir espaço ou fechar o vazio, e as fórmulas que apontam para as células deslocadas continuam apontando para elas.

## Inserir

1. Selecione os números das linhas, as letras das colunas ou as células onde você deseja as novas. Para adicionar três linhas, selecione três linhas.
2. Pressione **Ctrl e a tecla de adição** (**Ctrl e a tecla de subtração** exclui), use **Página Inicial > Inserir** ou clique na seleção com o botão direito do mouse e escolha **Inserir**.

Com linhas ou colunas inteiras selecionadas, as novas são inseridas imediatamente, acima das linhas ou à esquerda das colunas. Com células selecionadas, uma pequena janela pergunta como as células seguintes devem se deslocar:

- **Deslocar células para a direita** ou **Deslocar células para baixo** move somente as células nas colunas (ou linhas) da seleção e deixa o restante da planilha onde está.
- **Linha inteira** ou **Coluna inteira** adiciona linhas ou colunas inteiras.

## Excluir

Selecione o que deseja retirar e pressione **Ctrl e a tecla de subtração** ou use **Página Inicial > Excluir**. Com células selecionadas, a janela pergunta se você deseja **deslocar células para a esquerda**, **deslocar células para cima** ou excluir a **linha inteira** ou a **coluna inteira**. A exclusão é uma única etapa, e **Desfazer** restaura tudo o que foi retirado, inclusive as fórmulas.

## O que acontece em seguida

Uma fórmula que aponta para células que se deslocam passa a apontar para elas nos novos locais. Exemplo: em B1 você tem **=SOMA(A1:A5)**. Insira uma linha acima da linha 3, e a fórmula se torna **=SOMA(A1:A6)**, porque o intervalo cresceu para incluir a nova linha. Se você excluir a linha 3, ela se torna **=SOMA(A1:A4)**. Uma fórmula que apontava para uma célula excluída mostra **#REF!**, o que significa que a célula de que ela precisa não existe mais. **Desfazer** traz de volta a célula e a fórmula.

Também acompanham as células: fórmulas em todas as planilhas, nomes (como áreas de impressão e títulos de impressão), células mescladas, as células às quais se aplicam regras de validação e formatações condicionais, tabelas, gráficos, imagens, anotações, hiperlinks e quebras de página.

## O que não é possível fazer

- Não é possível inserir ou excluir células atravessando parte de uma **fórmula de matriz** ou de uma tabela de dados. Mova-as inteiras ou não as altere. Também não é possível excluir uma célula que uma tabela de dados lê como entrada.
- Uma **tabela dinâmica** ainda não pode ser movida nem atravessada: não é possível inserir ou excluir linhas, colunas ou células onde isso a moveria ou alteraria, nem recortar células dela ou colá-las sobre ela. O restante da planilha pode ser alterado normalmente. Uma tabela dinâmica é alterada com os comandos da guia **Tabela Dinâmica** (atualizar, alterar fonte de dados, campos e layout), e não digitando em suas células.
- Uma **tabela** não pode ser dividida no meio: não é possível inserir uma coluna que a atravesse parcialmente, nem excluir a linha de cabeçalho ou todas as suas linhas. Insira ou exclua linhas dentro dela ou altere as células ao redor.
- Em uma **planilha protegida**, não é possível inserir ou excluir linhas, colunas ou células. Para isso, escolha Desproteger Planilha na guia Revisão.
- Células no fim da planilha que contêm algo não podem ser empurradas para fora do limite. Exclua-as primeiro.
- Depois de salvar, **Desfazer** ainda volta pelo que você fez antes, inclusive linhas e colunas inseridas (o próximo salvamento as retira novamente). A única coisa que ele não consegue restaurar são linhas, colunas ou células **excluídas**: o arquivo salvo perdeu o conteúdo delas, então **Desfazer** para na última exclusão salva, e a linha na parte inferior da janela avisa isso quando você salva.
- Uma etapa que move ou exclui tantas células que **Desfazer** não conseguiria guardá-la (mais de cerca de 128 MB) pergunta antes, como fazem os programas de planilha conhecidos: "Esta ação não pode ser desfeita." Se você continuar, ela será executada, e **Desfazer** não voltará por ela nem pelo que veio antes dela.
