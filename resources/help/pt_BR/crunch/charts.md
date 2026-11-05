# Gráficos, minigráficos, imagens e formas

<!-- keywords: filtros do gráfico, ocultar série, gráficos recomendados, sugestões de gráfico, gráfico, diagrama, colunas, barras, linha, pizza, área, dispersão, combinação, legenda, título, linhas de grade, rótulos de dados, dados do gráfico, tipo de gráfico, aparência, layouts, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R-quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, minigráfico pequeno, marcadores, nome, texto alternativo, logarítmica, escala logarítmica, salvar como imagem, modelo, crtx, texto do título, chave da legenda, painel de seleção, preenchimento da forma, contorno da forma, girar, inverter, cortar, editar texto, borda da imagem, inserir imagem, inserir forma, caixa de texto, formas, seta, oval, retângulo, estrela -->

Um gráfico é uma imagem de números que permanece ligada a eles: se você alterar um número, o gráfico muda junto. Imagens e formas ficam sobre as células da mesma forma, mas não estão ligadas a nenhum número.

## Criar um gráfico

1. Selecione os números que você quer representar no gráfico. Se a primeira linha tiver nomes e a primeira coluna tiver rótulos, selecione-os também. O JeffCrunch analisa a primeira linha e a primeira coluna e as usa como nomes e rótulos quando forem palavras (ou datas, no caso dos rótulos).
2. Na guia **Inserir**, em **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Pizza**, **Área**, **Dispersão** ou **Combinação**. A pequena seta ao lado de um tipo mostra as variações, como colunas empilhadas, linha com marcadores ou gráfico de dispersão com pontos, com pontos unidos ou com linhas suaves.
3. Para ver todos os tipos antes, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado com as suas células. **Trocar linhas e colunas** inverte a disposição das séries.

Não sabe qual tipo escolher? Selecione as células e escolha **Gráficos Recomendados** na guia **Inserir**. O JeffCrunch analisa as células e lista alguns tipos adequados a elas, do melhor para o menos indicado. Por exemplo, datas na vertical sugerem uma linha, e uma coluna com poucos números positivos e rótulos sugere uma pizza. Cada tipo é desenhado com as suas células. Escolha um e clique em **OK**.

Se você selecionar apenas uma célula, o JeffCrunch cria o gráfico com o bloco de células ao redor dela que contém dados.

As séries seguem nas colunas quando a seleção tem pelo menos tantas linhas quanto colunas, e nas linhas quando tem mais colunas do que linhas. Uma pizza tem uma única série. Um gráfico de dispersão usa os valores horizontais da primeira coluna.

Exemplo: você seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Leste e Oeste, e o restante são números. O gráfico terá duas séries, Leste e Oeste, e três grupos de colunas, de Q1 a Q3.

## Ocultar uma série temporariamente

Selecione um gráfico. Três pequenos botões aparecem ao lado dele. O que tem um funil é **Filtros do Gráfico**. Ele lista as séries do gráfico, cada uma com uma caixa de seleção. Desmarque uma caixa para deixar essa série fora do gráfico. Marque-a novamente para trazê-la de volta, no mesmo lugar. **Desfazer** também funciona. Um gráfico sempre mantém pelo menos uma série. Uma série que você deixou de fora não é mantida ao salvar e fechar o arquivo, então marque-a novamente antes, se quiser mantê-la.

## Mover e redimensionar

Clique em um gráfico, imagem ou forma para selecioná-lo. Arraste-o para movê-lo, ou arraste um dos oito quadrados da borda para redimensioná-lo.

- Mantenha **Alt** pressionada ao arrastar para alinhar uma borda com a borda de uma célula.
- Mantenha **Shift** pressionada ao arrastar um canto para manter as proporções.
- As setas do teclado movem o objeto alguns pixels por vez. **Ctrl** com uma seta move o objeto um pixel, **Shift** com uma seta o torna maior ou menor, e **Alt** com uma seta vai até a próxima borda de uma célula.
- **Esc** durante o arrastar devolve o objeto à posição original.

Cada movimentação ou redimensionamento é uma etapa que você pode desfazer. Clique com o botão direito no objeto e escolha **Propriedades** para digitar o **Nome** (cada objeto em uma planilha precisa do seu próprio) e o **Texto alternativo** (uma frase curta que informa a uma pessoa que não pode ver o objeto o que ele mostra), para digitar o tamanho e para escolher o que ele faz ao inserir, excluir ou redimensionar células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou permanecer no lugar. A escolha é salva no arquivo. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (ele continua aparecendo na tela); o arquivo também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, dois botões aparecem ao lado dele e uma guia **Gráfico** surge na faixa de opções.

- O botão **+** (e **Partes do Gráfico** na guia) liga ou desliga o título do gráfico, os eixos, os títulos dos eixos, as linhas de grade, a legenda e os rótulos de dados, e define onde ficam a legenda e os rótulos.
- O botão do pincel (e **Cores** e **Aparência** na guia) dá ao gráfico um dos conjuntos de cores do JeffCrunch e uma das aparências. **Layouts** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Você pode adicionar, editar, remover e reordenar as séries, alterar as células que cada uma lê, definir as células com os rótulos, trocar linhas por colunas e escolher como as células vazias e as linhas e colunas ocultas são representadas.
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para mostrar para onde eles estão indo. Escolha **Linear** para uma reta, **Exponencial** para números que crescem em um percentual constante, **Logarítmica** para números que sobem rápido e depois se estabilizam, **Polinomial** para uma curva com inflexões, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R-quadrado** escreve o quanto a linha se ajusta bem aos dados (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência recebe uma entrada própria na legenda.
- Quando uma série é desenhada em um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as opções **Segundo Horizontal** e **Segundo Vertical** para ela. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área de plotagem em vez de ao lado dela.
- **Rótulos de Dados** também tem **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a porcentagem e a chave da legenda, que é um pequeno quadrado na cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato do Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe vazio para mostrar os números como eles são). Desative todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** na guia **Gráfico** abrem uma caixa para um eixo. Ela define os valores mínimo e máximo, as unidades principal e secundária (cada uma automática ou um número que você digita), se os valores aparecem em ordem inversa, se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que é adequado a números de tamanhos muito diferentes e que devem ser todos maiores que 0), onde o outro eixo cruza com ele, o formato numérico dos rótulos, as marcas de divisão, a posição dos rótulos e o quanto eles são girados. Somente um eixo numérico tem limites e unidades.
- **Mover Gráfico…** na guia **Gráfico** pede outra planilha e coloca o gráfico nela, nas mesmas células. Ele continua lendo as células que lia, mantém o seu tamanho e a sua posição na planilha, mesmo onde as linhas e colunas são mais altas ou mais largas, e se move e redimensiona com as células de lá como fazia antes. Planilhas ocultas não são oferecidas. É uma única ação de **Desfazer**, mesmo depois de salvar. Uma folha de gráfico (uma planilha que é apenas um gráfico) ainda não pode ser criada, e um gráfico que esteja nela não pode ser movido.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) também têm **Texto do Título…**. Digite as palavras que quiser. Ou digite um sinal de igual seguido de uma célula, como **=Planilha1!A1**, e o título mostrará o conteúdo dessa célula. Quando a célula mudar, o título muda junto. O arquivo guarda o vínculo.
- **Salvar como Imagem…** na guia **Gráfico** (e no menu do botão direito) salva o gráfico selecionado como um arquivo de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na planilha, para que fique nítido.
- **Salvar como Modelo…** e **Aplicar Modelo…** (na guia **Gráfico**, no grupo **Tipo**) guardam a aparência de um gráfico em um arquivo **.crtx** e a aplicam a outro gráfico. A aparência inclui o tipo, as cores, as configurações de título e legenda, os eixos e as fontes. O gráfico ao qual você aplica o modelo mantém os próprios números e as células que lê. Uma ação de **Desfazer** remove a aparência novamente.
- **Tipo de Gráfico** altera o tipo do gráfico inteiro ou de cada série: colunas com uma linha por cima, ou uma série em um segundo eixo vertical.

Uma referência a células é escrita como **Planilha1!$B$2:$B$9**, com o nome da planilha entre aspas simples quando ele tiver espaços.

## Minigráficos

Um minigráfico é um gráfico minúsculo que cabe dentro de uma única célula. Ele mostra a tendência de uma linha ou de uma coluna de números, bem ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. Na guia **Inserir**, em **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. A **Linha** une os números com uma linha. A **Coluna** desenha uma barra para cada número. **Vitória/Derrota** desenha uma barra para cima em cada número maior que zero e uma barra para baixo em cada número menor que zero, todas do mesmo tamanho, o que é adequado a resultados que são apenas bons ou ruins.
3. Na caixa **Criar Minigráficos**, confira o **Intervalo de dados** (as células que os minigráficos leem) e digite o **Intervalo de localização** (as células em que eles ficarão). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lê todos os dados.

Os minigráficos criados em uma única etapa formam um grupo e compartilham a mesma aparência. Clique em uma célula com minigráfico e a guia **Minigráfico** aparece.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do minigráfico em que você clicou. Também define o que fazer com as células vazias (deixar uma lacuna, tratá-las como zero ou unir os pontos sobre elas) e se devem ser lidas as células em linhas e colunas ocultas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** marca pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, para uma linha, **Marcadores** em todos os pontos.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras, e a **Espessura** (a espessura da linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto marcado.
- **Eixo** mostra uma linha no zero, plota os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma única escala, para que você possa compará-los. **Valor Personalizado** permite digitar um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: digite as células que contêm a data de cada ponto, e um ponto com data posterior fica mais à frente. Deixe vazio para espaçar os pontos igualmente novamente.
- **Agrupar** coloca os minigráficos selecionados em um mesmo grupo. **Desagrupar** dá a cada um a sua própria aparência. **Limpar** remove os minigráficos selecionados, ou os grupos inteiros deles.

Um minigráfico acompanha as suas células: se você inserir uma linha acima dos dados, ele continua lendo os mesmos números. Cada alteração é uma etapa que você pode desfazer. Os minigráficos são salvos no arquivo em um formato que outros programas de planilha conseguem ler.

## Inserir uma imagem, uma forma ou uma caixa de texto

Na guia **Inserir**, em **Ilustrações**, **Imagem** abre um seletor de arquivos. Escolha um arquivo de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele será colocado na planilha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber, mantendo a forma. A pasta de trabalho guarda a própria imagem. Uma imagem de um tipo que a pasta de trabalho não consegue guardar como está (um SVG, por exemplo) é salva como PNG, uma imagem comum formada por pontos. Um arquivo que não seja uma imagem, ou que tenha mais de 16 MB, é recusado com uma mensagem.

**Formas** abre uma lista: retângulo, retângulo de cantos arredondados, oval, triângulo, losango, setas, estrela, coração, nuvem, balão de fala e linha. Escolha uma delas e ela será colocada na planilha em azul-claro, com uma borda mais escura. **Caixa de Texto**, no grupo **Texto**, insere uma caixa vazia, sem preenchimento e sem borda.

Cada um deles é uma única etapa que você pode desfazer. O novo objeto fica selecionado, então você pode arrastá-lo, redimensioná-lo, trazê-lo para a frente ou enviá-lo para trás, dar-lhe um nome e um texto alternativo em **Propriedades**, ou pressionar **Delete**.

## Alterar a aparência de uma imagem, forma ou caixa de texto

Ao selecionar uma imagem, a guia **Formato da Imagem** aparece. Ao selecionar uma forma, uma linha ou uma caixa de texto, aparece a guia **Formato da Forma**. Cada alteração é uma etapa que você pode desfazer, e o objeto mantém o seu nome, o seu texto alternativo, a sua posição e a sua ordem em relação aos outros objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (o quão grossa é a linha) e os traços (sólido, tracejado ou pontilhado). Uma imagem tem **Borda da Imagem**, que é o seu contorno.
- **Girar** gira o objeto em um quarto de volta para a direita ou para a esquerda, ou o inverte horizontalmente ou verticalmente.
- **Cortar**, para uma imagem, a recorta em uma forma de janela: quadrado, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9 ou 9 por 16. O corte é feito de forma uniforme a partir do centro do que está visível, e a imagem é ampliada até o tamanho do que sobrou. **Mostrar a Imagem Inteira** traz de volta o que foi cortado. O arquivo da imagem não é alterado: a parte cortada apenas fica oculta.
- **Editar Texto** serve para formas e caixas de texto. Digite as palavras e escolha como elas se posicionam na horizontal e na vertical, o tamanho, negrito ou normal, e se quebram dentro da forma ou ficam em uma só linha.
- **Painel de Seleção** abre uma lista de todos os objetos da planilha pelo nome, com o que está na frente em primeiro lugar. Clique em um nome para selecionar esse objeto, o que ajuda quando objetos se escondem uns aos outros. **Avançar** e **Recuar** movem o objeto na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou ampliadas.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo diferente de uma imagem da pasta de trabalho, como um vínculo com um arquivo, não pode ser copiada, e o JeffCrunch informa o motivo.

Copie um gráfico e cole-o na mesma pasta de trabalho, em qualquer planilha: a cópia lê as mesmas células que a original. Se você colá-lo em outra pasta de trabalho, ele mantém os números que mostrava, mas não lê mais células, porque as células que leria ali não são as de onde foi criado.

## Dar um nome

Com o objeto selecionado, a caixa de nome mostra o nome dele. Digite um novo nome ali e pressione **Enter**. **Desfazer** restaura o nome antigo, e o novo nome é salvo no arquivo.

## Excluir

Pressione **Delete** com o objeto selecionado. **Desfazer** o traz de volta.

## O que é salvo

Tudo o que foi descrito acima é salvo no arquivo em um formato que outros programas de planilha conseguem ler, então um gráfico criado aqui abre da mesma forma lá. Um gráfico vindo de outro programa, com algo que o JeffCrunch não consegue manter ao reescrever um gráfico (barras de erro, um gráfico tridimensional), ainda pode ser movido, redimensionado e excluído, mas os seus elementos e dados não são alterados aqui.
