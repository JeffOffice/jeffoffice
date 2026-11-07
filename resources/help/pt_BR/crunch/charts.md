# Gráficos, minigráficos, imagens e formas

<!-- keywords: botões de campo, botões de campo de gráfico dinâmico, agrupar, desagrupar, alinhar, alinhar à esquerda, distribuir, selecionar objetos, várias formas, escolher vários, caixa de seleção, filtros do gráfico, ocultar série, gráficos recomendados, sugestões de gráfico, gráfico, diagrama, coluna, barra, linha, pizza, área, dispersão, combinação, legenda, título, linhas de grade, rótulos de dados, dados do gráfico, tipo de gráfico, aparência, layouts, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R-quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, mini gráfico, marcadores, nome, texto alternativo, logarítmica, escala logarítmica, salvar como imagem, modelo, crtx, texto do título, chave de legenda, painel de seleção, preenchimento de forma, contorno de forma, girar, inverter, cortar, editar texto, borda da imagem, inserir imagem, ícones, ícone, wmf, emf, estilos de forma, estilos, inserir forma, caixa de texto, formas, seta, oval, retângulo, estrela -->

Um gráfico é uma imagem de números que permanece ligada a eles: quando você altera um número, o gráfico muda junto. Imagens e formas ficam sobre as células da mesma forma, mas não estão ligadas a nenhum número.

## Criar um gráfico

1. Selecione os números que deseja representar. Se a primeira linha tiver nomes e a primeira coluna tiver rótulos, selecione-os também. O JeffCrunch examina a primeira linha e a primeira coluna e os usa como nomes e rótulos quando são palavras (ou datas, no caso dos rótulos).
2. Na guia **Inserir**, no grupo **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Pizza**, **Área**, **Dispersão** ou **Combinação**. A pequena seta ao lado de um tipo mostra as variações, como colunas empilhadas, uma linha com marcadores ou um gráfico de dispersão de pontos, de pontos unidos ou de linhas suaves.
3. Para ver antes todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado com as suas células. **Alternar Linhas e Colunas** inverte a orientação das séries.

Não sabe qual tipo escolher? Selecione as células e escolha **Gráficos Recomendados** na guia **Inserir**. O JeffCrunch examina as células e lista alguns tipos adequados, do melhor para o pior. Por exemplo, datas na vertical sugerem uma linha, e uma coluna com poucos números positivos e rótulos sugere uma pizza. Cada tipo é desenhado com as suas células. Escolha um e clique em **OK**.

Se você selecionar apenas uma célula, o JeffCrunch cria o gráfico com o bloco de células ao redor dela que contém algum dado.

As séries seguem pelas colunas quando a seleção tem pelo menos tantas linhas quanto colunas, e pelas linhas quando tem mais colunas do que linhas. Uma pizza tem uma única série. Um gráfico de dispersão toma os valores horizontais da primeira coluna.

Exemplo: você seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Leste e Oeste, e o restante são números. O gráfico terá duas séries, Leste e Oeste, e três grupos de colunas, de Q1 a Q3.

## Ocultar uma série por enquanto

Selecione um gráfico. Três pequenos botões aparecem ao lado dele. O que tem um funil é **Filtros do Gráfico**. Ele lista as séries do gráfico, cada uma com uma caixa de seleção. Desmarque uma caixa para deixar a série de fora do gráfico. Marque-a novamente para trazê-la de volta ao mesmo lugar. **Desfazer** também funciona. Um gráfico sempre mantém pelo menos uma série. Uma série que você deixou de fora continua no arquivo ao salvá-lo e permanece oculta quando o arquivo é aberto, portanto nada se perde. Marque-a novamente para trazê-la de volta. (Um gráfico de rosca mostra todas as séries ao ser aberto.)

## Mover e redimensionar

Clique em um gráfico, imagem ou forma para selecioná-lo. Arraste-o para movê-lo, ou arraste um dos oito quadrados da borda para redimensioná-lo.

- Mantenha **Alt** pressionada enquanto arrasta para alinhar uma borda com a borda de uma célula.
- Mantenha **Shift** pressionada enquanto arrasta um canto para manter as proporções.
- As teclas de seta movem o objeto alguns pixels por vez. **Ctrl** com uma seta move o objeto um pixel, **Shift** com uma seta o aumenta ou diminui, e **Alt** com uma seta vai até a próxima borda de uma célula.
- **Esc** enquanto arrasta devolve o objeto à posição original.

Cada movimento ou redimensionamento é uma etapa que você pode desfazer. Clique com o botão direito no objeto e escolha **Propriedades** para digitar o **Nome** (cada objeto em uma planilha precisa ter o seu próprio) e o **Texto alternativo** (uma frase curta que explica a uma pessoa que não pode ver o objeto o que ele mostra), para digitar o tamanho e para escolher o que ele faz ao inserir, excluir ou redimensionar células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou permanecer no lugar. A escolha é salva no arquivo. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (ele continua aparecendo na tela); o arquivo também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, dois botões aparecem ao lado dele e uma guia **Gráfico** surge na faixa de opções.

- O botão **+** (e **Elementos do Gráfico** na guia) liga ou desliga o título do gráfico, os eixos, os títulos dos eixos, as linhas de grade, a legenda e os rótulos de dados, e define onde ficam a legenda e os rótulos.
- O botão de pincel (e **Cores** e **Aparência** na guia) dá ao gráfico um dos conjuntos de cores do JeffCrunch e um dos seus estilos visuais. **Layouts** define vários elementos de uma vez.
- **Dados do Gráfico** lista as séries do gráfico. Você pode adicionar, editar, remover e reordenar séries, alterar as células que cada uma lê, definir as células com os rótulos, trocar linhas por colunas e escolher como são representadas as células vazias e as linhas e colunas ocultas.
- **Botões de Campo** (no menu **+**, apenas para um gráfico dinâmico) mostram pequenos botões com os nomes dos campos da tabela dinâmica que o gráfico acompanha: os filtros do relatório abaixo do título, e as linhas, valores e colunas na parte inferior. Ative ou desative cada tipo, ou escolha **Mostrar Tudo** ou **Ocultar Tudo**. Um novo gráfico dinâmico mostra esses botões.
- **Barras de Erro** (no menu **+**, para um gráfico de coluna, barra, linha, área ou dispersão) desenha uma pequena linha acima e abaixo de cada ponto para mostrar o quanto o valor real pode diferir. Escolha **Erro Padrão**, **Porcentagem (5%)**, **Desvio Padrão** ou **Valor Fixo (1)**. Escolha **Personalizado** para usar valores próprios: digite as células que contêm os valores acima de cada ponto (como `=Planilha1!$D$2:$D$4`) e, depois, as células dos valores abaixo. Deixe uma delas vazia para desenhar as barras de um lado só. As barras mudam quando essas células mudam. Um arquivo que já tem valores próprios para cada ponto os mantém.
- **Linhas** (no menu **+**, para um gráfico de linha) tem **Linhas de Queda**, que descem do ponto mais alto de cada categoria até o eixo, **Linhas Máx-Mín**, que unem o ponto mais alto e o mais baixo de cada categoria, e **Barras para Cima/Para Baixo**, que são caixas entre a primeira e a última série de cada categoria. Uma barra para cima indica que a última série está mais alta. As Linhas Máx-Mín e as Barras para Cima/Para Baixo exigem duas séries ou mais. Gráficos de área têm Linhas de Queda.
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para mostrar para onde eles estão indo. Escolha **Linear** para uma reta, **Exponencial** para números que crescem em porcentagem constante, **Logarítmica** para números que sobem rápido e depois se estabilizam, **Polinomial** para uma curva com inflexões, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R-quadrado** escreve o quão bem a linha se ajusta (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência recebe uma entrada própria na legenda.
- Quando uma série é desenhada em um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as entradas **Segundo Horizontal** e **Segundo Vertical** para ela. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área de plotagem em vez de ao lado dela.
- **Rótulos de Dados** também tem **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a porcentagem e a chave de legenda, que é um pequeno quadrado na cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato de Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe vazio para mostrar os números como são). **Valor de Células…** mostra o texto de algumas células como rótulos, uma célula para cada ponto. Digite as células, como =Planilha1!$D$2:$D$4. Os rótulos mudam quando as células mudam. Deixe vazio para removê-los. Desmarque todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** na guia **Gráfico** abrem uma caixa para cada eixo. Ela define os valores mínimo e máximo e as unidades principal e secundária (cada uma automática ou um número digitado), se os valores aparecem em ordem inversa, as unidades de exibição (os rótulos mostram os números divididos, por exemplo por 1.000 para milhares, de modo que 30.000 aparece como 30; marque **Mostrar o nome das unidades no gráfico** para imprimir o nome, como Milhares, junto ao título do eixo), se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que serve para números de tamanhos muito diferentes e que precisam ser todos maiores que 0), onde o outro eixo cruza este, o formato numérico dos rótulos, as marcas de divisão, a posição dos rótulos e o ângulo de rotação dos rótulos. Apenas um eixo numérico tem limites e unidades.
- **Mover Gráfico…** na guia **Gráfico** pede outra planilha e coloca o gráfico nela, nas mesmas células. Ele continua lendo as células que lia e mantém seu tamanho e sua posição na planilha, mesmo onde as linhas e colunas são mais altas ou mais largas, e se move e redimensiona com as células dali, como antes. Planilhas ocultas não são oferecidas. É uma única etapa de **Desfazer**, mesmo depois de salvar. Uma folha de gráfico (uma planilha que contém apenas um gráfico) ainda não pode ser criada, e um gráfico que está em uma delas não pode ser movido.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) também têm **Texto do Título…**. Digite as palavras que deseja. Ou digite um sinal de igual e uma célula, como **=Planilha1!A1**, e o título mostrará o conteúdo dessa célula. Quando a célula muda, o título também muda. O arquivo mantém o vínculo.
- **Salvar como Imagem…** na guia **Gráfico** (e no menu do botão direito) salva o gráfico selecionado como um arquivo de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na planilha, para que fique nítido.
- **Salvar como Modelo…** e **Aplicar Modelo…** (na guia **Gráfico**, no grupo **Tipo**) guardam a aparência de um gráfico em um arquivo **.crtx** e a aplicam a outro gráfico. A aparência inclui o tipo, as cores, as configurações de título e legenda, os eixos e as fontes. O gráfico ao qual você aplica o modelo mantém os próprios números e as células que lê. Um único **Desfazer** remove a aparência novamente.
- **Alterar Tipo de Gráfico** muda o tipo do gráfico inteiro ou o de cada série: colunas com uma linha sobre elas, ou uma série em um segundo eixo vertical.

Uma referência a células é escrita como **Planilha1!$B$2:$B$9**, com o nome da planilha entre aspas simples quando o nome tem espaços.

## Minigráficos

Um minigráfico é um gráfico minúsculo que cabe dentro de uma única célula. Ele mostra a tendência de uma linha ou coluna de números bem ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. Na guia **Inserir**, no grupo **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. **Linha** une os números com uma linha. **Coluna** desenha uma barra para cada número. **Vitória/Derrota** desenha uma barra para cima para cada número acima de zero e uma barra para baixo para cada número abaixo dele, todas do mesmo tamanho, o que serve para resultados que são apenas bons ou ruins.
3. Na caixa **Criar Minigráficos**, confira o **Intervalo de dados** (as células que os minigráficos leem) e digite o **Intervalo de localização** (as células em que eles ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lerá todos os dados.

Os minigráficos criados em uma etapa formam um grupo e compartilham a mesma aparência. Clique em uma célula com um minigráfico, e a guia **Minigráfico** aparece.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do minigráfico que você clicou. Também define o que fazer com as células vazias (deixar um espaço, tratá-las como zero ou unir os pontos através delas) e se as células de linhas e colunas ocultas devem ser lidas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** marca pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto** e, para uma linha, **Marcadores** em todos os pontos.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras, e a **Espessura** (quão grossa é a linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto marcado.
- **Eixo** mostra uma linha no zero, plota os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma única escala, para que possam ser comparados. **Valor Personalizado** permite digitar um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: digite as células que contêm a data de cada ponto, e um ponto com data posterior fica mais adiante. Deixe vazio para espaçar os pontos igualmente de novo.
- **Agrupar** coloca os minigráficos selecionados em um só grupo. **Desagrupar** dá a cada um a sua própria aparência. **Limpar** remove os minigráficos selecionados ou os grupos inteiros.

Um minigráfico acompanha as suas células: se você inserir uma linha acima dos dados, ele continua lendo os mesmos números. Cada alteração é uma etapa que você pode desfazer. Os minigráficos são salvos no arquivo em um formato que outros programas de planilha conseguem ler.

## Inserir uma imagem, uma forma ou uma caixa de texto

Na guia **Inserir**, no grupo **Ilustrações**, **Imagem** abre uma janela de seleção de arquivos. Escolha um arquivo de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele aparece na planilha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber e mantém sua forma. A pasta de trabalho guarda a própria imagem. Uma imagem de um tipo que a pasta de trabalho não consegue manter como está (um SVG, por exemplo) é guardada como PNG, uma imagem comum formada por pontos. Um arquivo que não é uma imagem, ou que tem mais de 16 MB, é recusado com uma mensagem. Um arquivo WMF ou EMF (os tipos de imagem que alguns programas antigos criam a partir de linhas e formas) também é desenhado e guardado como PNG.

**Ícones** mostra a mesma janela de pequenas imagens de linha que o JeffPub e o JeffDeck têm, com uma caixa para pesquisá-las pelo nome. Escolha um ou mais, e cada um aparece na planilha como uma imagem, com 48 pontos de largura e altura. Ele é desenhado em cinza-escuro, em um tamanho que continua nítido quando você o aumenta. Cada um é uma única etapa que você pode desfazer.

**Formas** abre uma lista: retângulo, retângulo de cantos arredondados, oval, triângulo, losango, setas, uma estrela, um coração, uma nuvem, um balão de fala e uma linha. Escolha uma e ela aparece na planilha em azul-claro, com uma borda mais escura. **Caixa de Texto**, no grupo **Texto**, insere uma caixa vazia sem preenchimento e sem borda.

Com uma forma selecionada, **Estilos** na guia **Formato da Forma** dá a ela um visual pronto: um preenchimento e um contorno que combinam, como azul-claro com borda azul mais escura. As duas últimas opções mantêm apenas o contorno ou apenas o preenchimento. Um clique é uma etapa que você pode desfazer. **Preenchimento da Forma** e **Contorno da Forma** alteram cada parte separadamente.

Cada uma dessas ações é uma única etapa que você pode desfazer. O novo objeto fica selecionado, para que você possa arrastá-lo, redimensioná-lo, levá-lo para a frente ou para trás, dar-lhe um nome e um texto alternativo em **Propriedades**, ou pressionar **Delete**.

## Alterar a aparência de uma imagem, forma ou caixa de texto

Selecione uma imagem e a guia **Formato da Imagem** aparece. Selecione uma forma, uma linha ou uma caixa de texto e a guia **Formato da Forma** aparece. Cada alteração é uma etapa que você pode desfazer, e o objeto mantém o seu nome, o seu texto alternativo, a sua posição e a sua ordem entre os demais objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (quão grossa é a linha) e os traços (sólido, tracejado ou pontilhado). Uma imagem tem **Borda da Imagem**, que é o seu contorno.
- **Girar** gira o objeto em um quarto de volta para a direita ou para a esquerda, ou o inverte horizontal ou verticalmente.
- **Cortar**, para uma imagem, a recorta em uma forma de janela: quadrado, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9 ou 9 por 16. O corte é feito igualmente a partir do meio do que aparece, e a imagem é ampliada até o tamanho do que sobrou. **Mostrar a Imagem Inteira** recupera o que foi cortado. O arquivo da imagem não é alterado: a parte cortada apenas fica oculta.
- **Editar Texto** serve para formas e caixas de texto. Digite as palavras e escolha como elas ficam na horizontal e na vertical, o tamanho, negrito ou normal, e se quebram conforme a forma ou ficam em uma só linha.
- **Painel de Seleção** abre uma lista de todos os objetos da planilha pelo nome, com o que está na frente em primeiro lugar. Clique em um nome para selecionar esse objeto, o que ajuda quando objetos se escondem uns aos outros. **Avançar** e **Recuar** movem o objeto na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou ampliadas.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo diferente de uma imagem da pasta de trabalho, como um vínculo com um arquivo, não pode ser copiada, e o JeffCrunch informa o motivo.

Copie um gráfico e cole-o na mesma pasta de trabalho, em qualquer planilha: a cópia lê as mesmas células que o original. Se colá-lo em outra pasta de trabalho, ele mantém os números que mostrava, mas não lê mais células, porque as células que leria ali não são as que deram origem a ele.

## Selecionar vários objetos, agrupá-los e alinhá-los

Clique em um objeto e, em seguida, mantenha Shift ou Ctrl pressionada e clique em outro para selecioná-lo também. Clique novamente em um objeto selecionado com Shift ou Ctrl para desmarcá-lo. Cada objeto selecionado recebe uma moldura tracejada. Você também pode escolher **Selecionar Objetos** na guia **Formato da Forma** e arrastar uma caixa sobre células vazias: todo objeto que estiver totalmente dentro da caixa é selecionado. Pressione Esc, ou escolha **Selecionar Objetos** de novo, para voltar a selecionar células. Pressione Delete para remover todos os objetos selecionados de uma só vez.

Com dois ou mais objetos selecionados, a guia **Formato da Forma** tem estes comandos no grupo **Organizar**:

- **Agrupar** une as formas, imagens e linhas selecionadas em um único objeto. Ele se move e se redimensiona como um todo, e o arquivo o mantém como grupo. Um gráfico não pode ser colocado em um grupo.
- **Desagrupar** separa um grupo selecionado. Cada parte volta ao lugar em que estava na planilha.
- **Alinhar** alinha os objetos selecionados: **Alinhar à Esquerda**, **Centralizar**, **Alinhar à Direita**, **Alinhar em Cima**, **Alinhar no Meio** e **Alinhar Embaixo**. Eles se alinham à borda ou ao centro de todo o conjunto. **Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam igualmente três ou mais objetos entre os dois das extremidades.

Agrupar, Desagrupar e cada opção de Alinhar são uma única etapa que você pode desfazer.

## Dar um nome

Com um objeto selecionado, a caixa de nome mostra o nome dele. Digite um novo nome ali e pressione **Enter**. **Desfazer** restaura o nome antigo, e o novo nome é salvo no arquivo.

## Excluir

Pressione **Delete** com o objeto selecionado. **Desfazer** o traz de volta.

## O que é salvo

Tudo o que foi descrito acima é salvo no arquivo em um formato que outros programas de planilha conseguem ler, de modo que um gráfico criado aqui abre da mesma forma em outro programa. Um gráfico vindo de outro programa que tenha algo que o JeffCrunch não consegue manter ao regravar um gráfico (barras de erro, um gráfico tridimensional) ainda pode ser movido, redimensionado e excluído, mas os seus elementos e dados não são alterados aqui.
