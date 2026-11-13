# Gráficos, minigráficos, imagens e formas

<!-- keywords: botões de campo, botões de campo de gráfico dinâmico, agrupar, desagrupar, alinhar, alinhar à esquerda, distribuir, selecionar objetos, várias formas, selecionar várias, caixa de arrasto, filtros do gráfico, ocultar séries, gráficos recomendados, sugestões de gráfico, gráfico, diagrama de colunas, barras, linha, pizza, área, dispersão, combinação, legenda, título, linhas de grade, rótulos de dados, dados do gráfico, tipo de gráfico, aparência, layouts, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R-quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, mini gráfico, marcadores, nome, texto alternativo, logarítmica, escala logarítmica, salvar como imagem, modelo, crtx, texto do título, chave da legenda, painel de seleção, preenchimento de forma, contorno de forma, girar, inverter, cortar, editar texto, borda da imagem, inserir imagem, ícones, ícone, wmf, emf, estilos de forma, estilos, inserir forma, caixa de texto, formas, seta, oval, retângulo, estrela, diagrama, diagramas, imagens online, imagens gratuitas, openverse, wikimedia commons, licença, crédito, texto artístico, wordart, distorção, compactar imagens, reduzir imagens, tamanho do arquivo, ppi, excluir áreas cortadas, código de barras, QR code -->

Um gráfico é uma imagem de números que permanece ligada a eles: quando você altera um número, o gráfico muda junto. Imagens e formas ficam sobre as células da mesma forma, mas não estão ligadas a nenhum número.

## Criar um gráfico

1. Selecione os números que deseja representar. Se a primeira linha tiver nomes e a primeira coluna tiver rótulos, selecione-os também. O JeffCrunch examina a primeira linha e a primeira coluna e os usa como nomes e rótulos quando forem palavras (ou datas, para os rótulos).
2. Na guia **Inserir**, no grupo **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Pizza**, **Área**, **Dispersão** ou **Combinação**. A pequena seta ao lado de um tipo mostra as variações, como colunas empilhadas, linha com marcadores ou um gráfico de dispersão de pontos, de pontos unidos ou de linhas suavizadas.
3. Para ver primeiro todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Alternar Linhas e Colunas** inverte o sentido das séries.

Não sabe qual tipo escolher? Selecione suas células e escolha **Gráficos Recomendados** na guia **Inserir**. O JeffCrunch analisa as células e lista alguns tipos adequados a elas, do melhor para o pior. Por exemplo, datas na lateral sugerem uma linha, e uma coluna de poucos números positivos com rótulos sugere uma pizza. Cada tipo é desenhado a partir das suas células. Escolha um e clique em **OK**.

Se você selecionar apenas uma célula, o JeffCrunch representa o bloco de células em volta dela que tenha algum conteúdo.

As séries seguem pelas colunas quando a seleção tem pelo menos tantas linhas quanto colunas, e pelas linhas quando tem mais colunas do que linhas. Uma pizza tem uma única série. Um gráfico de dispersão usa os valores horizontais da primeira coluna.

Exemplo: você seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Leste e Oeste, e o restante são números. O gráfico terá duas séries, Leste e Oeste, e três grupos de colunas, de Q1 a Q3.

## Ocultar uma série por enquanto

Selecione um gráfico. Três pequenos botões aparecem ao lado dele. O que tem um funil é **Filtros do Gráfico**. Ele lista as séries do gráfico, cada uma com uma caixa de seleção. Desmarque uma caixa para tirar aquela série do gráfico. Marque-a novamente para trazê-la de volta no mesmo lugar. **Desfazer** também funciona. Um gráfico sempre mantém pelo menos uma série. Uma série que você tirou continua no arquivo quando você o salva e permanece oculta ao abri-lo, portanto nada se perde. Marque-a novamente para trazê-la de volta. (Um gráfico de rosca mostra todas as séries ao ser aberto.)

## Mover e redimensionar

Clique em um gráfico, imagem ou forma para selecioná-lo. Arraste-o para movê-lo, ou arraste um dos oito quadradinhos da borda para redimensioná-lo.

- Mantenha **Alt** pressionada enquanto arrasta para alinhar uma borda com a borda de uma célula.
- Mantenha **Shift** pressionada enquanto arrasta um canto para manter as proporções.
- As setas do teclado movem o objeto alguns pixels por vez. **Ctrl** com uma seta move o objeto um pixel, **Shift** com uma seta o aumenta ou diminui, e **Alt** com uma seta vai até a próxima borda de uma célula.
- **Esc** durante o arrasto devolve o objeto à posição original.

Cada movimento ou redimensionamento é uma etapa que você pode desfazer. Clique com o botão direito no objeto e escolha **Propriedades** para digitar o **Nome** (cada objeto de uma planilha precisa de um nome próprio) e o **Texto alternativo** (uma frase curta que explica a quem não pode ver o objeto o que ele mostra), para digitar o tamanho e para escolher o que ele faz ao inserir, excluir ou redimensionar células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou permanecer no lugar. A escolha é salva no arquivo. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (ele continua aparecendo na tela); o arquivo também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, dois botões aparecem ao lado dele e a guia **Gráfico** surge na faixa de opções.

- O botão **+** (e **Elementos do Gráfico** na guia) ativa ou desativa o título, os eixos, os títulos dos eixos, as linhas de grade, a legenda e os rótulos de dados, e indica onde ficam a legenda e os rótulos.
- O botão de pincel (e **Cores** e **Aparência** na guia) dá ao gráfico um dos conjuntos de cores do JeffCrunch e uma das suas aparências. **Layouts** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Você pode adicionar, editar, remover e reordenar as séries, alterar as células que cada uma lê, definir as células com os rótulos, trocar linhas por colunas e escolher como as células vazias e as linhas e colunas ocultas são representadas.
- **Botões de Campo** (no menu **+**, apenas para um gráfico dinâmico) mostra pequenos botões com os nomes dos campos da tabela dinâmica que o gráfico acompanha: os filtros do relatório abaixo do título e os campos de linhas, valores e colunas na parte inferior. Ative ou desative cada tipo, ou escolha **Mostrar Tudo** ou **Ocultar Tudo**. Um novo gráfico dinâmico mostra esses botões.
- **Barras de Erro** (no menu **+**, para um gráfico de colunas, barras, linhas, área ou dispersão) desenha uma pequena linha acima e abaixo de cada ponto para mostrar o quanto o valor real pode diferir. Escolha **Erro Padrão**, **Porcentagem (5%)**, **Desvio Padrão** ou **Valor Fixo (1)**. Escolha **Personalizado** para usar valores próprios: digite as células que contêm os valores acima de cada ponto (como `=Planilha1!$D$2:$D$4`) e, em seguida, as células dos valores abaixo. Deixe uma vazia para desenhar as barras de um lado só. As barras mudam quando essas células mudam. Um arquivo que tenha valores próprios para cada ponto os mantém.
- **Linhas** (no menu **+**, para um gráfico de linhas) tem **Linhas de Queda**, que descem do ponto mais alto de cada categoria até o eixo, **Linhas Máx-Mín**, que unem o ponto mais alto e o mais baixo de cada categoria, e **Barras para Cima/para Baixo**, que são caixas entre a primeira e a última série de cada categoria. Uma barra para cima indica que a última série está mais alta. As Linhas Máx-Mín e as Barras para Cima/para Baixo exigem duas séries ou mais. Gráficos de área têm Linhas de Queda.
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para você ver para onde eles se dirigem. Escolha **Linear** para uma reta, **Exponencial** para números que crescem em uma porcentagem constante, **Logarítmica** para números que sobem rapidamente e depois se estabilizam, **Polinomial** para uma curva com inflexões, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R-quadrado** escreve o quanto a linha se ajusta bem (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência recebe uma entrada própria na legenda.
- Quando uma série é desenhada em um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as opções **Horizontal Secundário** e **Vertical Secundário** para ela. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área de plotagem em vez de ao lado dela.
- **Rótulos de Dados** também tem **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a porcentagem e a chave da legenda, que é um pequeno quadrado na cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato de Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe-o vazio para mostrar os números como estão). **Valor de Células…** mostra o texto de algumas células como rótulos, uma célula para cada ponto. Digite as células, como =Planilha1!$D$2:$D$4. Os rótulos mudam quando as células mudam. Deixe vazio para removê-los. Desative todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** na guia **Gráfico** abrem uma caixa para um eixo. Ela define os valores mínimo e máximo e as unidades principal e secundária (cada uma automática ou um número digitado), se os valores aparecem em ordem inversa, as unidades de exibição (os rótulos mostram os números divididos, como por 1.000 para milhares, de modo que 30.000 aparece como 30; marque **Mostrar o nome das unidades no gráfico** para imprimir o nome, como Milhares, junto ao título do eixo), se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que serve para números de tamanhos muito diferentes e que devem ser todos maiores que 0), onde o outro eixo o cruza, o formato numérico dos seus rótulos, suas marcas de divisão, onde ficam seus rótulos e quanto os rótulos são girados. Apenas um eixo numérico tem limites e unidades.
- **Mover Gráfico…** na guia **Gráfico** pergunta onde o gráfico deve ficar. Escolha **Nova planilha** e digite um nome, e o gráfico recebe uma folha de gráfico própria: uma planilha que contém apenas um gráfico, sem células. Escolha **Objeto em** e selecione uma planilha, e o gráfico vai para essa planilha. Um gráfico em uma folha de gráfico só pode ir para uma planilha, e a folha de gráfico que ele deixa é excluída. O gráfico continua lendo as células que lia. Em uma planilha, ele mantém seu tamanho e sua posição na folha, mesmo onde as linhas e colunas são mais altas ou mais largas, e se move e se redimensiona com as células, como antes. As planilhas ocultas não são oferecidas. É uma única etapa de **Desfazer**, mesmo depois de salvar, a menos que uma planilha tenha sido excluída por causa dela. Um arquivo no formato `.ods` não pode conter uma folha de gráfico, por isso salve uma folha de gráfico no formato `.xlsx`.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) também têm **Texto do Título…**. Digite as palavras que desejar. Ou digite um sinal de igual e uma célula, como **=Planilha1!A1**, e o título mostrará o conteúdo dessa célula. Quando a célula muda, o título também muda. O arquivo guarda o vínculo.
- **Salvar como Imagem…** na guia **Gráfico** (e no menu do botão direito) salva o gráfico selecionado como um arquivo de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na planilha, para que continue nítido.
- **Salvar como Modelo…** e **Aplicar Modelo…** (na guia **Gráfico**, no grupo **Tipo**) guardam a aparência de um gráfico em um arquivo **.crtx** e a aplicam a outro gráfico. A aparência inclui o tipo, as cores, as configurações de título e legenda, os eixos e as fontes. O gráfico ao qual você aplica o modelo mantém seus próprios números e as células que lê. Um **Desfazer** remove a aparência novamente.
- **Alterar Tipo de Gráfico** muda o tipo do gráfico inteiro ou de cada série: colunas com uma linha por cima, ou uma série em um segundo eixo vertical.

Uma referência a células é escrita como **Planilha1!$B$2:$B$9**, com o nome da planilha entre aspas simples quando ele tiver espaços.

## Minigráficos

Um minigráfico é um gráfico minúsculo que cabe dentro de uma única célula. Ele mostra a tendência de uma linha ou coluna de números bem ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. Na guia **Inserir**, no grupo **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. A **Linha** une os números com uma linha. A **Coluna** desenha uma barra para cada número. **Vitória/Derrota** desenha uma barra para cima para cada número acima de zero e uma barra para baixo para cada número abaixo dele, todas do mesmo tamanho, o que serve para resultados que são apenas bons ou ruins.
3. Na caixa **Criar Minigráficos**, marque o **Intervalo de dados** (as células que os minigráficos leem) e digite o **Intervalo de localização** (as células onde eles ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lê todos os dados.

Os minigráficos criados em uma etapa formam um grupo e compartilham uma mesma aparência. Clique em uma célula com minigráfico, e a guia **Minigráfico** aparece.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do minigráfico em que você clicou. Também define o que fazer com as células vazias (deixar uma lacuna, tratá-las como zero ou unir os pontos através delas) e se as células de linhas e colunas ocultas devem ser lidas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** marca pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, para uma linha, **Marcadores** em cada ponto.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras, e a **Espessura** (quão grossa é a linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto marcado.
- **Eixo** mostra uma linha no zero, plota os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma só escala, para que você possa compará-los. **Valor Personalizado** permite digitar um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: digite as células que contêm a data de cada ponto, e um ponto com data posterior fica mais adiante. Deixe-o vazio para espaçar os pontos igualmente de novo.
- **Agrupar** coloca os minigráficos selecionados em um só grupo. **Desagrupar** dá a cada um a sua própria aparência. **Limpar** remove os minigráficos selecionados, ou os grupos inteiros a que pertencem.

Um minigráfico acompanha as suas células: se você inserir uma linha acima dos dados, ele continua lendo os mesmos números. Cada alteração é uma etapa que você pode desfazer. Os minigráficos são salvos no arquivo em um formato que outros programas de planilha conseguem ler.

## Inserir uma imagem, uma forma ou uma caixa de texto

Na guia **Inserir**, no grupo **Ilustrações**, **Imagem** abre uma janela de escolha de arquivo. Escolha um arquivo de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele aparece na planilha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber e mantém sua forma. A pasta de trabalho guarda a própria imagem. Uma imagem de um tipo que a pasta de trabalho não consegue guardar do jeito que está (um SVG, por exemplo) é guardada como PNG, uma imagem simples formada por pontos. Um arquivo que não é uma imagem, ou que tem mais de 16 MB, é recusado com uma mensagem. Um arquivo WMF ou EMF (os tipos de imagem que alguns programas antigos criam a partir de linhas e formas) também é desenhado e guardado como PNG.

**Ícones** mostra a mesma janela de pequenas imagens de linha que o JeffPub e o JeffDeck têm, com uma caixa para pesquisá-las pelo nome. Escolha um ou mais, e cada um aparece na planilha como uma imagem de 48 pontos de largura e altura. Ele é desenhado em cinza-escuro, em um tamanho que continua nítido ao ser ampliado. Cada um é uma única etapa que você pode desfazer.

**Código de Barras** abre um pequeno menu com duas opções. **Código de Barras** abre uma janela para criar um código de barras (**Códigos de Barras das Células**, abaixo, cria vários): ISBN de livro (com ou sem preço), EAN-13, UPC-A, EAN-8, Code 128, Code 39 ou um QR code, que pode conter qualquer texto, como um endereço da web. Escolha um **Tipo**, digite o texto ou os dígitos, e a janela mostra o código de barras enquanto você digita, indicando em vermelho o que está errado caso ele não possa ser criado. Clique em **Inserir**, e ele aparece na planilha como uma imagem, em uma etapa que você pode desfazer. Mova-o e redimensione-o como qualquer imagem. Para alterá-lo, selecione-o e clique em **Código de Barras** novamente, mude o texto e clique em **Atualizar**; o novo código de barras ocupa o lugar e a largura do antigo. Ao salvar, ele é guardado como uma imagem com um SVG que continua nítido, para que todos os programas o exibam, e o JeffCrunch guarda suas configurações no arquivo, de modo que continua sendo um código de barras que você pode editar quando abrir a pasta de trabalho novamente no JeffCrunch.

**Código de Barras > Códigos de Barras das Células** (no mesmo menu **Código de Barras**) cria um código de barras para cada célula preenchida das células selecionadas, para uma folha de etiquetas. Selecione as células, clique nessa opção e depois escolha o tipo e a aparência na mesma janela. O que você digitar ali é apenas um exemplo, porque o texto de cada célula se torna o código de barras dela. Os códigos de barras são colocados em uma coluna logo à direita das células selecionadas, um abaixo do outro, em uma única etapa que você pode desfazer. Uma célula cujo texto o código não consegue conter (uma letra em um EAN-13, por exemplo) é indicada em uma mensagem e não recebe código de barras. No máximo 500 células são processadas de uma vez; selecione menos se a mensagem indicar que há células demais. Cada código de barras é uma imagem comum desenhada a partir do texto da célula no momento do clique: se você alterar a célula depois, o código de barras não acompanha a mudança, então selecione as células e crie-os novamente.

**Formas** abre uma lista: retângulo, retângulo de cantos arredondados, oval, triângulo, losango, setas, uma estrela, um coração, uma nuvem, um balão de fala e uma linha. Escolha uma, e ela aparece na planilha em azul-claro com borda mais escura. **Caixa de Texto**, no grupo **Texto**, insere uma caixa vazia, sem preenchimento e sem borda.

Com uma forma selecionada, **Estilos** na guia **Formato da Forma** dá a ela uma aparência pronta: um preenchimento e um contorno que combinam, como azul-claro com borda azul mais escura. As duas últimas opções mantêm apenas o contorno ou apenas o preenchimento. Um clique é uma etapa que você pode desfazer. **Preenchimento da Forma** e **Contorno da Forma** alteram cada parte separadamente.

## Encontrar imagens gratuitas na internet

**Imagens Online**, no grupo **Ilustrações** da guia **Inserir**, encontra imagens gratuitas. Elas vêm de duas bibliotecas abertas: o Openverse, que pesquisa muitas coleções, e o Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Clique em **Imagens Online…**. Um painel se abre à direita.
2. Escolha uma biblioteca, digite o que procura e clique em **Pesquisar** (ou pressione Enter).
3. Clique em uma imagem para ver quem a criou e qual licença ela tem. Uma **licença** diz como você pode usar a imagem. Clique no nome da licença para lê-la.
4. Clique em **Inserir** (ou clique duas vezes na imagem).

**O que o JeffCrunch envia.** As palavras da sua pesquisa são enviadas à biblioteca que você escolheu, e somente quando você clica em **Pesquisar** ou pressiona Enter. Depois, o JeffCrunch pede à biblioteca as pequenas visualizações do que foi encontrado e a imagem que você inserir. Como acontece com qualquer página da web, a biblioteca pode ver o endereço de internet do seu computador, e é informada de que o JeffCrunch está fazendo a solicitação. Nada da sua pasta de trabalho é enviado.

A imagem aparece na planilha como uma imagem de arquivo, e a pasta de trabalho a guarda. Seu título é salvo como **texto alternativo**. A maioria das licenças pede que você cite o autor. Quando **Adicionar crédito abaixo da imagem** está marcado, o JeffCrunch acrescenta uma pequena caixa de texto sob a imagem com o título, o autor e a licença. Você pode movê-la, alterá-la ou excluí-la. A imagem e o crédito entram em uma única etapa, de modo que **Desfazer** remove os dois. Verifique se a licença se adequa ao modo como você vai usar a imagem.

## Adicionar Texto Artístico

**Texto Artístico** são palavras decoradas: letras com cor, contorno, sombra ou forma curva.

1. Clique em **Inserir > Texto > Texto Artístico**. Abre-se uma galeria de estilos.
2. Clique em um estilo. As palavras "Seu Texto Aqui" aparecem na planilha nesse estilo.
3. Para mudar as palavras, selecione o Texto Artístico e clique em **Editar Texto…** na guia **Formato da Forma**. Uma caixa pede as palavras, a fonte, o tamanho, o negrito e o itálico. Um tamanho maior deixa o Texto Artístico maior.
4. Na guia **Formato da Forma**, **Estilos de Texto Artístico** dá às letras outro estilo, e **Distorção de Texto** dobra as palavras em uma forma, como uma onda ou um arco. "Simples" as mantém retas.

O Texto Artístico é uma forma do desenho da planilha, com sua distorção, seu preenchimento e seu contorno, por isso mantém a aparência quando você salva a pasta de trabalho como arquivo .xlsx e a abre novamente, no JeffCrunch ou em outro programa. Cada alteração é uma etapa que você pode desfazer. Uma pasta de trabalho salva como arquivo .ods mantém também as imagens, formas, caixas de texto, grupos, linhas e Textos Artísticos. O Texto Artístico é salvo como a forma de fontwork que o OpenDocument possui, com suas palavras, fonte, preenchimento, contorno e distorção, e volta como Texto Artístico ao abrir o arquivo .ods novamente. Algumas distorções para as quais o OpenDocument não tem forma (Onda 2, Onda 4, e as formas inferior e superior de Inflar e Desinflar) são salvas como palavras retas, com as mesmas letras e estilo. Os gráficos também são mantidos em um arquivo .ods: veja abaixo.

## Reduzir imagens

Uma foto tirada com câmera pode ser muito maior do que aparece na planilha e deixar o arquivo grande. Clique em uma imagem e depois em **Compactar Imagens…** no grupo **Ajustar** da guia **Formato da Imagem**. Escolha onde a pasta de trabalho será usada: **Impressão comercial (300 ppi)**, **Impressão em desktop (220 ppi)** ou **Web (96 ppi)**. Deixe **Aplicar a todas as imagens desta planilha** marcado para reduzir todas as imagens da planilha, ou desmarque para reduzir apenas a imagem selecionada. **Excluir áreas cortadas das imagens** também remove as partes ocultas que você cortou, e o corte deixa de existir. Uma imagem só é substituída quando a nova é menor. Seu tamanho, lugar, nome e texto alternativo permanecem como estavam. **Desfazer** (Ctrl+Z) devolve as imagens originais, e a pasta de trabalho salva guarda apenas as imagens menores. As imagens de outras planilhas não são alteradas: vá até cada planilha e faça o procedimento novamente. Uma imagem que seja um desenho (SVG, EMF ou WMF), ou uma animação, é mantida como está.

Cada uma delas é uma única etapa que você pode desfazer. O novo objeto fica selecionado, então você pode arrastá-lo, redimensioná-lo, levá-lo para frente ou para trás, dar-lhe um nome e um texto alternativo em **Propriedades**, ou pressionar **Delete**.

## Alterar a aparência de uma imagem, forma ou caixa de texto

Selecione uma imagem e a guia **Formato da Imagem** aparece. Selecione uma forma, uma linha ou uma caixa de texto e a guia **Formato da Forma** aparece. Cada alteração é uma etapa que você pode desfazer, e o objeto mantém seu nome, seu texto alternativo, seu lugar e sua ordem em relação aos demais objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (quão grossa é a linha) e os traços (contínua, tracejada ou pontilhada). Uma imagem tem **Borda da Imagem**, que é o seu contorno.
- **Girar** gira o objeto um quarto de volta para a direita ou para a esquerda, ou o inverte horizontal ou verticalmente.
- **Cortar**, para uma imagem, recorta-a em uma forma de janela: quadrada, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9, ou 9 por 16. O corte é feito de forma uniforme a partir do centro do que aparece, e a imagem é ampliada até o tamanho do que sobrou. **Mostrar a Imagem Inteira** traz de volta o que foi cortado. O arquivo da imagem não é alterado: a parte cortada apenas fica oculta.
- **Editar Texto** serve para formas e caixas de texto. Digite as palavras e escolha como elas ficam na horizontal e na vertical, o tamanho, negrito ou normal, e se quebram conforme a forma ou ficam em uma só linha. Para Texto Artístico, a caixa pede as palavras, a fonte, o tamanho, o negrito e o itálico.
- **Painel de Seleção** abre uma lista de todos os objetos da planilha pelo nome, com o que está na frente em primeiro lugar. Clique em um nome para selecionar esse objeto, o que ajuda quando objetos se escondem uns aos outros. **Avançar** e **Recuar** movem o objeto na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou ampliadas.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo diferente de uma imagem da pasta de trabalho, como um vínculo para um arquivo, não pode ser copiada, e o JeffCrunch informa o motivo.

Copie um gráfico e cole-o na mesma pasta de trabalho, em qualquer planilha: a cópia lê as mesmas células que o original. Se colá-lo em outra pasta de trabalho, ele mantém os números que mostrava, mas não lê mais células, porque as células que leria ali não são as de onde ele foi criado.

## Selecionar vários objetos, agrupá-los e alinhá-los

Clique em um objeto e, em seguida, mantenha Shift ou Ctrl pressionada e clique em outro para selecioná-lo também. Clique novamente em um objeto selecionado com Shift ou Ctrl para desmarcá-lo. Os objetos selecionados recebem uma moldura tracejada. Você também pode escolher **Selecionar Objetos** na guia **Formato da Forma** e arrastar uma caixa sobre células vazias: todos os objetos que caibam inteiramente dentro da caixa são selecionados. Pressione Esc, ou escolha **Selecionar Objetos** novamente, para voltar à seleção de células. Pressione Delete para remover todos os objetos selecionados em uma única etapa.

Com dois ou mais objetos selecionados, a guia **Formato da Forma** tem estas opções no grupo **Organizar**:

- **Agrupar** une as formas, imagens e linhas selecionadas em um único objeto. Ele se move e se redimensiona como um todo, e o arquivo o mantém como um grupo. Um gráfico não pode ser colocado em um grupo.
- **Desagrupar** desfaz um grupo selecionado. Cada parte volta ao lugar que tinha na planilha.
- **Alinhar** alinha os objetos selecionados: **Alinhar à Esquerda**, **Centralizar**, **Alinhar à Direita**, **Alinhar em Cima**, **Alinhar no Meio** e **Alinhar Embaixo**. Eles se alinham pela borda ou pelo centro do conjunto inteiro. **Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam três ou mais objetos igualmente entre os dois das extremidades.

Agrupar, Desagrupar e cada opção de Alinhar são uma etapa que você pode desfazer.

## Dar um nome

Com um objeto selecionado, a caixa de nome mostra o nome dele. Digite um novo nome ali e pressione **Enter**. Desfazer restaura o nome antigo, e o novo nome é salvo no arquivo.

## Excluir

Pressione **Delete** com o objeto selecionado. Desfazer o traz de volta.

## O que é salvo

Tudo o que foi descrito acima é salvo no arquivo em um formato que outros programas de planilha conseguem ler, por isso um gráfico criado aqui abre da mesma forma em outro programa. Um gráfico vindo de outro programa com algo que o JeffCrunch não consegue manter ao reescrevê-lo (barras de erro, um gráfico tridimensional) ainda pode ser movido, redimensionado e excluído, mas seus elementos e dados não são alterados aqui.

## Diagramas de outros programas

Um **diagrama** é uma imagem de caixas e setas, como um processo, criado por outro programa. O JeffCrunch mostra um diagrama a partir da imagem dele que o arquivo guarda junto. Clique nele para selecioná-lo e arraste-o para movê-lo. Ao salvar o arquivo, o diagrama é gravado de volta exatamente como era, para que o programa que o criou ainda possa alterá-lo.

O JeffCrunch ainda não consegue alterar o conteúdo de um diagrama, e um diagrama não pode ser copiado nem colocado em um grupo. Se o arquivo não guardar uma imagem de um diagrama, o JeffCrunch mostra uma caixa cinza simples que informa que se trata de um diagrama.

## Gráficos em arquivos .ods

Uma pasta de trabalho salva como arquivo .ods mantém seus gráficos: colunas, barras, linhas, áreas, pizzas, roscas e dispersões, cada um com suas séries e as células que leem (nesta planilha ou em outra), categorias, título, legenda, eixos (títulos, menor e maior valor, formato numérico), rótulos de dados, cores e lugar. Um arquivo .ods criado por outro programa abre com os seus gráficos, desenhados a partir das células que eles leem. O que um gráfico .ods não tem onde ser guardado é deixado de fora e indicado ao salvar: linhas de tendência, barras de erro, eixos secundários e os efeitos extras de um único ponto. Gráficos de tipos que o JeffCrunch não desenha (de ações e de radar) são abertos como colunas com seus valores.
