# Gráficos, minigráficos, imagens e formas

<!-- keywords: botões de campo, botões de gráfico dinâmico, agrupar, desagrupar, alinhar, alinhar à esquerda, distribuir, selecionar objetos, várias formas, escolher vários, caixa de seleção, filtros do gráfico, ocultar série, gráficos recomendados, sugestões de gráfico, gráfico, diagrama, coluna, barra, linha, circular, área, dispersão, combinado, legenda, título, linhas de grelha, rótulos de dados, dados do gráfico, tipo de gráfico, aspetos, esquemas, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R ao quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, ganhos/perdas, minigráfico pequeno, marcadores, nome, texto alternativo, logarítmica, escala logarítmica, guardar como imagem, modelo, crtx, texto do título, chave da legenda, painel de seleção, preenchimento da forma, contorno da forma, rodar, inverter, recortar, editar texto, limite da imagem, inserir imagem, ícones, ícone, wmf, emf, estilos de forma, estilos, inserir forma, caixa de texto, formas, seta, oval, retângulo, estrela -->

Um gráfico é uma imagem de números que permanece ligada a eles: se alterar um número, o gráfico acompanha a alteração. As imagens e as formas são colocadas sobre as células da mesma forma, mas não estão ligadas a nenhum número.

## Criar um gráfico

1. Selecione os números que pretende representar. Se a primeira linha tiver nomes e a primeira coluna tiver etiquetas, selecione-as também. O JeffCrunch analisa a primeira linha e a primeira coluna e usa-as como nomes e etiquetas quando são palavras (ou datas, no caso das etiquetas).
2. No separador **Inserir**, no grupo **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Circular**, **Área**, **Dispersão** ou **Combinado**. A pequena seta ao lado de um tipo mostra as suas variantes, como colunas empilhadas, uma linha com marcadores ou um gráfico de dispersão de pontos, de pontos unidos ou de linhas suaves.
3. Para ver primeiro todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Trocar linhas e colunas** inverte o sentido das séries.

Não sabe qual o tipo a escolher? Selecione as células e escolha **Gráficos Recomendados** no separador **Inserir**. O JeffCrunch analisa as células e apresenta alguns tipos adequados, do mais indicado para o menos indicado. Por exemplo, datas na vertical sugerem uma linha, e uma coluna de poucos números positivos com etiquetas sugere um gráfico circular. Cada tipo é desenhado a partir das suas células. Escolha um e selecione **OK**.

Se selecionar apenas uma célula, o JeffCrunch representa o bloco de células à sua volta que tenha conteúdo.

As séries seguem as colunas quando a seleção tem pelo menos tantas linhas como colunas, e as linhas quando tem mais colunas do que linhas. Um gráfico circular tem uma única série. Um gráfico de dispersão recebe os valores horizontais da primeira coluna.

Exemplo: seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Leste e Oeste, e o restante são números. O gráfico tem duas séries, Leste e Oeste, e três grupos de colunas, de Q1 a Q3.

## Ocultar uma série por agora

Escolha um gráfico. Surgem três pequenos botões ao lado dele. O que tem um funil é **Filtros do Gráfico**. Mostra as séries do gráfico, cada uma com uma caixa de seleção. Desmarque uma caixa para retirar essa série do gráfico. Volte a marcá-la para a repor no mesmo lugar. **Anular** também funciona. Um gráfico mantém sempre pelo menos uma série. Uma série que tenha retirado continua no ficheiro quando o guarda e fica oculta quando o abre, pelo que nada se perde. Volte a marcá-la para a repor. (Um gráfico de anel mostra todas as séries ao ser aberto.)

## Mover e dimensionar

Clique num gráfico, numa imagem ou numa forma para o selecionar. Arraste-o para o mover, ou arraste um dos oito quadrados do seu contorno para o dimensionar.

- Mantenha **Alt** premida ao arrastar para que um lado fique alinhado com a margem de uma célula.
- Mantenha **Shift** premida ao arrastar um canto para manter as proporções.
- As teclas de seta deslocam o objeto alguns pixels de cada vez. **Ctrl** com uma seta desloca-o um pixel, **Shift** com uma seta aumenta ou diminui o objeto, e **Alt** com uma seta vai até à margem seguinte de uma célula.
- **Esc** durante o arrasto repõe o objeto na posição inicial.

Cada movimento ou alteração de tamanho é um passo que pode anular. Clique com o botão direito no objeto e escolha **Propriedades** para escrever o **Nome** (cada objeto numa folha precisa do seu próprio nome) e o **Texto alternativo** (uma frase curta que explica a uma pessoa que não o pode ver o que ele mostra), para escrever o tamanho e para escolher o que acontece quando insere, elimina ou redimensiona células: mover e dimensionar com as células, mover com elas mas manter o tamanho, ou ficar onde está. A escolha fica guardada no ficheiro. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (continua visível no ecrã); o ficheiro também guarda esta escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, surgem dois botões ao seu lado e aparece no friso um separador **Gráfico**.

- O botão **+** (e **Elementos do Gráfico** no separador) ativa ou desativa o título do gráfico, os eixos, os títulos dos eixos, as linhas de grelha, a legenda e os rótulos de dados, e indica onde ficam a legenda e os rótulos.
- O botão do pincel (e **Cores** e **Aspetos** no separador) dá ao gráfico um dos conjuntos de cores do JeffCrunch e um dos seus aspetos. **Esquemas** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Pode adicionar, editar, remover e reordenar séries, alterar as células que cada uma lê, definir as células com as etiquetas, trocar linhas e colunas e escolher como são representadas as células vazias e as linhas e colunas ocultas.
- **Botões de Campo** (no menu **+**, apenas para um gráfico dinâmico) mostra pequenos botões com os nomes dos campos da tabela dinâmica que o gráfico segue: os filtros do relatório por baixo do título, e as linhas, os valores e as colunas ao longo da base. Ative ou desative cada tipo, ou escolha **Mostrar Tudo** ou **Ocultar Tudo**. Um novo gráfico dinâmico mostra-os.
- **Barras de Erro** (no menu **+**, para um gráfico de colunas, barras, linhas, área ou dispersão) desenha uma pequena linha acima e abaixo de cada ponto para mostrar quanto o valor real pode diferir. Escolha **Erro Padrão**, **Percentagem (5%)**, **Desvio Padrão** ou **Valor Fixo (1)**. Escolha **Personalizado** para usar valores próprios: escreva as células que contêm os valores acima de cada ponto (por exemplo, `=Folha1!$D$2:$D$4`) e, a seguir, as células dos valores abaixo. Deixe uma vazia para desenhar as barras de um só lado. As barras mudam quando essas células mudam. Um ficheiro que tenha valores próprios para cada ponto mantém-nos.
- **Linhas** (no menu **+**, para um gráfico de linhas) tem **Linhas de Queda**, que descem do ponto mais alto de cada categoria até ao eixo, **Linhas Máx.-Mín.**, que unem o ponto mais alto e o mais baixo de cada categoria, e **Barras Superior/Inferior**, que são caixas entre a primeira e a última série de cada categoria. Uma barra superior indica que a última série é mais alta. As Linhas Máx.-Mín. e as Barras Superior/Inferior precisam de duas séries ou mais. Os gráficos de área têm Linhas de Queda.
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para ver para onde se dirigem. Escolha **Linear** para uma reta, **Exponencial** para números que crescem a uma percentagem constante, **Logarítmica** para números que sobem depressa e depois estabilizam, **Polinomial** para uma curva com inflexões, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R ao Quadrado** indica o grau de ajuste da linha (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência tem uma entrada própria na legenda.
- Quando uma série é desenhada num segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as entradas **Segundo Horizontal** e **Segundo Vertical** para essa série. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área do gráfico em vez de ao lado.
- **Rótulos de Dados** tem também **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a percentagem e a chave da legenda, que é um pequeno quadrado com a cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato de Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe-o vazio para mostrar os números como estão). **Valor a partir das células…** mostra o texto de algumas células como rótulos, uma célula por ponto. Escreva as células, como `=Folha1!$D$2:$D$4`. Os rótulos mudam quando as células mudam. Deixe-o vazio para os remover. Desative todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** no separador **Gráfico** abrem uma caixa para um eixo. Define os valores mínimo e máximo, as unidades principal e secundária (cada uma automática ou um número que escreve), se os valores aparecem por ordem inversa, as unidades de visualização (as etiquetas mostram os números divididos, por exemplo por 1000 para milhares, pelo que 30 000 aparece como 30; marque **Mostrar o nome das unidades no gráfico** para imprimir o nome, como Milhares, com o título do eixo), se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que é adequado para números com grandes diferenças de tamanho e que têm de ser todos superiores a 0), onde o outro eixo cruza este, o formato de número das etiquetas, as marcas de graduação, a posição das etiquetas e o ângulo de rotação das etiquetas. Só um eixo numérico tem limites e unidades.
- **Mover Gráfico…** no separador **Gráfico** pede outra folha de cálculo e coloca lá o gráfico, nas mesmas células. Continua a ler as células que lia, mantém o seu tamanho e o seu lugar na folha, mesmo quando as linhas e as colunas são mais altas ou mais largas, e move-se e redimensiona-se com as células como antes. As folhas ocultas não são oferecidas. É um único **Anular**, mesmo depois de ter guardado. Ainda não é possível criar uma folha de gráfico (uma folha que seja apenas um gráfico), e um gráfico que esteja numa dessas folhas não pode ser movido.
- **Texto do Título** e **Títulos dos Eixos** (no menu **+**) têm também **Texto do Título…**. Escreva as palavras que pretende. Ou escreva um sinal de igual e uma célula, como **=Folha1!A1**, e o título mostra o conteúdo dessa célula. Quando a célula muda, o título também muda. O ficheiro guarda a ligação.
- **Guardar como Imagem…** no separador **Gráfico** (e no menu do botão direito) guarda o gráfico selecionado como ficheiro de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na folha, para continuar nítido.
- **Guardar como Modelo…** e **Aplicar Modelo…** (no separador **Gráfico**, no grupo **Tipo**) guardam o aspeto de um gráfico num ficheiro **.crtx** e aplicam-no a outro gráfico. O aspeto inclui o tipo, as cores, as definições do título e da legenda, os eixos e os tipos de letra. O gráfico ao qual o aplica mantém os seus próprios números e as células que lê. Um único **Anular** remove novamente o aspeto.
- **Tipo de Gráfico** altera o tipo de todo o gráfico, ou de cada série: colunas com uma linha por cima, ou uma série num segundo eixo vertical.

Uma referência a células escreve-se como **Folha1!$B$2:$B$9**, com o nome da folha entre plicas quando tem espaços.

## Minigráficos

Um minigráfico é um gráfico pequeno que cabe dentro de uma única célula. Mostra a tendência de uma linha ou de uma coluna de números, mesmo ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. No separador **Inserir**, no grupo **Minigráficos**, escolha **Linha**, **Coluna** ou **Ganhos/Perdas**. Uma **Linha** une os números com uma linha. Uma **Coluna** desenha uma barra para cada número. **Ganhos/Perdas** desenha uma barra para cima por cada número acima de zero e uma barra para baixo por cada número abaixo de zero, todas do mesmo tamanho, o que se adequa a resultados que são apenas bons ou maus.
3. Na caixa **Criar Minigráficos**, marque o **Intervalo de dados** (as células que os minigráficos leem) e escreva o **Intervalo de localização** (as células onde ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lê todos os dados.

Os minigráficos criados num só passo formam um grupo e partilham o mesmo aspeto. Clique numa célula com um minigráfico e surge o separador **Minigráfico**.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do que selecionou. Define também o que fazer às células vazias (deixar uma lacuna, tratá-las como zero ou unir os pontos através delas) e se as células em linhas e colunas ocultas são lidas.
- **Tipo** alterna entre linha, coluna e ganhos/perdas.
- **Mostrar** assinala pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, para uma linha, **Marcadores** em cada ponto.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras e a **Espessura** (a espessura da linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto assinalado.
- **Eixo** mostra uma linha no zero, desenha os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos seus próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma só escala, para os poder comparar. **Valor Personalizado** permite escrever um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: escreva as células que contêm a data de cada ponto, e um ponto com uma data posterior fica mais à frente. Deixe-o vazio para voltar a espaçar os pontos de forma uniforme.
- **Agrupar** coloca os minigráficos selecionados num grupo. **Desagrupar** dá a cada um o seu próprio aspeto. **Limpar** remove os minigráficos selecionados, ou os grupos inteiros a que pertencem.

Um minigráfico segue as suas células: se inserir uma linha acima dos dados, continua a ler os mesmos números. Cada alteração é um passo que pode anular. Os minigráficos são guardados no ficheiro num formato que outros programas de folhas de cálculo conseguem ler.

## Inserir uma imagem, uma forma ou uma caixa de texto

No separador **Inserir**, no grupo **Ilustrações**, **Imagem** abre uma janela de seleção de ficheiros. Escolha um ficheiro de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele fica na folha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber, mantendo a forma. A folha de cálculo guarda a própria imagem. Uma imagem de um tipo que o livro não consegue guardar tal como está (um SVG, por exemplo) é guardada como PNG, uma imagem simples formada por pontos. Um ficheiro que não é uma imagem, ou que tem mais de 16 MB, é recusado com uma mensagem. Um ficheiro WMF ou EMF (os tipos de imagem que alguns programas mais antigos criam a partir de linhas e formas) também é desenhado e guardado como PNG.

**Ícones** mostra a mesma janela de pequenas imagens de linhas que o JeffPub e o JeffDeck têm, com uma caixa para as pesquisar pelo nome. Escolha um ou mais, e cada um fica na folha como uma imagem, com 48 pontos de largura e de altura. É desenhado a cinzento-escuro, com um tamanho que se mantém nítido quando o aumenta. Cada um é um único passo que pode anular.

**Formas** abre uma lista: retângulo, retângulo arredondado, oval, triângulo, losango, setas, uma estrela, um coração, uma nuvem, um balão de fala e uma linha. Escolha uma e ela fica na folha, em azul-claro com um contorno mais escuro. **Caixa de Texto**, no grupo **Texto**, coloca uma caixa vazia, sem preenchimento e sem contorno.

Com uma forma selecionada, **Estilos** no separador **Formato da Forma** dá-lhe um aspeto pronto: um preenchimento e um contorno que combinam, como azul-claro com um contorno azul mais escuro. Os dois últimos mantêm apenas o contorno ou apenas o preenchimento. Um clique é um passo que pode anular. **Preenchimento da Forma** e **Contorno da Forma** alteram cada parte separadamente.

Cada um destes é um único passo que pode anular. O novo objeto fica selecionado, pelo que pode arrastá-lo, redimensioná-lo, movê-lo para a frente ou para trás, dar-lhe um nome e texto alternativo em **Propriedades**, ou premir **Eliminar**.

## Alterar o aspeto de uma imagem, forma ou caixa de texto

Selecione uma imagem e surge o separador **Formato da Imagem**. Selecione uma forma, uma linha ou uma caixa de texto e surge o separador **Formato da Forma**. Cada alteração é um passo que pode anular, e o objeto mantém o seu nome, o seu texto alternativo, o seu lugar e a sua ordem em relação aos outros objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (a grossura da linha) e os tracejados (contínuo, tracejado ou pontilhado). Uma imagem tem **Limite da Imagem**, que é o seu contorno.
- **Rodar** roda o objeto um quarto de volta para a direita ou para a esquerda, ou inverte-o de um lado para o outro ou de cima para baixo.
- **Recortar**, numa imagem, corta-a a uma forma de janela: quadrado, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9 ou 9 por 16. O corte é feito uniformemente a partir do centro do que é visível, e a imagem é ampliada até ao tamanho do que resta. **Mostrar a Imagem Completa** repõe o que foi cortado. O ficheiro da imagem não é alterado: a parte cortada fica apenas oculta.
- **Editar Texto** é para formas e caixas de texto. Escreva as palavras e escolha como ficam alinhadas na horizontal e na vertical, o tamanho, negrito ou normal, e se quebram para caber na forma ou ficam numa só linha.
- **Painel de Seleção** abre uma lista de todos os objetos da folha pelo nome, com o que está à frente em primeiro lugar. Clique num nome para selecionar esse objeto, o que ajuda quando os objetos se escondem uns aos outros. **Trazer para a Frente** e **Enviar para Trás** movem-no na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou aumentadas.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo que não é uma imagem do livro, como uma ligação a um ficheiro, não pode ser copiada, e o JeffCrunch indica o motivo.

Copie um gráfico e cole-o no mesmo livro, em qualquer folha: a cópia lê as mesmas células que o original. Se o colar noutro livro, mantém os números que mostrava, mas deixa de ler células, porque as células que leria lá não são as que deram origem ao gráfico.

## Selecionar vários objetos, agrupá-los e alinhá-los

Clique num objeto e, a seguir, mantenha **Shift** ou **Ctrl** premida e clique noutro para o selecionar também. Clique de novo num objeto selecionado com **Shift** ou **Ctrl** para o desselecionar. Os objetos selecionados têm cada um um contorno tracejado. Também pode escolher **Selecionar Objetos** no separador **Formato da Forma** e arrastar uma caixa sobre células vazias: todos os objetos que caibam totalmente dentro da caixa ficam selecionados. Prima Esc, ou escolha **Selecionar Objetos** de novo, para voltar a selecionar células. Prima Delete para remover todos os objetos selecionados de uma só vez.

Com dois ou mais objetos selecionados, o separador **Formato da Forma** tem estes comandos no grupo **Organizar**:

- **Agrupar** junta as formas, imagens e linhas selecionadas num único objeto. Move-se e redimensiona-se como um só, e o ficheiro guarda-o como grupo. Um gráfico não pode ser colocado num grupo.
- **Desagrupar** separa um grupo selecionado. Cada parte volta à posição que tem na folha.
- **Alinhar** alinha os objetos selecionados: **Alinhar à Esquerda**, **Alinhar ao Centro**, **Alinhar à Direita**, **Alinhar em Cima**, **Alinhar ao Meio** e **Alinhar em Baixo**. Alinham-se pela margem ou pelo centro do conjunto inteiro. **Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam três ou mais objetos de forma uniforme entre os dois das extremidades.

Agrupar, Desagrupar e cada opção de alinhamento são um passo que pode anular.

## Dar-lhe um nome

Com um objeto selecionado, a caixa de nome mostra o seu nome. Escreva um novo nome nessa caixa e prima **Enter**. Anular repõe o nome antigo, e o novo nome fica guardado no ficheiro.

## Eliminá-lo

Prima **Delete** com o objeto selecionado. Anular repõe-no.

## O que fica guardado

Tudo o que está descrito acima é guardado no ficheiro num formato que outros programas de folhas de cálculo conseguem ler, pelo que um gráfico criado aqui abre-se da mesma forma noutro programa. Um gráfico vindo de outro programa com algo que o JeffCrunch não consegue manter ao reescrever um gráfico (barras de erro, um gráfico tridimensional) pode continuar a ser movido, dimensionado e eliminado, mas os seus elementos e dados não são alterados aqui.
