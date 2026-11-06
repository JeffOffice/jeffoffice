# Nomes para células e valores
<!-- keywords: nome, nomes, nome definido, novo nome, intervalo nomeado, escopo, comentário, nome a partir das células, inserir nome, aplicar nomes, Ctrl+F3, refere-se a, classificar nomes, nome de erro, aviso-->

Um **nome** é uma palavra que representa uma célula, um intervalo de células, um número ou uma fórmula, de modo que =Preço*Quantidade pode ser escrito no lugar de =B2*C2. Um nome pertence à pasta de trabalho inteira ou a uma única planilha. Um nome em uma planilha oculta um nome da pasta de trabalho que tem a mesma grafia, naquela planilha.

## As regras para um nome

- Começa com uma letra, um sublinhado ou uma barra invertida, e depois contém apenas letras, algarismos, sublinhados e pontos. Não tem espaços.
- Tem no máximo 255 caracteres.
- Não é o endereço de uma célula, como **A1** ou **L2C3**.
- Nenhum outro nome da mesma planilha (ou da pasta de trabalho) tem a mesma grafia, em maiúsculas ou não.

Enquanto você digita, a caixa de diálogo indica o que está errado, e o botão **OK** fica indisponível.

## Criar um nome

1. Selecione as células e clique em **Novo Nome** na guia **Fórmulas**.
2. Digite o nome, escolha onde ele funciona e informe o que ele representa. As células selecionadas são preenchidas automaticamente.

Há um modo mais rápido para um nome da pasta de trabalho inteira. Selecione as células, clique na **Caixa de Nome** (à esquerda da barra de fórmulas), digite o nome e pressione **Enter**. Se nenhum nome com essa grafia existir ainda, ele é criado para as células selecionadas. Se já existir, **Enter** leva às células dele.

**Nome a Partir das Células** cria vários nomes de uma só vez a partir dos rótulos ao redor de um intervalo: com os títulos na linha superior e os números abaixo deles, cada título nomeia as células sob ele. Escolha onde estão os rótulos (linha superior, coluna da esquerda, linha inferior ou coluna da direita). Um rótulo com espaços ou com caracteres que um nome não pode ter recebe sublinhados no lugar deles. Um rótulo vazio, repetido ou que já seja um nome da pasta de trabalho é ignorado, e a barra de status informa quantos foram ignorados.

## Ver todos os nomes

Clique em **Nomes** na guia **Fórmulas**, ou pressione **Ctrl+F3**. A lista mostra cada nome, o valor que ele tem agora, o que ele representa, onde funciona e seu comentário. Digite no filtro para mostrar apenas os nomes correspondentes. **Novo**, **Alterar** e **Excluir** criam, alteram e removem um nome. Clique em um cabeçalho para classificar a lista por ele, e clique de novo para inverter a ordem. Um nome que aponta para um erro, como um local que foi excluído, tem um pequeno sinal de aviso ao lado. Uma pasta de trabalho com mais de 20.000 nomes mostra apenas os primeiros 20.000.

Alterar o nome de um nome o altera em todas as fórmulas que o usam, em todas as planilhas, em uma única etapa. Excluir um nome faz com que as fórmulas que o usavam mostrem **#NOME?**. Cada alteração pode ser desfeita.

## Colocar um nome em uma fórmula

Enquanto você digita uma fórmula, os nomes correspondentes aparecem em uma lista abaixo da célula. **Inserir Nome** na guia **Fórmulas** lista os nomes para você escolher.

## Colocar nomes em fórmulas já escritas

Suponha que você criou o nome Taxa para $A$1 depois de muitas fórmulas já usarem $A$1. Clique em **Aplicar Nomes** na guia **Fórmulas** para escrever Taxa no lugar delas. Selecione antes as células a verificar, ou selecione uma única célula para verificar a planilha inteira. Escolha os nomes a usar. Uma referência só é alterada quando corresponde exatamente às células do nome, com ou sem os cifrões: A1:B2 não é trocado por um nome de A1. Só são oferecidos nomes de um intervalo de células de uma planilha e nomes da pasta de trabalho inteira. A alteração é feita em uma única etapa, então **Desfazer** a desfaz.
