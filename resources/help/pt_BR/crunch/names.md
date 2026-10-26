# Nomes para células e valores
<!-- keywords: nome, nomes, nome definido, novo nome, intervalo nomeado, escopo, comentário, nome a partir das células, inserir nome, Ctrl+F3, refere-se a -->

Um **nome** é uma palavra que representa uma célula, um bloco de células, um número ou uma fórmula, de modo que você pode escrever =Preço*Quantidade em vez de =B2*C2. Um nome pertence à pasta de trabalho inteira ou a uma planilha. Um nome de uma planilha oculta, nessa planilha, um nome da pasta de trabalho escrito da mesma forma.

## As regras para um nome

- Começa com uma letra, um sublinhado ou uma barra invertida e, depois disso, contém apenas letras, dígitos, sublinhados e pontos. Não tem espaços.
- Tem no máximo 255 caracteres.
- Não é o endereço de uma célula, como **A1** ou **R2C3**.
- Nenhum outro nome da mesma planilha (ou da pasta de trabalho) é escrito da mesma forma, com ou sem maiúsculas.

Enquanto você digita, a caixa de diálogo informa o que está errado, e o botão **OK** fica indisponível.

## Criar um nome

1. Selecione as células e clique em **Novo Nome** na guia **Fórmulas**.
2. Digite o nome, escolha onde ele é válido e informe o que ele representa. As células selecionadas já vêm preenchidas.

**Nome a Partir das Células** cria vários nomes de uma vez a partir dos rótulos ao redor de um bloco: com os títulos na linha superior e os números abaixo deles, cada título dá nome às células abaixo dele. Escolha onde estão os rótulos (linha superior, coluna esquerda, linha inferior ou coluna direita). Em um rótulo com espaços ou sinais que um nome não pode ter, esses caracteres são substituídos por sublinhados. Um rótulo vazio, repetido ou que já seja um nome da pasta de trabalho é ignorado, e a barra de status informa quantos foram ignorados.

## Ver todos os nomes

Clique em **Nomes** na guia **Fórmulas** ou pressione **Ctrl+F3**. A lista mostra cada nome, o valor atual, o que ele representa, onde é válido e o comentário. Digite no filtro para mostrar apenas os nomes correspondentes. **Novo**, **Editar** e **Excluir** criam, alteram e removem um nome. Uma pasta de trabalho com mais de 20.000 nomes lista os primeiros 20.000.

Alterar o nome de um nome o altera em todas as fórmulas que o usam, em todas as planilhas, em uma única etapa. Remover um nome faz as fórmulas que o usavam mostrarem **#NOME?**. Cada alteração pode ser desfeita.

## Inserir um nome em uma fórmula

Enquanto você digita uma fórmula, os nomes correspondentes aparecem na lista abaixo da célula. **Inserir Nome** na guia **Fórmulas** lista os nomes para escolher.
