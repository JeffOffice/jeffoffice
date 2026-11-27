# Gráficos

<!-- keywords: gráfico, gráficos, chart, diagrama, gráfico de colunas, gráfico de barras, gráfico de linhas, gráfico de pizza, gráfico de rosca, área, dispersão, inserir gráfico, dados do gráfico, editar dados, título do gráfico, legenda, rótulos de dados, tipo de gráfico, formatar gráfico, eixos, título do eixo, linhas de grade, estilo do gráfico, partes do gráfico, layouts, cores, aparências, dados do gráfico, trocar linhas e colunas, tipo de gráfico, formatar eixo, barras de erro, linha de tendência, filtros do gráfico, elementos do gráfico -->

Um **gráfico** transforma números em uma imagem, como colunas, uma linha ou fatias de uma pizza. Este tópico mostra como colocar um gráfico em um slide, como digitar os números dele e como mudar a aparência.

## Colocar um gráfico em um slide

1. Vá para o slide em que o gráfico deve ficar.
2. Clique em **Inserir > Gráfico** e escolha um tipo: coluna, coluna empilhada, coluna 100% empilhada, barra, barra empilhada, linha, linha com marcadores, pizza, rosca, área, área empilhada, dispersão, radar, radar com marcadores, radar preenchido, alta-baixa-fechamento, abertura-alta-baixa-fechamento, volume-alta-baixa-fechamento ou volume-abertura-alta-baixa-fechamento.

Um gráfico de **ações** mostra preços: uma linha para cada categoria, do preço mais alto ao mais baixo, com um pequeno ponto para o fechamento; quando há abertura, uma barra entre a abertura e o fechamento (clara para alta, escura para queda). O gráfico é feito com as suas colunas exatamente na ordem em que estão, da esquerda para a direita, e nada verifica se os números combinam com a posição deles; por isso, coloque-os nesta ordem: alta, baixa e fechamento; abertura, alta, baixa e fechamento; volume, alta, baixa e fechamento; ou volume, abertura, alta, baixa e fechamento. Se as colunas estiverem em outra ordem, o gráfico ainda é desenhado, mas as marcas têm outro significado (o ponto de fechamento vai para a terceira coluna de preço, e a caixa vai da primeira coluna de preço até a última). Com volume, o volume é lido como colunas no eixo da esquerda, e os preços são lidos em um segundo eixo, à direita. Se o número de colunas não corresponder ao tipo, o JeffDeck informa quantas são necessárias e não cria o gráfico.

Um gráfico **empilhado** coloca as séries uma sobre a outra. Um gráfico **100% empilhado** deixa cada pilha com a mesma altura, para que você veja proporções. Um gráfico de **dispersão** posiciona cada ponto onde seus dois números indicam.

O gráfico aparece com números de exemplo, no centro do slide, e a guia **Formatar Gráfico** é aberta. Um gráfico é um objeto como uma imagem ou uma forma: arraste-o para movê-lo, arraste uma alça para mudar o tamanho (o gráfico é desenhado de novo para caber, e suas palavras continuam na forma certa) e gire-o com **Girar** no grupo **Desenho** da guia **Página Inicial**.

## Digitar os seus próprios números

Selecione o gráfico e clique em **Formatar Gráfico > Editar Dados**. Abre-se uma tabela.

- A primeira coluna guarda as palavras sob os pontos (as **categorias**), como os meses.
- A primeira linha guarda o nome de cada **série**, que é um conjunto de números, como um produto.
- As demais células guardam os números.

Clique em **Adicionar Linha** ou **Adicionar Coluna** para incluir mais, ou em **Remover Última Linha** ou **Remover Última Coluna** para tirar. Clique em **OK**. Se uma célula não contiver um número, o gráfico não é alterado e a barra de status informa em qual linha está o problema. Um gráfico pode ter até 200 categorias e 24 séries.

Em um gráfico de dispersão, a primeira coluna guarda os números que posicionam os pontos da esquerda para a direita.

## Gráficos salvos por outro programa

Alguns gráficos salvos por outros programas guardam seus números em uma pequena **pasta de trabalho** (um arquivo de planilha) armazenada dentro do seu arquivo. Selecione um gráfico assim e clique em **Formatar Gráfico > Editar Dados**. A tabela mostra as células dessa pasta de trabalho que o gráfico lê. Altere um número, um nome sob um ponto ou o nome de uma série e clique em **OK**. A alteração vai para a pasta de trabalho e para o gráfico ao mesmo tempo, como uma única etapa a desfazer, de modo que outros programas também mostrem os novos números.

Não é possível adicionar nem remover uma linha ou uma coluna desse tipo de gráfico, e **Dados do Gráfico** e **Trocar Linhas e Colunas** ficam desativados, porque o gráfico está ligado a essas células. O mesmo acontece quando o gráfico lê células de outro arquivo, em um bloco que não é uma única coluna nem uma única linha, ou em células que contêm uma fórmula, uma data ou palavras onde deveria haver um número. O comando fica desativado, e sua dica explica o motivo.

## Mudar a aparência

Estes comandos estão na guia **Formatar Gráfico**. Cada um é uma única etapa a desfazer, e o gráfico mantém suas cores e o restante da aparência.

- **Alterar Tipo de Gráfico** transforma o gráfico em outro tipo com os mesmos números. Para mudar para um tipo de ações, as séries do gráfico precisam estar na ordem que esse tipo exige; se não estiverem, o programa informa a ordem necessária e deixa o gráfico como está.
- **Título do Gráfico** mostra ou oculta o título. **Editar Título do Gráfico** altera o texto dele.
- **Legenda** é a chave que diz qual cor corresponde a cada série. Posicione-a à direita, no topo, à esquerda ou embaixo, ou retire-a.
- **Eixos** mostra ou oculta a linha de palavras sob o gráfico (as **categorias**) e a linha de números ao lado (os **valores**), e define um título para cada uma. Um gráfico de pizza não tem eixos.
- **Linhas de Grade** desenha linhas discretas pelo gráfico para ajudar a acompanhar os números. As linhas **principais** ficam nos números principais do lateral, e as linhas **secundárias** ficam entre elas.
- **Estilos do Gráfico** escolhe um entre oito estilos gerais.
- **Rótulos de Dados** escreve cada número sobre sua coluna, ponto ou fatia.

## A guia Gráfico

Selecione um gráfico e a guia **Gráfico** é aberta ao lado da guia **Formatar Gráfico**. Tudo nela altera o gráfico em uma etapa que você pode desfazer.

- **Partes do Gráfico** adiciona, remove ou posiciona uma parte do gráfico: **Eixos**, **Títulos dos Eixos**, **Título do Gráfico**, **Rótulos de Dados** (o que mostram, onde ficam e o separador entre eles), **Linhas de Grade**, **Legenda**, **Barras de Erro** (barras curtas que mostram o quanto um número pode variar), **Linhas** (linhas de queda, linhas de alta e baixa e barras de alta e baixa), **Tabela de Dados** (uma grade com os números sob o gráfico, com uma coluna para cada categoria; sua lista liga e desliga a tabela e define as linhas entre as linhas, as linhas entre as colunas, o contorno e as chaves de legenda, que mostram a cor de cada série ao lado do nome; é oferecida para gráficos de colunas, de linhas e de área) e **Linha de Tendência** (uma linha que mostra para onde os números estão indo, com sua equação, se você quiser).
- **Layouts** define várias partes de uma vez, como o título, a legenda, os rótulos e as linhas de grade.
- **Cores** dá às séries um novo conjunto de cores. **Aparências** dá ao gráfico inteiro uma aparência, como simples, limpa, escura ou suave.
- **Trocar Linhas e Colunas** inverte os números: as séries passam a ser as palavras sob os pontos, e essas palavras passam a ser as séries.
- **Dados do Gráfico** lista as séries. Marque as que devem ser desenhadas, altere os nomes delas e mova-as para cima ou para baixo.
- **Editar Dados** abre a tabela de números.
- **Tipo de Gráfico** mostra uma imagem de cada tipo. Escolha um tipo para o gráfico inteiro ou um tipo para cada série, como colunas com uma linha por cima.
- **Formatar Parte do Gráfico** define o preenchimento e a linha da área do gráfico, da área de plotagem (a caixa onde ficam as marcas) ou de uma série. Para uma linha, define também se ela é suavizada e qual marca fica em cada ponto.
- **Seleção Atual** lista as partes do gráfico: a área do gráfico, a área de plotagem e cada série. Escolha uma e **Formatar Seleção** abre a janela de formatação nessa parte. **Redefinir para Corresponder ao Estilo** remove o que foi definido na parte escolhida e devolve o que o estilo do gráfico atribui a ela. Para a área do gráfico, isso significa seu preenchimento, sua linha e a cor do texto definida por uma aparência (o texto claro da aparência escura volta para a cor de texto do tema). Para a área de plotagem, significa seu preenchimento e sua linha. Para uma série, é sua cor (a que a escolha de Cores ou o estilo do gráfico lhe dá), seu contorno, suas marcas conforme o tipo de gráfico as prevê e as cores de seus pontos; uma linha suavizada permanece suavizada quando o tipo de gráfico é um tipo suavizado. Isso não altera títulos, legenda, rótulos, eixos, linhas de grade nem dados, e não faz nada em uma parte que já corresponde ao estilo. É uma etapa de Desfazer.
- **Formatar Eixo Horizontal** e **Formatar Eixo Vertical** definem os menores e maiores números, o tamanho de cada passo, a ordem, onde o outro eixo cruza, a aparência dos números e as marcas de divisão.

## Os botões ao lado de um gráfico

Três pequenos botões ficam à direita de um gráfico selecionado.

- **Elementos do Gráfico** (o sinal de mais) abre o mesmo menu de **Partes do Gráfico**.
- **Cores e Estilos do Gráfico** (o pincel) lista os conjuntos de cores e as aparências.
- **Filtros do Gráfico** (o funil) lista as séries. Desmarque uma série para deixá-la fora da imagem. O arquivo continua guardando essa série, então ela volta quando você a marcar de novo. Um gráfico sempre mantém pelo menos uma série visível.

## Gráficos em outras apresentações

Um gráfico em um arquivo **.pptx** é aberto como um gráfico que você pode alterar, quando guarda seus números dentro do próprio gráfico. Se um gráfico lê os números de uma planilha guardada junto com ele, você pode alterar tudo, exceto os números. Um gráfico com partes que o JeffDeck não altera, como efeitos especiais, é mantido como está.

## Salvar, imprimir e PDF

- Um arquivo **.pptx** mantém o gráfico como gráfico, de modo que ele abre como gráfico aqui e em outros programas.
- Um arquivo **.odp** não tem lugar para um gráfico que abra como gráfico aqui. O JeffDeck salva o gráfico como imagem, e a mensagem exibida após o salvamento informa isso.
- **PDF** e impressão desenham o gráfico como linhas, formas e texto real, não como imagem, de modo que ele continua nítido em qualquer tamanho.
