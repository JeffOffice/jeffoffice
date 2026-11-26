# Gráficos, minigráficos, imagens e formas

<!-- keywords: objeto incorporado, inserir objeto, objeto a partir de um intervalo, objeto a partir de um gráfico, objeto a partir de uma página, colar como objeto, lista de objetos, editar objeto, botões de campo, botões de gráfico dinâmico, agrupar, desagrupar, alinhar, alinhar à esquerda, distribuir, selecionar objetos, várias formas, escolher várias, caixa de arrastar, filtros do gráfico, ocultar série, gráficos recomendados, sugestões de gráficos, gráfico, diagrama de barras, coluna, barra, linha, circular, área, dispersão, combinado, radar, radar preenchido, gráfico de aranha, legenda, título, linhas de grelha, rótulos de dados, dados do gráfico, tipo de gráfico, aspeto, esquemas, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, linha de tendência, equação, R-quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, mini gráfico, marcadores, nome, texto alternativo, texto alternativo, logarítmica, escala logarítmica, guardar como imagem, modelo, crtx, texto do título, chave da legenda, painel de seleção, preenchimento da forma, contorno da forma, rodar, inverter, recortar, editar texto, contorno da imagem, inserir imagem, ícones, ícone, wmf, emf, estilos de forma, estilos, inserir forma, caixa de texto, formas, seta, oval, retângulo, estrela, diagrama, diagramas, imagens online, imagens gratuitas, openverse, wikimedia commons, licença, crédito, texto artístico, wordart, deformar, comprimir imagens, tornar imagens mais pequenas, tamanho do ficheiro, ppp, eliminar áreas recortadas, código de barras, código QR, ligar uma forma a uma célula, forma mostra uma célula, caixa de texto mostra uma célula, valor de célula numa forma -->

Um gráfico é uma imagem de números que fica ligada a eles: se alterar um número, o gráfico muda com ele. As imagens e as formas assentam sobre as células da mesma forma, mas não estão ligadas a números.

## Criar um gráfico

1. Selecione os números que pretende representar. Se a primeira linha tiver nomes e a primeira coluna tiver etiquetas, selecione também essas. O JeffCrunch analisa a primeira linha e a primeira coluna e usa-as como nomes e como etiquetas quando são palavras (ou datas, no caso das etiquetas).
2. No separador **Inserir**, em **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Circular**, **Área**, **Dispersão**, **Combinado** ou **Radar**. O pequeno botão junto a um tipo tem as suas variações, como colunas empilhadas, uma linha com marcadores, um gráfico de dispersão de pontos, de pontos unidos ou de linhas suaves, ou um gráfico de radar simples, com marcadores ou preenchido. Um gráfico de radar desenha cada etiqueta como um raio a partir do centro e cada série como uma forma fechada que passa pelos seus números nos raios, pelo que mostra lado a lado várias medidas de uma mesma coisa.
3. Para ver primeiro todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Trocar Linhas e Colunas** inverte o sentido das séries.

Não sabe qual o tipo a escolher? Selecione as células e escolha **Gráficos Recomendados** no separador **Inserir**. O JeffCrunch analisa as células e apresenta alguns tipos adequados, do mais indicado para o menos. Por exemplo, datas na vertical sugerem uma linha, e uma coluna com poucos números positivos e com etiquetas sugere um gráfico circular. Cada tipo é desenhado a partir das suas células. Escolha um e selecione **OK**.

Se selecionar apenas uma célula, o JeffCrunch representa o bloco de células à volta dela que tenha conteúdo.

As séries seguem as colunas quando a seleção tem pelo menos tantas linhas como colunas, e as linhas quando tem mais colunas do que linhas. Um gráfico circular tem uma só série. Um gráfico de dispersão toma os valores horizontais da primeira coluna.

Exemplo: seleciona A1:C4, em que A2:A4 contêm T1, T2 e T3, B1 e C1 contêm Este e Oeste, e o resto são números. O gráfico tem duas séries, Este e Oeste, e três grupos de colunas, de T1 a T3.

## Ocultar uma série por agora

Escolha um gráfico. Aparecem junto a ele três pequenos botões. O que tem um funil é **Filtros do Gráfico**. Lista as séries do gráfico, cada uma com uma marca. Desmarque uma para deixar essa série fora do gráfico. Volte a marcá-la para a repor no mesmo lugar. **Anular** também funciona. Um gráfico mantém sempre pelo menos uma série. Uma série que deixou de fora continua no ficheiro quando o guarda, e fica oculta quando o abre, pelo que nada se perde. Volte a marcá-la para a repor. (Um gráfico de anel mostra todas as séries quando é aberto.)

## Mover e redimensionar

Clique num gráfico, numa imagem ou numa forma para o selecionar. Arraste-o para o mover, ou arraste um dos oito quadrados do seu contorno para o redimensionar.

- Mantenha **Alt** premido ao arrastar para fazer uma linha do contorno alinhar com a margem de uma célula.
- **Ajustar à Grelha** e **Ajustar à Forma** estão no separador **Formatar Forma** e são guardados para si, em todos os livros. Com **Ajustar à Grelha** ativado, uma margem que arrasta vai para a margem mais próxima de uma célula. Com **Ajustar à Forma** ativado, uma margem vai para a margem de outro objeto quando está a menos de 6 pixels dela. Apenas os primeiros 2000 outros objetos de uma folha contam para **Ajustar à Forma**. Com os dois ativados, prevalecem as margens das células, porque um objeto está ancorado às células, e as margens dos outros objetos não são tidas em conta. Uma alteração chega de imediato a todas as janelas abertas.
- **Alt** inverte o ajuste que está ativado para um só arrastar: com **Ajustar à Grelha** ou **Ajustar à Forma** ativados, mantendo **Alt** premido desativa o ajuste nesse arrastar (nunca muda para o outro tipo), e com ambos desativados, **Alt** faz as margens irem para as células, como acima.
- Um objeto rodado é ajustado pelo contorno que vê (a caixa mais pequena que o contém), e não pelo seu quadro antes de ser rodado, o mesmo acontecendo com os objetos a que se ajusta.
- Mantenha **Shift** premido ao arrastar um canto para manter as proporções.
- As teclas de seta deslocam-no alguns pixels de cada vez. **Ctrl** com uma seta desloca-o um pixel, **Shift** com uma seta aumenta ou diminui o seu tamanho, e **Alt** com uma seta vai até à margem seguinte de uma célula.
- **Esc** durante o arrastar devolve-o à posição inicial.

Cada movimento ou redimensionamento é um passo que pode anular. Clique com o botão direito no objeto e escolha **Propriedades** para escrever o seu **Nome** (cada objeto de uma folha precisa do seu) e o seu **Texto alternativo** (uma frase curta que diz a uma pessoa que não consegue ver o objeto o que ele mostra), para escrever o seu tamanho e para escolher o que acontece quando insere, elimina ou redimensiona células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou ficar onde está. A escolha fica guardada no ficheiro. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (continua a ser apresentado no ecrã); o ficheiro também guarda esta escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, aparecem dois botões junto a ele e surge no friso um separador **Gráfico**.

- O botão **+** (e **Partes do Gráfico** no separador) liga ou desliga o título do gráfico, os eixos, os títulos dos eixos, as linhas de grelha, a legenda e os rótulos de dados, e indica onde ficam a legenda e os rótulos.
- O botão do pincel (e **Cores** e **Aspeto** no separador) dá ao gráfico um dos conjuntos de cores do JeffCrunch e um dos seus aspetos. **Esquemas** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Pode adicionar, editar, remover e reordenar séries, alterar as células que cada uma lê, definir as células com as etiquetas, trocar linhas e colunas, e escolher como as células vazias e as linhas e colunas ocultas são representadas.
- **Botões de Campo** (no menu **+**, apenas para um gráfico dinâmico) mostra pequenos botões com os nomes dos campos da tabela dinâmica que o gráfico segue: os filtros do relatório sob o título, e as linhas, os valores e as colunas ao longo da base. Ligue ou desligue cada tipo, ou escolha **Mostrar Tudo** ou **Ocultar Tudo**. Um novo gráfico dinâmico mostra-os.
- **Barras de Erro** (no menu **+**, para um gráfico de colunas, barras, linhas, área ou dispersão) desenha uma pequena linha acima e abaixo de cada ponto para mostrar o quanto o valor real pode diferir. Escolha **Erro Padrão**, **Percentagem (5%)**, **Desvio Padrão** ou **Valor Fixo (1)**. Escolha **Personalizado** para usar valores seus: escreva as células que contêm os valores acima de cada ponto (como `=Folha1!$D$2:$D$4`) e depois as células dos valores abaixo. Deixe uma vazia para desenhar as barras só de um lado. As barras mudam quando essas células mudam. Um ficheiro que tenha os seus próprios valores para cada ponto mantém-nos.
- **Linhas** (no menu **+**, para um gráfico de linhas) tem **Linhas de Queda**, que descem do ponto mais alto de cada categoria até ao eixo, **Linhas Máx-Mín**, que unem o ponto mais alto e o mais baixo de cada categoria, e **Barras Ascendentes/Descendentes**, que são caixas entre a primeira e a última série de cada categoria. Uma barra ascendente mostra que a última série é mais alta. As Linhas Máx-Mín e as Barras Ascendentes/Descendentes precisam de duas séries ou mais. Os gráficos de área têm Linhas de Queda.
- **Tabela de Dados** (no menu **+**, para um gráfico de colunas, linhas ou área) coloca por baixo do gráfico uma grelha com os números, com uma coluna para cada categoria e uma linha para cada série. A sua lista liga e desliga a tabela, e define as linhas entre as linhas, as linhas entre as colunas, o contorno e as chaves da legenda (a cor de cada série junto ao seu nome).
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para ver para onde se encaminham. Escolha **Linear** para uma reta, **Exponencial** para números que crescem uma percentagem constante, **Logarítmica** para números que sobem depressa e depois estabilizam, **Polinomial** para uma curva com curvas, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve no gráfico a fórmula da linha, e **Mostrar R-quadrado** escreve o quão bem a linha se ajusta (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência tem entrada própria na legenda.
- Quando uma série é desenhada contra um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as entradas **Segundo Horizontal** e **Segundo Vertical** para ela. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área do gráfico em vez de ao lado.
- **Rótulos de Dados** tem também **Etiqueta contém** (o valor, o nome da categoria, o nome da série, a percentagem e a chave da legenda, que é um pequeno quadrado na cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato de Número…** (um código de formato como 0,0 para os números nos rótulos; deixe-o vazio para mostrar os números como estão). **Valor a Partir de Células…** mostra as palavras de algumas células como rótulos, uma célula para cada ponto. Escreva as células, como =Folha1!$D$2:$D$4. Os rótulos mudam quando as células mudam. Deixe-o vazio para os retirar. Desligue todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** no separador **Gráfico** abrem uma caixa para um eixo. Define os valores mais baixo e mais alto e as unidades principal e secundária (cada uma automática ou um número que escreve), se os valores aparecem por ordem inversa, as unidades de visualização (os rótulos mostram os números divididos, por exemplo por 1000 para milhares, pelo que 30 000 aparece como 30; marque **Mostrar o nome das unidades no gráfico** para imprimir o nome, como Milhares, com o título do eixo), se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que convém a números com grandes diferenças de tamanho e que têm de ser todos superiores a 0), onde o outro eixo o cruza, o formato de número dos seus rótulos, as suas marcas de graduação, onde ficam os seus rótulos e quanto são rodados. Só um eixo numérico tem limites e unidades.
- **Formatar Parte do Gráfico…** no separador **Gráfico** (e no menu do botão direito) define o preenchimento e a linha da área do gráfico, da área de traçado (a caixa onde estão as marcas) ou de uma série. Para uma linha, define também se é suave e que marca aparece em cada ponto. O menu do botão direito de um gráfico tem também **Trocar Linhas e Colunas** e os dois comandos dos eixos.
- **Seleção Atual** lista as partes do gráfico: a área do gráfico, a área de traçado e cada série. Escolha uma, e **Formatar Seleção** abre a janela de formatação nessa parte. **Repor para Corresponder ao Estilo** retira o que foi definido na parte escolhida e devolve o que o estilo do gráfico lhe atribui. Para a área do gráfico, isso é o seu preenchimento, a sua linha e a cor do texto que um aspeto definiu (o texto claro do aspeto escuro volta a ser a cor de texto do tema). Para a área de traçado, são o seu preenchimento e a sua linha. Para uma série, é a sua cor (a que a escolha de Cores ou o estilo do gráfico lhe dá), o seu contorno, as suas marcas tal como o tipo de gráfico as tem, e as cores dos seus pontos; uma linha suave mantém-se quando o tipo de gráfico é suave. Não altera títulos, legenda, rótulos, eixos, linhas de grelha nem dados, e não faz nada a uma parte que já corresponda. É um passo de Anular.
- **Mover Gráfico…** no separador **Gráfico** pergunta para onde deve ir o gráfico. Escolha **Nova folha** e escreva um nome, e o gráfico fica numa folha de gráfico própria: uma folha que é só um gráfico, sem células. Escolha **Objeto em** e selecione uma folha de cálculo, e o gráfico vai para essa folha. Um gráfico numa folha de gráfico só pode ir para uma folha de cálculo, e a folha de gráfico que deixa é eliminada. O gráfico continua a ler as células que lia. Numa folha de cálculo mantém o seu tamanho e o seu lugar na folha, mesmo onde as linhas e colunas são mais altas ou mais largas, e move-se e redimensiona-se com as células como antes. Folhas ocultas não são oferecidas. É um só **Anular**, mesmo depois de ter guardado, a menos que uma folha tenha sido eliminada por ele. Um ficheiro no formato `.ods` não pode guardar uma folha de gráfico, pelo que deve guardar uma folha de gráfico no formato `.xlsx`.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) têm também **Texto do Título…**. Escreva as palavras que quer. Ou escreva um sinal de igual e uma célula, como **=Folha1!A1**, e o título mostra o que essa célula contém. Quando a célula muda, o título também muda. O ficheiro guarda a ligação.
- **Guardar como Imagem…** no separador **Gráfico** (e no menu do botão direito) guarda o gráfico selecionado como um ficheiro de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na folha, para ficar nítido.
- **Guardar como Modelo…** e **Aplicar Modelo…** (no separador **Gráfico**, no grupo **Tipo**) guardam o aspeto de um gráfico num ficheiro **.crtx** e dão-no a outro gráfico. O aspeto é o tipo, as cores, as definições do título e da legenda, os eixos e os tipos de letra. O gráfico a que o aplica mantém os seus próprios números e as células que lê. Um **Anular** retira de novo o aspeto.
- **Tipo de Gráfico** muda o tipo de todo o gráfico, ou de cada série: colunas com uma linha por cima, ou uma série contra um segundo eixo vertical.

Uma referência a células escreve-se como **Folha1!$B$2:$B$9**, com o nome da folha entre plicas quando tem espaços.

## Minigráficos

Um minigráfico é um gráfico minúsculo que cabe dentro de uma só célula. Mostra a tendência de uma linha ou de uma coluna de números mesmo ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. No separador **Inserir**, em **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. Uma **Linha** une os números com uma linha. Uma **Coluna** desenha uma barra para cada número. Um **Vitória/Derrota** desenha uma barra para cima por cada número acima de zero e uma barra para baixo por cada número abaixo dele, todas do mesmo tamanho, o que convém a resultados que são apenas bons ou maus.
3. Na caixa **Criar Minigráficos**, confirme o **Intervalo de dados** (as células que os minigráficos leem) e escreva o **Intervalo de localização** (as células onde ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma só célula, o minigráfico lê todos os dados.

Os minigráficos criados num só passo formam um grupo, e partilham o mesmo aspeto. Clique numa célula com um minigráfico e surge o separador **Minigráfico**.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do que clicou. Também indica o que fazer às células vazias (deixar um espaço, tratá-las como zero ou unir os pontos através delas) e se deve ler células de linhas e colunas ocultas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** assinala pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, para uma linha, **Marcadores** em cada ponto.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras e a **Espessura** (o grau de grossura da linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto assinalado.
- **Eixo** mostra uma linha no zero, desenha os números da direita para a esquerda e define os valores mais baixo e mais alto do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos seus próprios números. **Igual para Todos os Minigráficos** dá a todo o grupo uma só escala, para os poder comparar. **Valor Personalizado** permite escrever um número. **Eixo de Datas…** espaça os pontos pelas datas: escreva as células que contêm a data de cada ponto, e um ponto com uma data posterior fica mais à frente. Deixe-o vazio para voltar a espaçar os pontos de forma uniforme.
- **Agrupar** põe os minigráficos selecionados num só grupo. **Desagrupar** dá a cada um o seu próprio aspeto. **Limpar** retira os minigráficos selecionados, ou os seus grupos inteiros.

Um minigráfico segue as suas células: se inserir uma linha acima dos dados, continua a ler os mesmos números. Cada alteração é um passo que pode anular. Os minigráficos são guardados no ficheiro na forma que outros programas de folha de cálculo leem.

## Inserir uma imagem, uma forma ou uma caixa de texto

No separador **Inserir**, em **Ilustrações**, **Imagem** abre um seletor de ficheiros. Escolha um ficheiro de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele fica na folha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber e mantém a sua forma. O livro guarda a própria imagem. Uma imagem de um tipo que o livro não consegue guardar tal como está (um SVG, por exemplo) é guardada como PNG, uma imagem simples feita de pontos. Um ficheiro que não é uma imagem, ou que tem mais de 16 MB, é recusado com uma mensagem. Um ficheiro WMF ou EMF (os tipos de imagem que alguns programas mais antigos criam a partir de linhas e formas) é desenhado e guardado também como PNG.

**Ícones** mostra a mesma janela de pequenas imagens de linha que o JeffPub e o JeffDeck têm, com uma caixa para as pesquisar pelo nome. Escolha um ou mais, e cada um fica na folha como uma imagem, com 48 pontos de largura e de altura. É desenhado a cinzento-escuro, num tamanho que se mantém nítido quando o aumenta. Cada um é um passo único que pode anular.

**Código de Barras** abre um pequeno menu com duas opções. **Código de Barras** abre uma janela para criar um código de barras (**Códigos de Barras a Partir de Células**, abaixo, cria vários): ISBN de livros (com ou sem preço), EAN-13, UPC-A, EAN-8, Code 128, Code 39 ou um código QR, que pode conter qualquer texto, como um endereço Web. Escolha um **Tipo**, escreva o texto ou os algarismos, e a janela mostra o código de barras à medida que escreve e indica a vermelho o que está errado se não for possível criá-lo. Clique em **Inserir**, e ele fica na folha como uma imagem, num passo que pode anular. Mova-o e redimensione-o como qualquer imagem. Para o alterar, selecione-o e clique de novo em **Código de Barras**, altere o texto e clique em **Atualizar**; o novo código de barras ocupa o lugar e a largura do anterior. Ao guardar, é guardado como uma imagem com um SVG que se mantém nítido, para que todos os programas o mostrem, e o JeffCrunch guarda as suas definições no ficheiro, pelo que continua a poder editá-lo quando abre o livro de novo no JeffCrunch.

**Código de Barras > Códigos de Barras a Partir de Células** (no mesmo menu **Código de Barras**) cria um código de barras para cada célula preenchida das células selecionadas, para uma folha de etiquetas. Selecione as células, clique nele e escolha o tipo e o aspeto na mesma janela. O que escrever ali é apenas um exemplo, porque o texto de cada célula se torna o seu próprio código de barras. Os códigos de barras ficam numa coluna logo à direita das células selecionadas, um por baixo do outro, num único passo que pode anular. Uma célula cujo texto o código não consegue conter (uma letra num EAN-13, por exemplo) é indicada numa mensagem e não recebe código de barras. No máximo 500 células são feitas de cada vez; selecione menos se a mensagem indicar que há demasiadas. Cada código de barras memoriza a célula de onde veio e acompanha-a: quando altera essa célula, o código de barras é feito de novo a partir do novo texto, no mesmo lugar e com o mesmo tamanho, como um passo próprio que pode anular. A ligação acompanha a célula quando insere ou elimina linhas ou colunas. Se eliminar a célula, o código de barras mantém o último texto que tinha. Ao guardar, a imagem do ficheiro é feita a partir do texto atual da célula, para que os outros programas também mostrem o valor atual.

**Formas** abre uma lista: retângulo, retângulo arredondado, oval, triângulo, losango, setas, uma estrela, um coração, uma nuvem, um balão de fala e uma linha. Escolha uma e ela fica na folha em azul-claro com uma margem azul mais escura. **Caixa de Texto**, no grupo **Texto**, coloca uma caixa vazia sem preenchimento e sem margem. Clique em **Desenhar Caixa de Texto** (o botão principal, ou a primeira opção do seu menu) e arraste na folha para desenhar a caixa. Um arrastar cria uma caixa do tamanho que arrasta, mas nunca com menos de 18 pontos (cerca de 0,6 cm) de lado, e um clique com menos de 4 pontos dá o tamanho habitual. Mantenha **Shift** premido para manter a caixa quadrada. O ponteiro é uma cruz até soltar; **Esc**, ou qualquer outro comando, larga a ferramenta. A caixa é um passo que pode anular.

Para rodar as palavras de uma caixa de texto, selecione-a e clique em **Direção do Texto** no separador **Formatar Forma**: **Horizontal**, **Rodar todo o texto 90°** ou **Rodar todo o texto 270°**. As palavras são escritas já rodadas, tal como são mostradas, e um livro guardado no formato .xlsx ou .ods mantém a direção.

Com uma forma selecionada, **Estilos**, no separador **Formatar Forma**, dá-lhe um aspeto pronto: um preenchimento e um contorno que combinam, como azul-claro com uma margem azul mais escura. As duas últimas opções mantêm apenas o contorno ou apenas o preenchimento. Um clique é um passo que pode anular. **Preenchimento da Forma** e **Contorno da Forma** alteram cada parte por si.

## Encontrar imagens gratuitas na Internet

**Imagens Online**, no grupo **Ilustrações** do separador **Inserir**, encontra imagens gratuitas. Vêm de duas bibliotecas abertas: o Openverse, que pesquisa muitas coleções, e o Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Clique em **Imagens Online…**. Abre-se um painel à direita.
2. Escolha uma biblioteca, escreva o que procura e clique em **Pesquisar** (ou prima Enter).
3. Clique numa imagem para ver quem a fez e que licença tem. Uma **licença** diz como pode usar uma imagem. Clique no nome da licença para a ler.
4. Clique em **Inserir** (ou faça duplo clique na imagem).

**O que o JeffCrunch envia.** As palavras da pesquisa são enviadas para a biblioteca que escolher, e só quando clica em **Pesquisar** ou prime Enter. Depois, o JeffCrunch pede à biblioteca as pequenas pré-visualizações do que encontrou e a imagem que insere. Como qualquer página Web, a biblioteca pode ver o endereço de Internet do seu computador, e é-lhe comunicado que o JeffCrunch está a fazer o pedido. Nada do seu livro é enviado.

A imagem fica na folha como uma imagem de ficheiro, e o livro guarda-a. O seu título é guardado como **texto alternativo**. A maioria das licenças pede que indique o autor. Quando **Adicionar um crédito por baixo da imagem** está marcado, o JeffCrunch acrescenta por baixo da imagem uma pequena caixa de texto com o seu título, o autor e a licença. Pode mover, alterar ou eliminar essa caixa. A imagem e o seu crédito entram num só passo, pelo que **Anular** retira ambos. Verifique se a licença se adequa ao modo como vai usar a imagem.

## Adicionar Texto Artístico

**Texto Artístico** são palavras com fantasia: letras com cor, contorno, sombra ou uma forma curva.

1. Clique em **Inserir > Texto > Texto Artístico**. Abre-se uma galeria de estilos.
2. Clique num estilo. As palavras "O Seu Texto Aqui" ficam na folha nesse estilo.
3. Para mudar as palavras, selecione o Texto Artístico e clique em **Editar Texto…** no separador **Formatar Forma**. Uma caixa pede as palavras, o tipo de letra, o tamanho, o negrito e o itálico. Um tamanho maior torna o Texto Artístico maior.
4. No separador **Formatar Forma**, **Estilos de Texto Artístico** dá às letras outro estilo, e **Deformar Texto** dobra as palavras numa forma, como uma onda ou um arco. "Simples" mantém-nas retas.

O Texto Artístico é uma forma no desenho da folha, com a sua deformação, o seu preenchimento e o seu contorno, pelo que mantém o aspeto quando guarda o livro no formato .xlsx e o volta a abrir, no JeffCrunch ou noutro programa. Cada alteração é um passo que pode anular. Um livro guardado no formato .ods mantém também as imagens, formas, caixas de texto, grupos, linhas e Texto Artístico. O Texto Artístico é guardado como a forma de texto artístico que o OpenDocument tem, com as suas palavras, tipo de letra, preenchimento, contorno e deformação, e volta a ser Texto Artístico quando abre o ficheiro .ods de novo. Algumas deformações para as quais o OpenDocument não tem forma (Onda 2, Onda 4, e as formas inferior e superior de Insuflar e Esvaziar) são guardadas como palavras retas com as mesmas letras e o mesmo estilo. Os gráficos também são guardados num ficheiro .ods: veja abaixo.

## Tornar as imagens mais pequenas

Uma imagem tirada com uma câmara pode ser muito maior do que aparece na folha, e torna o ficheiro grande. Clique numa imagem e depois em **Comprimir Imagens…** no grupo **Ajustar** do separador **Formato da Imagem**. Escolha onde o livro vai ser usado: **Impressão comercial (300 ppp)**, **Impressão em secretária (220 ppp)** ou **Web (96 ppp)**. Deixe marcada a opção **Aplicar a todas as imagens desta folha** para tornar menores todas as imagens da folha, ou desmarque-a para tornar menor apenas a imagem selecionada. **Eliminar áreas recortadas das imagens** retira também as partes escondidas que recortou, e o recorte deixa de existir. Uma imagem só é substituída quando a nova é mais pequena. O seu tamanho, lugar, nome e texto alternativo mantêm-se como estavam. Anular (Ctrl+Z) devolve as imagens, e o livro guardado contém apenas as imagens mais pequenas. As imagens de outras folhas não são alteradas: vá a cada folha e repita a operação. Uma imagem que é um desenho (SVG, EMF ou WMF), ou uma animação, fica como está.

Cada uma é um único passo que pode anular. O novo objeto fica selecionado, pelo que pode arrastá-lo, redimensioná-lo, levá-lo para a frente ou para trás, dar-lhe um nome e um texto alternativo em **Propriedades**, ou premir **Delete**.

## Mostrar o texto de uma célula numa forma ou caixa de texto

Uma forma ou caixa de texto pode mostrar o que uma célula mostra, e acompanhá-la. Selecione a forma, clique na **barra de fórmulas**, escreva um sinal de igual e uma célula, e prima **Enter**.

- `=B2` liga a forma a B2 nesta folha. `=Folha2!B3` liga-a a uma célula de outra folha. `=Total` liga-a a um nome que representa uma célula.
- A forma mostra então o texto da célula tal como a célula o mostra, com o seu formato de número. Quando altera a célula, o seu formato ou uma fórmula de que depende, a forma também muda. O texto mais longo do que uma célula pode conter (32 767 carateres) é cortado. Todas as formas ligadas de um livro mostram, em conjunto, no máximo 4 MB de texto: acima disso, uma forma mostra o seu texto cortado.
- Com a forma selecionada, a barra de fórmulas mostra a sua ligação, como `=Folha1!$B$2`. Limpe a barra de fórmulas e prima **Enter** para retirar a ligação. A forma mantém então as palavras que mostra.
- Escrever palavras na forma também retira a ligação.
- Uma ligação é a uma célula. Um intervalo como `=A1:B2`, uma lista, um cálculo, ou um nome que não é uma célula é recusado com uma mensagem, e a forma mantém a sua ligação.
- Se a linha ou a coluna da célula forem eliminadas, ou a sua folha, a forma mostra `#REF!` e mantém a ligação, como o faria uma fórmula. Anular devolve a célula.
- Cada ligação, e cada vez que a retira, é um passo que pode anular.

A ligação é guardada num ficheiro .xlsx com o texto que a célula mostrava, pelo que um programa que não segue ligações continua a mostrar as palavras, e o JeffCrunch abre o ficheiro com a ligação a funcionar. O OpenDocument não tem forma de ligar uma caixa de texto a uma célula, pelo que um livro guardado no formato .ods mantém as palavras que a forma mostra nesse momento, e não a ligação.

## Alterar o aspeto de uma imagem, forma ou caixa de texto

Selecione uma imagem e surge o separador **Formato da Imagem**. Selecione uma forma, uma linha ou uma caixa de texto e surge o separador **Formatar Forma**. Cada alteração é um passo que pode anular, e o objeto mantém o seu nome, o seu texto alternativo, o seu lugar e a sua ordem entre os outros objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (o grau de grossura da linha) e os tracejados (contínuo, tracejado ou pontilhado). Uma imagem tem **Limite da Imagem**, que é o seu contorno.
- **Rodar** roda o objeto um quarto de volta para a direita ou para a esquerda, ou inverte-o de um lado para o outro ou de cima para baixo.
- **Recortar**, numa imagem, corta-a numa forma de janela: quadrada, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9 ou 9 por 16. O corte é feito de forma uniforme a partir do centro do que se vê, e a imagem fica tão grande quanto o que sobra. **Mostrar a Imagem Completa** repõe o que foi cortado. **Preencher** mantém o quadro e corta a imagem de forma que a cubra por completo. **Ajustar** mostra a imagem inteira, e o quadro toma a forma da imagem dentro do antigo. **Recortar para Forma** (a galeria ao lado de **Recortar**) corta a imagem numa forma como uma oval, uma estrela ou um retângulo arredondado; a forma é guardada com a imagem. O ficheiro da imagem não é alterado: a parte cortada fica apenas oculta.
- **Editar Texto** serve para formas e caixas de texto. Escreva as palavras e escolha como se dispõem na horizontal e na vertical, o tamanho, o negrito ou o normal, e se se ajustam à forma ou ficam numa só linha. No Texto Artístico, pede as palavras, o tipo de letra, o tamanho, o negrito e o itálico.
- **Painel de Seleção** abre uma lista de todos os objetos da folha pelo nome, o primeiro à frente. Clique num nome para selecionar esse objeto, o que ajuda quando os objetos se escondem uns aos outros. **Trazer para a Frente** e **Enviar para Trás** movem-no na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou aumentadas.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo que não é uma imagem do livro, como uma ligação a um ficheiro, não pode ser copiada, e o JeffCrunch indica o motivo.

Copie um gráfico e cole-o no mesmo livro, em qualquer folha: a cópia lê as mesmas células que o primeiro. Cole-o noutro livro e mantém os números que mostrava, mas deixa de ler células, porque as células que lá leria não são as que lhe deram origem.

## Selecionar vários objetos, agrupá-los e alinhá-los

Clique num objeto e depois mantenha Shift ou Ctrl premido e clique noutro para o selecionar também. Clique de novo num objeto selecionado com Shift ou Ctrl para o largar. Os objetos selecionados têm cada um um quadro tracejado. Também pode escolher **Selecionar Objetos** no separador **Formatar Forma** e arrastar uma caixa sobre células vazias: todos os objetos que caibam por completo dentro da caixa ficam selecionados. Prima Esc, ou escolha **Selecionar Objetos** de novo, para voltar a selecionar células. Prima Delete para remover todos os objetos selecionados de uma só vez.

Com dois ou mais selecionados, o separador **Formatar Forma** tem estes comandos no grupo **Organizar**:

- **Agrupar** junta as formas, imagens e linhas selecionadas num só objeto. Move-se e redimensiona-se como um todo, e o ficheiro mantém-no como grupo. Um gráfico não pode ser colocado num grupo.
- **Desagrupar** separa um grupo selecionado. Cada parte volta ao lugar que tinha na folha.
- **Alinhar** alinha os objetos selecionados: **Alinhar à Esquerda**, **Alinhar ao Centro**, **Alinhar à Direita**, **Alinhar em Cima**, **Alinhar ao Meio** e **Alinhar em Baixo**. Alinham-se pela margem ou pelo centro do conjunto todo. **Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam igualmente três ou mais objetos entre os dois das extremidades.

Agrupar, Desagrupar e cada opção de alinhamento são um passo que pode anular.

## Dar-lhe um nome

Com um objeto selecionado, a caixa de nomes mostra o seu nome. Escreva lá um novo nome e prima **Enter**. Anular repõe o nome antigo, e o novo nome fica guardado no ficheiro.

## Eliminá-lo

Prima **Delete** com o objeto selecionado. Anular devolve-o.

## O que fica guardado

Tudo o que está acima é guardado no ficheiro na forma que outros programas de folha de cálculo leem, pelo que um gráfico que cria aqui abre-se ali da mesma maneira. Um gráfico vindo de outro programa com algo que o JeffCrunch não consegue manter quando reescreve um gráfico (barras de erro, um gráfico tridimensional) pode ainda ser movido, redimensionado e eliminado, mas os seus elementos e dados não são alterados aqui.

## Inserir um objeto

Um **objeto** guarda o seu próprio ficheiro dentro do livro, para que a aplicação que o criou o possa abrir de novo. Mostra uma imagem de si próprio, mas não é uma imagem: continua a ser um objeto quando o move, redimensiona, copia ou guarda o livro.

1. Clique em **Inserir** e depois em **Objeto**, e escolha **A Partir do Intervalo do JeffCrunch**, **A Partir do Gráfico do JeffCrunch** ou **A Partir da Página do JeffWrite**.
2. Escolha o ficheiro, e o intervalo ou o número da página se a caixa o pedir. Se copiou um na outra aplicação, a caixa oferece-se para o usar.
3. O objeto entra na célula selecionada. Arraste-o para o mover, e arraste uma alça para o redimensionar. Um intervalo, gráfico ou página que redimensione é desenhado de novo no novo tamanho, pelo que se mantém nítido. Prima **Delete** para o retirar. Cada um é um único passo de Anular.

Para inserir algo que copiou noutra aplicação, clique na seta sob **Colar** e escolha **Como Objeto**.

**Faça duplo clique** num objeto para o abrir na aplicação que o criou. Guarde-o aí, e o objeto aqui mostra a alteração como um passo de Anular. Uma célula em que estava a escrever mantém-se como está. Uma cópia que cole é um objeto próprio: alterar uma não altera a outra. O mesmo acontece com uma folha que copie: os seus objetos são cópias, e a cópia da folha mantém-nos. **Ficheiro**, depois **Informações**, depois **Objetos** lista todos os objetos do livro. Selecione um para o abrir, o transformar numa imagem simples ou o eliminar.

Um objeto criado noutro programa aparece como a imagem que o seu ficheiro guarda, seja qual for o tipo dessa imagem. Pode selecioná-lo, movê-lo, redimensioná-lo, alinhá-lo com outros objetos e eliminá-lo, e move-se com as células como os seus próprios objetos. Não o pode editar aqui, colocá-lo num grupo nem alterar o seu lugar na ordem dos objetos. É guardado tal como o ficheiro o tinha, com apenas o seu lugar alterado se o tiver movido. Um objeto que um ficheiro .ods continha funciona da mesma forma. Guardados como ficheiro .xlsx ou .ods, os seus objetos ficam no formato padrão que outros programas leem. Quando é escrito um ficheiro .ods para um objeto cuja imagem não pode ser desenhada, o objeto é mantido sem imagem.

## Diagramas de outros programas

Um **diagrama** é uma imagem de caixas e setas, como um processo, criada por outro programa. O JeffCrunch mostra um diagrama a partir da imagem que o ficheiro guarda ao seu lado. Clique nele para o selecionar, e arraste-o para o mover. Quando guarda o ficheiro, o diagrama é escrito de volta exatamente como era, para que o programa que o criou o possa continuar a alterar.

O JeffCrunch ainda não consegue alterar o conteúdo de um diagrama, e um diagrama não pode ser copiado nem colocado num grupo. Se o ficheiro não guardar uma imagem de um diagrama, o JeffCrunch mostra uma caixa cinzenta simples que indica que é um diagrama.

## Gráficos em ficheiros .ods

Um livro guardado no formato .ods mantém os seus gráficos: de colunas, barras, linhas, áreas, circulares, de anel, de dispersão e de radar, cada um com as suas séries e as células que lê (nesta folha ou noutra), categorias, título, legenda, eixos (títulos, valores mais baixo e mais alto, formato de número), rótulos de dados, cores e lugar. Um ficheiro .ods criado por outro programa abre-se com os seus gráficos, desenhados a partir das células que leem. O que um gráfico .ods não tem onde guardar é deixado de fora e indicado ao guardar: linhas de tendência, barras de erro, segundos eixos e os aspetos extra de um só ponto. Os gráficos de tipos que o JeffCrunch não desenha (de cotações e de radar) abrem como colunas com os seus valores.
