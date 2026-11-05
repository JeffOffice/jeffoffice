# Nomes para células e valores
<!-- keywords: nome, nomes, nome definido, novo nome, intervalo com nome, âmbito, comentário, nome a partir de células, inserir nome, Ctrl+F3, refere-se a, ordenar nomes, nome de erro, aviso-->

Um **nome** é uma palavra que representa uma célula, um bloco de células, um número ou uma fórmula, para que =Preço*Quantidade possa ser escrito em vez de =B2*C2. Um nome pertence a toda a pasta de trabalho ou a uma só folha. Um nome numa folha oculta um nome da pasta de trabalho que se escreve da mesma forma, nessa folha.

## As regras para um nome

- Começa por uma letra, um sublinhado ou uma barra invertida, e depois só tem letras, algarismos, sublinhados e pontos. Não tem espaços.
- Tem no máximo 255 carateres.
- Não é o endereço de uma célula, como **A1** ou **R2C3**.
- Nenhum outro nome da mesma folha (ou da pasta de trabalho) se escreve da mesma forma, em maiúsculas ou não.

Enquanto escreve, a caixa de diálogo indica o que está errado, e o botão **OK** fica indisponível.

## Criar um nome

1. Selecione as células e clique em **Novo Nome** no separador **Fórmulas**.
2. Escreva o nome, escolha onde funciona e indique o que representa. As células que selecionou são preenchidas automaticamente.

Há uma forma mais rápida de criar um nome para toda a pasta de trabalho. Selecione as células, clique na **Caixa de nome** (à esquerda da barra de fórmulas), escreva o nome e prima **Enter**. Se ainda não existir nenhum nome com essa escrita, é criado para as células selecionadas. Se existir, **Enter** leva-o às respetivas células.

**Nome a Partir das Células** cria vários nomes de uma só vez a partir das etiquetas em redor de um bloco: com os cabeçalhos na linha superior e os números por baixo, cada cabeçalho dá nome às células por baixo dele. Escolha onde estão as etiquetas (linha superior, coluna da esquerda, linha inferior ou coluna da direita). Uma etiqueta com espaços ou com marcas que um nome não pode ter recebe sublinhados no lugar delas. Uma etiqueta que esteja vazia, que se repita ou que seja um nome já existente na pasta de trabalho é ignorada, e a barra de estado indica quantas foram.

## Ver todos os nomes

Clique em **Nomes** no separador **Fórmulas**, ou prima **Ctrl+F3**. A lista mostra cada nome, o valor que tem neste momento, o que representa, onde funciona e o seu comentário. Escreva no filtro para mostrar apenas os nomes correspondentes. **Novo**, **Alterar** e **Eliminar** criam um nome, alteram-no ou removem-no. Clique num cabeçalho para ordenar a lista por esse critério, e clique novamente para inverter a ordem. Um nome que aponta para um erro, como um local que foi eliminado, tem um pequeno sinal de aviso ao lado. Numa pasta de trabalho com mais de 20 000 nomes, a lista mostra os primeiros 20 000.

Alterar o nome de um nome altera-o em todas as fórmulas que o utilizam, em todas as folhas, num único passo. Eliminar um nome faz com que as fórmulas que o utilizavam mostrem **#NOME?**. Cada alteração pode ser anulada.

## Colocar um nome numa fórmula

Enquanto escreve uma fórmula, os nomes correspondentes aparecem na lista por baixo da célula. **Inserir Nome** no separador **Fórmulas** mostra a lista de nomes entre os quais pode escolher.
