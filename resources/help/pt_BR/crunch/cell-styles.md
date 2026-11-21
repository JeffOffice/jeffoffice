# Estilos de célula
<!-- keywords: modificar estilo de célula, alterar um estilo, mesclar estilos, importar estilos, estilo de célula, estilos, estilo nomeado, Bom, Ruim, Neutro, Título, Cabeçalho, Título, Total, Ênfase, Entrada, Saída, Cálculo, Célula de Verificação, Texto Explicativo, Nota, Texto de Aviso, Célula Vinculada, Vírgula, Moeda, Porcentagem, Normal, novo estilo, nome, excluir estilo, remover estilo, Heading, Title -->

Um **estilo de célula** é um conjunto nomeado de aparências: uma fonte, um preenchimento, uma borda, um formato de número e a forma como o texto fica posicionado. Ao aplicar o estilo "Título 1" a uma célula, ela recebe todos esses itens de uma só vez. Os estilos mantêm a pasta de trabalho consistente, porque todas as células com o mesmo estilo têm a mesma aparência.

## Aplicar um estilo

1. Selecione as células.
2. Clique em **Página Inicial > Estilos de Célula** e clique em um estilo. O menu mostra uma pequena amostra de cada um.

O menu tem estes grupos:

- **Bom, Ruim e Neutro**: Normal, Bom, Ruim e Neutro.
- **Dados e Modelo**: Cálculo, Célula de Verificação, Texto Explicativo, Entrada, Célula Vinculada, Nota, Saída e Texto de Aviso.
- **Títulos e Cabeçalhos**: Título, Título 1 a 4, e Total.
- **Estilos de Célula com Tema**: quatro tonalidades de cada uma das seis cores de destaque do tema da pasta de trabalho.
- **Formato de Número**: Vírgula, Vírgula [0], Moeda, Moeda [0] e Porcentagem.
- **Personalizado**: estilos que a pasta de trabalho já tem com outros nomes.

Um estilo que a pasta de trabalho já tem é usado como está, com a aparência que tem ali. Um estilo que a pasta de trabalho não tem é criado na primeira vez que você o usa, com a primeira fonte da própria pasta de trabalho, e depois passa a fazer parte do arquivo, junto com os demais.

## Criar um estilo próprio

1. Formate uma célula do jeito que você quer: fonte, preenchimento, borda, formato de número e assim por diante.
2. Deixe o cursor nessa célula. Selecione as células que também devem receber o estilo.
3. Clique em **Página Inicial > Estilos de Célula > Novo Estilo de Célula**, digite um nome e clique em **OK**.

O estilo mantém a aparência da célula com o cursor, e as células selecionadas recebem esse estilo. Ele passa a aparecer no grupo **Personalizado** do menu e fica salvo na pasta de trabalho. Cada estilo precisa de um nome próprio. Um nome que já está em uso (não importa se com letras maiúsculas ou minúsculas) é recusado, assim como um nome vazio.

## Alterar um estilo

Clique em **Página Inicial > Estilos de Célula > Modificar Estilo de Célula** e escolha um estilo que a pasta de trabalho tenha (um estilo oferecido na galeria já está na pasta de trabalho assim que uma célula o tiver usado). A caixa de diálogo Formatar Células é aberta, partindo da própria aparência do estilo. Altere o que quiser e clique em **OK**: o estilo recebe a alteração, e também todas as células que o usam, em uma única etapa que você pode desfazer. Só muda o que a célula herda do estilo: uma célula que tem formatação própria a mantém, por isso uma célula com preenchimento próprio continua com esse preenchimento quando o preenchimento do estilo muda. Colunas e linhas inteiras formatadas com o estilo também recebem a alteração. Uma planilha protegida contra formatação de células, que tenha uma célula usando o estilo, recusa a alteração, e nada é modificado. Não é possível modificar o Normal dessa forma. O estilo é salvo na pasta de trabalho com a nova aparência, e uma célula que receber o estilo depois também a terá.

## Mesclar estilos de outra pasta de trabalho

Abra a pasta de trabalho que tem os estilos e, na pasta de trabalho que deve recebê-los, clique em **Página Inicial > Estilos de Célula > Mesclar Estilos** e escolha a pasta de trabalho. Cada estilo dela que esta pasta não tenha é adicionado. Quando um estilo tem o mesmo nome aqui (não importa se com letras maiúsculas ou minúsculas) e outra aparência, você é perguntado uma única vez se quer substituí-los: **Substituir** dá a eles a aparência da outra pasta de trabalho, como acontece em Modificar (as células que os usam mudam somente no que herdam do estilo), e **Manter** deixa os seus. Uma cor que é cor do tema é trazida como a cor que aparece na outra pasta de trabalho, a não ser que as duas pastas tenham o mesmo tema. Estilos com a mesma aparência são ignorados. É uma etapa que você pode desfazer, e o Desfazer remove todos os estilos que foram adicionados e restaura todas as aparências que foram alteradas. Os estilos da outra pasta de trabalho são apenas lidos, e ela não é alterada. No máximo 1.000 estilos são trazidos de uma vez (se houver mais, a operação é recusada e nada é mesclado); se a outra pasta de trabalho estiver em uso, você é avisado para tentar novamente; o Normal permanece como está, e um estilo que você excluiu aqui continua excluído.

## Excluir um estilo

Clique em **Página Inicial > Estilos de Célula > Excluir Estilo de Célula** e escolha o estilo. Todas as células que o usam voltam ao Normal, a aparência simples, e o estilo sai do menu. É uma etapa que você pode desfazer, e desfazer restaura o estilo e a aparência das células. Não é possível excluir o Normal. O arquivo da pasta de trabalho mantém o nome do estilo, marcado como oculto, para que outros programas também não o mostrem.

## Observações

- Aplicar um estilo é uma etapa que você pode desfazer. Ele substitui a fonte, o preenchimento, a borda, o formato de número, o alinhamento e a proteção da célula pelos do estilo.
- A célula lembra o seu estilo. Formatar a célula depois, por exemplo deixando-a em negrito, altera somente essa célula, e ela continua seguindo o restante do estilo.
- O JeffCrunch ainda não tem um comando para alterar a definição de um estilo nem para trazer estilos de outra pasta de trabalho. Uma pasta de trabalho que tenha estilos próprios os mostra em **Personalizado** e no respectivo grupo.
