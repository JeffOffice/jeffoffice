# Gráficos, minigráficos, imagens e formas

<!-- keywords: gráfico, diagrama, colunas, barras, linhas, circular, área, dispersão, combinado, legenda, título, linhas de grelha, rótulos de dados, dados do gráfico, tipo de gráfico, aspetos, esquemas, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R ao quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitórias/derrotas, mini gráfico, marcadores, nome, texto alternativo, logarítmica, escala logarítmica, guardar como imagem, modelo, crtx, texto do título, chave de legenda -->

Um gráfico é uma imagem de números que permanece ligada a eles: se alterar um número, o gráfico acompanha a alteração. As imagens e as formas são colocadas sobre as células da mesma forma, mas não estão ligadas a números.

## Criar um gráfico

1. Selecione os números que pretende representar. Se a primeira linha tiver nomes e a primeira coluna tiver etiquetas, selecione também essas células. O JeffCrunch analisa a primeira linha e a primeira coluna e usa-as como nomes e etiquetas quando forem palavras (ou datas, no caso das etiquetas).
2. No separador **Inserir**, no grupo **Gráficos**, escolha um tipo: **Colunas**, **Barras**, **Linhas**, **Circular**, **Área**, **Dispersão** ou **Combinado**. A pequena seta ao lado de um tipo mostra as variantes, como colunas empilhadas, uma linha com marcadores ou um gráfico de dispersão de pontos, de pontos unidos ou de linhas suaves.
3. Para ver primeiro todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Trocar linhas e colunas** inverte o sentido das séries.

Se selecionar apenas uma célula, o JeffCrunch representa o bloco de células à volta dela que tenha conteúdo.

As séries seguem as colunas quando a seleção tiver pelo menos tantas linhas como colunas, e seguem as linhas quando tiver mais colunas do que linhas. Um gráfico circular tem uma série. Um gráfico de dispersão toma os valores horizontais da primeira coluna.

Exemplo: seleciona A1:C4, em que A2:A4 contêm T1, T2 e T3, B1 e C1 contêm Este e Oeste, e o resto são números. O gráfico tem duas séries, Este e Oeste, e três grupos de colunas, T1 a T3.

## Mover e redimensionar

Clique num gráfico, imagem ou forma para o selecionar. Arraste-o para o mover, ou arraste um dos oito quadrados do seu contorno para o redimensionar.

- Mantenha **Alt** premida enquanto arrasta para alinhar um limite com o limite de uma célula.
- Mantenha **Shift** premida enquanto arrasta um canto para manter as proporções.
- As teclas de seta deslocam o objeto alguns pixels de cada vez. **Ctrl** com uma seta desloca-o um pixel, **Shift** com uma seta aumenta ou diminui o tamanho, e **Alt** com uma seta vai até ao limite seguinte de uma célula.
- **Esc** enquanto arrasta repõe o objeto na posição inicial.

Cada movimento ou alteração de tamanho é um passo que pode anular. Clique com o botão direito no objeto e escolha **Propriedades** para escrever o **Nome** (cada objeto numa folha precisa de um nome próprio) e o **Texto alternativo** (uma frase curta que indica a uma pessoa que não consegue ver o objeto o que ele mostra), para escrever o tamanho e para escolher o que acontece quando insere, elimina ou redimensiona células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou manter-se no lugar. A escolha fica guardada no ficheiro. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (continua a ser apresentado no ecrã); o ficheiro também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, aparecem dois botões ao lado dele e surge o separador **Gráfico** no friso.

- O botão **+** (e **Partes do Gráfico** no separador) ativa ou desativa o título do gráfico, os eixos, os títulos dos eixos, as linhas de grelha, a legenda e os rótulos de dados, e indica onde ficam a legenda e os rótulos.
- O botão do pincel (e **Cores** e **Aspetos** no separador) dá ao gráfico um dos conjuntos de cores do JeffCrunch e um dos seus aspetos. **Esquemas** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Pode adicionar, editar, remover e reordenar séries, alterar as células que cada uma lê, definir as células com as etiquetas, trocar linhas e colunas e escolher a forma como as células vazias e as linhas e colunas ocultas são representadas.
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para ver para onde se dirigem. Escolha **Linear** para uma reta, **Exponencial** para números que crescem a uma percentagem constante, **Logarítmica** para números que sobem depressa e depois estabilizam, **Polinomial** para uma curva com inflexões, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R ao Quadrado** escreve o grau de ajuste da linha (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência tem entrada própria na legenda.
- Quando uma série é desenhada com um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as entradas **Segundo Horizontal** e **Segundo Vertical** para essa série. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área de desenho em vez de ao lado.
- **Rótulos de Dados** tem também **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a percentagem e a chave de legenda, que é um pequeno quadrado com a cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato de Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe vazio para mostrar os números tal como são). Desative todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** no separador **Gráfico** abrem uma caixa para um eixo. Define os valores mínimo e máximo e as unidades principal e secundária (cada uma automática ou um número que escreve), se os valores aparecem por ordem inversa, se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que convém a números com grandezas muito diferentes e que têm de ser superiores a 0), onde o outro eixo cruza este, o formato de número das suas etiquetas, as marcas de graduação, a posição das etiquetas e o ângulo de rotação das mesmas. Só um eixo numérico tem limites e unidades.
- **Mover Gráfico…** no separador **Gráfico** pede outra folha de cálculo e coloca lá o gráfico, nas mesmas células. Continua a ler as células que lia, mantém o tamanho e a posição na folha, mesmo quando as linhas e colunas são mais altas ou mais largas, e move-se e redimensiona-se com as células como antes. As folhas ocultas não são oferecidas. Corresponde a um único **Anular**, mesmo depois de ter guardado. Não é possível criar ainda uma folha de gráfico (uma folha que seja apenas um gráfico), e um gráfico que esteja numa folha dessas não pode ser movido.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) têm também **Texto do Título…**. Escreva as palavras que pretende. Ou escreva um sinal de igual e uma célula, como **=Folha1!A1**, e o título mostra o que essa célula contém. Quando a célula muda, o título também muda. O ficheiro guarda a ligação.
- **Guardar como Imagem…** no separador **Gráfico** (e no menu do botão direito) guarda o gráfico selecionado como ficheiro de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na folha, para se manter nítido.
- **Guardar como Modelo…** e **Aplicar Modelo…** (no separador **Gráfico**, no grupo **Tipo**) guardam o aspeto de um gráfico num ficheiro **.crtx** e aplicam-no a outro gráfico. O aspeto inclui o tipo, as cores, as definições do título e da legenda, os eixos e os tipos de letra. O gráfico a que o aplica mantém os seus próprios números e as células que lê. Um único **Anular** remove o aspeto novamente.
- **Alterar Tipo de Gráfico** altera o tipo de todo o gráfico, ou de cada série: colunas com uma linha por cima, ou uma série num segundo eixo vertical.

Uma referência a células escreve-se, por exemplo, **Folha1!$B$2:$B$9**, com o nome da folha entre plicas quando tiver espaços.

## Minigráficos

Um minigráfico é um gráfico muito pequeno que cabe dentro de uma célula. Mostra a tendência de uma linha ou de uma coluna de números, mesmo ao lado deles.

1. Selecione os números. Para ter um minigráfico por linha, selecione várias linhas de números.
2. No separador **Inserir**, no grupo **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitórias/Derrotas**. Uma **Linha** une os números com uma linha. Uma **Coluna** desenha uma barra para cada número. **Vitórias/Derrotas** desenha uma barra para cima por cada número superior a zero e uma barra para baixo por cada número inferior, todas do mesmo tamanho, o que convém a resultados que são apenas bons ou maus.
3. Na caixa **Criar Minigráficos**, confirme o **Intervalo de dados** (as células que os minigráficos lêem) e escreva o **Intervalo de localização** (as células onde ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lê todos os dados.

Os minigráficos criados num único passo formam um grupo e partilham o mesmo aspeto. Clique numa célula com um minigráfico e surge o separador **Minigráfico**.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do minigráfico em que clicou. Indica também o que fazer às células vazias (deixar uma interrupção, tratá-las como zero ou unir os pontos através delas) e se as células das linhas e colunas ocultas são lidas.
- **Tipo** alterna entre linha, coluna e vitórias/derrotas.
- **Mostrar** assinala pontos: o **Ponto Mais Alto**, o **Ponto Mais Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, numa linha, **Marcadores** em todos os pontos.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras e a **Espessura** (a espessura da linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto assinalado.
- **Eixo** mostra uma linha no zero, representa os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos seus próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma só escala, para os poder comparar. **Valor Personalizado** permite escrever um número. **Eixo de Datas…** espaça os pontos pelas datas: escreva as células que contêm a data de cada ponto, e um ponto com uma data posterior fica mais à frente. Deixe-o vazio para voltar a espaçar os pontos uniformemente.
- **Agrupar** coloca os minigráficos selecionados num só grupo. **Desagrupar** dá a cada um o seu próprio aspeto. **Limpar** remove os minigráficos selecionados, ou os grupos inteiros a que pertencem.

Um minigráfico segue as suas células: se inserir uma linha acima dos dados, continua a ler os mesmos números. Cada alteração é um passo que pode anular. Os minigráficos são guardados no ficheiro num formato que outros programas de folhas de cálculo conseguem ler.

## Recortar, copiar e colar um objeto

Uma imagem ou forma copia-se e cola-se da mesma forma que um gráfico. Uma forma que usa algo que não seja uma imagem do livro, como uma ligação a um ficheiro, não pode ser copiada, e o JeffCrunch indica-lhe o motivo.

Copie um gráfico e cole-o no mesmo livro, em qualquer folha: a cópia lê as mesmas células que o original. Se o colar noutro livro, mantém os números que mostrava, mas deixa de ler células, porque as células que leria ali não são as de onde foi criado.

## Dar-lhe um nome

Com o objeto selecionado, a caixa de nomes mostra o respetivo nome. Escreva lá um novo nome e prima **Enter**. Anular repõe o nome antigo, e o novo nome fica guardado no ficheiro.

## Eliminar

Prima **Delete** com o objeto selecionado. Anular repõe o objeto.

## O que é guardado

Tudo o que está descrito acima é guardado no ficheiro num formato que outros programas de folhas de cálculo conseguem ler, pelo que um gráfico criado aqui abre-se da mesma forma noutro programa. Um gráfico vindo de outro programa que tenha elementos que o JeffCrunch não consegue manter quando reescreve um gráfico (barras de erro, um gráfico tridimensional) pode ainda ser movido, redimensionado e eliminado, mas os seus elementos e dados não são alterados aqui.
