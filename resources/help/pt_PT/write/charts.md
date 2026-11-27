# Gráficos

<!-- keywords: gráfico, gráficos, diagrama, gráfico de colunas, gráfico de barras, gráfico de linhas, gráfico circular, gráfico de anel, área, dispersão, inserir gráfico, dados do gráfico, editar dados, título do gráfico, legenda, etiquetas de dados, tipo de gráfico, formatar gráfico, eixos, título do eixo, linhas de grelha, estilo do gráfico, elementos do gráfico, esquemas, cores, aspetos, dados do gráfico, trocar linhas e colunas, tipo de gráfico, formatar eixo, barras de erro, linha de tendência, filtros do gráfico, elementos do gráfico</span>

Um **gráfico** transforma números numa imagem, como colunas, uma linha ou fatias de um círculo. Este tópico mostra como inserir um gráfico num documento, como escrever os seus números e como alterar o seu aspeto.

## Inserir um gráfico

1. Coloque o cursor onde deve ficar o gráfico.
2. Clique em **Inserir > Gráfico** e escolha um tipo: colunas, colunas empilhadas, colunas empilhadas a 100%, barras, barras empilhadas, linhas, linhas com marcadores, circular, anel, área, área empilhada, dispersão, radar, radar com marcadores, radar preenchido, máximo-mínimo-fecho, abertura-máximo-mínimo-fecho, volume-máximo-mínimo-fecho ou volume-abertura-máximo-mínimo-fecho.

Um gráfico de **cotações** mostra preços: uma linha para cada categoria, desde o preço mais alto até ao mais baixo, com um pequeno ponto para o fecho ou, quando há abertura, uma barra entre a abertura e o fecho (clara se subiu, escura se desceu). O gráfico é feito com as suas colunas exatamente como estão, da esquerda para a direita, e nada verifica se os números correspondem à posição em que estão. Por isso, coloque-os nesta ordem: máximo, mínimo e fecho; abertura, máximo, mínimo e fecho; volume, máximo, mínimo e fecho; ou volume, abertura, máximo, mínimo e fecho. Se as colunas estiverem noutra ordem, o gráfico é desenhado na mesma, mas as marcas têm outro significado (o ponto do fecho passa para a terceira coluna de preços, e a caixa vai da primeira coluna de preços até à última). Com um volume, o volume é representado por colunas no eixo da esquerda e os preços são representados num segundo eixo, à direita. Se o número de colunas não corresponder ao tipo, o JeffWrite indica quantas são necessárias e não cria nenhum gráfico.

Um gráfico **empilhado** sobrepõe as séries umas às outras. Um gráfico **empilhado a 100%** faz com que cada pilha tenha a mesma altura, para que veja as proporções. Um gráfico de **dispersão** coloca cada ponto onde indicarem os seus dois números.

O gráfico entra com números de exemplo, alinhado com o texto, como uma letra capitular. O separador **Formatar Gráfico** abre-se.

## Escrever os seus próprios números

Selecione o gráfico e clique em **Formatar Gráfico > Editar Dados**. Abre-se uma tabela.

- A primeira coluna tem as palavras debaixo dos pontos (as **categorias**), por exemplo os meses.
- A primeira linha tem o nome de cada **série**, ou seja, um conjunto de números, por exemplo um produto.
- As restantes células têm os números.

Clique em **Adicionar Linha** ou **Adicionar Coluna** para acrescentar, ou em **Remover Última Linha** ou **Remover Última Coluna** para retirar. Clique em **OK**. Se uma célula não tiver um número, o gráfico não é alterado e a barra de estado indica em que linha está o problema. Um gráfico pode ter até 200 categorias e 24 séries.

Num gráfico de dispersão, a primeira coluna tem os números que colocam os pontos da esquerda para a direita.

## Gráficos guardados por outro programa

Alguns gráficos guardados por outros programas mantêm os seus números num pequeno **livro de cálculo** (um ficheiro de folha de cálculo) guardado dentro do seu ficheiro. Selecione um gráfico destes e clique em **Formatar Gráfico > Editar Dados**. A tabela mostra as células desse livro que o gráfico lê. Altere um número, um nome debaixo de um ponto ou o nome de uma série, e clique em **OK**. A alteração passa para o livro e para o gráfico ao mesmo tempo, como um só passo a anular, para que os outros programas também mostrem os novos números.

Não é possível acrescentar nem retirar uma linha ou uma coluna a este tipo de gráfico, e **Dados do Gráfico** e **Trocar Linhas e Colunas** ficam a cinzento, porque o gráfico está ligado a essas células. O mesmo acontece quando o gráfico lê células de outro ficheiro, num bloco que não é uma única coluna nem uma única linha, ou em células que contêm uma fórmula, uma data ou palavras onde devia haver um número. O comando fica a cinzento e a sua sugestão indica o motivo.

## Alterar o aspeto

Estes comandos estão no separador **Formatar Gráfico**. Cada um é um único passo a anular, e o gráfico mantém as suas cores e o restante aspeto.

- **Alterar Tipo de Gráfico** transforma o gráfico noutro tipo com os mesmos números. Para mudar para um tipo de cotações, as séries do gráfico têm de estar na ordem que esse tipo exige; se não estiverem, o JeffWrite indica a ordem e deixa o gráfico como está.
- **Título do Gráfico** mostra ou oculta o título. **Editar Título do Gráfico** altera as suas palavras.
- **Legenda** é a chave que indica qual a cor de cada série. Coloque-a à direita, em cima, à esquerda ou em baixo, ou retire-a.
- **Eixos** mostra ou oculta a linha de palavras debaixo do gráfico (as **categorias**) e a linha de números ao lado (os **valores**), e define um título para cada uma. Um gráfico circular não tem eixos.
- **Linhas de Grelha** desenha linhas ténues atravessadas no gráfico, para ajudar a seguir os números. As linhas **principais** ficam nos números principais do lado, e as linhas **secundárias** ficam entre elas.
- **Estilos de Gráfico** escolhe um de oito estilos gerais.
- **Etiquetas de Dados** escreve cada número na sua coluna, no seu ponto ou na sua fatia.
- **Quebrar Texto**, **Texto Alternativo** e **Altura** e **Largura** funcionam como nas imagens. Consulte [Adicionar e alterar imagens](pictures).

## O separador Gráfico

Ao selecionar um gráfico, abre-se o separador **Gráfico**, ao lado do separador **Formatar Gráfico**. Tudo o que está nele altera o gráfico num único passo que pode anular.

- **Elementos do Gráfico** acrescenta, retira ou posiciona uma parte do gráfico: **Eixos**, **Títulos dos Eixos**, **Título do Gráfico**, **Etiquetas de Dados** (o que mostram, onde ficam e o separador entre elas), **Linhas de Grelha**, **Legenda**, **Barras de Erro** (barras curtas que mostram quanto um número pode estar errado), **Linhas** (linhas de queda, linhas máximo-mínimo e barras ascendentes e descendentes), **Tabela de Dados** (uma grelha com os números debaixo do gráfico, com uma coluna para cada categoria; a sua lista liga e desliga a tabela e define as linhas entre as linhas, as linhas entre as colunas, o contorno e as chaves da legenda, que mostram a cor de cada série ao lado do nome; é oferecida para gráficos de colunas, de linhas e de área) e **Linha de Tendência** (uma linha que mostra para onde os números se dirigem, com a sua equação, se a quiser).
- **Esquemas** define várias partes de uma só vez, como o título, a legenda, as etiquetas e as linhas de grelha.
- **Cores** dá às séries um novo conjunto de cores. **Aspetos** dá ao gráfico inteiro um aspeto, como simples, limpo, escuro ou suave.
- **Trocar Linhas e Colunas** inverte os números: as séries passam a ser as palavras debaixo dos pontos, e essas palavras passam a ser as séries.
- **Dados do Gráfico** lista as séries. Marque as que deseja desenhar, altere os seus nomes e mova-as para cima ou para baixo.
- **Editar Dados** abre a tabela de números.
- **Tipo de Gráfico** mostra uma imagem de cada tipo. Escolha um tipo para o gráfico inteiro, ou um tipo para cada série, por exemplo colunas com uma linha por cima.
- **Formatar Elemento do Gráfico** define o preenchimento e a linha da área do gráfico, da área de traçado (a caixa onde ficam as marcas) ou de uma série. Para uma linha, define também se é suavizada e que marca fica em cada ponto.
- **Selecção Atual** lista as partes do gráfico: a área do gráfico, a área de traçado e cada série. Escolha uma, e **Formatar Seleção** abre a janela de formatação nessa parte. **Repor para Corresponder ao Estilo** retira o que foi definido na parte escolhida e devolve o que o estilo do gráfico lhe atribui. Para a área do gráfico, isso são o seu preenchimento, a sua linha e a cor do texto definida por um aspeto (o texto claro do aspeto escuro volta à cor de texto do tema). Para a área de traçado, são o seu preenchimento e a sua linha. Para uma série, é a sua cor (a que lhe é dada pela escolha de Cores ou pelo estilo do gráfico), o seu contorno, as suas marcas tal como o tipo de gráfico as tem e as cores dos seus pontos; uma linha suavizada mantém-se quando o tipo de gráfico é suavizado. Não altera títulos, a legenda, etiquetas, eixos, linhas de grelha nem os dados, e não faz nada a uma parte que já corresponda. É um passo a anular.
- **Formatar Eixo Horizontal** e **Formatar Eixo Vertical** definem os números mais baixos e mais altos, o tamanho de cada passo, a ordem, o ponto onde o outro eixo cruza, o aspeto dos números e as marcas de graduação.

## Os botões ao lado de um gráfico

Três pequenos botões ficam à direita de um gráfico selecionado.

- **Elementos do Gráfico** (o sinal de mais) abre o mesmo menu que **Elementos do Gráfico** no separador.
- **Cores e Estilos do Gráfico** (o pincel) lista os conjuntos de cores e os aspetos.
- **Filtros do Gráfico** (o funil) lista as séries. Desmarque uma série para a deixar fora do gráfico. O ficheiro continua a guardá-la, por isso volta a aparecer quando a marcar de novo. Um gráfico mantém sempre pelo menos uma série visível.

## Guardar um gráfico num ficheiro OpenDocument

Se guardar o documento como ficheiro de **texto OpenDocument** (`.odt`), cada gráfico é guardado como um objeto próprio e volta a abrir-se como um gráfico que pode alterar. O tipo, os números, o título, a legenda, as etiquetas, as cores, os eixos, as linhas de grelha, até quatro linhas de tendência para cada série e as barras de erro mantêm-se. Algumas partes não cabem nesse formato, como a tabela de dados, as linhas de queda, as linhas máximo-mínimo, as barras ascendentes e descendentes e as barras de erro cujas quantidades escreveu. O JeffWrite indica o que ficou de fora ao guardar.

## Gráficos noutros documentos

O JeffWrite mostra os gráficos que um documento contém e mantém-nos tal como estavam. Pode alterar o tipo, o título, a legenda e as etiquetas de um gráfico que o JeffWrite consiga escrever por inteiro. Se um gráfico tiver partes que o JeffWrite não altera, como efeitos especiais, o programa indica-o e deixa o gráfico como está. Se um gráfico lê os seus números de uma folha de cálculo guardada com ele, pode alterar tudo exceto os números.
