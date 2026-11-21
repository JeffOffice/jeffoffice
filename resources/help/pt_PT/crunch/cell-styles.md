# Estilos de célula
<!-- keywords: alterar estilo de célula, modificar estilo, unir estilos, importar estilos, estilo de célula, estilos, estilo com nome, Bom, Mau, Neutro, Título, Total, Destaque, Entrada, Saída, Cálculo, Célula de Verificação, Texto Explicativo, Nota, Texto de Aviso, Célula Ligada, Vírgula, Moeda, Percentagem, Normal, novo estilo, nome, eliminar estilo, remover estilo -->

Um **estilo de célula** é um conjunto com nome de aspetos: um tipo de letra, um preenchimento, um limite, um formato de número e a forma como o texto fica posicionado. Se atribuir à célula o estilo "Título 1", ela assume todos estes aspetos de uma só vez. Os estilos mantêm um livro de cálculo coerente, porque todas as células com o mesmo estilo têm o mesmo aspeto.

## Aplicar um estilo

1. Selecione as células.
2. Clique em **Base > Estilos de Célula** e clique num estilo. O menu mostra uma pequena amostra de cada um.

O menu tem estes grupos:

- **Bom, Mau e Neutro**: Normal, Bom, Mau e Neutro.
- **Dados e Modelo**: Cálculo, Célula de Verificação, Texto Explicativo, Entrada, Célula Ligada, Nota, Saída e Texto de Aviso.
- **Títulos e Cabeçalhos**: Título, Título 1 a 4, e Total.
- **Estilos de Célula com Tema**: quatro tonalidades de cada uma das seis cores de destaque do tema do livro de cálculo.
- **Formato de Número**: Vírgula, Vírgula [0], Moeda, Moeda [0] e Percentagem.
- **Personalizado**: estilos que o livro de cálculo já tem com outros nomes.

Um estilo que o livro de cálculo já tem é usado tal como está, com o aspeto que tem aí. Um estilo que o livro não tem é criado na primeira vez que o usar, a partir do primeiro tipo de letra do próprio livro, e passa a fazer parte do ficheiro com os restantes.

## Criar um estilo próprio

1. Formate uma célula como pretende: um tipo de letra, um preenchimento, um limite, um formato de número, e assim por diante.
2. Deixe o cursor nessa célula. Selecione as células que também devem receber o estilo.
3. Clique em **Base > Estilos de Célula > Novo Estilo de Célula**, escreva um nome e clique em **OK**.

O estilo mantém o aspeto da célula onde está o cursor, e as células selecionadas recebem-no. Passa a estar no grupo **Personalizado** do menu e fica guardado no livro de cálculo. Cada estilo precisa de um nome próprio. Um nome que já esteja em uso (não há diferença entre maiúsculas e minúsculas) é recusado, e também é recusado um nome vazio.

## Alterar um estilo

Clique em **Base > Estilos de Célula > Modificar Estilo de Célula** e escolha um estilo que o livro de cálculo tenha (um estilo oferecido pela galeria fica no livro depois de uma célula o ter usado). A caixa de diálogo Formatar Células abre-se a partir do próprio aspeto do estilo. Altere o que pretender e clique em **OK**: o estilo assume a alteração, e também todas as células que o usam, num único passo que pode anular. Só muda aquilo que uma célula recebe do estilo: uma célula com formatação própria mantém-na, pelo que uma célula com preenchimento próprio conserva esse preenchimento quando o preenchimento do estilo muda. Colunas e linhas inteiras formatadas com o estilo também recebem a alteração. Uma folha protegida contra a formatação de células, que tenha uma célula a usar o estilo, recusa a alteração, e nada é alterado. Não é possível modificar o Normal desta forma. O estilo fica guardado no livro com o novo aspeto, e uma célula a que o atribuir mais tarde terá esse aspeto.

## Unir estilos de outro livro

Abra o livro que tem os estilos e, no livro que deve passar a tê-los, clique em **Base > Estilos de Célula > Unir Estilos** e escolha o livro. Cada estilo que o outro livro tem e que este não tem é acrescentado. Quando um estilo tem o mesmo nome aqui (não há diferença entre maiúsculas e minúsculas) e um aspeto diferente, pergunta-se uma única vez se deve substituí-los: **Substituir** dá-lhes o aspeto do outro livro, tal como Modificar (as células que os usam mudam apenas naquilo que recebem do estilo), e **Manter** deixa os seus próprios. Uma cor que seja uma cor do tema é trazida como a cor que mostra no outro livro, a não ser que os dois livros tenham o mesmo tema. Os estilos com o mesmo aspeto são ignorados. A união é um passo que pode anular, e anular retira todos os estilos que acrescentou e repõe todos os aspetos que alterou. Os estilos do outro livro são apenas lidos, e esse livro não é alterado. São trazidos no máximo 1000 estilos de cada vez (os restantes são recusados, e nada é unido); se o outro livro estiver ocupado, é-lhe pedido que tente novamente; o Normal mantém-se como está, e um estilo que tenha eliminado aqui continua eliminado.

## Eliminar um estilo

Clique em **Base > Estilos de Célula > Eliminar Estilo de Célula** e escolha o estilo. Todas as células que o usam voltam ao Normal, o aspeto simples, e o estilo sai do menu. É um passo que pode anular, e anular repõe o estilo e o aspeto das células. Não é possível eliminar o Normal. O ficheiro do livro mantém o nome do estilo, marcado como oculto, para que outros programas também não o mostrem.

## Notas

- Aplicar um estilo é um passo que pode anular. Substitui o tipo de letra, o preenchimento, o limite, o formato de número, o alinhamento e a proteção da célula pelos do estilo.
- Uma célula memoriza o seu estilo. Formatar a célula depois, por exemplo pondo-a a negrito, altera apenas essa célula, e a célula continua a seguir o resto do estilo.
- O JeffCrunch ainda não tem um comando para alterar a definição de um estilo nem para trazer estilos de outro livro. Um livro que tenha estilos próprios mostra-os em **Personalizado** e no respetivo grupo.
