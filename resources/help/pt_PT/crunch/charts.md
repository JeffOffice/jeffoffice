# Gráficos, minigráficos, imagens e formas

<!-- keywords: botões de campo, botões de campo do gráfico dinâmico, agrupar, desagrupar, alinhar, alinhar à esquerda, distribuir, selecionar objetos, várias formas, selecionar várias, caixa de arrastar, filtros do gráfico, ocultar série, gráficos recomendados, sugestões de gráfico, gráfico, diagrama de barras, coluna, barra, linha, circular, área, dispersão, combinado, legenda, título, linhas de grelha, rótulos de dados, dados do gráfico, tipo de gráfico, aspeto, esquemas, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R ao quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, gráfico pequeno, marcadores, nome, texto alternativo, logarítmica, escala logarítmica, guardar como imagem, modelo, crtx, texto do título, chave da legenda, painel de seleção, preenchimento da forma, contorno da forma, rodar, inverter, recortar, editar texto, limite da imagem, inserir imagem, ícones, ícone, wmf, emf, estilos de forma, estilos, inserir forma, caixa de texto, formas, seta, elipse, retângulo, estrela, diagrama, diagramas, imagens online, imagens gratuitas, openverse, wikimedia commons, licença, crédito, texto artístico, wordart, deformar, comprimir imagens, reduzir imagens, tamanho do ficheiro, ppp, eliminar áreas recortadas, código de barras, código QR -->

Um gráfico é uma imagem de números que permanece ligada a eles: se alterar um número, o gráfico muda com ele. As imagens e as formas são colocadas sobre as células da mesma forma, mas não estão ligadas a nenhum número.

## Criar um gráfico

1. Selecione os números que pretende representar. Se a primeira linha tiver nomes e a primeira coluna tiver etiquetas, selecione-as também. O JeffCrunch analisa a primeira linha e a primeira coluna e usa-as como nomes e etiquetas quando são palavras (ou datas, no caso das etiquetas).
2. No separador **Inserir**, em **Gráficos**, escolha um tipo: **Coluna**, **Barra**, **Linha**, **Circular**, **Área**, **Dispersão** ou **Combinado**. A pequena seta ao lado de um tipo mostra as suas variantes, como colunas empilhadas, uma linha com marcadores ou um gráfico de dispersão de pontos, de pontos unidos ou de linhas suaves.
3. Para ver primeiro todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado a partir das suas células. **Trocar linhas e colunas** inverte o sentido das séries.

Não sabe qual o tipo a escolher? Selecione as células e escolha **Gráficos Recomendados** no separador **Inserir**. O JeffCrunch analisa as células e apresenta alguns tipos adequados, por ordem de preferência. Por exemplo, datas na vertical sugerem uma linha, e uma coluna com alguns números positivos e etiquetas sugere um gráfico circular. Cada tipo é desenhado a partir das suas células. Escolha um e clique em **OK**.

Se selecionar apenas uma célula, o JeffCrunch representa o bloco de células à volta dela que tenha algum conteúdo.

As séries seguem as colunas quando a seleção tem pelo menos tantas linhas como colunas, e seguem as linhas quando tem mais colunas do que linhas. Um gráfico circular tem uma só série. Um gráfico de dispersão toma os valores horizontais da primeira coluna.

Exemplo: seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Este e Oeste, e o resto são números. O gráfico tem duas séries, Este e Oeste, e três grupos de colunas, de Q1 a Q3.

## Ocultar uma série por agora

Escolha um gráfico. Ao lado dele aparecem três pequenos botões. O que tem um funil é **Filtros do Gráfico**. Lista as séries do gráfico, cada uma com uma marca de verificação. Desmarque uma série para a retirar do gráfico. Volte a marcá-la para a repor no mesmo lugar. **Anular** também funciona. Um gráfico tem sempre pelo menos uma série. Uma série que retirou continua no ficheiro quando o guarda, e permanece oculta quando abre o ficheiro, pelo que nada se perde. Marque-a de novo para a repor. (Um gráfico de anel mostra todas as séries quando é aberto.)

## Mover e redimensionar

Clique num gráfico, imagem ou forma para o selecionar. Arraste-o para o mover, ou arraste um dos oito quadrados da respetiva margem para o redimensionar.

- Mantenha **Alt** premida ao arrastar para alinhar uma margem com a margem de uma célula.
- Mantenha **Shift** premida ao arrastar um canto para manter as proporções.
- As teclas de seta deslocam o objeto alguns pixels de cada vez. **Ctrl** com uma seta move-o um pixel, **Shift** com uma seta aumenta ou diminui o tamanho, e **Alt** com uma seta leva-o à margem seguinte de uma célula.
- **Esc** enquanto arrasta repõe o objeto na posição original.

Cada movimento ou redimensionamento é um passo que pode anular. Clique com o botão direito no objeto e escolha **Propriedades** para escrever o **Nome** (cada objeto numa folha precisa de um nome próprio) e o **Texto alternativo** (uma frase curta que explica a quem não pode ver o objeto o que ele mostra), para indicar o tamanho e para escolher o que acontece quando insere, elimina ou redimensiona células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou ficar onde está. A escolha fica guardada no ficheiro. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (continua visível no ecrã); o ficheiro também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, aparecem dois botões ao lado dele e surge no friso um separador **Gráfico**.

- O botão **+** (e **Elementos do Gráfico** no separador) ativa ou desativa o título do gráfico, os eixos, os títulos dos eixos, as linhas de grelha, a legenda e os rótulos de dados, e indica onde ficam a legenda e os rótulos.
- O botão de pincel (e **Cores** e **Estilos** no separador) dá ao gráfico um dos conjuntos de cores do JeffCrunch e um dos seus aspetos. **Esquemas** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Pode adicioná-las, editá-las, removê-las e reordená-las, alterar as células que cada uma lê, definir as células com as etiquetas, trocar as linhas e as colunas, e escolher como são representadas as células vazias e as linhas e colunas ocultas.
- **Botões de Campo** (no menu **+**, apenas para um gráfico dinâmico) mostra pequenos botões com os nomes dos campos da tabela dinâmica que o gráfico segue: os filtros do relatório sob o título e as linhas, os valores e as colunas ao longo da base. Ative ou desative cada tipo, ou escolha **Mostrar Tudo** ou **Ocultar Tudo**. Um novo gráfico dinâmico mostra-os.
- **Barras de Erro** (no menu **+**, para um gráfico de colunas, barras, linhas, áreas ou dispersão) desenha uma pequena linha acima e abaixo de cada ponto para mostrar o quanto o valor real pode diferir. Escolha **Erro Padrão**, **Percentagem (5%)**, **Desvio Padrão** ou **Valor Fixo (1)**. Escolha **Personalizado** para usar valores próprios: escreva as células que contêm os valores acima de cada ponto (como `=Folha1!$D$2:$D$4`) e depois as células dos valores abaixo. Deixe uma vazia para desenhar as barras de um só lado. As barras mudam quando essas células mudam. Um ficheiro que tenha valores próprios para cada ponto mantém-nos.
- **Linhas** (no menu **+**, para um gráfico de linhas) tem **Linhas de Queda**, que descem do ponto mais alto de cada categoria até ao eixo, **Linhas Máx.-Mín.**, que unem o ponto mais alto e o mais baixo de cada categoria, e **Barras Superior/Inferior**, que são caixas entre a primeira e a última série de cada categoria. Uma barra superior indica que a última série está mais alta. As linhas máx.-mín. e as barras superior/inferior precisam de duas ou mais séries. Os gráficos de área têm linhas de queda.
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para ver para onde se dirigem. Escolha **Linear** para uma reta, **Exponencial** para números que crescem a uma percentagem constante, **Logarítmica** para números que sobem depressa e depois estabilizam, **Polinomial** para uma curva com inflexões, **Potência**, ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R ao Quadrado** indica o grau de ajuste da linha (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência tem uma entrada própria na legenda.
- Quando uma série é desenhada contra um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as entradas **Segundo Horizontal** e **Segundo Vertical** para ela. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área do gráfico em vez de ao lado.
- **Rótulos de Dados** tem também **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a percentagem e a chave da legenda, que é um pequeno quadrado com a cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato de Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe-o vazio para mostrar os números tal como são). **Valor a Partir das Células…** mostra o texto de algumas células como rótulos, uma célula para cada ponto. Escreva as células, como =Folha1!$D$2:$D$4. Os rótulos mudam quando as células mudam. Deixe o campo vazio para os retirar. Desative todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** no separador **Gráfico** abrem uma caixa para um eixo. Define os valores mínimo e máximo, as unidades principal e secundária (cada uma automática ou um número que escreva), se os valores aparecem por ordem inversa, as unidades de visualização (os rótulos mostram os números divididos, por exemplo por 1000 para milhares, pelo que 30 000 aparece como 30; marque **Mostrar o nome das unidades no gráfico** para imprimir o nome, como Milhares, junto ao título do eixo), se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que convém a números com grandes diferenças de tamanho e que têm de ser superiores a 0), onde o outro eixo o cruza, o formato numérico dos seus rótulos, as suas marcas de graduação, onde ficam os rótulos e quantos graus são rodadas. Só um eixo numérico tem limites e unidades.
- **Mover Gráfico…** no separador **Gráfico** pergunta para onde deve ir o gráfico. Escolha **Nova folha** e escreva um nome, e o gráfico fica numa folha de gráfico própria: uma folha que é só um gráfico, sem células. Escolha **Objeto em** e selecione uma folha de cálculo, e o gráfico passa para essa folha. Um gráfico numa folha de gráfico só pode passar para uma folha de cálculo, e a folha de gráfico de onde saiu é eliminada. O gráfico continua a ler as células que lia. Numa folha de cálculo mantém o tamanho e o lugar na folha, mesmo onde as linhas e as colunas são mais altas ou mais largas, e move-se e redimensiona-se com as células como antes. As folhas ocultas não são oferecidas. É um só **Anular**, mesmo depois de ter guardado, a menos que uma folha tenha sido eliminada por esta ação. Um ficheiro no formato `.ods` não pode conter uma folha de gráfico, por isso guarde uma folha de gráfico no formato `.xlsx`.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) têm também **Texto do Título…**. Escreva as palavras que pretende. Ou escreva um sinal de igual e uma célula, como **=Folha1!A1**, e o título mostra o que essa célula contém. Quando a célula muda, o título também muda. O ficheiro guarda a ligação.
- **Guardar como Imagem…** no separador **Gráfico** (e no menu do botão direito) guarda o gráfico selecionado como um ficheiro de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na folha, para se manter nítido.
- **Guardar como Modelo…** e **Aplicar Modelo…** (no separador **Gráfico**, no grupo **Tipo**) guardam o aspeto de um gráfico num ficheiro **.crtx** e aplicam-no a outro gráfico. O aspeto inclui o tipo, as cores, as definições do título e da legenda, os eixos e os tipos de letra. O gráfico ao qual o aplica mantém os seus próprios números e as células que lê. Um **Anular** retira de novo o aspeto.
- **Tipo de Gráfico** altera o tipo de todo o gráfico, ou de cada série: colunas com uma linha por cima, ou uma série contra um segundo eixo vertical.

Uma referência a células escreve-se como **Folha1!$B$2:$B$9**, com o nome da folha entre plicas quando tem espaços.

## Minigráficos

Um minigráfico é um pequeno gráfico que cabe dentro de uma célula. Mostra a tendência de uma linha ou de uma coluna de números, mesmo ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. No separador **Inserir**, em **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. Uma **Linha** une os números com uma linha. Uma **Coluna** desenha uma barra para cada número. Um **Vitória/Derrota** desenha uma barra para cima por cada número acima de zero e uma barra para baixo por cada número abaixo dele, todas do mesmo tamanho, o que convém a resultados que são apenas bons ou maus.
3. Na caixa **Criar Minigráficos**, confirme o **Intervalo de dados** (as células que os minigráficos leem) e escreva o **Intervalo de localização** (as células onde ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma só célula, o minigráfico lê todos os dados.

Os minigráficos criados num único passo formam um grupo e partilham o mesmo aspeto. Clique numa célula com um minigráfico e surge o separador **Minigráfico**.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou os dados do que selecionou. Também indica o que fazer com as células vazias (deixar um intervalo, tratá-las como zero ou unir os pontos através delas) e se devem ser lidas as células de linhas e colunas ocultas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** assinala pontos: o **Ponto Mais Alto**, o **Ponto Mais Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto** e, numa linha, **Marcadores** em cada ponto.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras, e a **Espessura** (a espessura da linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto assinalado.
- **Eixo** mostra uma linha no zero, desenha os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos seus próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma só escala, para os poder comparar. **Valor Personalizado** permite escrever um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: escreva as células que contêm a data de cada ponto, e um ponto com data posterior fica mais à frente. Deixe o campo vazio para voltar a espaçar os pontos de forma uniforme.
- **Agrupar** coloca os minigráficos selecionados num só grupo. **Desagrupar** dá a cada um o seu próprio aspeto. **Limpar** retira os minigráficos selecionados, ou os grupos inteiros a que pertencem.

Um minigráfico segue as suas células: se inserir uma linha acima dos dados, continua a ler os mesmos números. Cada alteração é um passo que pode anular. Os minigráficos são guardados no ficheiro num formato que outros programas de folhas de cálculo leem.

## Inserir uma imagem, uma forma ou uma caixa de texto

No separador **Inserir**, em **Ilustrações**, **Imagem** abre um seletor de ficheiros. Escolha um ficheiro de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele fica na folha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber, e mantém a sua forma. A folha de cálculo guarda a própria imagem. Uma imagem de um tipo que a folha não consegue guardar tal como está (um SVG, por exemplo) é guardada como PNG, uma imagem simples feita de pontos. Um ficheiro que não é uma imagem, ou que tem mais de 16 MB, é recusado com uma mensagem. Um ficheiro WMF ou EMF (os tipos de imagem que alguns programas mais antigos criam a partir de linhas e formas) é desenhado e guardado também como PNG.

**Ícones** mostra a mesma janela de pequenas imagens de linhas que o JeffPub e o JeffDeck têm, com uma caixa para as pesquisar pelo nome. Escolha um ou mais, e cada um fica na folha como imagem, com 48 pontos de largura e de altura. É desenhado a cinzento escuro, num tamanho que se mantém nítido quando o aumenta. Cada um é um passo único que pode anular.

**Código de Barras** abre um pequeno menu com duas opções. **Código de Barras** abre uma janela para criar um código de barras (**Códigos de Barras a Partir de Células**, abaixo, cria vários): ISBN de livros (com ou sem preço), EAN-13, UPC-A, EAN-8, Code 128, Code 39, ou um código QR, que pode conter qualquer texto, como um endereço Web. Escolha um **Tipo**, escreva o texto ou os algarismos, e a janela mostra o código de barras à medida que escreve. Se não for possível criá-lo, indica a vermelho o que está errado. Clique em **Inserir**, e o código fica na folha como imagem, num passo que pode anular. Mova-o e redimensione-o como qualquer imagem. Para o alterar, selecione-o e clique de novo em **Código de Barras**, altere o texto e clique em **Atualizar**; o novo código de barras ocupa o lugar e a largura do anterior. Ao guardar, fica guardado como imagem com um SVG que se mantém nítido, para que qualquer programa o mostre, e o JeffCrunch guarda as suas definições no ficheiro, pelo que continua a ser um código de barras que pode editar quando voltar a abrir a folha no JeffCrunch.

**Código de Barras > Códigos de Barras a Partir de Células** (no mesmo menu **Código de Barras**) cria um código de barras para cada célula preenchida das células selecionadas, para uma folha de etiquetas. Selecione as células, clique nessa opção e escolha o tipo e o aspeto na mesma janela. O que escrever ali é apenas um exemplo, porque o texto de cada célula se torna o seu próprio código de barras. Os códigos de barras ficam numa coluna logo à direita das células selecionadas, um por baixo do outro, num único passo que pode anular. Uma célula cujo texto o código não consegue conter (uma letra num EAN-13, por exemplo) é indicada numa mensagem e não recebe código de barras. No máximo, 500 células são processadas de cada vez; selecione menos se a mensagem indicar que há demasiadas. Cada código de barras memoriza a célula de origem e segue-a: quando altera essa célula, o código é gerado novamente a partir do novo texto, no mesmo lugar e com o mesmo tamanho, como um passo próprio que pode anular. A ligação acompanha a célula quando insere ou elimina linhas ou colunas. Se eliminar a célula, o código de barras mantém o último texto que tinha. Ao guardar, a imagem do ficheiro é criada a partir do texto atual da célula, para que outros programas mostrem também o valor atual.

**Formas** abre uma lista: retângulo, retângulo arredondado, elipse, triângulo, losango, setas, uma estrela, um coração, uma nuvem, um balão de fala e uma linha. Escolha uma e ela fica na folha, a azul-claro com uma margem azul mais escura. **Caixa de Texto**, no grupo **Texto**, coloca uma caixa vazia, sem preenchimento e sem margem.

Com uma forma selecionada, **Estilos** no separador **Formato da Forma** dá-lhe um aspeto pronto: um preenchimento e um contorno que combinam, como azul-claro com uma margem azul mais escura. As duas últimas opções mantêm apenas o contorno ou apenas o preenchimento. Um clique é um passo que pode anular. **Preenchimento da Forma** e **Contorno da Forma** alteram cada parte separadamente.

## Encontrar imagens gratuitas na Internet

**Imagens Online**, no grupo **Ilustrações** do separador **Inserir**, encontra imagens gratuitas. Vêm de duas bibliotecas abertas: a Openverse, que pesquisa em muitas coleções, e a Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Clique em **Imagens Online…**. Abre-se um painel à direita.
2. Escolha uma biblioteca, escreva o que procura e clique em **Pesquisar** (ou prima Enter).
3. Clique numa imagem para ver quem a criou e que licença tem. Uma **licença** indica como pode utilizar uma imagem. Clique no nome da licença para a ler.
4. Clique em **Inserir** (ou faça duplo clique na imagem).

**O que o JeffCrunch envia.** As palavras da pesquisa são enviadas para a biblioteca que escolher, e só quando clica em **Pesquisar** ou prime Enter. Depois, o JeffCrunch pede à biblioteca as pequenas pré-visualizações do que encontrou e a imagem que insere. Como qualquer página Web, a biblioteca pode ver o endereço de Internet do seu computador, e é-lhe indicado que o JeffCrunch está a fazer o pedido. Nada da sua folha de cálculo é enviado.

A imagem fica na folha como uma imagem de um ficheiro, e a folha de cálculo guarda-a. O seu título é guardado como **texto alternativo**. A maioria das licenças pede que indique o autor. Quando **Adicionar um crédito por baixo da imagem** está marcado, o JeffCrunch acrescenta por baixo da imagem uma pequena caixa de texto com o título, o autor e a licença. Pode mover, alterar ou eliminar essa caixa. A imagem e o crédito são inseridos num só passo, pelo que **Anular** os retira ambos. Verifique se a licença se adequa à forma como vai utilizar a imagem.

## Adicionar Texto Artístico

**Texto Artístico** são palavras com efeitos: letras com cor, contorno, sombra ou uma forma curva.

1. Clique em **Inserir > Texto > Texto Artístico**. Abre-se uma galeria de estilos.
2. Clique num estilo. As palavras "O Seu Texto Aqui" ficam na folha nesse estilo.
3. Para alterar as palavras, selecione o Texto Artístico e clique em **Editar Texto…** no separador **Formato da Forma**. Uma caixa pede as palavras, o tipo de letra, o tamanho, o negrito e o itálico. Um tamanho maior torna o Texto Artístico maior.
4. No separador **Formato da Forma**, **Estilos de Texto Artístico** dá às letras outro estilo, e **Deformar Texto** dobra as palavras numa forma, como uma onda ou um arco. "Simples" mantém-nas retas.

O Texto Artístico é uma forma do desenho da folha, com a sua deformação, o seu preenchimento e o seu contorno, pelo que mantém o aspeto quando guarda a folha de cálculo como ficheiro .xlsx e a volta a abrir, no JeffCrunch ou noutro programa. Cada alteração é um passo que pode anular. Uma folha de cálculo guardada como ficheiro .ods mantém também as suas imagens, formas, caixas de texto, grupos, linhas e Texto Artístico. O Texto Artístico é guardado como a forma de texto deformado que o OpenDocument tem, com as suas palavras, tipo de letra, preenchimento, contorno e deformação, e volta a surgir como Texto Artístico quando abre o ficheiro .ods de novo. Algumas deformações para as quais o OpenDocument não tem forma (Onda 2, Onda 4, e as formas inferior e superior de Insuflar e Esvaziar) são guardadas como palavras retas, com as mesmas letras e estilo. Os gráficos também são guardados num ficheiro .ods: veja abaixo.

## Reduzir imagens

Uma imagem tirada com uma câmara pode ser muito maior do que aparece na folha, e isso torna o ficheiro grande. Clique numa imagem e, depois, clique em **Comprimir Imagens…** no grupo **Ajustar** do separador **Formato da Imagem**. Escolha onde a folha de cálculo será utilizada: **Impressão comercial (300 ppp)**, **Impressão em secretária (220 ppp)** ou **Web (96 ppp)**. Deixe marcada a opção **Aplicar a todas as imagens desta folha** para reduzir todas as imagens da folha, ou desmarque-a para reduzir apenas a imagem selecionada. **Eliminar áreas recortadas das imagens** retira também as partes ocultas que recortou, e o recorte deixa de existir. Uma imagem só é substituída quando a nova é mais pequena. O seu tamanho, lugar, nome e texto alternativo mantêm-se. **Anular** (Ctrl+Z) devolve as imagens originais, e a folha guardada contém apenas as imagens mais pequenas. As imagens de outras folhas não são alteradas: vá a cada folha e repita a operação. Uma imagem que é um desenho (SVG, EMF ou WMF), ou uma animação, fica como está.

Cada uma destas ações é um passo que pode anular. O novo objeto fica selecionado, pelo que pode arrastá-lo, redimensioná-lo, enviá-lo para a frente ou para trás, dar-lhe um nome e um texto alternativo em **Propriedades**, ou premir **Delete**.

## Alterar o aspeto de uma imagem, forma ou caixa de texto

Ao selecionar uma imagem, surge o separador **Formato da Imagem**. Ao selecionar uma forma, uma linha ou uma caixa de texto, surge o separador **Formato da Forma**. Cada alteração é um passo que pode anular, e o objeto mantém o nome, o texto alternativo, o lugar e a ordem entre os outros objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (a espessura da linha) e os traços (contínuo, tracejado ou pontilhado). Uma imagem tem **Limite da Imagem**, que é o seu contorno.
- **Rodar** roda o objeto um quarto de volta para a direita ou para a esquerda, ou inverte-o de um lado para o outro ou de cima para baixo.
- **Recortar**, numa imagem, corta-a numa janela de forma: quadrada, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9 ou 9 por 16. O corte é feito de forma uniforme a partir do meio do que se vê, e a imagem é ampliada até ao tamanho do que resta. **Mostrar a Imagem Completa** repõe o que foi cortado. O ficheiro da imagem não é alterado: a parte cortada fica apenas oculta.
- **Editar Texto** é para formas e caixas de texto. Escreva as palavras e escolha a sua disposição na horizontal e na vertical, o tamanho, o negrito ou normal, e se se ajustam à forma ou ficam numa só linha. No Texto Artístico, pede as palavras, o tipo de letra, o tamanho, o negrito e o itálico.
- **Painel de Seleção** abre uma lista de todos os objetos da folha pelo nome, com o que está à frente em primeiro lugar. Clique num nome para selecionar esse objeto, o que ajuda quando os objetos se escondem uns aos outros. **Trazer para a Frente** e **Enviar para Trás** movem-no na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou aumentadas.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada da mesma forma que um gráfico. Uma forma que usa algo que não é uma imagem da folha de cálculo, como uma ligação a um ficheiro, não pode ser copiada, e o JeffCrunch indica o motivo.

Copie um gráfico e cole-o na mesma folha de cálculo, em qualquer folha: a cópia lê as mesmas células que o original. Se o colar noutra folha de cálculo, mantém os números que mostrava, mas deixa de ler células, porque as células que leria ali não são as de onde foi criado.

## Selecionar vários objetos, agrupá-los e alinhá-los

Clique num objeto e, depois, mantenha Shift ou Ctrl premida e clique noutro para o selecionar também. Clique de novo num objeto selecionado com Shift ou Ctrl para o largar. Os objetos selecionados têm cada um uma moldura tracejada. Também pode escolher **Selecionar Objetos** no separador **Formato da Forma** e arrastar uma caixa sobre células vazias: todos os objetos que caibam inteiramente dentro da caixa ficam selecionados. Prima Esc, ou escolha de novo **Selecionar Objetos**, para voltar a selecionar células. Prima Delete para remover todos os objetos selecionados de uma só vez.

Com dois ou mais objetos selecionados, o separador **Formato da Forma** tem estas opções no grupo **Organizar**:

- **Agrupar** junta as formas, imagens e linhas selecionadas num só objeto. Move-se e redimensiona-se como um todo, e o ficheiro guarda-o como grupo. Um gráfico não pode ser colocado num grupo.
- **Desagrupar** desfaz um grupo selecionado. Cada parte volta ao sítio onde estava na folha.
- **Alinhar** alinha os objetos selecionados: **Alinhar à Esquerda**, **Alinhar ao Centro**, **Alinhar à Direita**, **Alinhar em Cima**, **Alinhar ao Meio** e **Alinhar em Baixo**. Alinham-se pela margem ou pelo centro do conjunto inteiro. **Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam três ou mais objetos de forma uniforme entre os dois das extremidades.

Agrupar, Desagrupar e cada opção de alinhamento são um passo que pode anular.

## Dar-lhe um nome

Com um objeto selecionado, a caixa de nome mostra o seu nome. Escreva um novo nome ali e prima **Enter**. Anular repõe o nome antigo, e o novo nome fica guardado no ficheiro.

## Eliminá-lo

Prima **Delete** com o objeto selecionado. Anular devolve-o.

## O que é guardado

Tudo o que foi descrito acima é guardado no ficheiro num formato que outros programas de folhas de cálculo leem, pelo que um gráfico criado aqui abre-se igualmente nesses programas. Um gráfico vindo de outro programa com algo que o JeffCrunch não consegue manter quando reescreve um gráfico (barras de erro, um gráfico tridimensional) ainda pode ser movido, redimensionado e eliminado, mas os seus elementos e dados não são alterados aqui.

## Diagramas de outros programas

Um **diagrama** é uma imagem de caixas e setas, como um processo, criada por outro programa. O JeffCrunch mostra um diagrama a partir da imagem que o ficheiro guarda ao lado dele. Clique nele para o selecionar, e arraste-o para o mover. Ao guardar o ficheiro, o diagrama é escrito de novo exatamente como estava, para que o programa que o criou o possa continuar a alterar.

O JeffCrunch ainda não consegue alterar o que está dentro de um diagrama, e um diagrama não pode ser copiado nem colocado num grupo. Se o ficheiro não guardar uma imagem de um diagrama, o JeffCrunch mostra uma caixa cinzenta simples que indica que se trata de um diagrama.

## Gráficos em ficheiros .ods

Uma folha de cálculo guardada como ficheiro .ods mantém os seus gráficos: colunas, barras, linhas, áreas, circulares, anéis e dispersão, cada um com as suas séries e as células que lê (nesta folha ou noutra), categorias, título, legenda, eixos (títulos, valor mínimo e máximo, formato numérico), rótulos de dados, cores e lugar. Um ficheiro .ods criado por outro programa abre com os seus gráficos, desenhados a partir das células que leem. O que um gráfico .ods não tem onde guardar fica de fora e é indicado ao guardar: linhas de tendência, barras de erro, segundos eixos e os aspetos extra de um único ponto. Os gráficos de tipos que o JeffCrunch não desenha (gráficos de cotações e radar) abrem como colunas com os seus valores.
