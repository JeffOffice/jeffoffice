# Nomes para células e valores
<!-- keywords: nome, nomes, nome definido, novo nome, intervalo com nome, intervalo nomeado, âmbito, comentário, nome a partir das células, inserir nome, Ctrl+F3, refere-se a -->

Um **nome** é uma palavra que representa uma célula, um bloco de células, um número ou uma fórmula, para que possa escrever =Preço*Quantidade em vez de =B2*C2. Um nome pertence ao livro inteiro ou a uma folha. Numa folha, um nome dessa folha sobrepõe-se a um nome do livro escrito da mesma forma.

## As regras de um nome

- Começa por uma letra, um carácter de sublinhado ou uma barra invertida, e depois tem apenas letras, algarismos, carateres de sublinhado e pontos. Não tem espaços.
- Tem, no máximo, 255 carateres.
- Não é o endereço de uma célula, como **A1** ou **R2C3**.
- Nenhum outro nome da mesma folha (ou do livro) é escrito da mesma forma, com ou sem maiúsculas.

Enquanto escreve, a caixa de diálogo indica o que está errado e o botão **OK** fica indisponível.

## Criar um nome

1. Selecione as células e clique em **Novo Nome** no separador **Fórmulas**.
2. Escreva o nome, escolha onde é válido e indique o que representa. As células que selecionou são preenchidas automaticamente.

**Nome a Partir das Células** cria vários nomes de uma só vez a partir dos rótulos à volta de um bloco: com os cabeçalhos na linha superior e os números por baixo, cada cabeçalho dá nome às células abaixo dele. Escolha onde estão os rótulos (linha superior, coluna da esquerda, linha inferior ou coluna da direita). Num rótulo com espaços ou sinais que um nome não pode ter, estes são substituídos por carateres de sublinhado. Um rótulo vazio, repetido ou que já seja um nome do livro é omitido, e a barra de estado indica quantos foram omitidos.

## Ver todos os nomes

Clique em **Nomes** no separador **Fórmulas** ou prima **Ctrl+F3**. A lista mostra cada nome, o valor atual, o que representa, onde é válido e o respetivo comentário. Escreva no filtro para mostrar apenas os nomes correspondentes. **Novo**, **Alterar** e **Eliminar** criam, alteram e removem um nome. Num livro com mais de 20 000 nomes, são listados os primeiros 20 000.

Mudar o nome de um nome altera-o em todas as fórmulas que o utilizam, em todas as folhas, de uma só vez. Remover um nome faz com que as fórmulas que o utilizavam mostrem **#NAME?**. Cada alteração pode ser anulada.

## Colocar um nome numa fórmula

Enquanto escreve uma fórmula, os nomes correspondentes aparecem na lista por baixo da célula. **Inserir Nome** no separador **Fórmulas** apresenta a lista de nomes para escolher.
