# Gráficos, minigráficos, imagens e formas

<!-- keywords: gráfico, gráficos, diagrama, coluna, colunas, barra, barras, linha, pizza, área, dispersão, combinação, legenda, título, linhas de grade, rótulos de dados, dados do gráfico, tipo de gráfico, aparência, layouts, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R-quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, gráfico minúsculo, marcadores, nome, texto alternativo, logarítmica, escala logarítmica, salvar como imagem, modelo, crtx, texto do título, chave da legenda -->

Um gráfico é uma imagem de números que permanece vinculada a eles: quando você altera um número, o gráfico muda junto. Imagens e formas ficam sobre as células da mesma forma, mas não estão vinculadas a nenhum número.

## Criar um gráfico

1. Selecione os números que você quer representar no gráfico. Se a primeira linha tiver nomes e a primeira coluna tiver rótulos, selecione-os também. O JeffCrunch examina a primeira linha e a primeira coluna e os usa como nomes e rótulos quando forem palavras (ou datas, no caso dos rótulos).
2. Na guia **Inserir**, no grupo **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Pizza**, **Área**, **Dispersão** ou **Combinação**. A pequena seta ao lado de um tipo mostra suas variações, como colunas empilhadas, uma linha com marcadores ou um gráfico de dispersão de pontos, de pontos unidos ou de linhas suaves.
3. Para ver antes todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Trocar linhas e colunas** inverte a orientação das séries.

Se você selecionar apenas uma célula, o JeffCrunch representa no gráfico o bloco de células ao redor dela que contém alguma coisa.

As séries seguem pelas colunas quando a seleção tem pelo menos tantas linhas quanto colunas, e pelas linhas quando ela tem mais colunas do que linhas. Um gráfico de pizza tem uma única série. Um gráfico de dispersão usa a primeira coluna para os valores horizontais.

Exemplo: você seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Leste e Oeste, e o restante são números. O gráfico terá duas séries, Leste e Oeste, e três grupos de colunas, Q1 a Q3.

## Mover e redimensionar

Clique em um gráfico, imagem ou forma para selecioná-lo. Arraste-o para movê-lo, ou arraste um dos oito quadradinhos da borda para redimensioná-lo.

- Mantenha **Alt** pressionada enquanto arrasta para alinhar uma borda com a borda de uma célula.
- Mantenha **Shift** pressionada enquanto arrasta um canto para manter as proporções.
- As teclas de seta movem o objeto alguns pixels de cada vez. **Ctrl** com uma seta move o objeto um pixel, **Shift** com uma seta o torna maior ou menor, e **Alt** com uma seta vai até a próxima borda de uma célula.
- **Esc** enquanto arrasta devolve o objeto à posição original.

Cada movimento ou redimensionamento é uma etapa que você pode desfazer. Clique com o botão direito no objeto e escolha **Propriedades** para digitar o **Nome** (cada objeto em uma planilha precisa de um nome próprio) e o **Texto alternativo** (uma frase curta que informa, a quem não pode ver o objeto, o que ele mostra), para digitar o tamanho e para escolher o que ele faz ao inserir, excluir ou redimensionar células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou permanecer onde está. A escolha é salva no arquivo. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (ele continua aparecendo na tela); o arquivo também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, dois botões aparecem ao lado dele e a guia **Gráfico** aparece na faixa de opções.

- O botão **+** (e **Elementos do Gráfico** na guia) liga ou desliga o título do gráfico, os eixos, os títulos dos eixos, as linhas de grade, a legenda e os rótulos de dados, e define onde ficam a legenda e os rótulos.
- O botão do pincel (e **Cores** e **Estilos** na guia) aplica ao gráfico um dos conjuntos de cores do JeffCrunch e um dos seus estilos. **Layouts** define vários elementos de uma vez.
- **Dados do Gráfico** lista as séries do gráfico. Você pode adicionar, editar, remover e reordenar séries, alterar as células que cada uma lê, definir as células com os rótulos, trocar linhas por colunas e escolher como as células vazias e as linhas e colunas ocultas são representadas.
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para que você veja para onde eles estão indo. Escolha **Linear** para uma reta, **Exponencial** para números que crescem em uma porcentagem constante, **Logarítmica** para números que sobem rápido e depois se estabilizam, **Polinomial** para uma curva com curvaturas, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R-quadrado** escreve o quanto a linha se ajusta bem (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência recebe uma entrada própria na legenda.
- Quando uma série é desenhada em relação a um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as opções **Segundo Horizontal** e **Segundo Vertical** para ela. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área de plotagem, em vez de ao lado dela.
- **Rótulos de Dados** também tem **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a porcentagem e a chave da legenda, que é um pequeno quadrado na cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato do Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe vazio para mostrar os números como são). Desligue todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** na guia **Gráfico** abrem uma caixa para um eixo. Ela define os valores mínimo e máximo e as unidades principal e secundária (cada uma automática ou um número que você digita), se os valores aparecem em ordem inversa, se a escala é logarítmica (cada marca é uma base multiplicada pela anterior, como 1, 10, 100 e 1000, o que é adequado a números de tamanhos muito diferentes e que precisam ser todos maiores que 0), onde o outro eixo cruza com ele, o formato numérico dos rótulos, as marcas de divisão, a posição dos rótulos e o ângulo de rotação dos rótulos. Somente um eixo numérico tem limites e unidades.
- **Mover Gráfico…** na guia **Gráfico** pede outra planilha e coloca o gráfico nela, nas mesmas células. Ele continua lendo as células que lia, e mantém o tamanho e a posição na planilha, mesmo onde as linhas e colunas são mais altas ou mais largas, e se move e redimensiona com as células como antes. Planilhas ocultas não são oferecidas. É uma única ação de **Desfazer**, mesmo depois de salvar. Ainda não é possível criar uma folha de gráfico (uma planilha que contém apenas um gráfico), e um gráfico que está em uma delas não pode ser movido.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) também têm **Texto do Título…**. Digite as palavras que quiser. Ou digite um sinal de igual e uma célula, como **=Planilha1!A1**, e o título mostrará o conteúdo dessa célula. Quando a célula muda, o título muda junto. O arquivo guarda o vínculo.
- **Salvar como Imagem…** na guia **Gráfico** (e no menu do clique com o botão direito) salva o gráfico selecionado como um arquivo de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na planilha, para que continue nítido.
- **Salvar como Modelo…** e **Aplicar Modelo…** (na guia **Gráfico**, no grupo **Tipo**) guardam a aparência de um gráfico em um arquivo **.crtx** e a aplicam a outro gráfico. A aparência inclui o tipo, as cores, as configurações de título e legenda, os eixos e as fontes. O gráfico ao qual você aplica o modelo mantém seus próprios números e as células que lê. Uma ação de **Desfazer** remove a aparência novamente.
- **Alterar Tipo de Gráfico** muda o tipo do gráfico inteiro ou de cada série: colunas com uma linha sobre elas, ou uma série em relação a um segundo eixo vertical.

Uma referência a células é escrita assim: **Planilha1!$B$2:$B$9**, com o nome da planilha entre aspas simples quando ele tiver espaços.

## Minigráficos

Um minigráfico é um gráfico minúsculo que cabe dentro de uma única célula. Ele mostra a tendência de uma linha ou de uma coluna de números bem ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. Na guia **Inserir**, no grupo **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. A **Linha** une os números com uma linha. A **Coluna** desenha uma barra para cada número. A **Vitória/Derrota** desenha uma barra para cima para cada número maior que zero e uma barra para baixo para cada número menor que zero, todas do mesmo tamanho, o que é adequado a resultados que são apenas bons ou ruins.
3. Na caixa **Criar Minigráficos**, confira o **Intervalo de dados** (as células que os minigráficos leem) e digite o **Intervalo de localização** (as células em que eles ficarão). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lê todos os dados.

Os minigráficos criados em uma única etapa formam um grupo e compartilham a mesma aparência. Clique em uma célula com um minigráfico, e a guia **Minigráfico** aparece.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do minigráfico em que você clicou. Ela também define o que fazer com as células vazias (deixar uma lacuna, tratá-las como zero ou unir os pontos através delas) e se as células das linhas e colunas ocultas devem ser lidas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** marca pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, para uma linha, os **Marcadores** em cada ponto.
- **Estilos** aplica ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras, e a **Espessura** (a largura da linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto marcado.
- **Eixo** mostra uma linha no zero, plota os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma única escala, para que você possa compará-los. **Valor Personalizado** permite digitar um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: digite as células que contêm a data de cada ponto, e um ponto com data posterior fica mais à frente. Deixe o campo vazio para espaçar os pontos igualmente de novo.
- **Agrupar** coloca os minigráficos selecionados em um mesmo grupo. **Desagrupar** dá a cada um a sua própria aparência. **Limpar** remove os minigráficos selecionados ou os grupos inteiros a que pertencem.

Um minigráfico acompanha as suas células: se você inserir uma linha acima dos dados, ele continua lendo os mesmos números. Cada alteração é uma etapa que você pode desfazer. Os minigráficos são salvos no arquivo no formato que outros programas de planilha leem.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo diferente de uma imagem da pasta de trabalho, como um vínculo para um arquivo, não pode ser copiada, e o JeffCrunch informa o motivo.

Copie um gráfico e cole-o na mesma pasta de trabalho, em qualquer planilha: a cópia lê as mesmas células que a original. Se colá-lo em outra pasta de trabalho, ele mantém os números que mostrava, mas não lê mais células, porque as células que leria ali não são as que deram origem a ele.

## Dar um nome

Com o objeto selecionado, a caixa de nome mostra o nome dele. Digite um novo nome ali e pressione **Enter**. Desfazer restaura o nome antigo, e o novo nome é salvo no arquivo.

## Excluir

Pressione **Delete** com o objeto selecionado. Desfazer o traz de volta.

## O que é salvo

Tudo o que foi descrito acima é salvo no arquivo no formato que outros programas de planilha leem, de modo que um gráfico criado aqui abre da mesma forma lá. Um gráfico vindo de outro programa com algo que o JeffCrunch não consegue manter quando reescreve um gráfico (barras de erro, um gráfico tridimensional) ainda pode ser movido, redimensionado e excluído, mas seus elementos e dados não são alterados aqui.
