# Gráficos, minigráficos, imagens e formas

<!-- keywords: objeto incorporado, inserir objeto, objeto a partir de um intervalo, objeto a partir de um gráfico, objeto a partir de uma página, colar como objeto, lista de objetos, editar objeto, botões de campo, botões de gráfico dinâmico, agrupar, desagrupar, alinhar, alinhar à esquerda, distribuir, selecionar objetos, várias formas, escolher várias, caixa de arrastar, filtros do gráfico, ocultar série, gráficos recomendados, sugestões de gráfico, gráfico, diagrama de barras, coluna, barra, linha, circular, área, dispersão, combinado, radar, radar preenchido, gráfico de teia, legenda, título, linhas de grelha, etiquetas de dados, dados do gráfico, tipo de gráfico, aspeto, esquemas, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R-quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, mini gráfico, marcadores, nome, texto alternativo, logarítmica, escala logarítmica, guardar como imagem, modelo, crtx, texto do título, chave da legenda, painel de seleção, preenchimento da forma, contorno da forma, rodar, inverter, recortar, editar texto, contorno da imagem, inserir imagem, ícones, ícone, wmf, emf, estilos de forma, estilos, inserir forma, caixa de texto, formas, seta, oval, retângulo, estrela, diagrama, diagramas, imagens online, imagens gratuitas, openverse, wikimedia commons, licença, crédito, texto artístico, wordart, deformar, comprimir imagens, reduzir imagens, tamanho do ficheiro, ppp, eliminar áreas recortadas, código de barras, código QR, ligar uma forma a uma célula, forma mostra uma célula, caixa de texto mostra uma célula, valor da célula numa forma -->

Um gráfico é uma imagem de números que permanece ligada a eles: se alterar um número, o gráfico muda com ele. As imagens e as formas são colocadas sobre as células da mesma forma, mas não estão ligadas a nenhum número.

## Criar um gráfico

1. Selecione os números que pretende representar. Se a primeira linha tiver nomes e a primeira coluna tiver etiquetas, selecione também essas. O JeffCrunch analisa a primeira linha e a primeira coluna e usa-as como nomes e etiquetas quando forem palavras (ou datas, no caso das etiquetas).
2. No separador **Inserir**, em **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Circular**, **Área**, **Dispersão**, **Combinado**, **Radar** ou **Ações**. A pequena seta ao lado de um tipo mostra as variantes, como colunas empilhadas, uma linha com marcadores, um gráfico de dispersão de pontos, de pontos unidos ou de linhas suavizadas, ou um gráfico de radar simples, com marcadores ou preenchido. Um gráfico de radar desenha cada etiqueta como um raio a partir do centro e cada série como uma forma fechada que passa pelos seus números nos raios, pelo que mostra várias medidas de uma mesma coisa lado a lado.
   Um gráfico de **ações** mostra preços: uma linha para cada categoria, desde o preço mais alto até ao mais baixo, com um pequeno ponto para o fecho, ou, quando há abertura, uma barra entre a abertura e o fecho (clara para subida, escura para descida). O gráfico é feito a partir das suas colunas exatamente como estão, da esquerda para a direita, e nada verifica se os números correspondem ao lugar em que estão; por isso, coloque-os nesta ordem: máximo, mínimo e fecho; abertura, máximo, mínimo e fecho; volume, máximo, mínimo e fecho; ou volume, abertura, máximo, mínimo e fecho. Se as colunas estiverem noutra ordem, o gráfico é desenhado mesmo assim, mas as marcas têm outro significado (o ponto de fecho fica na terceira coluna de preços, e a caixa vai da primeira coluna de preços até à última). Com volume, o volume é lido no eixo esquerdo e os preços são lidos num segundo eixo, à direita. Se o número de colunas não corresponder ao tipo, o JeffOffice indica quantas são necessárias e não cria o gráfico.
3. Para ver primeiro todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Trocar linhas e colunas** inverte o sentido das séries.

Não sabe qual o tipo a escolher? Selecione as células e escolha **Gráficos Recomendados** no separador **Inserir**. O JeffCrunch analisa as células e apresenta alguns tipos adequados, do melhor para o menos indicado. Por exemplo, datas na vertical sugerem uma linha, e uma coluna de poucos números positivos com etiquetas sugere um gráfico circular. Cada tipo é desenhado a partir das suas células. Escolha um e prima **OK**.

Se selecionar apenas uma célula, o JeffCrunch representa o bloco de células à sua volta que tenha conteúdo.

As séries seguem as colunas quando a seleção tem pelo menos tantas linhas como colunas, e as linhas quando tem mais colunas do que linhas. Um gráfico circular tem uma única série. Um gráfico de dispersão toma os valores horizontais da primeira coluna.

Exemplo: seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Este e Oeste, e o restante são números. O gráfico tem duas séries, Este e Oeste, e três grupos de colunas, de Q1 a Q3.

## Ocultar uma série por agora

Escolha um gráfico. Aparecem junto dele três pequenos botões. O que tem um funil é **Filtros do Gráfico**. Mostra as séries do gráfico, cada uma com uma marca de verificação. Desmarque uma para a retirar do gráfico. Marque-a novamente para a repor no mesmo lugar. **Anular** também funciona. Um gráfico mantém sempre pelo menos uma série. Uma série retirada continua no ficheiro quando o guarda e fica oculta quando o abre, pelo que nada se perde. Marque-a novamente para a repor. (Um gráfico de anel mostra todas as séries quando é aberto.)

## Mover e redimensionar

Clique num gráfico, numa imagem ou numa forma para o selecionar. Arraste-o para o mover, ou arraste um dos oito quadrados da sua margem para o redimensionar.

- Mantenha **Alt** premido enquanto arrasta para fazer com que uma linha da margem fique alinhada com a margem de uma célula.
- **Ajustar à Grelha** e **Ajustar à Forma** estão no separador **Formato da Forma**, e são guardados para si, em todos os livros. Com **Ajustar à Grelha** ativo, uma margem que arrasta vai para a margem mais próxima de uma célula. Com **Ajustar à Forma** ativo, uma margem vai para a margem de outro objeto quando está a menos de 6 píxeis dela. Apenas os primeiros 2000 outros objetos de uma folha contam para **Ajustar à Forma**. Com os dois ativos, prevalecem as margens das células, porque um objeto está ancorado às células, e as margens dos outros objetos não são consideradas. Uma alteração chega a todas as janelas abertas de imediato.
- **Alt** inverte o ajuste que está ativo para um arrastamento: com **Ajustar à Grelha** ou **Ajustar à Forma** ativos, manter **Alt** premido desliga o ajuste nesse arrastamento (nunca muda para o outro tipo), e com ambos desativados, **Alt** faz com que as margens se ajustem às células, como acima.
- Um objeto rodado é ajustado pelo contorno que vê (a caixa mais pequena que o contém), e não pela moldura que tinha antes de ser rodado; o mesmo acontece com os objetos a que se ajusta.
- Mantenha **Shift** premido enquanto arrasta um canto para manter as proporções.
- As teclas de seta deslocam o objeto alguns píxeis de cada vez. **Ctrl** com uma seta desloca-o um píxel, **Shift** com uma seta aumenta ou diminui o objeto, e **Alt** com uma seta leva-o até à margem seguinte de uma célula.
- **Esc** enquanto arrasta repõe o objeto no lugar.

Cada movimento ou redimensionamento é um passo que pode anular. Clique com o botão direito no objeto e escolha **Propriedades** para escrever o **Nome** (cada objeto de uma folha precisa do seu próprio nome) e o **Texto alternativo** (uma frase curta que diz a uma pessoa que não consegue ver o objeto o que ele mostra), para escrever o tamanho e para escolher o que acontece ao objeto quando insere, elimina ou redimensiona células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou ficar onde está. A escolha fica guardada no ficheiro. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (continua a ver-se no ecrã); o ficheiro também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, aparecem dois botões junto dele e surge no friso um separador **Gráfico**.

- O botão **+** (e **Elementos do Gráfico** no separador) liga ou desliga o título do gráfico, os eixos, os títulos dos eixos, as linhas de grelha, a legenda e as etiquetas de dados, e indica onde ficam a legenda e as etiquetas.
- O botão do pincel (e **Cores** e **Estilos** no separador) dá ao gráfico um dos conjuntos de cores do JeffCrunch e um dos seus estilos. **Esquemas** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Pode adicionar, editar, remover e reordenar as séries, alterar as células que cada uma lê, definir as células com as etiquetas, trocar linhas e colunas, e escolher como são representadas as células vazias e as linhas e colunas ocultas.
- **Botões de Campo** (no menu **+**, apenas para um gráfico dinâmico) mostra pequenos botões com os nomes dos campos da tabela dinâmica que o gráfico segue: os filtros do relatório sob o título, e as linhas, valores e colunas ao longo da base. Ative ou desative cada tipo, ou escolha **Mostrar Tudo** ou **Ocultar Tudo**. Um novo gráfico dinâmico mostra-os.
- **Barras de Erro** (no menu **+**, para um gráfico de colunas, barras, linhas, áreas ou dispersão) desenha uma pequena linha acima e abaixo de cada ponto para mostrar quanto o valor real poderia diferir. Escolha **Erro Padrão**, **Percentagem (5%)**, **Desvio Padrão** ou **Valor Fixo (1)**. Escolha **Personalizado** para usar valores seus: escreva as células que contêm os valores acima de cada ponto (como `=Folha1!$D$2:$D$4`) e, depois, as células dos valores abaixo. Deixe uma vazia para desenhar as barras apenas de um lado. As barras mudam quando essas células mudam. Um ficheiro que tenha os seus próprios valores para cada ponto mantém-nos.
- **Linhas** (no menu **+**, para um gráfico de linhas) tem **Linhas de Queda**, que descem do ponto mais alto de cada categoria até ao eixo, **Linhas Máx-Mín**, que unem o ponto mais alto e o mais baixo de cada categoria, e **Barras Ascendentes/Descendentes**, que são caixas entre a primeira e a última série de cada categoria. Uma barra ascendente indica que a última série é mais alta. As linhas Máx-Mín e as barras Ascendentes/Descendentes precisam de duas séries ou mais. Os gráficos de área têm Linhas de Queda.
- **Tabela de Dados** (no menu **+**, para um gráfico de colunas, linhas ou áreas) coloca uma grelha com os números por baixo do gráfico, com uma coluna para cada categoria e uma linha para cada série. A sua lista liga e desliga a tabela, e define as linhas entre as linhas, as linhas entre as colunas, o contorno e as chaves da legenda (a cor de cada série ao lado do nome).
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para ver para onde se dirigem. Escolha **Linear** para uma reta, **Exponencial** para números que crescem uma percentagem constante, **Logarítmica** para números que sobem depressa e depois estabilizam, **Polinomial** para uma curva com inflexões, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R-quadrado** escreve o grau de ajuste da linha (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência tem entrada própria na legenda.
- Quando uma série é desenhada contra um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as entradas **Segundo Horizontal** e **Segundo Vertical** para ela. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área do gráfico em vez de ao lado.
- **Etiquetas de Dados** tem também **Etiqueta Contém** (o valor, o nome da categoria, o nome da série, a percentagem e a chave da legenda, que é um pequeno quadrado com a cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato de Número…** (um código de formato, como 0,0, para os números das etiquetas; deixe-o vazio para mostrar os números como estão). **Valor a Partir de Células…** mostra o texto de algumas células como etiquetas, uma célula para cada ponto. Escreva as células, como `=Folha1!$D$2:$D$4`. As etiquetas mudam quando as células mudam. Deixe-o vazio para as retirar. Desative todos os conteúdos e as etiquetas desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** no separador **Gráfico** abrem uma caixa para um eixo. Define os valores mínimo e máximo e as unidades principal e secundária (cada uma automática ou um número que escreve), se os valores aparecem por ordem inversa, as unidades de visualização (as etiquetas mostram os números divididos, por exemplo por 1000 para milhares, pelo que 30 000 aparece como 30; marque **Mostrar o nome das unidades no gráfico** para imprimir o nome, como Milhares, com o título do eixo), se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que convém a números com grandes diferenças de grandeza e que têm de ser todos superiores a 0), onde o outro eixo cruza este, o formato numérico das suas etiquetas, as suas marcas de graduação, onde ficam as etiquetas e quanto são rodadas. Apenas um eixo numérico tem limites e unidades.
- **Formatar Parte do Gráfico…** no separador **Gráfico** (e no menu do botão direito) define o preenchimento e a linha da área do gráfico, da área de traçado (a caixa onde estão as marcas) ou de uma série. Para uma linha, define também se é suavizada e que marca fica em cada ponto. O menu do botão direito de um gráfico tem ainda **Trocar Linhas e Colunas** e os dois comandos de eixo.
- **Elementos Atuais** lista as partes do gráfico: a área do gráfico, a área de traçado e cada série. Escolha uma e **Formatar Seleção** abre a janela de formatação nessa parte. **Repor para Corresponder ao Estilo** retira o que foi definido na parte escolhida e devolve o que o estilo do gráfico lhe atribui. Para a área do gráfico, isso é o seu preenchimento, a sua linha e a cor do texto que um aspeto definiu (o texto claro do aspeto escuro volta à cor de texto do tema). Para a área de traçado, são o seu preenchimento e a sua linha. Para uma série, é a sua cor (a que a escolha de Cores ou o estilo do gráfico lhe atribui), o seu contorno, as suas marcas tal como o tipo de gráfico as tem e as cores dos seus pontos; uma linha suavizada mantém-se quando o tipo de gráfico é suavizado. Não altera títulos, a legenda, etiquetas, eixos, linhas de grelha ou os dados, e não faz nada a uma parte que já corresponda. É um passo de Anular.
- **Mover Gráfico…** no separador **Gráfico** pergunta onde deve ficar o gráfico. Escolha **Nova folha** e escreva um nome, e o gráfico fica numa folha de gráfico própria: uma folha que é apenas um gráfico, sem células. Escolha **Objeto em** e selecione uma folha de cálculo, e o gráfico fica nessa folha. Um gráfico numa folha de gráfico só pode ir para uma folha de cálculo, e a folha de gráfico que deixa é eliminada. O gráfico continua a ler as células que lia. Numa folha de cálculo mantém o seu tamanho e o seu lugar na folha, mesmo onde as linhas e colunas são mais altas ou mais largas, e move-se e redimensiona-se com as células como antes. As folhas ocultas não são oferecidas. É um único **Anular**, mesmo depois de ter guardado, a menos que uma folha tenha sido eliminada por esta ação. Um ficheiro no formato `.ods` não consegue guardar uma folha de gráfico, por isso guarde uma folha de gráfico no formato `.xlsx`.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) têm também **Texto do Título…**. Escreva as palavras que pretende. Ou escreva um sinal de igual e uma célula, como **=Folha1!A1**, e o título mostra o que essa célula contém. Quando a célula muda, o título também muda. O ficheiro guarda a ligação.
- **Guardar como Imagem…** no separador **Gráfico** (e no menu do botão direito) guarda o gráfico selecionado como ficheiro de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na folha, para se manter nítido.
- **Guardar como Modelo…** e **Aplicar Modelo…** (no separador **Gráfico**, no grupo **Tipo**) guardam o aspeto de um gráfico num ficheiro **.crtx** e aplicam-no a outro gráfico. O aspeto inclui o tipo, as cores, as definições do título e da legenda, os eixos e os tipos de letra. O gráfico a que aplica o modelo mantém os seus próprios números e as células que lê. Um **Anular** retira de novo o aspeto.
- **Alterar Tipo de Gráfico** muda o tipo de todo o gráfico, ou de cada série: colunas com uma linha por cima, ou uma série contra um segundo eixo vertical.

Uma referência a células escreve-se como **Folha1!$B$2:$B$9**, com o nome da folha entre aspas simples quando tem espaços.

## Minigráficos

Um minigráfico é um pequeno gráfico que cabe dentro de uma única célula. Mostra a tendência de uma linha ou de uma coluna de números, mesmo ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. No separador **Inserir**, em **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. Uma **Linha** une os números com uma linha. Uma **Coluna** desenha uma barra para cada número. **Vitória/Derrota** desenha uma barra para cima por cada número acima de zero e uma barra para baixo por cada número abaixo dele, todas do mesmo tamanho, o que convém a resultados que são apenas bons ou maus.
3. Na caixa **Criar Minigráficos**, confirme o **Intervalo de dados** (as células que os minigráficos leem) e escreva o **Intervalo de localização** (as células onde ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lê todos os dados.

Os minigráficos criados num só passo formam um grupo e partilham o mesmo aspeto. Clique numa célula com um minigráfico e aparece o separador **Minigráfico**.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou os dados do que clicou. Define também o que fazer às células vazias (deixar um intervalo, tratá-las como zero ou unir os pontos através delas) e se as células das linhas e colunas ocultas são lidas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** assinala pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, para uma linha, **Marcadores** em cada ponto.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras e a **Espessura** (a espessura da linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto assinalado.
- **Eixo** mostra uma linha no zero, desenha os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos seus próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma só escala, para os poder comparar. **Valor Personalizado** permite escrever um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: escreva as células que contêm a data de cada ponto, e um ponto com data posterior fica mais à frente. Deixe-o vazio para voltar a espaçar os pontos de forma uniforme.
- **Agrupar** coloca os minigráficos selecionados num só grupo. **Desagrupar** dá a cada um o seu próprio aspeto. **Limpar** retira os minigráficos selecionados, ou os grupos inteiros.

Um minigráfico segue as suas células: se inserir uma linha acima dos dados, continua a ler os mesmos números. Cada alteração é um passo que pode anular. Os minigráficos são guardados no ficheiro no formato que outros programas de folhas de cálculo leem.

## Inserir uma imagem, uma forma ou uma caixa de texto

No separador **Inserir**, em **Ilustrações**, **Imagem** abre uma janela de escolha de ficheiros. Escolha um ficheiro de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele fica na folha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber e mantém a sua forma. O livro guarda a própria imagem. Uma imagem de um tipo que o livro não consegue guardar tal como está (um SVG, por exemplo) é guardada como PNG, uma imagem simples feita de pontos. Um ficheiro que não é uma imagem, ou que tem mais de 16 MB, é recusado com uma mensagem. Um ficheiro WMF ou EMF (os tipos de imagem que alguns programas mais antigos criam a partir de linhas e formas) é desenhado e também guardado como PNG.

**Ícones** mostra a mesma janela de pequenas imagens de linha que o JeffPub e o JeffDeck têm, com uma caixa para as procurar pelo nome. Escolha um ou mais, e cada um fica na folha como imagem, com 48 pontos de largura e de altura. É desenhado a cinzento-escuro, num tamanho que se mantém nítido quando o amplia. Cada um é um único passo que pode anular.

**Código de Barras** abre um pequeno menu com duas opções. **Código de Barras** abre uma janela para criar um código de barras (**Códigos de Barras a Partir de Células**, abaixo, cria muitos): ISBN de livros (com ou sem preço), EAN-13, UPC-A, EAN-8, Code 128, Code 39 ou um código QR, que contém qualquer texto, como um endereço Web. Escolha um **Tipo**, escreva o texto ou os algarismos, e a janela mostra o código de barras à medida que escreve, e indica a vermelho o que está errado se não for possível criá-lo. Clique em **Inserir**, e ele fica na folha como imagem, num passo que pode anular. Mova-o e redimensione-o como qualquer imagem. Para o alterar, selecione-o e clique novamente em **Código de Barras**, altere o texto e clique em **Atualizar**; o novo código de barras ocupa o lugar e a largura do anterior. Ao guardar, é guardado como imagem com um SVG que se mantém nítido, para que todos os programas o mostrem, e o JeffCrunch guarda as suas definições no ficheiro, pelo que continua a ser um código de barras que pode editar quando voltar a abrir o livro no JeffCrunch.

**Código de Barras > Códigos de Barras a Partir de Células** (no mesmo menu **Código de Barras**) cria um código de barras para cada célula preenchida das células selecionadas, para uma folha de etiquetas. Selecione as células, clique nele e escolha o tipo e o aspeto na mesma janela. O que escrever ali é apenas um exemplo, porque o texto de cada célula se torna o seu próprio código de barras. Os códigos de barras ficam numa coluna logo à direita das células selecionadas, um por baixo do outro, num único passo que pode anular. Uma célula cujo texto o código não consegue conter (uma letra num EAN-13, por exemplo) é indicada numa mensagem e não recebe código de barras. São feitas no máximo 500 células de cada vez; selecione menos se for indicado que há demasiadas. Cada código de barras memoriza a célula de que vem e segue-a: quando altera essa célula, o código de barras é refeito a partir do novo texto, no mesmo lugar e com o mesmo tamanho, como um passo próprio que pode anular. A ligação move-se com a célula quando insere ou elimina linhas ou colunas. Se eliminar a célula, o código de barras mantém o último texto que tinha. Ao guardar, a imagem do ficheiro é feita a partir do texto da célula tal como está nesse momento, para que os outros programas também mostrem o valor atual.

**Formas** abre uma lista: retângulo, retângulo arredondado, oval, triângulo, losango, setas, uma estrela, um coração, uma nuvem, um balão de fala e uma linha. Escolha uma e fica na folha, a azul-claro com uma margem azul mais escura. **Caixa de Texto**, no grupo **Texto**, coloca uma caixa vazia, sem preenchimento e sem margem. Clique em **Desenhar Caixa de Texto** (o botão principal, ou a primeira opção do seu menu) e arraste na folha para desenhar a caixa. Um arrastamento cria uma caixa do tamanho que arrasta, mas nunca com menos de 18 pontos (cerca de 0,6 cm) de lado, e um clique de menos de 4 pontos dá o tamanho habitual. Mantenha **Shift** premido para manter a caixa quadrada. O ponteiro é uma cruz até soltar; **Esc**, ou qualquer outro comando, larga a ferramenta. A caixa é um passo que pode anular.

Para rodar as palavras de uma caixa de texto, selecione-a e clique em **Direção do Texto** no separador **Formato da Forma**: **Horizontal**, **Rodar todo o texto 90°** ou **Rodar todo o texto 270°**. As palavras são escritas já rodadas, tal como são mostradas, e um livro guardado no formato .xlsx ou .ods mantém a direção.

Com uma forma selecionada, **Estilos** no separador **Formato da Forma** dá-lhe um aspeto pronto: um preenchimento e um contorno que combinam, como azul-claro com uma margem azul mais escura. As duas últimas opções mantêm apenas o contorno ou apenas o preenchimento. Um clique é um passo que pode anular. **Preenchimento da Forma** e **Contorno da Forma** alteram cada parte por si.

## Encontrar imagens gratuitas na Internet

**Imagens Online**, no grupo **Ilustrações** do separador **Inserir**, encontra imagens gratuitas. Vêm de duas bibliotecas abertas: o Openverse, que pesquisa muitas coleções, e o Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Clique em **Imagens online…**. Abre-se um painel à direita.
2. Escolha uma biblioteca, escreva o que procura e clique em **Pesquisar** (ou prima Enter).
3. Clique numa imagem para ver quem a fez e que licença tem. Uma **licença** diz como pode usar uma imagem. Clique no nome da licença para a ler.
4. Clique em **Inserir** (ou faça duplo clique na imagem).

**O que o JeffCrunch envia.** As palavras que pesquisa são enviadas para a biblioteca que escolher, e só quando clica em **Pesquisar** ou prime Enter. Depois, o JeffCrunch pede à biblioteca as pequenas pré-visualizações do que encontrou e a imagem que insere. Como qualquer página Web, a biblioteca pode ver o endereço de Internet do seu computador, e é-lhe dito que o JeffCrunch está a pedir. Nada do seu livro é enviado.

A imagem fica na folha como uma imagem de ficheiro, e o livro guarda-a. O seu título é guardado como **texto alternativo**. A maioria das licenças pede que indique o autor. Quando **Adicionar um crédito por baixo da imagem** está marcado, o JeffCrunch acrescenta uma pequena caixa de texto por baixo da imagem com o seu título, o seu autor e a sua licença. Pode mover, alterar ou eliminar essa caixa. A imagem e o seu crédito são inseridos num só passo, por isso **Anular** retira ambos. Verifique se a licença se adequa à forma como vai usar a imagem.

## Acrescentar Texto Artístico

**Texto Artístico** são palavras com efeitos: letras com cor, contorno, sombra ou uma forma curva.

1. Clique em **Inserir > Texto > Texto Artístico**. Abre-se uma galeria de estilos.
2. Clique num estilo. As palavras "O Seu Texto Aqui" ficam na folha nesse estilo.
3. Para alterar as palavras, selecione o Texto Artístico e clique em **Editar Texto…** no separador **Formato da Forma**. Uma caixa pede as palavras, o tipo de letra, o tamanho, o negrito e o itálico. Um tamanho maior torna o Texto Artístico maior.
4. No separador **Formato da Forma**, **Estilos de Texto Artístico** dá às letras outro estilo, e **Deformar Texto** dobra as palavras numa forma, como uma onda ou um arco. "Simples" mantém-nas direitas.

O Texto Artístico é uma forma no desenho da folha, com a sua deformação, o seu preenchimento e o seu contorno, pelo que mantém o aspeto quando guarda o livro no formato .xlsx e o volta a abrir, no JeffCrunch ou noutro programa. Cada alteração é um passo que pode anular. Um livro guardado no formato .ods mantém também as suas imagens, formas, caixas de texto, grupos, linhas e Texto Artístico. O Texto Artístico é guardado como a forma de texto artístico que o OpenDocument tem, com as suas palavras, tipo de letra, preenchimento, contorno e deformação, e volta a ser Texto Artístico quando abre de novo o ficheiro .ods. Algumas deformações para as quais o OpenDocument não tem forma (Onda 2, Onda 4, e as formas inferior e superior de Insuflar e Esvaziar) são guardadas como palavras retas com as mesmas letras e estilo. Os gráficos também são guardados num ficheiro .ods: veja abaixo.

## Reduzir o tamanho das imagens

Uma imagem tirada com uma câmara pode ser muito maior do que aparece na folha, e isso torna o ficheiro grande. Clique numa imagem e, depois, clique em **Comprimir Imagens…** no grupo **Ajustar** do separador **Formato da Imagem**. Escolha onde o livro será usado: **Impressão comercial (300 ppp)**, **Impressão em secretária (220 ppp)** ou **Web (96 ppp)**. Deixe **Aplicar a todas as imagens desta folha** marcado para reduzir todas as imagens da folha, ou desmarque-o para reduzir apenas a imagem selecionada. **Eliminar áreas recortadas das imagens** retira também as partes escondidas que recortou, e o recorte deixa de existir. Uma imagem só é substituída quando a nova é mais pequena. O seu tamanho, lugar, nome e texto alternativo mantêm-se. Anular (Ctrl+Z) devolve as imagens, e o livro guardado contém apenas as imagens mais pequenas. As imagens de outras folhas não são alteradas: vá a cada folha e repita a operação. Uma imagem que é um desenho (SVG, EMF ou WMF), ou uma animação, fica como está.

Cada uma é um único passo que pode anular. O novo objeto fica selecionado, pelo que pode arrastá-lo, redimensioná-lo, deslocá-lo para a frente ou para trás, dar-lhe um nome e um texto alternativo em **Propriedades**, ou premir **Delete**.

## Mostrar o texto de uma célula numa forma ou caixa de texto

Uma forma ou caixa de texto pode mostrar o que uma célula mostra, e acompanhá-la. Selecione a forma, clique na **barra de fórmulas**, escreva um sinal de igual e uma célula, e prima **Enter**.

- `=B2` liga a forma a B2 desta folha. `=Folha2!B3` liga-a a uma célula de outra folha. `=Total` liga-a a um nome que representa uma célula.
- A forma mostra então o texto da célula tal como a célula o mostra, com o seu formato numérico. Quando altera a célula, o seu formato, ou uma fórmula de que ela depende, a forma também muda. Um texto mais longo do que uma célula consegue conter (32 767 carateres) é cortado. Todas as formas ligadas de um livro mostram, em conjunto, no máximo 4 MB de texto: acima disso, uma forma mostra o seu texto cortado.
- Com a forma selecionada, a barra de fórmulas mostra a sua ligação, como `=Folha1!$B$2`. Apague o conteúdo da barra de fórmulas e prima **Enter** para retirar a ligação. A forma mantém então as palavras que mostra.
- Escrever palavras na forma também retira a ligação.
- Uma ligação é a uma única célula. Um intervalo como `=A1:B2`, uma lista, um cálculo ou um nome que não seja uma célula é recusado com uma mensagem, e a forma mantém a sua ligação.
- Se a linha ou a coluna da célula for eliminada, ou a sua folha, a forma mostra `#REF!` e mantém a ligação, como uma fórmula faria. Anular devolve a célula.
- Cada ligação, e cada vez que a retira, é um passo que pode anular.

A ligação é guardada num ficheiro .xlsx com o texto que a célula mostrava, pelo que um programa que não segue ligações continua a mostrar as palavras, e o JeffCrunch abre o ficheiro com a ligação a funcionar. O OpenDocument não tem forma de ligar uma caixa de texto a uma célula, por isso um livro guardado no formato .ods mantém as palavras que a forma mostra nesse momento, e não a ligação.

## Alterar o aspeto de uma imagem, forma ou caixa de texto

Selecione uma imagem e aparece o separador **Formato da Imagem**. Selecione uma forma, uma linha ou uma caixa de texto e aparece o separador **Formato da Forma**. Cada alteração é um passo que pode anular, e o objeto mantém o seu nome, o seu texto alternativo, o seu lugar e a sua ordem entre os outros objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (a grossura da linha) e os tracejados (contínuo, tracejado ou pontilhado). Uma imagem tem **Contorno da Imagem**, que é o seu contorno.
- **Rodar** roda o objeto um quarto de volta para a direita ou para a esquerda, ou inverte-o horizontal ou verticalmente.
- **Recortar**, numa imagem, corta-a para uma forma de janela: quadrada, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9 ou 9 por 16. O corte é feito de forma uniforme a partir do meio do que se vê, e a imagem é ampliada até ao tamanho do que resta. **Mostrar a Imagem Completa** repõe o que foi cortado. **Preencher** mantém a moldura e corta a imagem para a cobrir toda. **Ajustar** mostra a imagem inteira, e a moldura toma a forma da imagem dentro da antiga. **Recortar para Forma** (a galeria ao lado de **Recortar**) corta a imagem para uma forma como uma oval, uma estrela ou um retângulo arredondado; a forma fica guardada com a imagem. O ficheiro da imagem não é alterado: a parte cortada fica apenas escondida.
- **Editar Texto** serve para formas e caixas de texto. Escreva as palavras e escolha como ficam na horizontal e na vertical, o tamanho, o negrito ou o normal, e se se ajustam à forma ou ficam numa só linha. Para o Texto Artístico pede as palavras, o tipo de letra, o tamanho, o negrito e o itálico.
- **Painel de Seleção** abre uma lista de todos os objetos da folha pelo nome, com o que está à frente em primeiro. Clique num nome para selecionar esse objeto, o que ajuda quando os objetos se escondem uns aos outros. **Trazer para a Frente** e **Enviar para Trás** movem-no na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou aumentadas.

## Cortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo que não é uma imagem do livro, como uma ligação a um ficheiro, não pode ser copiada, e o JeffCrunch indica o motivo.

Copie um gráfico e cole-o no mesmo livro, em qualquer folha: a cópia lê as mesmas células que a primeira. Cole-o noutro livro e mantém os números que mostrava, mas deixa de ler células, porque as células que lá leria não são as que lhe deram origem.

## Selecionar vários objetos, agrupá-los e alinhá-los

Clique num objeto e, depois, mantenha Shift ou Ctrl premido e clique noutro para o selecionar também. Clique de novo num objeto selecionado com Shift ou Ctrl para o largar. Os objetos selecionados têm cada um uma moldura tracejada. Também pode escolher **Selecionar Objetos** no separador **Formato da Forma** e arrastar uma caixa sobre células vazias: todos os objetos que caibam por completo dentro da caixa ficam selecionados. Prima Esc, ou escolha de novo **Selecionar Objetos**, para voltar a selecionar células. Prima Delete para remover todos os objetos selecionados num só passo.

Com dois ou mais selecionados, o separador **Formato da Forma** tem estes comandos no grupo **Organizar**:

- **Agrupar** junta as formas, imagens e linhas selecionadas num só objeto. Move-se e redimensiona-se como um todo, e o ficheiro guarda-o como grupo. Um gráfico não pode ser colocado num grupo.
- **Desagrupar** desfaz um grupo selecionado. Cada parte volta ao lugar que tinha na folha.
- **Alinhar** alinha os objetos selecionados: **Alinhar à Esquerda**, **Centralizar**, **Alinhar à Direita**, **Alinhar em Cima**, **Alinhar ao Meio** e **Alinhar em Baixo**. Alinham-se pela margem ou pelo meio do conjunto inteiro. **Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam igualmente três ou mais objetos entre os dois das extremidades.

Agrupar, desagrupar e cada opção de alinhamento são um passo que pode anular.

## Dar-lhe um nome

Com um objeto selecionado, a caixa de nomes mostra o seu nome. Escreva aí um novo nome e prima **Enter**. Anular repõe o nome antigo, e o novo nome fica guardado no ficheiro.

## Eliminá-lo

Prima **Delete** com o objeto selecionado. Anular devolve-o.

## O que é guardado

Tudo o que está acima é guardado no ficheiro no formato que outros programas de folhas de cálculo leem, pelo que um gráfico que cria aqui se abre lá da mesma forma. Um gráfico vindo de outro programa com algo que o JeffCrunch não consegue manter quando reescreve um gráfico (barras de erro, um gráfico tridimensional) pode ser movido, redimensionado e eliminado, mas os seus elementos e dados não são alterados aqui.

## Inserir um objeto

Um **objeto** contém o seu próprio ficheiro dentro do livro, para que a aplicação que o criou o possa voltar a abrir. Mostra uma imagem de si próprio, mas não é uma imagem: mantém-se como objeto quando o move, redimensiona, copia ou guarda o livro.

1. Clique em **Inserir** e, depois, em **Objeto**, e escolha **A partir de Intervalo do JeffCrunch**, **A partir do Gráfico do JeffCrunch** ou **A partir da Página do JeffWrite**.
2. Escolha o ficheiro e o intervalo ou o número da página, se a caixa o pedir. Se copiou um na outra aplicação, a caixa oferece-se para o usar.
3. O objeto fica na célula selecionada. Arraste-o para o mover e arraste uma alça para o redimensionar. Um intervalo, gráfico ou página que redimensiona é desenhado de novo no novo tamanho, para se manter nítido. Prima **Delete** para o retirar. Cada um é um único passo de Anular.

Para inserir o que copiou noutra aplicação, clique na seta por baixo de **Colar** e escolha **Como Objeto**.

**Faça duplo clique** num objeto para o abrir na aplicação que o criou. Guarde-o aí, e o objeto aqui mostra a alteração como um passo de Anular. Uma célula em que estava a escrever mantém-se como está. Uma cópia que cola é um objeto próprio: alterar uma não altera a outra. O mesmo acontece com uma folha que copia: os seus objetos são cópias, e a cópia da folha mantém-nos. **Ficheiro**, depois **Informações**, depois **Objetos** lista todos os objetos do livro. Selecione um para o abrir, o transformar numa imagem simples ou o eliminar.

Um objeto criado noutro programa aparece como a imagem que o seu ficheiro guarda, seja que tipo de imagem for. Pode selecioná-lo, mover-o, redimensioná-lo, alinhá-lo com outros objetos e eliminá-lo, e move-se com as células como os seus próprios objetos. Não o pode editar aqui, colocá-lo num grupo nem alterar o seu lugar na ordem dos objetos. É guardado tal como o ficheiro o tinha, apenas com o lugar alterado se o mover. Um objeto que um ficheiro .ods continha funciona da mesma forma. Guardado como ficheiro .xlsx ou .ods, os seus objetos ficam no formato padrão que outros programas leem. Quando é escrito um ficheiro .ods para um objeto cuja imagem não pode ser desenhada, o objeto é mantido sem imagem.

## Diagramas de outros programas

Um **diagrama** é uma imagem de caixas e setas, como um processo, criada por outro programa. O JeffCrunch mostra um diagrama a partir da imagem que o ficheiro guarda junto dele. Clique nele para o selecionar e arraste-o para o mover. Quando guarda o ficheiro, o diagrama é escrito de volta tal como era, para que o programa que o criou o possa continuar a alterar.

O JeffCrunch ainda não consegue alterar o conteúdo de um diagrama, e um diagrama não pode ser copiado nem colocado num grupo. Se o ficheiro não guardar uma imagem de um diagrama, o JeffCrunch mostra uma simples caixa cinzenta que indica que é um diagrama.

## Gráficos em ficheiros .ods

Um livro guardado no formato .ods mantém os seus gráficos: colunas, barras, linhas, áreas, circulares, anéis, dispersão, radar e gráficos de ações de máximo, mínimo e fecho, ou de abertura, máximo, mínimo e fecho, cada um com as suas séries e as células que lê (nesta folha ou noutra), categorias, título, legenda, eixos (títulos, valor mínimo e máximo, formato numérico), etiquetas de dados, cores e lugar. Um ficheiro .ods criado por outro programa abre com os seus gráficos, desenhados a partir das células que leem. O que um gráfico .ods não tem onde guardar fica de fora e é indicado quando guarda: linhas de tendência, barras de erro, segundos eixos (por isso, um gráfico de ações com volume é guardado com o volume no primeiro eixo, e abre como colunas), e os aspetos extra de um único ponto. Os gráficos de tipos que o JeffCrunch não desenha abrem como colunas dos seus valores.
