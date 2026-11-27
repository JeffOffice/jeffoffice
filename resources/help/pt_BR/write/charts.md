# Gráficos
<!-- keywords: gráfico, gráficos, diagrama, gráfico de colunas, gráfico de barras, gráfico de linhas, gráfico de pizza, rosca, área, dispersão, inserir gráfico, dados do gráfico, editar dados, título do gráfico, legenda, rótulos de dados, tipo de gráfico, formatar gráfico, eixos, título do eixo, linhas de grade, estilo do gráfico, partes do gráfico, layouts, cores, aparências, dados do gráfico, trocar linhas e colunas, tipo de gráfico, formatar eixo, barras de erro, linha de tendência, filtros do gráfico, elementos do gráfico -->

Um **gráfico** transforma números em uma imagem, como colunas, uma linha ou fatias de uma pizza. Este tópico mostra como inserir um gráfico em um documento, como digitar os seus números e como alterar a sua aparência.

## Inserir um gráfico

1. Coloque o cursor onde o gráfico deve ficar.
2. Clique em **Inserir > Gráfico** e escolha um tipo: coluna, coluna empilhada, coluna 100% empilhada, barra, barra empilhada, linha, linha com marcadores, pizza, rosca, área, área empilhada, dispersão, radar, radar com marcadores, radar preenchido, alta-baixa-fechamento, abertura-alta-baixa-fechamento, volume-alta-baixa-fechamento ou volume-abertura-alta-baixa-fechamento.

Um gráfico de **ações** mostra preços: uma linha para cada categoria, do preço mais alto até o mais baixo, com um pequeno ponto para o fechamento ou, quando há abertura, uma barra entre a abertura e o fechamento (clara para alta, escura para queda). O gráfico é feito com as suas colunas exatamente como estão, da esquerda para a direita, e nada verifica se os números correspondem à posição em que estão; por isso, coloque-os nesta ordem: alta, baixa e fechamento; abertura, alta, baixa e fechamento; volume, alta, baixa e fechamento; ou volume, abertura, alta, baixa e fechamento. Se as colunas estiverem em outra ordem, o gráfico ainda será desenhado, mas as marcas terão outro significado (o ponto de fechamento fica na terceira coluna de preços, e a caixa vai da primeira coluna de preços até a última). Com volume, o volume é representado por colunas lidas no eixo da esquerda, e os preços são lidos em um segundo eixo, à direita. Se o número de colunas não corresponder ao tipo, o JeffOffice informa quantas são necessárias e não cria o gráfico.

Um gráfico **empilhado** sobrepõe as séries umas sobre as outras. Um gráfico **100% empilhado** faz com que cada pilha tenha a mesma altura, para que você veja proporções. Um gráfico de **dispersão** posiciona cada ponto onde indicam os seus dois números.

O gráfico entra com números de exemplo, alinhado com o texto, como uma letra capitular. A guia **Formatar Gráfico** é aberta.

## Digitar os seus próprios números

Selecione o gráfico e clique em **Formatar Gráfico > Editar Dados**. Uma tabela é aberta.

- A primeira coluna contém as palavras abaixo dos pontos (as **categorias**), como os meses.
- A primeira linha contém o nome de cada **série**, que é um conjunto de números, como um produto.
- As demais células contêm os números.

Clique em **Adicionar Linha** ou **Adicionar Coluna** para incluir mais, ou em **Remover Última Linha** ou **Remover Última Coluna** para tirar. Clique em **OK**. Se uma célula não tiver um número, o gráfico não é alterado, e a barra de status informa em qual linha está o problema. Um gráfico pode ter até 200 categorias e 24 séries.

Em um gráfico de dispersão, a primeira coluna contém os números que posicionam os pontos da esquerda para a direita.

## Gráficos salvos por outro programa

Alguns gráficos salvos por outros programas guardam seus números em uma pequena **pasta de trabalho** (um arquivo de planilha) armazenada dentro do seu arquivo. Selecione esse gráfico e clique em **Formatar Gráfico > Editar Dados**. A tabela mostra as células dessa pasta de trabalho que o gráfico lê. Altere um número, um nome abaixo de um ponto ou o nome de uma série e clique em **OK**. A alteração vai para a pasta de trabalho e para o gráfico ao mesmo tempo, como uma única etapa a desfazer, para que os outros programas também mostrem os novos números.

Não é possível adicionar nem remover uma linha ou uma coluna desses gráficos, e **Dados do Gráfico** e **Trocar Linhas e Colunas** ficam desativados, porque o gráfico está ligado a essas células. O mesmo ocorre quando o gráfico lê células de outro arquivo, em um bloco que não é uma única coluna ou uma única linha, ou em células que contêm uma fórmula, uma data ou palavras onde deveria haver um número. O comando fica desativado, e a sua dica informa o motivo.

## Alterar a aparência

Estes comandos estão na guia **Formatar Gráfico**. Cada um é uma única etapa a desfazer, e o gráfico mantém as suas cores e o restante da aparência.

- **Alterar Tipo de Gráfico** transforma o gráfico em outro tipo com os mesmos números. Para mudar para um tipo de ações, as séries do gráfico precisam estar na ordem que esse tipo exige; se não estiverem, o JeffOffice informa a ordem e deixa o gráfico como está.
- **Título do Gráfico** mostra ou oculta o título. **Editar Título do Gráfico** altera as suas palavras.
- **Legenda** é a chave que indica qual cor corresponde a cada série. Coloque-a à direita, no topo, à esquerda ou embaixo, ou remova-a.
- **Eixos** mostra ou oculta a linha de palavras abaixo do gráfico (as **categorias**) e a linha de números ao lado (os **valores**), e define um título para cada uma. Um gráfico de pizza não tem eixos.
- **Linhas de Grade** desenha linhas discretas pelo gráfico para ajudar a acompanhar os números. As linhas **principais** ficam nos números principais do lado, e as linhas **secundárias** ficam entre elas.
- **Estilos de Gráfico** escolhe um entre oito estilos gerais.
- **Rótulos de Dados** escreve cada número sobre a sua coluna, ponto ou fatia.
- **Quebra de Texto**, **Texto Alternativo** e **Altura** e **Largura** funcionam como funcionam para imagens. Veja [Adicionar e alterar imagens](pictures).

## A guia Gráfico

Selecione um gráfico e a guia **Gráfico** é aberta ao lado da guia **Formatar Gráfico**. Tudo nela altera o gráfico em uma etapa que você pode desfazer.

- **Partes do Gráfico** adiciona, remove ou posiciona uma parte do gráfico: **Eixos**, **Títulos dos Eixos**, **Título do Gráfico**, **Rótulos de Dados** (o que mostram, onde ficam e a marca entre eles), **Linhas de Grade**, **Legenda**, **Barras de Erro** (barras curtas que mostram o quanto um número pode ter de variação), **Linhas** (linhas de queda, linhas de alta-baixa e barras de alta e baixa) , **Tabela de Dados** (uma grade com os números abaixo do gráfico, com uma coluna para cada categoria; sua lista liga e desliga a tabela e define as linhas entre as linhas, as linhas entre as colunas, o contorno e as chaves de legenda, que mostram a cor de cada série ao lado do seu nome; é oferecida para gráficos de colunas, de linhas e de áreas) e **Linha de Tendência** (uma linha que mostra para onde os números estão indo, com a sua equação, se você quiser).
- **Layouts** define várias partes de uma vez, como o título, a legenda, os rótulos e as linhas de grade.
- **Cores** dá às séries um novo conjunto de cores. **Aparências** dá ao gráfico inteiro um visual, como simples, limpo, escuro ou suave.
- **Trocar Linhas e Colunas** inverte os números: as séries passam a ser as palavras abaixo dos pontos, e essas palavras passam a ser as séries.
- **Dados do Gráfico** lista as séries. Marque as que devem ser desenhadas, altere os seus nomes e mova-as para cima ou para baixo.
- **Editar Dados** abre a tabela de números.
- **Tipo de Gráfico** mostra uma imagem de cada tipo. Escolha um tipo para o gráfico inteiro ou um tipo para cada série, como colunas com uma linha por cima.
- **Formatar Parte do Gráfico** define o preenchimento e a linha da página do gráfico, da área de plotagem (a caixa onde ficam as marcas) ou de uma série. Para uma linha, também define se ela é suavizada e qual marca fica em cada ponto.
- **Seleção Atual** lista as partes do gráfico: a área do gráfico, a área de plotagem e cada série. Escolha uma, e **Formatar Seleção** abre a janela de formatação nessa parte. **Redefinir para Corresponder ao Estilo** remove o que foi definido na parte escolhida e devolve o que o estilo do gráfico lhe atribui. Para a área do gráfico, isso significa o seu preenchimento, a sua linha e a cor do texto definida por uma aparência (o texto claro da aparência escura volta à cor de texto do tema). Para a área de plotagem, são o seu preenchimento e a sua linha. Para uma série, é a sua cor (a que a escolha de Cores ou o estilo do gráfico lhe atribui), o seu contorno, as suas marcas como o tipo de gráfico as apresenta, e as cores dos seus pontos; uma linha suavizada permanece suavizada quando o tipo de gráfico é suavizado. Não altera títulos, legenda, rótulos, eixos, linhas de grade nem os dados, e não faz nada numa parte que já corresponde ao estilo. É uma etapa de Desfazer.
- **Formatar Eixo Horizontal** e **Formatar Eixo Vertical** definem os números mais baixo e mais alto, o tamanho de cada passo, a ordem, o ponto em que o outro eixo cruza, a aparência dos números e as marcas de divisão.

## Os botões ao lado de um gráfico

Três pequenos botões ficam à direita de um gráfico selecionado.

- **Elementos do Gráfico** (o sinal de mais) abre o mesmo menu de **Partes do Gráfico**.
- **Cores e Estilos do Gráfico** (o pincel) lista os conjuntos de cores e as aparências.
- **Filtros do Gráfico** (o funil) lista as séries. Desmarque uma série para deixá-la fora da imagem. O arquivo continua guardando-a, e ela volta quando você a marcar de novo. Um gráfico sempre mantém pelo menos uma série visível.

## Salvar um gráfico em um arquivo OpenDocument

Se você salvar o documento como um arquivo **Texto OpenDocument** (`.odt`), cada gráfico é gravado como um objeto próprio e, ao ser aberto de novo, continua sendo um gráfico que você pode alterar. O tipo, os números, o título, a legenda, os rótulos, as cores, os eixos, as linhas de grade, até quatro linhas de tendência para cada série e as barras de erro são mantidos. Algumas partes não cabem nesse formato, como a tabela de dados, as linhas de queda, as linhas de alta-baixa, as barras de alta e baixa e as barras de erro cujas quantidades você digitou. O JeffWrite lista o que ficou de fora ao salvar.

## Gráficos em outros documentos

O JeffWrite mostra os gráficos que um documento contém e os mantém como estavam. Ele pode alterar o tipo, o título, a legenda e os rótulos de um gráfico que consegue gravar por inteiro. Se um gráfico tiver partes que o JeffWrite não altera, como efeitos especiais, ele informa isso e deixa o gráfico como está. Se um gráfico lê os seus números de uma planilha guardada junto com ele, você pode alterar tudo, exceto os números.
