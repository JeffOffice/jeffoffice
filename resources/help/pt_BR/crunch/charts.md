# Gráficos, imagens e formas
<!-- keywords: gráfico, diagrama, coluna, barra, linha, pizza, área, dispersão, combinação, legenda, título, linhas de grade, rótulos de dados, dados do gráfico, tipo de gráfico, aparência, layouts, mover, redimensionar, imagem, figura, forma, objeto, série -->

Um gráfico é uma imagem de números que permanece vinculada a eles: altere um número e o gráfico muda junto. Imagens e formas ficam sobre as células da mesma maneira, mas não estão vinculadas a nenhum número.

## Criar um gráfico

1. Selecione os números que você deseja representar no gráfico. Se a primeira linha tiver nomes e a primeira coluna tiver rótulos, selecione-os também. O JeffCrunch examina a primeira linha e a primeira coluna e as usa como nomes e rótulos quando contêm palavras (ou datas, no caso dos rótulos).
2. Na guia **Inserir**, no grupo **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Pizza**, **Área**, **Dispersão** ou **Combinação**. A pequena seta ao lado de um tipo mostra as variações dele, como colunas empilhadas ou linha com marcadores.
3. Para ver todos os tipos antes, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado com as suas células. **Alternar linhas e colunas** inverte a direção das séries.

Se você selecionar apenas uma célula, o JeffCrunch cria o gráfico com o bloco de células preenchidas ao redor dela.

As séries seguem pelas colunas quando a seleção tem pelo menos tantas linhas quanto colunas, e pelas linhas quando tem mais colunas do que linhas. Um gráfico de pizza contém uma série. Um gráfico de dispersão obtém os valores horizontais da primeira coluna.

Exemplo: você seleciona A1:C4, em que A2:A4 contêm T1, T2 e T3, B1 e C1 contêm Leste e Oeste, e as demais células contêm números. O gráfico tem duas séries, Leste e Oeste, e três grupos de colunas, de T1 a T3.

## Mover e redimensionar

Clique em um gráfico, imagem ou forma para selecioná-lo. Arraste-o para movê-lo ou arraste um dos oito quadrados da borda para redimensioná-lo.

- Mantenha **Alt** pressionada enquanto arrasta para alinhar uma borda com a borda de uma célula.
- Mantenha **Shift** pressionada enquanto arrasta um canto para manter as proporções.
- As teclas de direção movem o objeto alguns pixels de cada vez. **Ctrl** com uma seta move um pixel, **Shift** com uma seta aumenta ou diminui o objeto, e **Alt** com uma seta leva até a próxima borda de uma célula.
- **Esc** durante o arraste devolve o objeto à posição original.

Cada movimento ou redimensionamento é uma etapa que pode ser desfeita. Clique com o botão direito do mouse no objeto e escolha **Propriedades** para digitar o tamanho dele e escolher o que ele faz quando você insere, exclui ou redimensiona células: mover e redimensionar com as células, mover com elas mantendo o tamanho, ou não se mover. A escolha é salva no arquivo.

## Alterar o que o gráfico mostra

Quando um gráfico está selecionado, dois botões aparecem ao lado dele e a guia **Gráfico** aparece na faixa de opções.

- O botão **+** (e **Partes do Gráfico** na guia) ativa ou desativa o título, os eixos, os títulos dos eixos, as linhas de grade, a legenda e os rótulos de dados do gráfico, e define onde ficam a legenda e os rótulos.
- O botão de pincel (e **Cores** e **Aparência** na guia) aplica ao gráfico um dos conjuntos de cores e uma das aparências do JeffCrunch. **Layouts** define vários elementos de uma vez.
- **Dados do Gráfico** lista as séries do gráfico. Você pode adicioná-las, editá-las, removê-las e reordená-las, alterar as células que cada uma lê, definir as células com os rótulos, alternar linhas e colunas e escolher como as células vazias e as linhas e colunas ocultas são representadas.
- **Tipo de Gráfico** altera o tipo do gráfico inteiro ou de cada série: colunas com uma linha sobre elas, ou uma série em um eixo vertical secundário.

Uma referência a células é escrita como **Planilha1!$B$2:$B$9**, com o nome da planilha entre aspas simples quando contém espaços.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo diferente de uma imagem da pasta de trabalho, como um vínculo a um arquivo, não pode ser copiada, e o JeffCrunch informa o motivo.


Copie um gráfico e cole-o na mesma pasta de trabalho, em qualquer planilha: a cópia lê as mesmas células que o original. Cole-o em outra pasta de trabalho e ele mantém os números que mostrava, mas deixa de ler células, pois as células que ele leria lá não são aquelas a partir das quais foi criado.

## Nomear

Com um objeto selecionado, a caixa de nome mostra o nome dele. Digite um novo nome ali e pressione **Enter**. Desfazer restaura o nome anterior, e o novo nome é salvo no arquivo.

## Excluir

Pressione **Delete** com o objeto selecionado. Desfazer o traz de volta.

## O que é salvo

Tudo o que foi descrito acima é salvo no arquivo no formato que outros programas de planilha leem, para que um gráfico criado aqui seja aberto lá da mesma forma. Um gráfico vindo de outro programa com algo que o JeffCrunch não consegue manter ao regravar um gráfico (um preenchimento com gradiente, uma linha de tendência, um gráfico tridimensional) ainda pode ser movido, redimensionado e excluído, mas seus elementos e dados não são alterados aqui.
