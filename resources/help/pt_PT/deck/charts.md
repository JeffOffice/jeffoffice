# Gráficos

<!-- keywords: gráfico, gráficos, diagrama, gráfico de colunas, gráfico de barras, gráfico de linhas, gráfico circular, anel, área, dispersão, inserir gráfico, dados do gráfico, editar dados, título do gráfico, legenda, etiquetas de dados, tipo de gráfico, formatar gráfico, eixos, título do eixo, linhas de grelha, estilo de gráfico, partes do gráfico, esquemas, cores, aspetos, dados do gráfico, trocar linhas e colunas, tipo de gráfico, formatar eixo, barras de erro, linha de tendência, filtros do gráfico, elementos do gráfico -->

Um **gráfico** transforma números numa imagem, como colunas, uma linha ou fatias de um círculo. Este tópico mostra como colocar um gráfico num diapositivo, como escrever os seus números e como alterar o seu aspeto.

## Colocar um gráfico num diapositivo

1. Aceda ao diapositivo onde o gráfico deve ficar.
2. Clique em **Inserir > Gráfico** e escolha um tipo: colunas, colunas empilhadas, colunas empilhadas a 100%, barras, barras empilhadas, linhas, linhas com marcadores, circular, anel, área, área empilhada, dispersão, radar, radar com marcadores, radar preenchido, máximo-mínimo-fecho, abertura-máximo-mínimo-fecho, volume-máximo-mínimo-fecho ou volume-abertura-máximo-mínimo-fecho.

Um gráfico de **cotações** mostra preços: uma linha para cada categoria, desde o preço mais alto até ao mais baixo, com um pequeno ponto para o fecho or, quando existe abertura, uma barra entre a abertura e o fecho (clara para uma subida, escura para uma descida). O gráfico é feito a partir das suas colunas exatamente pela ordem em que estão, da esquerda para a direita, e nada verifica se os números correspondem à sua posição; por isso, coloque-os nesta ordem: máximo, mínimo e fecho; abertura, máximo, mínimo e fecho; volume, máximo, mínimo e fecho; ou volume, abertura, máximo, mínimo e fecho. Se as colunas estiverem noutra ordem, o gráfico é desenhado na mesma, mas as marcas têm outro significado (o ponto do fecho fica na terceira coluna de preços, e a caixa vai da primeira coluna de preços até à última). Com volume, o volume é representado por colunas lidas no eixo esquerdo, e os preços são lidos num segundo eixo, à direita. Se o número de colunas não corresponder ao tipo escolhido, o JeffDeck indica quantas são necessárias e não cria nenhum gráfico.

Um gráfico **empilhado** coloca as séries umas sobre as outras. Um gráfico **empilhado a 100%** faz com que cada pilha tenha a mesma altura, para ver as proporções. Um gráfico de **dispersão** coloca cada ponto onde os seus dois números indicarem.

O gráfico surge com números de exemplo, no centro do diapositivo, e abre-se o separador **Formatar Gráfico**. Um gráfico é um objeto como uma imagem ou uma forma: arraste-o para o mover, arraste uma alça para alterar o seu tamanho (o gráfico é desenhado de novo para caber, pelo que as suas palavras mantêm a forma correta), e rode-o com **Rodar** no grupo **Desenho** do separador **Base**.

## Escrever os seus próprios números

Selecione o gráfico e clique em **Formatar Gráfico > Editar Dados**. Abre-se uma tabela.

- A primeira coluna contém as palavras por baixo dos pontos (as **categorias**), como os meses.
- A primeira linha contém o nome de cada **série**, que é um conjunto de números, como um produto.
- As restantes células contêm os números.

Clique em **Adicionar Linha** ou **Adicionar Coluna** para acrescentar, ou em **Remover Última Linha** ou **Remover Última Coluna** para retirar. Clique em **OK**. Se uma célula não tiver um número, o gráfico não é alterado e a barra de estado indica a linha em causa. Um gráfico pode ter até 200 categorias e 24 séries.

Num gráfico de dispersão, a primeira coluna contém os números que colocam os pontos da esquerda para a direita.

## Gráficos guardados por outro programa

Alguns gráficos guardados por outros programas mantêm os seus números num pequeno **livro de cálculo** (um ficheiro de folha de cálculo) guardado dentro do seu ficheiro. Selecione um desses gráficos e clique em **Formatar Gráfico > Editar Dados**. A tabela mostra as células desse livro que o gráfico lê. Altere um número, um nome por baixo de um ponto ou o nome de uma série, e clique em **OK**. A alteração passa para o livro e para o gráfico ao mesmo tempo, como um único passo a anular, para que outros programas também mostrem os novos números.

Não é possível acrescentar nem retirar uma linha ou uma coluna deste tipo de gráfico, e **Dados do Gráfico** e **Trocar Linhas e Colunas** ficam a cinzento, porque o gráfico está ligado a essas células. O mesmo acontece quando o gráfico lê células de outro ficheiro, num bloco que não é uma coluna nem uma linha, ou em células que contêm uma fórmula, uma data ou palavras onde deveria haver um número. O comando fica a cinzento, e a sua dica indica o motivo.

## Alterar o aspeto

Estes comandos estão no separador **Formatar Gráfico**. Cada um é um único passo a anular, e o gráfico mantém as suas cores e o restante aspeto.

- **Alterar Tipo de Gráfico** transforma o gráfico noutro tipo, com os mesmos números. Para mudar para um tipo de cotações, as séries do gráfico têm de estar na ordem que esse tipo exige; se não estiverem, o JeffDeck indica a ordem e deixa o gráfico como está.
- **Título do Gráfico** mostra ou oculta o título. **Editar Título do Gráfico** altera as suas palavras.
- **Legenda** é a chave que indica a cor de cada série. Coloque-a à direita, em cima, à esquerda ou em baixo, ou retire-a.
- **Eixos** mostra ou oculta a linha de palavras por baixo do gráfico (as **categorias**) e a linha de números ao lado (os **valores**), e define um título para cada um. Um gráfico circular não tem eixos.
- **Linhas de Grelha** desenha linhas ténues atravessadas no gráfico, para ajudar a seguir os números. As linhas **principais** ficam nos números principais do lado, e as linhas **secundárias** ficam entre elas.
- **Estilos de Gráfico** escolhe um de oito estilos gerais.
- **Etiquetas de Dados** escreve cada número na sua coluna, ponto ou fatia.

## O separador Gráfico

Selecione um gráfico e abre-se o separador **Gráfico**, ao lado do separador **Formatar Gráfico**. Tudo o que está nele altera o gráfico num passo que pode anular.

- **Partes do Gráfico** acrescenta, retira ou posiciona uma parte do gráfico: **Eixos**, **Títulos dos Eixos**, **Título do Gráfico**, **Etiquetas de Dados** (o que mostram, onde ficam e o separador entre elas), **Linhas de Grelha**, **Legenda**, **Barras de Erro** (barras curtas que mostram o quanto um número pode variar), **Linhas** (linhas de queda, linhas máximo-mínimo e barras ascendentes e descendentes), **Tabela de Dados** (uma grelha com os números por baixo do gráfico, com uma coluna para cada categoria; a sua lista liga e desliga a tabela, e define as linhas entre linhas, as linhas entre colunas, o contorno e as chaves da legenda, que mostram a cor de cada série junto ao nome; é oferecida para gráficos de colunas, de linhas e de áreas) e **Linha de Tendência** (uma linha que mostra para onde os números se dirigem, com a respetiva equação, se a quiser).
- **Esquemas** define várias partes de uma só vez, como o título, a legenda, as etiquetas e as linhas de grelha.
- **Cores** dá às séries um novo conjunto de cores. **Aspetos** dá ao gráfico inteiro um aspeto, como simples, limpo, escuro ou suave.
- **Trocar Linhas e Colunas** inverte os números: as séries passam a ser as palavras por baixo dos pontos, e essas palavras passam a ser as séries.
- **Dados do Gráfico** lista as séries. Marque as que quer desenhar, altere os seus nomes e mova-as para cima ou para baixo.
- **Editar Dados** abre a tabela de números.
- **Tipo de Gráfico** mostra uma imagem de cada tipo. Escolha um tipo para todo o gráfico, ou um tipo para cada série, como colunas com uma linha por cima.
- **Formatar Elemento do Gráfico** define o preenchimento e a linha da área do gráfico, da área de traçado (a caixa onde se encontram as marcas) ou de uma série. Para uma linha, define também se é suavizada e que marca surge em cada ponto.
- **Elemento Atual** lista as partes do gráfico: a área do gráfico, a área de traçado e cada série. Escolha uma e **Formatar Seleção** abre a janela de formatação nessa parte. **Repor para Corresponder ao Estilo** retira o que foi definido na parte escolhida e devolve o que o estilo do gráfico lhe atribui. Para a área do gráfico, isso é o seu preenchimento, a sua linha e a cor do texto definida por um aspeto (o texto claro do aspeto escuro volta à cor de texto do tema). Para a área de traçado, são o seu preenchimento e a sua linha. Para uma série, é a sua cor (a que a escolha de Cores ou o estilo do gráfico lhe atribui), o seu contorno, as suas marcas tal como o tipo de gráfico as tem, e as cores dos seus pontos; uma linha suavizada mantém-se quando o tipo de gráfico é um tipo suavizado. Não altera títulos, a legenda, etiquetas, eixos, linhas de grelha nem os dados, e não faz nada a uma parte que já corresponda ao estilo. É um passo de Anular.
- **Formatar Eixo Horizontal** e **Formatar Eixo Vertical** definem os números mais baixos e mais altos, o tamanho de cada passo, a ordem, o ponto onde o outro eixo cruza, o aspeto dos números e as marcas de graduação.

## Os botões ao lado de um gráfico

Três pequenos botões ficam à direita de um gráfico selecionado.

- **Elementos do Gráfico** (o sinal de mais) abre o mesmo menu que **Partes do Gráfico**.
- **Cores e Estilos do Gráfico** (o pincel) lista os conjuntos de cores e os aspetos.
- **Filtros do Gráfico** (o funil) lista as séries. Desmarque uma série para a deixar de fora da imagem. O ficheiro continua a guardá-la, por isso ela volta quando a voltar a marcar. Um gráfico mantém sempre pelo menos uma série visível.

## Gráficos noutras apresentações

Um gráfico num ficheiro **.pptx** abre-se como um gráfico que pode alterar, quando contém os seus números dentro do próprio gráfico. Se um gráfico lê os seus números de uma folha de cálculo guardada com ele, pode alterar tudo exceto os números. Um gráfico com partes que o JeffDeck não altera, como efeitos especiais, é mantido tal como está.

## Gravar, imprimir e PDF

- Um ficheiro **.pptx** guarda o gráfico como gráfico, por isso abre-se como gráfico aqui e noutros programas.
- Um ficheiro **.odp** não tem lugar para um gráfico que se abra como tal aqui. O JeffDeck guarda o gráfico como imagem, e a mensagem após a gravação indica-o.
- **PDF** e a impressão desenham o gráfico como linhas, formas e texto real, e não como imagem, por isso mantém-se nítido em qualquer tamanho.
