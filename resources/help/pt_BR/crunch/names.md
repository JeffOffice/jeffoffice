# Nomes para células e valores
<!-- keywords: nome, nomes, nome definido, novo nome, intervalo nomeado, escopo, comentário, nome a partir das células, inserir nome, Ctrl+F3, refere-se a -->

Um **nome** é uma palavra que representa uma célula, um intervalo de células, um número ou uma fórmula, para que =Preço*Quantidade possa ser escrito no lugar de =B2*C2. Um nome pertence à pasta de trabalho inteira ou a uma única planilha. Um nome em uma planilha oculta um nome da pasta de trabalho que tenha a mesma grafia, naquela planilha.

## As regras para um nome

- Começa com uma letra, um sublinhado ou uma barra invertida, e depois tem apenas letras, algarismos, sublinhados e pontos. Não tem espaços.
- Tem no máximo 255 caracteres.
- Não é o endereço de uma célula, como **A1** ou **R2C3**.
- Nenhum outro nome da mesma planilha (ou da pasta de trabalho) tem a mesma grafia, com maiúsculas ou não.

Enquanto você digita, a caixa de diálogo informa o que está errado, e o botão **OK** fica indisponível.

## Criar um nome

1. Selecione as células e clique em **Novo Nome** na guia **Fórmulas**.
2. Digite o nome, escolha onde ele funciona e informe o que ele representa. As células que você selecionou são preenchidas automaticamente.

Há um modo mais rápido para um nome da pasta de trabalho inteira. Selecione as células, clique na **Caixa de nome** (à esquerda da barra de fórmulas), digite o nome e pressione **Enter**. Se ainda não existir nenhum nome com essa grafia, ele é criado para as células selecionadas. Se existir, **Enter** vai até as células dele.

**Criar Nomes a Partir das Células** cria muitos nomes de uma só vez a partir dos rótulos ao redor de um bloco: com os títulos na linha superior e os números abaixo deles, cada título dá nome às células abaixo dele. Escolha onde estão os rótulos (linha superior, coluna da esquerda, linha inferior ou coluna da direita). Um rótulo que tenha espaços ou sinais que um nome não pode ter recebe sublinhados no lugar deles. Um rótulo que esteja vazio, se repita ou seja um nome que a pasta de trabalho já possui é ignorado, e a barra de status informa quantos foram ignorados.

## Ver todos os nomes

Clique em **Nomes** na guia **Fórmulas**, ou pressione **Ctrl+F3**. A lista mostra cada nome, o valor que ele tem agora, o que ele representa, onde ele funciona e seu comentário. Digite no filtro para mostrar apenas os nomes correspondentes. **Novo**, **Editar** e **Excluir** criam um nome, alteram um nome ou removem um nome. Uma pasta de trabalho com mais de 20.000 nomes lista os primeiros 20.000.

Alterar o nome de um nome o altera em todas as fórmulas que o usam, em todas as planilhas, em uma única etapa. Excluir um nome deixa as fórmulas que o usavam exibindo **#NOME?**. Cada alteração pode ser desfeita.

## Colocar um nome em uma fórmula

Enquanto você digita uma fórmula, os nomes correspondentes aparecem na lista sob a célula. **Inserir Nome** na guia **Fórmulas** lista os nomes para você escolher.
