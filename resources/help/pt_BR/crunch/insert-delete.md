# Inserir e excluir linhas, colunas e células
<!-- keywords: inserir, excluir, remover, apagar, linha, coluna, célula, deslocar, mover, adicionar, em branco, Ctrl+sinal de mais, Ctrl+sinal de menos, clique com o botão direito, cabeçalho, desfazer, referência, #REF!, fórmula, nome, mesclada, tabela, matriz, borda -->

Você pode adicionar linhas, colunas ou células vazias no meio dos seus dados e também retirar linhas, colunas ou células. Tudo o que vem depois se desloca para abrir espaço ou para fechar a lacuna, e as fórmulas que se referem às células deslocadas continuam se referindo a elas.

## Inserir

1. Selecione os números de linha, as letras de coluna ou as células onde você deseja as novas. Para adicionar três linhas, selecione três linhas.
2. Pressione **Ctrl e a tecla de sinal de mais** (**Ctrl e a tecla de sinal de menos** exclui), use **Página Inicial > Inserir** ou clique com o botão direito do mouse na seleção e escolha **Inserir**.

Com linhas ou colunas inteiras selecionadas, as novas são inseridas imediatamente, acima das linhas ou à esquerda das colunas. Com células selecionadas, uma pequena janela pergunta como as células seguintes devem se deslocar:

- **Deslocar células para a direita** ou **Deslocar células para baixo** desloca somente as células nas colunas (ou linhas) da sua seleção e deixa o resto da planilha onde está.
- **Linha inteira** ou **Coluna inteira** adiciona linhas ou colunas inteiras.

## Excluir

Selecione o que você deseja retirar e pressione **Ctrl e a tecla de sinal de menos** ou use **Página Inicial > Excluir**. Com células selecionadas, a janela pergunta se você deseja **deslocar células para a esquerda**, **deslocar células para cima** ou excluir a **linha inteira** ou a **coluna inteira**. A exclusão é uma única etapa, e **Desfazer** restaura tudo o que ela retirou, inclusive as fórmulas.

## O que acontece em seguida

Uma fórmula que se refere a células que se deslocam passa a se referir a elas nas novas posições. Exemplo: em B1 você tem **=SOMA(A1:A5)**. Insira uma linha acima da linha 3, e a fórmula passa a ser **=SOMA(A1:A6)**, porque o intervalo cresceu para incluir a nova linha. Se você excluir a linha 3, ela passa a ser **=SOMA(A1:A4)**. Uma fórmula que se referia a uma célula excluída mostra **#REF!**, o que significa que a célula de que ela precisa não existe mais. **Desfazer** traz a célula e a fórmula de volta.

Estes itens também acompanham as células: fórmulas em todas as planilhas, nomes (como áreas de impressão e títulos de impressão), células mescladas, as células às quais se aplicam regras de validação e formatações condicionais, tabelas, gráficos, imagens, anotações, hiperlinks e quebras de página.

## O que não pode ser feito

- Não é possível inserir ou excluir células atravessando parte de uma **fórmula de matriz** ou de uma tabela de dados. Mova-as inteiras ou não mexa nelas. Também não é possível excluir uma célula que uma tabela de dados lê como entrada.
- Uma **tabela dinâmica** ainda não pode ser movida nem atravessada: não é possível inserir ou excluir linhas, colunas ou células onde isso a moveria ou alteraria, nem recortar células dela ou colar sobre ela. O resto da planilha pode ser alterado normalmente.
- Uma **tabela** não pode ser atravessada: não é possível inserir uma coluna dentro dela, nem excluir sua linha de cabeçalho ou todas as suas linhas. Insira ou exclua linhas dentro dela ou altere as células ao redor.
- Em uma **planilha protegida**, não é possível inserir nem excluir linhas, colunas ou células. Para fazer isso, escolha Desproteger Planilha na guia Revisão.
- Células com conteúdo no final da planilha não podem ser empurradas para além da borda. Exclua-as primeiro.
- Depois de salvar, **Desfazer** ainda volta pelo que você fez antes, incluindo linhas e colunas inseridas (o próximo salvamento as retira novamente). A única coisa que não é possível recuperar são linhas, colunas ou células **excluídas**: o arquivo salvo perdeu o que havia nelas, então **Desfazer** para na última exclusão salva, e a linha na parte inferior da janela avisa isso quando você salva.
- Uma etapa que move ou exclui tantas células que **Desfazer** não conseguiria guardá-la (mais de cerca de 128 MB) pede confirmação antes, como fazem os programas de planilha conhecidos: "Esta ação não pode ser desfeita." Se você continuar, ela é executada, e **Desfazer** não volta por ela nem pelo que veio antes.
