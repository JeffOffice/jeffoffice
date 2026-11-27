# Gráficos, minigráficos, imagens e formas

<!-- keywords: objeto incorporado, inserir objeto, objeto a partir de um intervalo, objeto a partir de um gráfico, objeto a partir de uma página, colar como objeto, lista de objetos, editar objeto, botões de campo, botões de gráfico dinâmico, grupo, desagrupar, alinhar, alinhar à esquerda, distribuir, selecionar objetos, várias formas, selecionar várias, caixa de arrastar, filtros do gráfico, ocultar série, gráficos recomendados, sugestões de gráfico, gráfico, diagrama de colunas, barras, linha, pizza, área, dispersão, combinação, radar, radar preenchido, gráfico de aranha, legenda, título, linhas de grade, rótulos de dados, dados do gráfico, tipo de gráfico, aparência, layouts, mover, redimensionar, imagem, forma, objeto, série, linha de tendência, tendência, equação, R ao quadrado, segundo eixo, minigráfico, minigráficos, eixo de datas, vitória/derrota, gráfico pequeno, marcadores, nome, texto alternativo, texto alt, logarítmico, escala logarítmica, salvar como imagem, modelo, crtx, texto do título, chave de legenda, painel de seleção, preenchimento da forma, contorno da forma, girar, inverter, cortar, editar texto, borda da imagem, inserir imagem, ícones, ícone, wmf, emf, estilos de forma, estilos, inserir forma, caixa de texto, formas, seta, oval, retângulo, estrela, diagrama, diagramas, imagens online, imagens gratuitas, openverse, wikimedia commons, licença, crédito, texto artístico, wordart, distorção, compactar imagens, reduzir imagens, tamanho do arquivo, ppi, excluir áreas cortadas, código de barras, código QR, vincular forma a célula, forma mostra uma célula, caixa de texto mostra uma célula, valor de célula em forma -->

Um gráfico é uma imagem de números que permanece ligada a eles: ao alterar um número, o gráfico muda junto. Imagens e formas ficam sobre as células do mesmo modo, mas não estão ligadas a nenhum número.

## Criar um gráfico

1. Selecione os números que deseja representar. Se a primeira linha tiver nomes e a primeira coluna tiver rótulos, selecione-os também. O JeffCrunch examina a primeira linha e a primeira coluna e as usa como nomes e rótulos quando forem palavras (ou datas, no caso dos rótulos).
2. Na guia **Inserir**, no grupo **Gráficos**, escolha um tipo: **Coluna**, **Barras**, **Linha**, **Pizza**, **Área**, **Dispersão**, **Combinação**, **Radar** ou **Ações**. A pequena seta ao lado de um tipo mostra as variações, como colunas empilhadas, uma linha com marcadores, um gráfico de dispersão de pontos, de pontos unidos ou de linhas suavizadas, ou um gráfico de radar simples, com marcadores ou preenchido. Um gráfico de radar desenha cada rótulo como um raio que sai do centro e cada série como uma forma fechada que passa pelos seus números nos raios; assim, ele mostra várias medidas de uma mesma coisa lado a lado.
   Um gráfico de **ações** mostra preços: uma linha para cada categoria, do preço mais alto ao mais baixo, com um pequeno ponto para o fechamento; quando há abertura, uma barra entre a abertura e o fechamento (clara para alta, escura para queda). O gráfico é feito com as suas colunas exatamente como estão, da esquerda para a direita, e nada verifica se os números correspondem à sua posição. Por isso, coloque-os nesta ordem: máxima, mínima e fechamento; abertura, máxima, mínima e fechamento; volume, máxima, mínima e fechamento; ou volume, abertura, máxima, mínima e fechamento. Se as colunas estiverem em outra ordem, o gráfico ainda será desenhado, mas as marcas terão outro significado (o ponto de fechamento vai para a terceira coluna de preço, e a caixa vai da primeira coluna de preço até a última). Com volume, o volume é lido no eixo da esquerda, e os preços, em um segundo eixo, à direita. Se o número de colunas não corresponder ao tipo, o JeffCrunch informa quantas são necessárias e não cria o gráfico.
3. Para ver antes todos os tipos, escolha **Mais Gráficos**. A lista à esquerda mostra cada tipo desenhado com as suas células. **Trocar linhas e colunas** inverte o sentido das séries.

Não sabe qual tipo escolher? Selecione as células e escolha **Gráficos Recomendados** na guia **Inserir**. O JeffCrunch examina as células e lista alguns tipos adequados, do melhor para o menos adequado. Por exemplo, datas na lateral sugerem uma linha, e uma coluna de poucos números positivos com rótulos sugere uma pizza. Cada tipo é desenhado com as suas células. Escolha um e clique em **OK**.

Se você selecionar apenas uma célula, o JeffCrunch representa o bloco de células ao redor dela que tenha algum conteúdo.

As séries seguem as colunas quando a seleção tem pelo menos tantas linhas quanto colunas, e seguem as linhas quando tem mais colunas do que linhas. Uma pizza tem uma única série. Um gráfico de dispersão usa a primeira coluna para os valores horizontais.

Exemplo: você seleciona A1:C4, em que A2:A4 contêm Q1, Q2 e Q3, B1 e C1 contêm Leste e Oeste, e o restante são números. O gráfico tem duas séries, Leste e Oeste, e três grupos de colunas, de Q1 a Q3.

## Ocultar uma série por enquanto

Selecione um gráfico. Três pequenos botões aparecem ao lado dele. O que tem um funil é **Filtros do Gráfico**. Ele lista as séries do gráfico, cada uma com uma marca de seleção. Desmarque uma série para deixá-la fora do gráfico. Marque-a novamente para trazê-la de volta, no mesmo lugar. **Desfazer** também funciona. Um gráfico sempre mantém pelo menos uma série. Uma série que você deixou de fora continua no arquivo ao salvá-lo e permanece oculta ao abri-lo, de modo que nada se perde. Marque-a novamente para trazê-la de volta. (Um gráfico de rosca mostra todas as séries ao ser aberto.)

## Mover e redimensionar

Clique em um gráfico, imagem ou forma para selecioná-lo. Arraste-o para movê-lo, ou arraste um dos oito quadrados na borda para redimensioná-lo.

- Mantenha **Alt** pressionado enquanto arrasta para alinhar uma borda com a borda de uma célula.
- **Ajustar à Grade** e **Ajustar à Forma** ficam na guia **Formato da Forma** e são mantidos para você, em todas as pastas de trabalho. Com **Ajustar à Grade** ativado, uma borda arrastada vai até a borda mais próxima de uma célula. Com **Ajustar à Forma** ativado, uma borda vai até a borda de outro objeto quando estiver a até 6 pixels dele. Apenas os primeiros 2.000 outros objetos de uma planilha são considerados para Ajustar à Forma. Com os dois ativados, prevalecem as bordas das células, porque um objeto é ancorado às células, e as bordas dos outros objetos não são examinadas. Uma alteração chega a todas as janelas abertas de uma vez.
- **Alt** inverte o ajuste ativo em um único arrasto: com **Ajustar à Grade** ou **Ajustar à Forma** ativado, manter **Alt** pressionado desliga o ajuste apenas para aquele arrasto (nunca troca um tipo pelo outro), e com os dois desativados, **Alt** faz as bordas irem para as células, como descrito acima.
- Um objeto girado é ajustado pelo contorno que você vê (a menor caixa que o contém), e não pelo quadro anterior à rotação; o mesmo vale para os objetos aos quais ele se ajusta.
- Mantenha **Shift** pressionado enquanto arrasta um canto para manter as proporções.
- As teclas de seta movem o objeto alguns pixels por vez. **Ctrl** com uma seta move um pixel por vez, **Shift** com uma seta aumenta ou diminui o objeto, e **Alt** com uma seta vai até a próxima borda de uma célula.
- **Esc** enquanto arrasta devolve o objeto à posição original.

Cada movimento ou redimensionamento é uma etapa que você pode desfazer. Clique com o botão direito no objeto e escolha **Propriedades** para digitar o **Nome** (cada objeto de uma planilha precisa de um nome próprio) e o **Texto alternativo** (uma frase curta que informa, a quem não pode ver o objeto, o que ele mostra), para digitar o tamanho e para escolher o que ele faz ao inserir, excluir ou redimensionar células: mover e redimensionar com as células, mover com elas mas manter o tamanho, ou permanecer onde está. A escolha é salva no arquivo. **Imprimir este objeto**, na mesma caixa, desativa o objeto nas páginas impressas (ele continua aparecendo na tela); o arquivo também guarda essa escolha.

## Alterar o que um gráfico mostra

Quando um gráfico está selecionado, dois botões aparecem ao lado dele e a guia **Gráfico** aparece na faixa de opções.

- O botão **+** (e **Elementos do Gráfico** na guia) liga ou desliga o título do gráfico, os eixos, os títulos dos eixos, as linhas de grade, a legenda e os rótulos de dados, e define onde ficam a legenda e os rótulos.
- O botão do pincel (e **Cores** e **Aparência** na guia) aplica ao gráfico um dos conjuntos de cores do JeffCrunch e uma de suas aparências. **Layouts** define vários elementos de uma só vez.
- **Dados do Gráfico** lista as séries do gráfico. Você pode adicionar, editar, remover e reordenar séries, alterar as células que cada uma lê, definir as células com os rótulos, trocar linhas e colunas, e escolher como as células vazias e as linhas e colunas ocultas são representadas.
- **Botões de Campo** (no menu **+**, apenas para um gráfico dinâmico) mostram pequenos botões com os nomes dos campos da tabela dinâmica que o gráfico acompanha: os filtros de relatório abaixo do título, e as linhas, os valores e as colunas na parte inferior. Ligue ou desligue cada tipo, ou escolha **Mostrar Tudo** ou **Ocultar Tudo**. Um novo gráfico dinâmico já mostra esses botões.
- **Barras de Erro** (no menu **+**, para um gráfico de colunas, barras, linhas, área ou dispersão) desenha uma pequena linha acima e abaixo de cada ponto, para mostrar o quanto o valor real pode diferir. Escolha **Erro Padrão**, **Porcentagem (5%)**, **Desvio Padrão** ou **Valor Fixo (1)**. Escolha **Personalizado** para usar valores próprios: digite as células que contêm os valores acima de cada ponto (como `=Plan1!$D$2:$D$4`) e, em seguida, as células dos valores abaixo. Deixe uma vazia para desenhar as barras de um lado só. As barras mudam quando essas células mudam. Um arquivo que tenha valores próprios para cada ponto os mantém.
- **Linhas** (no menu **+**, para um gráfico de linhas) tem **Linhas de Queda**, que descem do ponto mais alto de cada categoria até o eixo, **Linhas de Máxima e Mínima**, que unem os pontos mais alto e mais baixo de cada categoria, e **Barras de Alta/Baixa**, que são caixas entre a primeira e a última série de cada categoria. Uma barra de alta mostra que a última série está mais alta. As Linhas de Máxima e Mínima e as Barras de Alta/Baixa exigem duas séries ou mais. Gráficos de área têm Linhas de Queda.
- **Tabela de Dados** (no menu **+**, para um gráfico de colunas, linhas ou área) coloca uma grade com os números abaixo do gráfico, com uma coluna para cada categoria e uma linha para cada série. Sua lista liga e desliga a tabela e define as linhas entre as linhas, as linhas entre as colunas, o contorno e as chaves de legenda (a cor de cada série ao lado do nome).
- **Linha de Tendência** (no menu **+**) desenha uma linha ajustada aos números de cada série, para mostrar para onde eles caminham. Escolha **Linear** para uma reta, **Exponencial** para números que crescem em uma porcentagem constante, **Logarítmica** para números que sobem rápido e depois se estabilizam, **Polinomial** para uma curva com inflexões, **Potência** ou **Média Móvel**, que desenha a média dos últimos números. **Mostrar Equação** escreve a fórmula da linha no gráfico, e **Mostrar R ao quadrado** escreve o quanto a linha se ajusta bem (1 é um ajuste perfeito; 0 é nenhum ajuste). A linha de tendência ganha um item próprio na legenda.
- Quando uma série é desenhada em um segundo eixo vertical, os menus **Eixos** e **Títulos dos Eixos** têm as opções **Segundo Horizontal** e **Segundo Vertical** para ela. O menu **Legenda** tem **Sobre o Gráfico**, que coloca a legenda sobre a área de plotagem em vez de ao lado dela.
- **Rótulos de Dados** também tem **Rótulo Contém** (o valor, o nome da categoria, o nome da série, a porcentagem e a chave de legenda, que é um pequeno quadrado na cor da série, em qualquer combinação), **Separador** (o que fica entre eles: vírgula, ponto e vírgula, ponto, nova linha ou espaço) e **Formato de Número…** (um código de formato, como 0,0, para os números dos rótulos; deixe-o vazio para mostrar os números como são). **Valor das Células…** mostra as palavras de algumas células como rótulos, uma célula para cada ponto. Digite as células, como `=Plan1!$D$2:$D$4`. Os rótulos mudam quando as células mudam. Deixe-o vazio para removê-los. Desligue todos os conteúdos e os rótulos desaparecem.
- **Formatar Eixo Horizontal…** e **Formatar Eixo Vertical…** na guia **Gráfico** abrem uma caixa para um eixo. Ela define os valores mínimo e máximo e as unidades principal e secundária (cada uma automática ou um número que você digita), se os valores aparecem em ordem inversa, as unidades de exibição (os rótulos mostram os números divididos, como por 1.000 para milhares, de modo que 30.000 aparece como 30; marque **Mostrar o nome das unidades no gráfico** para imprimir o nome, como Milhares, junto ao título do eixo), se a escala é logarítmica (cada marca é uma base vezes a anterior, como 1, 10, 100 e 1000, o que serve para números de tamanhos muito diferentes e que devem ser todos maiores que 0), onde o outro eixo cruza este, o formato de número dos rótulos, as marcas de divisão, onde ficam os rótulos e quanto eles são girados. Apenas um eixo numérico tem limites e unidades.
- **Formatar Parte do Gráfico…** na guia **Gráfico** (e no menu do botão direito) define o preenchimento e a linha da área do gráfico, da área de plotagem (a caixa em que as marcas ficam) ou de uma série. Para uma linha, define também se ela é suavizada e qual marca fica em cada ponto. O menu do botão direito de um gráfico também tem **Trocar Linhas e Colunas** e os dois comandos de eixo.
- **Seleção Atual** lista as partes do gráfico: a área do gráfico, a área de plotagem e cada série. Escolha uma e **Formatar Seleção** abre a janela de formatação nessa parte. **Redefinir para Corresponder ao Estilo** remove o que foi definido na parte escolhida e devolve o que o estilo do gráfico lhe dá. Para a área do gráfico, isso significa o preenchimento, a linha e a cor do texto definida por uma aparência (o texto claro da aparência escura volta para a cor de texto do tema). Para a área de plotagem, são o preenchimento e a linha. Para uma série, é a sua cor (a que a escolha Cores ou o estilo do gráfico lhe dá), o contorno, as marcas como o tipo de gráfico as tem e as cores de seus pontos; uma linha suavizada permanece quando o tipo de gráfico é suavizado. Não altera títulos, legenda, rótulos, eixos, linhas de grade nem dados, e não faz nada com uma parte que já corresponde ao estilo. É uma etapa de Desfazer.
- **Mover Gráfico…** na guia **Gráfico** pergunta para onde o gráfico deve ir. Escolha **Nova planilha** e digite um nome, e o gráfico ganha uma planilha de gráfico própria: uma planilha que contém apenas um gráfico, sem células. Escolha **Objeto em** e selecione uma planilha, e o gráfico vai para essa planilha. Um gráfico em uma planilha de gráfico só pode ir para uma planilha, e a planilha de gráfico que ele deixa é excluída. O gráfico continua lendo as células que lia. Em uma planilha, ele mantém o seu tamanho e a sua posição, mesmo quando as linhas e colunas são mais altas ou mais largas, e se move e redimensiona com as células como antes. Planilhas ocultas não são oferecidas. É uma só etapa de **Desfazer**, mesmo depois de salvar, a menos que uma planilha tenha sido excluída por ela. Um arquivo no formato `.ods` não pode conter uma planilha de gráfico; por isso, salve uma planilha de gráfico no formato `.xlsx`.
- **Título do Gráfico** e **Títulos dos Eixos** (no menu **+**) também têm **Texto do Título…**. Digite as palavras que quiser. Ou digite um sinal de igual e uma célula, como **=Plan1!A1**, e o título mostrará o que essa célula contém. Quando a célula muda, o título também muda. O arquivo guarda o vínculo.
- **Salvar como Imagem…** na guia **Gráfico** (e no menu do botão direito) salva o gráfico selecionado como um arquivo de imagem **PNG** ou **JPEG**, com o dobro do tamanho que tem na planilha, para que fique nítido.
- **Salvar como Modelo…** e **Aplicar Modelo…** (na guia **Gráfico**, no grupo **Tipo**) guardam a aparência de um gráfico em um arquivo **.crtx** e a aplicam a outro gráfico. A aparência inclui o tipo, as cores, as configurações de título e legenda, os eixos e as fontes. O gráfico ao qual você aplica o modelo mantém os próprios números e as células que lê. Um **Desfazer** remove a aparência de volta.
- **Tipo de Gráfico** altera o tipo do gráfico inteiro ou de cada série: colunas com uma linha por cima, ou uma série em um segundo eixo vertical.

Uma referência a células é escrita como **Plan1!$B$2:$B$9**, com o nome da planilha entre aspas simples quando ele tiver espaços.

## Minigráficos

Um minigráfico é um gráfico minúsculo que cabe dentro de uma célula. Ele mostra a tendência de uma linha ou de uma coluna de números bem ao lado deles.

1. Selecione os números. Para um minigráfico por linha, selecione várias linhas de números.
2. Na guia **Inserir**, no grupo **Minigráficos**, escolha **Linha**, **Coluna** ou **Vitória/Derrota**. **Linha** une os números com uma linha. **Coluna** desenha uma barra para cada número. **Vitória/Derrota** desenha uma barra para cima para cada número acima de zero e uma barra para baixo para cada número abaixo de zero, todas do mesmo tamanho, o que serve para resultados que são apenas bons ou ruins.
3. Na caixa **Criar Minigráficos**, confira o **Intervalo de dados** (as células que os minigráficos leem) e digite o **Intervalo de localização** (as células em que eles ficam). Escolha uma célula de localização para cada linha de dados, ou para cada coluna de dados. Se escolher uma única célula, o minigráfico lê todos os dados.

Os minigráficos criados em uma só etapa formam um grupo e compartilham uma aparência. Clique em uma célula com minigráfico, e a guia **Minigráfico** aparece.

- **Editar Dados** altera os dados e a localização de todo o grupo, ou apenas os dados do minigráfico em que você clicou. Também define o que fazer com células vazias (deixar uma lacuna, tratá-las como zero ou unir os pontos através delas) e se as células em linhas e colunas ocultas devem ser lidas.
- **Tipo** alterna entre linha, coluna e vitória/derrota.
- **Mostrar** marca pontos: o **Ponto Alto**, o **Ponto Baixo**, os **Pontos Negativos**, o **Primeiro Ponto**, o **Último Ponto* e, para uma linha, **Marcadores** em cada ponto.
- **Estilos** dá ao grupo um conjunto de cores. **Cor do Minigráfico** define a cor da linha ou das barras, e a **Espessura** (o quão grossa é a linha, em pontos). **Cor do Marcador** define a cor de cada tipo de ponto marcado.
- **Eixo** mostra uma linha no zero, desenha os números da direita para a esquerda e define os valores mínimo e máximo do eixo vertical. **Automático para Cada Minigráfico** ajusta cada um aos próprios números. **Igual para Todos os Minigráficos** dá ao grupo inteiro uma só escala, para que possam ser comparados. **Valor Personalizado** permite digitar um número. **Eixo de Datas…** espaça os pontos de acordo com as datas: digite as células que contêm a data de cada ponto, e um ponto com data posterior fica mais adiante. Deixe-o vazio para voltar a espaçar os pontos igualmente.
- **Agrupar** coloca os minigráficos selecionados em um só grupo. **Desagrupar** dá a cada um a sua própria aparência. **Limpar** remove os minigráficos selecionados ou os grupos inteiros.

Um minigráfico acompanha as suas células: se você inserir uma linha acima dos dados, ele continua lendo os mesmos números. Cada alteração é uma etapa que você pode desfazer. Os minigráficos são salvos no arquivo no formato que outros programas de planilha leem.

## Inserir uma imagem, uma forma ou uma caixa de texto

Na guia **Inserir**, no grupo **Ilustrações**, **Imagem** abre uma janela de seleção de arquivos. Escolha um arquivo de imagem (PNG, JPEG, GIF, BMP, TIFF ou SVG) e ele aparece na planilha, na célula selecionada. Uma imagem com mais de 480 pontos de largura é reduzida para caber, e mantém a sua forma. A pasta de trabalho guarda a própria imagem. Uma imagem de um tipo que a pasta de trabalho não consegue guardar como está (um SVG, por exemplo) é mantida como PNG, uma imagem simples feita de pontos. Um arquivo que não é imagem, ou que tem mais de 16 MB, é recusado com uma mensagem. Um arquivo WMF ou EMF (os tipos de imagem que alguns programas antigos criam a partir de linhas e formas) é desenhado e também mantido como PNG.

**Ícones** mostra a mesma janela de pequenas imagens de linha que o JeffPub e o JeffDeck têm, com uma caixa para pesquisá-las pelo nome. Escolha um ou mais, e cada um aparece na planilha como uma imagem de 48 pontos de largura e altura. Ele é desenhado em cinza-escuro, em um tamanho que continua nítido quando você o amplia. Cada um é uma única etapa que você pode desfazer.

**Código de Barras** abre um pequeno menu com duas opções. **Código de Barras** abre uma janela para criar um código de barras (**Códigos de Barras a Partir de Células**, abaixo, cria vários): ISBN de livro (com ou sem preço), EAN-13, UPC-A, EAN-8, Code 128, Code 39 ou um código QR, que guarda qualquer texto, como um endereço da web. Escolha um **Tipo**, digite o texto ou os dígitos, e a janela mostra o código de barras enquanto você digita, e indica em vermelho o que está errado se ele não puder ser criado. Clique em **Inserir**, e ele aparece na planilha como uma imagem, em uma etapa que você pode desfazer. Mova-o e redimensione-o como qualquer imagem. Para alterá-lo, selecione-o e clique novamente em **Código de Barras**, altere o texto e clique em **Atualizar**; o novo código de barras ocupa o lugar e a largura do antigo. Ao salvar, ele é gravado como imagem com um SVG que continua nítido, para que todos os programas o mostrem, e o JeffCrunch guarda suas configurações no arquivo; assim, ao abrir a pasta de trabalho novamente no JeffCrunch, você ainda pode editá-lo.

**Código de Barras > Códigos de Barras a Partir de Células** (no mesmo menu **Código de Barras**) cria um código de barras para cada célula preenchida das células selecionadas, para uma folha de etiquetas. Selecione as células, clique nessa opção e depois escolha o tipo e a aparência na mesma janela. O que você digitar ali é apenas um exemplo, porque o texto de cada célula se torna o seu próprio código de barras. Os códigos de barras ficam em uma coluna logo à direita das células selecionadas, um abaixo do outro, em uma única etapa que você pode desfazer. Uma célula cujo texto o código não comporta (uma letra em um EAN-13, por exemplo) é informada em uma mensagem e não recebe código de barras. No máximo 500 células são processadas de uma vez; selecione menos se a mensagem disser que há células demais. Cada código de barras lembra a célula de onde veio e a acompanha: ao alterar essa célula, o código é refeito a partir do novo texto, no mesmo lugar e com o mesmo tamanho, como uma etapa própria que você pode desfazer. O vínculo acompanha a célula quando você insere ou exclui linhas ou colunas. Se você excluir a célula, o código de barras mantém o último texto que tinha. Ao salvar, a imagem do arquivo é feita a partir do texto atual da célula, para que outros programas também mostrem o valor atual.

**Formas** abre uma lista: retângulo, retângulo de cantos arredondados, oval, triângulo, losango, setas, uma estrela, um coração, uma nuvem, um balão de fala e uma linha. Escolha uma e ela aparece na planilha em azul-claro, com uma borda mais escura. **Caixa de Texto**, no grupo **Texto**, insere uma caixa vazia, sem preenchimento e sem borda. Clique em **Desenhar Caixa de Texto** (o botão principal, ou a primeira opção do seu menu) e arraste sobre a planilha para desenhar a caixa. Um arrasto cria uma caixa do tamanho que você arrastou, mas nunca menor que 18 pontos (0,64 cm) em um lado, e um clique de menos de 4 pontos dá o tamanho usual. Mantenha **Shift** pressionado para deixar a caixa quadrada. O ponteiro é uma cruz até você soltar o botão; **Esc**, ou qualquer outro comando, desativa a ferramenta. A caixa é uma etapa que você pode desfazer.

Para girar as palavras de uma caixa de texto, selecione-a e clique em **Direção do Texto** na guia **Formato da Forma**: **Horizontal**, **Girar todo o texto 90°** ou **Girar todo o texto 270°**. As palavras são digitadas giradas, como são mostradas, e uma pasta de trabalho salva no formato .xlsx ou .ods mantém a direção.

Com uma forma selecionada, **Estilos** na guia **Formato da Forma** dá a ela um visual pronto: um preenchimento e um contorno que combinam, como azul-claro com uma borda azul mais escura. As duas últimas opções mantêm apenas o contorno ou apenas o preenchimento. Um clique é uma etapa que você pode desfazer. **Preenchimento da Forma** e **Contorno da Forma** alteram cada parte separadamente.

## Encontrar imagens gratuitas na internet

**Imagens Online**, no grupo **Ilustrações** da guia **Inserir**, encontra imagens gratuitas. Elas vêm de duas bibliotecas abertas: o Openverse, que pesquisa em muitas coleções, e o Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Clique em **Imagens Online…**. Um painel se abre à direita.
2. Escolha uma biblioteca, digite o que procura e clique em **Pesquisar** (ou pressione Enter).
3. Clique em uma imagem para ver quem a criou e qual **licença** ela tem. A licença diz como você pode usar a imagem. Clique no nome da licença para ler o texto.
4. Clique em **Inserir** (ou clique duas vezes na imagem).

**O que o JeffCrunch envia.** As palavras da sua pesquisa vão para a biblioteca escolhida, e somente quando você clica em **Pesquisar** ou pressiona Enter. Em seguida, o JeffCrunch pede à biblioteca as pequenas prévias do que foi encontrado e a imagem única que você inserir. Como qualquer página da web, a biblioteca pode ver o endereço de internet do seu computador, e é informada de que o JeffCrunch está fazendo a solicitação. Nada da sua pasta de trabalho é enviado.

A imagem aparece na planilha como uma imagem de arquivo, e a pasta de trabalho a mantém. O título dela é salvo como **texto alternativo**. A maioria das licenças pede que você cite o autor. Quando **Adicionar crédito abaixo da imagem** estiver marcado, o JeffCrunch acrescenta uma pequena caixa de texto sob a imagem, com o título, o autor e a licença. Você pode mover, alterar ou excluir essa caixa. A imagem e o crédito são inseridos em uma só etapa, de modo que **Desfazer** remove os dois. Confirme se a licença permite o uso que você pretende dar à imagem.

## Adicionar Texto Artístico

**Texto Artístico** são palavras decoradas: letras com cor, contorno, sombra ou forma curva.

1. Clique em **Inserir > Texto > Texto Artístico**. Abre-se uma galeria de estilos.
2. Clique em um estilo. As palavras "Seu Texto Aqui" aparecem na planilha nesse estilo.
3. Para alterar as palavras, selecione o Texto Artístico e clique em **Editar Texto…** na guia **Formato da Forma**. Uma caixa pede as palavras, a fonte, o tamanho, o negrito e o itálico. Um tamanho maior deixa o Texto Artístico maior.
4. Na guia **Formato da Forma**, **Estilos de Texto Artístico** dá às letras outro estilo, e **Distorção de Texto** dobra as palavras em uma forma, como uma onda ou um arco. "Simples" as mantém retas.

O Texto Artístico é uma forma no desenho da planilha, com sua distorção, preenchimento e contorno, por isso mantém o seu visual ao salvar a pasta de trabalho no formato .xlsx e reabri-la, no JeffCrunch ou em outro programa. Cada alteração é uma etapa que você pode desfazer. Uma pasta de trabalho salva no formato .ods mantém também suas imagens, formas, caixas de texto, grupos, linhas e Texto Artístico. O Texto Artístico é salvo como a forma de texto de fontes que o OpenDocument possui, com suas palavras, fonte, preenchimento, contorno e distorção, e volta como Texto Artístico ao reabrir o arquivo .ods. Algumas distorções para as quais o OpenDocument não tem forma (Onda 2, Onda 4 e as formas inferior e superior de Inflar e Desinflar) são salvas como palavras retas, com as mesmas letras e estilo. Os gráficos também são mantidos em um arquivo .ods: veja abaixo.

## Reduzir o tamanho das imagens

Uma foto tirada com câmera pode ser muito maior do que aparece na planilha, e isso deixa o arquivo grande. Clique em uma imagem e, em seguida, clique em **Compactar Imagens…** no grupo **Ajustar** da guia **Formato da Imagem**. Escolha onde a pasta de trabalho será usada: **Impressão comercial (300 ppi)**, **Impressão em desktop (220 ppi)** ou **Web (96 ppi)**. Deixe **Aplicar a todas as imagens desta planilha** marcado para reduzir todas as imagens da planilha, ou desmarque-o para reduzir apenas a imagem selecionada. **Excluir áreas cortadas das imagens** também remove as partes ocultas que você cortou, e o corte deixa de existir. Uma imagem só é substituída quando a nova é menor. Seu tamanho, posição, nome e texto alternativo permanecem como estavam. Desfazer (Ctrl+Z) devolve as imagens, e a pasta de trabalho salva guarda apenas as imagens menores. Imagens de outras planilhas não são alteradas: vá até cada planilha e faça o processo novamente. Uma imagem que seja um desenho (SVG, EMF ou WMF), ou uma animação, é deixada como está.

Cada uma dessas operações é uma única etapa que você pode desfazer. O novo objeto fica selecionado, para que você possa arrastá-lo, redimensioná-lo, trazê-lo para a frente ou enviá-lo para trás, dar-lhe um nome e um texto alternativo em Propriedades, ou pressionar Delete.

## Mostrar o texto de uma célula em uma forma ou caixa de texto

Uma forma ou caixa de texto pode mostrar o que uma célula mostra, e acompanhá-la. Selecione a forma, clique na **barra de fórmulas**, digite um sinal de igual e uma célula e pressione **Enter**.

- `=B2` vincula a forma a B2 desta planilha. `=Planilha2!B3` vincula a uma célula de outra planilha. `=Total` vincula a um nome que representa uma célula.
- A forma então mostra o texto da célula da mesma forma como a célula o exibe, com o seu formato de número. Quando você altera a célula, o formato dela ou uma fórmula da qual ela depende, a forma também muda. Um texto maior do que uma célula comporta (32.767 caracteres) é cortado. Todas as formas vinculadas de uma pasta de trabalho juntas mostram no máximo 4 MB de texto: além disso, uma forma mostra seu texto cortado.
- Com a forma selecionada, a barra de fórmulas mostra o vínculo, como `=Plan1!$B$2`. Apague o conteúdo da barra de fórmulas e pressione **Enter** para remover o vínculo. A forma mantém as palavras que mostra nesse momento.
- Digitar palavras na forma também remove o vínculo.
- Um vínculo é de uma única célula. Um intervalo como `=A1:B2`, uma lista, um cálculo ou um nome que não seja uma célula é recusado com uma mensagem, e a forma mantém o seu vínculo.
- Se a linha ou a coluna da célula for excluída, ou a sua planilha, a forma mostra `#REF!` e mantém o vínculo, como faria uma fórmula. Desfazer restaura a célula.
- Cada vínculo, e cada vez que você o remove, é uma etapa que você pode desfazer.

O vínculo é salvo em um arquivo .xlsx junto com o texto que a célula mostrava, de modo que um programa que não acompanha vínculos ainda mostra as palavras, e o JeffCrunch abre o arquivo com o vínculo funcionando. O OpenDocument não tem como vincular uma caixa de texto a uma célula; por isso, uma pasta de trabalho salva no formato .ods mantém as palavras que a forma mostra naquele momento, e não o vínculo.

## Alterar a aparência de uma imagem, forma ou caixa de texto

Selecione uma imagem e a guia **Formato da Imagem** aparece. Selecione uma forma, uma linha ou uma caixa de texto e a guia **Formato da Forma** aparece. Cada alteração é uma etapa que você pode desfazer, e o objeto mantém o seu nome, o texto alternativo, a posição e a ordem entre os outros objetos.

- **Preenchimento da Forma** e **Contorno da Forma** escolhem uma cor, ou nenhuma. **Estilo da Linha** define a espessura (o quão grossa é a linha) e os traços (sólido, tracejado ou pontilhado). Uma imagem tem **Borda da Imagem**, que é o seu contorno.
- **Girar** gira o objeto em um quarto de volta para a direita ou para a esquerda, ou o inverte horizontal ou verticalmente.
- **Cortar**, para uma imagem, a recorta em uma forma de janela: quadrada, 4 de largura por 3 de altura, 3 de largura por 4 de altura, 16 por 9 ou 9 por 16. O corte é feito de modo uniforme a partir do centro do que aparece, e a imagem é ampliada até o tamanho do que sobrou. **Mostrar a Imagem Inteira** traz de volta o que foi cortado. **Preencher** mantém o quadro e corta a imagem para que ela cubra todo ele. **Ajustar** mostra a imagem inteira, e o quadro assume a forma da imagem dentro do antigo. **Cortar para Forma** (a galeria ao lado de **Cortar**) recorta a imagem em uma forma como oval, estrela ou retângulo de cantos arredondados; a forma é salva com a imagem. O arquivo da imagem não é alterado: a parte cortada apenas fica oculta.
- **Editar Texto** serve para formas e caixas de texto. Digite as palavras e escolha como elas ficam na horizontal e na vertical, o tamanho, o negrito ou o normal, e se elas se quebram conforme a forma ou ficam em uma só linha. Para o Texto Artístico, ela pede as palavras, a fonte, o tamanho, o negrito e o itálico.
- **Painel de Seleção** abre uma lista de todos os objetos da planilha, pelo nome, com o que está na frente em primeiro lugar. Clique em um nome para selecionar esse objeto, o que ajuda quando os objetos se escondem uns aos outros. **Trazer para a Frente** e **Enviar para Trás** movem o objeto na lista.
- **Propriedades** define o tamanho, o nome, o texto alternativo e o que o objeto faz quando as células são movidas ou ampliadas.

## Recortar, copiar e colar um objeto

Uma imagem ou forma é copiada e colada do mesmo modo que um gráfico. Uma forma que usa algo diferente de uma imagem da pasta de trabalho, como um vínculo para um arquivo, não pode ser copiada, e o JeffCrunch informa o motivo.

Copie um gráfico e cole-o na mesma pasta de trabalho, em qualquer planilha: a cópia lê as mesmas células que a primeira. Cole-o em outra pasta de trabalho e ele mantém os números que mostrava, mas não lê mais células, porque as células que leria ali não são as de onde ele foi criado.

## Selecionar vários objetos, agrupá-los e alinhá-los

Clique em um objeto e, em seguida, mantenha Shift ou Ctrl pressionado e clique em outro para selecioná-lo também. Clique novamente em um objeto selecionado, com Shift ou Ctrl, para desfazer a seleção dele. Os objetos selecionados recebem cada um um quadro tracejado. Você também pode escolher **Selecionar Objetos** na guia **Formato da Forma** e arrastar uma caixa sobre células vazias: todos os objetos que caibam inteiramente dentro da caixa são selecionados. Pressione Esc, ou escolha **Selecionar Objetos** novamente, para voltar à seleção de células. Pressione Delete para remover todos os objetos selecionados em uma só etapa.

Com dois ou mais objetos selecionados, a guia **Formato da Forma** tem estes comandos no grupo **Organizar**:

- **Agrupar** une as formas, imagens e linhas selecionadas em um só objeto. Ele se move e se redimensiona como um todo, e o arquivo o mantém como grupo. Um gráfico não pode ser colocado em um grupo.
- **Desagrupar** desfaz um grupo selecionado. Cada parte volta para o lugar onde estava na planilha.
- **Alinhar** alinha os objetos selecionados: **Alinhar à Esquerda**, **Centralizar**, **Alinhar à Direita**, **Alinhar em Cima**, **Alinhar ao Meio** e **Alinhar Embaixo**. Eles se alinham pela borda ou pelo centro do conjunto inteiro. **Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam igualmente três ou mais objetos entre os dois das extremidades.

Agrupar, Desagrupar e cada opção de Alinhar são uma etapa que você pode desfazer.

## Dar um nome

Com um objeto selecionado, a caixa de nomes mostra o nome dele. Digite um novo nome ali e pressione **Enter**. Desfazer restaura o nome antigo, e o novo nome é salvo no arquivo.

## Excluir

Pressione **Delete** com o objeto selecionado. Desfazer o traz de volta.

## O que é salvo

Tudo o que foi descrito acima é salvo no arquivo no formato que outros programas de planilha leem, para que um gráfico criado aqui abra da mesma forma lá. Um gráfico vindo de outro programa, com algo que o JeffCrunch não consegue manter ao regravar um gráfico (barras de erro, um gráfico tridimensional), ainda pode ser movido, redimensionado e excluído, mas seus elementos e dados não são alterados aqui.

## Inserir um objeto

Um **objeto** guarda o próprio arquivo dentro da pasta de trabalho, para que o aplicativo que o criou possa abri-lo novamente. Ele mostra uma imagem de si mesmo, mas não é uma imagem: continua sendo um objeto quando você o move, redimensiona, copia ou salva a pasta de trabalho.

1. Clique em **Inserir** e depois em **Objeto**, e escolha **De um Intervalo do JeffCrunch**, **De um Gráfico do JeffCrunch** ou **De uma Página do JeffWrite**.
2. Escolha o arquivo e, se a caixa pedir, o intervalo ou o número da página. Se você copiou um deles no outro aplicativo, a caixa oferece usá-lo.
3. O objeto é inserido na célula selecionada. Arraste-o para movê-lo e arraste uma alça para redimensioná-lo. Um intervalo, gráfico ou página que você redimensiona é desenhado novamente no novo tamanho, para continuar nítido. Pressione **Delete** para removê-lo. Cada uma dessas operações é uma etapa de Desfazer.

Para inserir o que você copiou em outro aplicativo, clique na seta abaixo de **Colar** e escolha **Como Objeto**.

**Clique duas vezes** em um objeto para abri-lo no aplicativo que o criou. Salve-o ali, e o objeto aqui mostra a alteração como uma etapa de Desfazer. Uma célula que você estava digitando é deixada como está. Uma cópia que você cola é um objeto próprio: alterar um não altera o outro. O mesmo vale para uma planilha que você copia: os objetos dela são cópias, e a cópia da planilha os mantém.

**Arquivo**, depois **Informações**, depois **Objetos** lista todos os objetos da pasta de trabalho. Selecione um para abri-lo, transformá-lo em uma imagem comum ou excluí-lo.

Um objeto criado em outro programa aparece como a imagem que o arquivo dele guarda, qualquer que seja o tipo dessa imagem. Você pode selecioná-lo, movê-lo, redimensioná-lo, alinhá-lo a outros objetos e excluí-lo, e ele se move com as células como os seus próprios objetos. Não é possível editá-lo aqui, colocá-lo em um grupo nem alterar sua posição na ordem dos objetos. Ele é salvo exatamente como o arquivo o tinha, com apenas a posição alterada, se você o moveu. Um objeto que um arquivo .ods continha funciona da mesma forma. Salvos como arquivo .xlsx ou .ods, seus objetos ficam no formato padrão que outros programas leem. Quando um arquivo .ods é gravado para um objeto cuja imagem não pode ser desenhada, o objeto é mantido sem imagem.

## Diagramas de outros programas

Um **diagrama** é uma imagem de caixas e setas, como um processo, criada por outro programa. O JeffCrunch mostra um diagrama a partir da imagem dele que o arquivo guarda junto. Clique nele para selecioná-lo e arraste-o para movê-lo. Ao salvar o arquivo, o diagrama é gravado de volta exatamente como era, para que o programa que o criou ainda possa alterá-lo.

O JeffCrunch ainda não consegue alterar o conteúdo de um diagrama, e um diagrama não pode ser copiado nem colocado em um grupo. Se o arquivo não guarda uma imagem de um diagrama, o JeffCrunch mostra uma caixa cinza simples que informa que se trata de um diagrama.

## Gráficos em arquivos .ods

Uma pasta de trabalho salva no formato .ods mantém os seus gráficos: colunas, barras, linhas, áreas, pizzas, roscas, dispersões, radares e gráficos de ações de máxima, mínima e fechamento, ou de abertura, máxima, mínima e fechamento, cada um com suas séries, as células que lê (nesta planilha ou em outra), categorias, título, legenda, eixos (títulos, menor e maior valor, formato de número), rótulos de dados, cores e posição. Um arquivo .ods criado por outro programa abre com os seus gráficos, desenhados a partir das células que eles leem. O que um gráfico .ods não tem onde guardar é deixado de fora e informado ao salvar: linhas de tendência, barras de erro, eixos secundários (assim, um gráfico de ações com volume é salvo com o volume no primeiro eixo e abre como colunas) e as aparências extras de um único ponto. Gráficos de tipos que o JeffCrunch não desenha abrem como colunas com os seus valores.
