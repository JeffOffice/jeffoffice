# Nomes para células e valores
<!-- keywords: nome, nomes, nome definido, novo nome, intervalo com nome, âmbito, comentário, nome a partir das células, inserir nome, Ctrl+F3, refere-se a -->

Um **nome** é uma palavra que representa uma célula, um bloco de células, um número ou uma fórmula, para que =Preço*Quantidade possa ser escrito em vez de =B2*C2. Um nome pertence a todo o livro ou a uma só folha. Um nome numa folha oculta um nome do livro com a mesma grafia, nessa folha.

## As regras para um nome

- Começa por uma letra, um sublinhado ou uma barra invertida, e depois tem apenas letras, algarismos, sublinhados e pontos. Não tem espaços.
- Tem, no máximo, 255 carateres.
- Não é o endereço de uma célula, como **A1** ou **R2C3**.
- Nenhum outro nome da mesma folha (ou do livro) tem a mesma grafia, em maiúsculas ou não.

Enquanto escreve, a caixa de diálogo indica o que está errado, e o botão **OK** fica à espera.

## Criar um nome

1. Selecione as células e clique em **Novo Nome** no separador **Fórmulas**.
2. Escreva o nome, escolha onde ele funciona e indique o que representa. As células que selecionou são preenchidas automaticamente.

Há uma forma mais rápida de criar um nome para todo o livro. Selecione as células, clique na **Caixa de nome** (à esquerda da barra de fórmulas), escreva o nome e prima **Enter**. Se ainda não existir nenhum nome com essa grafia, é criado para as células selecionadas. Se existir, **Enter** leva-o às respetivas células.

**Nome a Partir das Células** cria muitos nomes de uma só vez a partir das etiquetas em redor de um bloco: com os títulos na linha superior e os números por baixo, cada título dá nome às células abaixo dele. Escolha onde estão as etiquetas (linha superior, coluna da esquerda, linha inferior ou coluna da direita). Uma etiqueta com espaços ou com marcas que um nome não pode ter recebe sublinhados no lugar delas. Uma etiqueta vazia, repetida ou que seja um nome que o livro já tem fica de fora, e a barra de estado indica quantas ficaram de fora.

## Ver todos os nomes

Clique em **Nomes** no separador **Fórmulas**, ou prima **Ctrl+F3**. A lista mostra cada nome, o valor que tem neste momento, o que representa, onde funciona e o respetivo comentário. Escreva no filtro para mostrar apenas os nomes correspondentes. **Novo**, **Alterar** e **Eliminar** criam um nome, alteram um nome e removem um nome. Um livro com mais de 20 000 nomes apresenta na lista os primeiros 20 000.

Alterar o nome de um nome altera-o em todas as fórmulas que o utilizam, em todas as folhas, num único passo. Ao eliminar um nome, as fórmulas que o utilizavam passam a mostrar **#NOME?**. Cada alteração pode ser anulada.

## Colocar um nome numa fórmula

Enquanto escreve uma fórmula, os nomes correspondentes aparecem na lista por baixo da célula. **Inserir Nome** no separador **Fórmulas** mostra os nomes entre os quais pode escolher.
