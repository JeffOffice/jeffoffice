# Inserir e excluir linhas, colunas e células
<!-- keywords: inserir células recortadas, inserir células copiadas, deslocar para a direita, deslocar para baixo, inserir, excluir, remover, linha, coluna, célula, deslocar, mover, adicionar, em branco, Ctrl+mais, Ctrl+menos, clique com o botão direito, cabeçalho, desfazer, referência, #REF!, fórmula, nome, mesclada, tabela, matriz, borda -->

Você pode adicionar linhas, colunas ou células vazias no meio dos seus dados, e também remover linhas, colunas ou células. Tudo abaixo ou ao lado se desloca para abrir espaço ou para fechar a lacuna, e as fórmulas que apontam para as células que se deslocaram continuam apontando para elas.

## Inserir

1. Selecione os números das linhas, as letras das colunas ou as células onde você quer as novas. Para adicionar três linhas, selecione três linhas.
2. Pressione **Ctrl e a tecla de mais** (**Ctrl e a tecla de menos** exclui), ou use **Página Inicial > Inserir**, ou clique com o botão direito na seleção e escolha **Inserir**.

Com linhas ou colunas inteiras selecionadas, as novas entram de uma só vez, acima das linhas ou à esquerda das colunas. Com células selecionadas, uma pequena janela pergunta como as células seguintes devem se deslocar:

- **Deslocar células para a direita** ou **Deslocar células para baixo** move apenas as células nas colunas (ou linhas) da sua seleção, e deixa o restante da planilha onde está.
- **Linha inteira** ou **Coluna inteira** adiciona linhas ou colunas inteiras.

## Inserir células recortadas ou copiadas

Para colocar células que você recortou ou copiou entre outras células, em vez de sobre elas, recorte-as ou copie-as primeiro (**Ctrl+X** ou **Ctrl+C**). Depois, selecione a célula onde elas devem ficar e use **Página Inicial > Inserir > Inserir Células Recortadas ou Copiadas** (o comando mostra **Inserir Células Recortadas** ou **Inserir Células Copiadas** quando há algo para colocar), ou clique com o botão direito na célula e escolha esse comando. Uma pequena janela pergunta se as células que estão ali devem **deslocar para a direita** ou **deslocar para baixo** para abrir espaço.

O botão **Inserir** e **Ctrl e a tecla de mais** (também **Ctrl+Shift e a tecla de mais**) também fazem isso, mas só quando fica claro o que você quer:

- quando células foram **recortadas** e o recorte ainda está pendente (a borda tracejada aparece) na planilha de onde vieram, e
- quando células foram **copiadas** e você não alterou nada desde então (nada digitado, nenhuma outra edição, nem Desfazer ou Refazer).

Em qualquer outro caso, o botão e as teclas inserem células em branco, como sempre fizeram. Depois que você fizer uma alteração, a cópia continua disponível para **Colar** e para o comando do menu. **Esc** encerra um recorte ou uma cópia: a borda desaparece, e o botão e as teclas voltam a inserir células em branco. Se as células foram recortadas em uma planilha e você pressiona as teclas em outra, células em branco são inseridas; o comando do menu informa que células recortadas só podem ser colocadas entre outras na própria planilha. Com linhas ou colunas inteiras selecionadas, o comando fica desabilitado, e Inserir adiciona linhas ou colunas como antes.

- Células recortadas são **movidas**: saem do lugar antigo, e o que aponta para elas acompanha. É o mesmo que manter **Shift** pressionado enquanto arrasta a borda da seleção, e é tudo ou nada: se alguma parte for recusada, nada muda.
- Células copiadas são **inseridas** como cópia, da mesma forma que **Colar** as inseriria (conforme indicado em **Definir Colagem Padrão**, com as fórmulas ajustadas para o novo local), e a cópia continua pronta, para que você possa inseri-la de novo em outro lugar.
- As células são inseridas uma única vez, na célula superior esquerda do que você selecionou, mesmo que tenha selecionado mais células do que copiou.
- Se a lista estiver filtrada, as células entram nas linhas que aparecem, como acontece em uma colagem, e o espaço criado inclui as linhas que o filtro oculta entre elas. Uma linha que você ocultou manualmente é tratada como qualquer outra linha.
- Cada operação é um passo que pode ser desfeito. Células recortadas só podem ser colocadas entre outras na planilha de onde vieram; para movê-las para outra planilha, use **Colar**.
- Uma célula não pode ser inserida no próprio caminho (células recortadas de A3:A4 não podem ir entre A3 e A4), e uma planilha protegida recusa a operação. O JeffCrunch informa o motivo e não altera nada.

## Excluir

Selecione o que você quer remover e pressione **Ctrl e a tecla de menos**, ou use **Página Inicial > Excluir**. Com células selecionadas, a janela pergunta se deve **deslocar células para a esquerda**, **deslocar células para cima**, ou excluir a **linha inteira** ou a **coluna inteira**. A exclusão é um único passo, e **Desfazer** recoloca tudo o que foi removido, incluindo as fórmulas.

## O que acompanha

Uma fórmula que aponta para células que se deslocam continua apontando para elas em seus novos lugares. Exemplo: na célula B1 você tem **=SOMA(A1:A5)**. Insira uma linha acima da linha 3, e a fórmula passa a ser **=SOMA(A1:A6)**, porque o intervalo cresceu para incluir a nova linha. Se excluir a linha 3, ela passa a ser **=SOMA(A1:A4)**. Uma fórmula que apontava para uma célula excluída mostra **#REF!**, o que significa que a célula de que ela precisa não existe mais. **Desfazer** traz de volta a célula e a fórmula.

Estes também acompanham as células: fórmulas em todas as planilhas, nomes (como áreas de impressão e títulos de impressão), células mescladas, as células a que se aplicam regras de validação e formatações condicionais, tabelas, gráficos, imagens, anotações, hiperlinks e quebras de página.

## O que não pode ser feito

- Células não podem ser inseridas nem excluídas em parte de uma **fórmula de matriz** ou de uma tabela de dados. Mova-as inteiras ou deixe-as como estão. Uma célula que uma tabela de dados usa como entrada também não pode ser excluída.
- Uma **tabela dinâmica** não pode ser movida nem recortada por enquanto: não é possível inserir ou excluir linhas, colunas ou células onde isso a moveria ou alteraria, e células não podem ser recortadas dela nem colocadas nela. O restante da planilha pode ser alterado normalmente. A tabela dinâmica é modificada pelos comandos da guia **Tabela Dinâmica** (atualizar, alterar fonte de dados, campos e layout), e não digitando em suas células.
- Uma **tabela** não pode ser cortada no meio: você não pode inserir uma coluna dentro dela, nem excluir sua linha de cabeçalho ou todas as suas linhas. Insira ou exclua linhas dentro dela, ou altere as células ao redor.
- Uma **planilha protegida** não pode ter linhas, colunas ou células inseridas ou excluídas. Para isso, escolha Desproteger Planilha na guia Revisão.
- Células no fim da planilha que contenham algo não podem ser empurradas para fora da borda. Exclua-as primeiro.
- Depois de salvar, **Desfazer** ainda volta às operações feitas antes, inclusive às linhas e colunas inseridas (o próximo salvamento as remove novamente). A única coisa que ele não pode devolver são linhas, colunas ou células **excluídas**: o arquivo salvo perdeu o que estava nelas. Por isso, **Desfazer** para na última exclusão salva, e a linha na parte inferior da janela informa isso quando você salva.
- Um passo que move ou exclui tantas células que o **Desfazer** não consegue guardá-lo (mais de cerca de 128 MB) pede confirmação antes, como fazem os programas de planilha que as pessoas conhecem: "Isso não pode ser desfeito." Se você continuar, a operação é feita, e **Desfazer** não volta nem sobre ela nem sobre o que a antecedeu.
