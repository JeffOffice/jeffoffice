# Gráficos, minigráficos, imagens e formas

<!-- keywords: filtros de gráfico, ocultar série, gráficos recomendados, sugestões de gráfico, gráfico, diagrama, coluna, barra, linha, circular, área, dispersão, combinado, legenda, título, linhas de grelha, rótulos de dados, dados do gráfico, tipo de gráfico, aspeto, esquemas, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, linha de tendência, equação, R ao quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, minigráfico pequeno, marcadores, nome, texto alternativo, texto alternativo, logarítmica, escala logarítmica, guardar como imagem, modelo, crtx, texto do título, chave de legenda, painel de seleção, preenchimento da forma, contorno da forma, rodar, inverter, recortar, editar texto, contorno da imagem, inserir imagem, inserir forma, caixa de texto, formas, seta, oval, retângulo, estrela -->

Um gráfico é uma imagem de números que permanece ligada a eles: se alterar um número, o gráfico acompanha a alteração. As imagens e as formas são colocadas sobre as células da mesma forma, mas não estão ligadas a nenhum número.

## Criar um gráfico

1. Selecione os números que pretende representar. Se a primeira linha tiver nomes e a primeira coluna tiver etiquetas, selecione-as também. O JeffCrunch analisa a primeira linha e a primeira coluna e usa-as como nomes e etiquetas quando forem palavras (ou datas, no caso das etiquetas).
2. No separador **Inserir**, no grupo **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Circular**, **Área**, **Dispersão** ou **Combinado**. A pequena seta ao lado de um tipo mostra as variantes, como colunas empilhadas, uma linha com marcadores ou um gráfico de dispersão de pontos, de pontos unidos ou de linhas suaves.
3. Para ver primeiro todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Trocar linhas e colunas** inverte o sentido das séries.

Não sabe qual o tipo a escolher? Selecione as células e escolha **Gráficos Recomendados** no separador **Inserir**. O JeffCrunch analisa as células e apresenta alguns tipos adequados, do mais indicado para o menos indicado. Por exemplo, datas na vertical sugerem uma linha, e uma coluna com alguns números positivos e etiquetas sugere um gráfico circular. Cada tipo é desenhado a partir das suas células. Escolha um e clique em **OK**.

Se selecionar apenas uma célula, o JeffCrunch representa o bloco de células à volta dela que tenha conteúdo.

As séries seguem as colunas quando a seleção tem pelo menos tantas linhas como colunas, e seguem as linhas quando tem mais colunas do que linhas. Um gráfico circular tem uma só série. Um gráfico de dispersão obtém os valores horizontais da primeira coluna.

Exemplo: seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Leste e Oeste, e o restante são números. O gráfico tem duas séries, Leste e Oeste, e três grupos de colunas, de Q1 a Q3.

## Ocultar uma série por agora

Escolha um gráfico. Ao lado dele aparecem três pequenos botões. O que tem um funil é **Filtros do Gráfico**. Mostra as séries do gráfico, cada uma com uma marca de seleção. Desmarque uma série para a retirar do gráfico. Marque-a novamente para a repor no mesmo lugar. **Anular** também funciona. Um gráfico tem sempre pelo menos uma série. Uma série retirada não é guardada quando grava e fecha o ficheiro, por isso marque-a novamente antes, se a quiser manter.

## Mover e dimensionar

Clique num gráfico, numa imagem ou numa forma para o selecionar. Arraste-o para o mover, ou arraste um dos oito quadrados da respetiva margem para o dimensionar.

- Mantenha **Alt** premida enquanto arrasta para alinhar uma margem com a margem de uma célula.
- Mantenha **Shift** premida enquanto arrasta um canto para manter as proporções.
- As teclas de seta deslocam-no alguns pixels de cada vez. **Ctrl** com uma seta desloca-o um pixel, **Shift** com uma seta aumenta ou diminui o tamanho, e **Alt** com uma seta vai até à margem seguinte de uma célula.
- **Esc** enquanto arrasta repõe o objeto na posição original.

Cada movimento ou alteração de tamanho é um passo que pode anular. Clique com o botão direito no objeto e escolha **Propriedades** para escrever o **Nome** (cada objeto numa folha precisa do seu próprio nome) e o **Texto alternativo** (uma frase curta que explica a uma pessoa que não consegue ver o objeto o que ele mostra), para escrever o tamanho e para escolher o que acontece quando inserir, eliminar ou redimensionar células: mover e dimensionar com as células, mover com elas mas manter o tamanho, ou ficar onde está. A escolha fica guardada no ficheiro. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (continua a ser apresentado no ecrã); o ficheiro também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, aparecem dois botões ao lado dele e surge no friso um separador **Gráfico**.

- O botão **+** (e **Partes do Gráfico** no separador) ativa ou desativa o título do gráfico, os eixos, os títulos dos eixos, as linhas de grelha, a legenda e os rótulos de dados, e indica onde ficam a legenda e os rótulos.
- O botão do pincel (e **Cores** e **Estilos** no separador) dá ao gráfico um dos conjuntos de cores e um dos aspetos do JeffCrunch. **Esquemas** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Pode acrescentar, editar, remover e reordenar séries, alterar as células que cada uma lê, definir as células com as etiquetas, trocar linhas e colunas, e escolher como são representadas as células vazias e as linhas e colunas ocultas.
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para que veja para onde se dirigem. Escolha **Linear** para uma reta, **Exponencial** para números que crescem uma percentagem constante, **Logarítmica** para números que sobem depressa e depois estabilizam, **Polinomial** para uma curva com inflexões, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R ao Quadrado** escreve o grau de ajuste da linha (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência tem entrada própria na legenda.
- Quando uma série é desenhada em relação a um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as entradas **Segundo Horizontal** e **Segundo Vertical** para essa série. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área do gráfico em vez de a pôr ao lado.
- **Rótulos de Dados** tem também **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a percentagem e a chave de legenda, que é um pequeno quadrado com a cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato do Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe-o vazio para mostrar os números como estão). Desligue todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** no separador **Gráfico** abrem uma caixa para um eixo. Define os valores mínimo e máximo e as unidades principal e secundária (automáticas ou um número que escreva), se os valores aparecem por ordem inversa, se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que é útil para números com grandes diferenças de tamanho e que têm de ser superiores a 0), onde o outro eixo o cruza, o formato numérico das etiquetas, as marcas de graduação, a posição das etiquetas e a rotação das mesmas. Apenas um eixo numérico tem limites e unidades.
- **Mover Gráfico…** no separador **Gráfico** pede outra folha de cálculo e coloca o gráfico nela, nas mesmas células. Continua a ler as células que lia e mantém o tamanho e a posição na folha, mesmo quando as linhas e colunas são mais altas ou mais largas, e move-se e redimensiona-se com as células como antes. As folhas ocultas não são oferecidas. Corresponde a um único passo de **Anular**, mesmo depois de ter guardado. Ainda não é possível criar uma folha de gráfico (uma folha que seja só um gráfico), e um gráfico que esteja numa folha dessas não pode ser movido.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) têm também **Texto do Título…**. Escreva as palavras que pretende. Ou escreva um sinal de igual e uma célula, como **=Folha1!A1**, e o título mostra o conteúdo dessa célula. Quando a célula muda, o título também muda. O ficheiro guarda a ligação.
- **Guardar como Imagem…** no separador **Gráfico** (e no menu do botão direito) guarda o gráfico selecionado como ficheiro de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na folha, para que fique nítido.
- **Guardar como Modelo…** e **Aplicar Modelo…** (no separador **Gráfico**, no grupo **Tipo**) guardam o aspeto de um gráfico num ficheiro **.crtx** e aplicam-no a outro gráfico. O aspeto inclui o tipo, as cores, as definições do título e da legenda, os eixos e os tipos de letra. O gráfico ao qual aplica o modelo mantém os seus próprios números e as células que lê. Um passo de **Anular** remove novamente o aspeto.
- **Tipo de Gráfico** altera o tipo de todo o gráfico, ou de cada série: colunas com uma linha por cima, ou uma série em relação a um segundo eixo vertical.

Uma referência a células escreve-se como **Folha1!$B$2:$B$9**, com o nome da folha entre plicas quando tem espaços.

## Minigráficos

Um minigráfico é um pequeno gráfico que cabe dentro de uma célula. Mostra a tendência de uma linha ou de uma coluna de números, mesmo ao lado deles.

1. Selecione os números. Para ter um minigráfico por linha, selecione várias linhas de números.
2. No separador **Inserir**, no grupo **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. Uma **Linha** une os números com uma linha. Uma **Coluna** desenha uma barra para cada número. **Vitória/Derrota** desenha uma barra para cima por cada número acima de zero e uma barra para baixo por cada número abaixo de zero, todas do mesmo tamanho, o que é adequado para resultados que são apenas bons ou maus.
3. Na caixa **Criar Minigráficos**, verifique o **Intervalo de dados** (as células que os minigráficos leem) e escreva o **Intervalo de localização** (as células onde ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lê todos os dados.

Os minigráficos criados num só passo formam um grupo e partilham o mesmo aspeto. Clique numa célula com um minigráfico e aparece o separador **Minigráfico**.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do que selecionou. Também indica o que fazer com as células vazias (deixar um espaço, tratá-las como zero ou unir os pontos através delas) e se devem ser lidas as células em linhas e colunas ocultas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** assinala pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, numa linha, os **Marcadores** em cada ponto.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras e a **Espessura** (a espessura da linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto assinalado.
- **Eixo** mostra uma linha no zero, desenha os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos seus próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro a mesma escala, para que possa compará-los. **Valor Personalizado** permite escrever um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: escreva as células que contêm a data de cada ponto, e um ponto com data posterior fica mais afastado. Deixe o campo vazio para voltar a espaçar os pontos uniformemente.
- **Agrupar** coloca os minigráficos selecionados num só grupo. **Desagrupar** dá a cada um o seu próprio aspeto. **Limpar** remove os minigráficos selecionados ou os grupos inteiros a que pertencem.

Um minigráfico acompanha as suas células: se inserir uma linha acima dos dados, continua a ler os mesmos números. Cada alteração é um passo que pode anular. Os minigráficos ficam guardados no ficheiro no formato que outros programas de folhas de cálculo leem.

## Inserir uma imagem, uma forma ou uma caixa de texto

No separador **Inserir**, no grupo **Ilustrações**, **Imagem** abre um seletor de ficheiros. Escolha um ficheiro de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele é colocado na folha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber, mantendo a sua forma. A pasta de trabalho guarda a própria imagem. Uma imagem de um tipo que a pasta de trabalho não consegue guardar tal como está (um SVG, por exemplo) é guardada como PNG, uma imagem simples formada por pontos. Um ficheiro que não seja uma imagem, ou que tenha mais de 16 MB, é recusado com uma mensagem.

**Formas** abre uma lista: retângulo, retângulo arredondado, oval, triângulo, losango, setas, uma estrela, um coração, uma nuvem, um balão de fala e uma linha. Escolha uma e ela é colocada na folha, a azul-claro, com uma margem mais escura. **Caixa de Texto**, no grupo **Texto**, coloca uma caixa vazia sem preenchimento e sem margem.

Cada um destes passos pode ser anulado. O novo objeto fica selecionado, para que o possa arrastar, dimensionar, mover para a frente ou para trás, dar-lhe um nome e texto alternativo em **Propriedades**, ou premir **Delete**.

## Alterar o aspeto de uma imagem, forma ou caixa de texto

Ao selecionar uma imagem, aparece o separador **Formato da Imagem**. Ao selecionar uma forma, uma linha ou uma caixa de texto, aparece o separador **Formato da Forma**. Cada alteração é um passo que pode anular, e o objeto mantém o nome, o texto alternativo, a posição e a ordem em relação aos outros objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (a espessura da linha) e os traços (contínuo, tracejado ou pontilhado). Uma imagem tem **Contorno da Imagem**, que é o seu contorno.
- **Rodar** roda o objeto um quarto de volta para a direita ou para a esquerda, ou inverte-o horizontal ou verticalmente.
- **Recortar**, numa imagem, corta-a numa forma de janela: quadrada, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9 ou 9 por 16. O corte é feito de forma uniforme a partir do centro do que está visível, e a imagem é ampliada até ocupar o espaço que resta. **Mostrar a Imagem Completa** repõe o que foi cortado. O ficheiro da imagem não é alterado: a parte cortada fica apenas oculta.
- **Editar Texto** serve para formas e caixas de texto. Escreva as palavras e escolha a forma como ficam alinhadas na horizontal e na vertical, o tamanho, o negrito ou o normal, e se quebram para acompanhar a forma ou ficam numa só linha.
- **Painel de Seleção** abre uma lista de todos os objetos da folha pelo nome, com o que está à frente em primeiro lugar. Clique num nome para selecionar esse objeto, o que ajuda quando os objetos se tapam uns aos outros. **Trazer para a Frente** e **Enviar para Trás** movem-no na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou aumentadas.

## Cortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que use algo que não seja uma imagem da pasta de trabalho, como uma ligação a um ficheiro, não pode ser copiada, e o JeffCrunch indica o motivo.

Copie um gráfico e cole-o na mesma pasta de trabalho, em qualquer folha: a cópia lê as mesmas células que o original. Se o colar noutra pasta de trabalho, mantém os números que mostrava, mas deixa de ler células, porque as células que leria ali não são as que deram origem ao gráfico.

## Dar-lhe um nome

Com um objeto selecionado, a caixa de nomes mostra o respetivo nome. Escreva lá um novo nome e prima **Enter**. **Anular** repõe o nome antigo, e o novo nome fica guardado no ficheiro.

## Eliminar

Prima **Delete** com o objeto selecionado. **Anular** repõe-no.

## O que fica guardado

Tudo o que está descrito acima fica guardado no ficheiro no formato que outros programas de folhas de cálculo leem, por isso um gráfico criado aqui abre-se da mesma forma noutros programas. Um gráfico vindo de outro programa, com elementos que o JeffCrunch não consegue manter quando reescreve um gráfico (barras de erro, um gráfico tridimensional), pode ainda ser movido, dimensionado e eliminado, mas os seus elementos e dados não são alterados aqui.
