# Gráficos

<!-- keywords: gráfico, gráficos, diagrama, gráfico de colunas, gráfico de barras, gráfico de linhas, gráfico de pizza, rosca, área, dispersão, inserir gráfico, dados do gráfico, editar dados, título do gráfico, legenda, rótulos de dados, tipo de gráfico, formatar gráfico, eixos, título do eixo, linhas de grade, estilo do gráfico, linha de tendência, barras de erro, cores do gráfico, layout rápido, formatar parte do gráfico -->

Um **gráfico** transforma números em uma imagem, como colunas, uma linha ou fatias de uma pizza. Este tópico mostra como inserir um gráfico em uma página, como digitar seus números e como alterar sua aparência.

## Inserir um gráfico em uma página

1. Vá até a página em que o gráfico deve ficar.
2. Clique em **Inserir > Gráfico** e escolha um tipo: coluna, colunas empilhadas, colunas 100% empilhadas, barra, barras empilhadas, linha, linha com marcadores, pizza, rosca, área, área empilhada, dispersão, radar, radar com marcadores, radar preenchido, alta-baixa-fechamento, abertura-alta-baixa-fechamento, volume-alta-baixa-fechamento ou volume-abertura-alta-baixa-fechamento.

Um gráfico de **ações** mostra preços: uma linha para cada categoria, do preço mais alto ao mais baixo, com um pequeno ponto para o fechamento ou, quando há abertura, uma barra entre a abertura e o fechamento (clara para alta, escura para queda). O gráfico é feito com as suas colunas exatamente como estão, da esquerda para a direita, e nada verifica se os números combinam com a posição; por isso, coloque-os nesta ordem: alta, baixa e fechamento; abertura, alta, baixa e fechamento; volume, alta, baixa e fechamento; ou volume, abertura, alta, baixa e fechamento. Se as colunas estiverem em outra ordem, o gráfico ainda é desenhado, mas as marcas têm outro significado (o ponto de fechamento vai para a terceira coluna de preços, e a caixa vai da primeira coluna de preços até a última). Com volume, o volume é representado por colunas lidas no eixo da esquerda, e os preços são lidos em um segundo eixo, à direita. Se o número de colunas não corresponder ao tipo, o JeffPub informa quantas são necessárias e não cria o gráfico.

Um gráfico **empilhado** coloca as séries umas sobre as outras. Um gráfico **100% empilhado** deixa cada pilha com a mesma altura, para que você veja proporções. Um gráfico de **dispersão** posiciona cada ponto onde seus dois números indicam.

O gráfico é inserido com números de exemplo, no centro da página, e a guia **Formatar Gráfico** é aberta. Um gráfico é um objeto como uma imagem ou uma forma: arraste-o para movê-lo, arraste uma alça para mudar seu tamanho (o gráfico é redesenhado para caber, e suas palavras mantêm a forma correta) e gire-o com **Girar** no grupo **Organizar**.

## Digitar seus próprios números

Selecione o gráfico e clique em **Formatar Gráfico > Editar Dados**. Abre-se uma tabela.

- A primeira coluna contém as palavras sob os pontos (as **categorias**), como os meses.
- A primeira linha contém o nome de cada **série**, que é um conjunto de números, como um produto.
- As demais células contêm os números.

Clique em **Adicionar Linha** ou **Adicionar Coluna** para incluir mais, ou em **Remover Última Linha** ou **Remover Última Coluna** para remover. Clique em **OK**. Se uma célula não contiver um número, o gráfico não é alterado e a barra de status informa a linha. Um gráfico pode ter até 200 categorias e 24 séries.

Em um gráfico de dispersão, a primeira coluna contém os números que posicionam os pontos da esquerda para a direita.

## Alterar a aparência

Estes comandos estão na guia **Formatar Gráfico**. Cada um pode ser desfeito em uma única etapa, e o gráfico mantém suas cores e o restante da aparência.

- **Alterar Tipo de Gráfico** transforma o gráfico em outro tipo com os mesmos números. Para mudar para um tipo de ações, as séries do gráfico precisam estar na ordem que esse tipo exige; se não estiverem, o JeffPub informa a ordem e deixa o gráfico como está.
- **Título do Gráfico** mostra ou oculta o título. **Editar Título do Gráfico** altera suas palavras.
- **Legenda** é a chave que indica qual cor corresponde a cada série. Posicione-a à direita, no topo, à esquerda ou embaixo, ou remova-a.
- **Eixos** mostra ou oculta a linha de palavras sob o gráfico (as **categorias**) e a linha de números ao lado (os **valores**), e define um título para cada uma. Um gráfico de pizza não tem eixos.
- **Linhas de Grade** desenha linhas discretas pelo gráfico, para ajudar a acompanhar os números. As linhas **principais** ficam nos números principais do lado, e as linhas **secundárias** ficam entre elas.
- **Estilos de Gráfico** escolhe um de oito estilos gerais.
- **Rótulos de Dados** escreve cada número sobre sua coluna, ponto ou fatia.

## Adicionar linhas de tendência, barras de erro e cores

Selecione o gráfico. Três pequenos botões ficam ao lado dele, e os mesmos comandos estão na guia **Formatar Gráfico**.

- **Partes do Gráfico** (o botão de mais) adiciona ou remove uma parte do gráfico: o título, os rótulos, uma tabela com os números e outras. Sua opção **Linha de Tendência** desenha uma linha pelos pontos que mostra para onde eles estão indo (linear, exponencial e outros tipos), e pode escrever a equação da linha no gráfico. Sua opção **Barras de Erro** desenha uma pequena barra em cada ponto, indicando o quanto o número real pode diferir do exibido: um valor fixo, uma porcentagem ou a dispersão dos números.
- **Layouts** escolhe um de vários posicionamentos prontos de título, legenda e rótulos.
- **Cores** altera as cores em que as séries são desenhadas. **Aparência** (o botão de pincel) escolhe uma aparência geral.
- **Filtros do Gráfico** (o botão de funil) oculta uma série sem excluir seus números.
- **Formatar Gráfico** abre uma janela para a aparência de uma parte: o gráfico inteiro, a área de plotagem ou uma série. **Formatar Eixo Horizontal** e **Formatar Eixo Vertical** definem o intervalo, os passos e o formato numérico de um eixo.
- **Seleção Atual** lista as partes do gráfico: a área do gráfico, a área de plotagem e cada série. Escolha uma, e **Formatar Seleção** abre a janela de formatação dessa parte. **Redefinir para Corresponder ao Estilo** remove o que foi definido na parte escolhida e devolve o que o estilo do gráfico determina para ela. Na área do gráfico, isso significa o preenchimento, a linha e a cor do texto definidos por uma aparência (o texto claro do visual escuro volta à cor de texto do tema). Na área de plotagem, significa o preenchimento e a linha. Em uma série, significa sua cor (a que a escolha Cores ou o estilo do gráfico lhe atribui), seu contorno, seus marcadores conforme o tipo de gráfico e as cores de seus pontos; uma linha suavizada permanece quando o tipo de gráfico é suavizado. Isso não altera títulos, legenda, rótulos, eixos, linhas de grade nem os dados, e não faz nada em uma parte que já corresponde ao estilo. É uma etapa de Desfazer.
- **Dados do Gráfico** e **Trocar Linhas e Colunas** alteram quais números são as séries e quais são as categorias.

Cada um desses comandos pode ser desfeito em uma etapa.

## Salvar, imprimir e PDF

- Um arquivo **.jpub** mantém o gráfico como gráfico, para que você possa abri-lo depois e alterar seus números.
- Um arquivo **.pub** não consegue manter um gráfico que outros programas abrem como gráfico. O JeffPub salva o gráfico como uma imagem dele e informa quantos gráficos foram salvos dessa forma. Mantenha uma cópia em **.jpub** se quiser alterar os números depois.
- **PDF** e impressão desenham o gráfico como linhas, formas e texto real, não como imagem, por isso ele permanece nítido em qualquer tamanho.
