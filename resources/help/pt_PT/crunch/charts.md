# Gráficos, imagens e formas
<!-- keywords: gráfico, gráficos, coluna, barra, linha, circular, área, dispersão, combinado, legenda, título, linhas de grelha, rótulos de dados, dados do gráfico, tipo de gráfico, aspetos, esquemas, mover, redimensionar, imagem, forma, objeto, série -->

Um gráfico é uma imagem de números que se mantém associada a eles: se alterar um número, o gráfico altera-se com ele. As imagens e as formas ficam sobre as células da mesma forma, mas não estão associadas a nenhum número.

## Criar um gráfico

1. Selecione os números que pretende representar no gráfico. Se a primeira linha tiver nomes e a primeira coluna tiver etiquetas, selecione-as também. O JeffCrunch analisa a primeira linha e a primeira coluna e utiliza-as como nomes e etiquetas quando contêm palavras (ou datas, no caso das etiquetas).
2. No separador **Inserir**, no grupo **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Circular**, **Área**, **Dispersão** ou **Combinado**. A pequena seta junto a cada tipo apresenta as respetivas variações, como colunas empilhadas, uma linha com marcadores ou um gráfico de dispersão de pontos, de pontos unidos ou de linhas suaves.
3. Para ver primeiro todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Trocar linhas e colunas** inverte a orientação das séries.

Se selecionar apenas uma célula, o JeffCrunch cria o gráfico a partir do bloco de células com conteúdo à volta dela.

As séries seguem as colunas quando a seleção tem pelo menos tantas linhas como colunas, e seguem as linhas quando tem mais colunas do que linhas. Um gráfico circular contém uma única série. Um gráfico de dispersão obtém os valores horizontais da primeira coluna.

Exemplo: seleciona A1:C4, em que A2:A4 contêm T1, T2 e T3, B1 e C1 contêm Este e Oeste, e as restantes células contêm números. O gráfico tem duas séries, Este e Oeste, e três grupos de colunas, de T1 a T3.

## Mover e dimensionar

Clique num gráfico, imagem ou forma para o selecionar. Arraste-o para o mover ou arraste um dos oito quadrados no respetivo limite para o dimensionar.

- Mantenha premida a tecla **Alt** enquanto arrasta para alinhar um limite com o limite de uma célula.
- Mantenha premida a tecla **Shift** enquanto arrasta um canto para manter as proporções.
- As teclas de seta movem o objeto alguns píxeis de cada vez. **Ctrl** com uma seta move-o um píxel, **Shift** com uma seta aumenta-o ou reduz-o e **Alt** com uma seta leva-o até ao limite seguinte de uma célula.
- Premir **Esc** enquanto arrasta repõe o objeto na posição original.

Cada movimento ou redimensionamento é um passo que pode anular. Clique com o botão direito do rato no objeto e escolha **Propriedades** para escrever o respetivo tamanho e escolher o que acontece quando insere, elimina ou redimensiona células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou ficar onde está. A escolha é guardada no ficheiro.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, aparecem dois botões junto a ele e um separador **Gráfico** no friso.

- O botão **+** (e **Partes do Gráfico** no separador) ativa ou desativa o título, os eixos, os títulos dos eixos, as linhas de grelha, a legenda e os rótulos de dados do gráfico, e indica onde ficam a legenda e os rótulos.
- O botão do pincel (e **Cores** e **Aspetos** no separador) aplica ao gráfico um dos conjuntos de cores e um dos aspetos do JeffCrunch. **Esquemas** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Pode adicioná-las, editá-las, removê-las e reordená-las, alterar as células que cada uma lê, definir as células com as etiquetas, trocar as linhas e as colunas e escolher como são representadas as células vazias e as linhas e colunas ocultas.
- **Tipo de Gráfico** altera o tipo do gráfico inteiro ou de cada série: colunas com uma linha sobreposta, ou uma série num eixo vertical secundário.

Uma referência a células escreve-se como **Folha1!$B$2:$B$9**, com o nome da folha entre plicas quando contém espaços.

## Cortar, copiar e colar um objeto

Uma imagem ou forma copia-se e cola-se da mesma forma que um gráfico. Uma forma que utilize algo diferente de uma imagem do livro, como uma ligação a um ficheiro, não pode ser copiada, e o JeffCrunch indica-lhe porquê.


Copie um gráfico e cole-o no mesmo livro, em qualquer folha: a cópia lê as mesmas células que o original. Se o colar noutro livro, mantém os números que mostrava, mas deixa de ler células, porque as células que leria aí não são aquelas a partir das quais foi criado.

## Atribuir um nome

Com um objeto selecionado, a caixa de nome mostra o respetivo nome. Escreva aí um novo nome e prima **Enter**. Anular repõe o nome antigo, e o novo nome é guardado no ficheiro.

## Eliminar

Prima **Delete** com o objeto selecionado. Anular repõe-no.

## O que é guardado

Tudo o que foi descrito acima é guardado no ficheiro no formato que outros programas de folha de cálculo leem, pelo que um gráfico criado aqui abre da mesma forma nesses programas. Um gráfico proveniente de outro programa com algo que o JeffCrunch não consegue manter quando reescreve um gráfico (um preenchimento com gradação, uma linha de tendência, um gráfico tridimensional) pode ser movido, dimensionado e eliminado, mas os respetivos elementos e dados não são alterados aqui.
