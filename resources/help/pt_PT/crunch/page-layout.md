# Configurar as páginas
<!-- keywords: fundo, imagem atrás das células, mosaico, papel de parede, esquema de página, margens, orientação, vertical, horizontal, papel, tamanho, área de impressão, quebra de página, quebras, títulos de impressão, repetir linhas, títulos, ajustar à página, linhas de grelha, cabeçalhos, opções da folha, da direita para a esquerda, espelhar -->

O separador **Esquema de Página** define a forma como uma folha é dividida em páginas quando é impressa ou convertida em PDF. Cada folha mantém as suas próprias escolhas, que são guardadas no livro. Cada alteração é um passo que pode anular.

## Margens, orientação e tamanho

- **Margens** define o espaço em branco à volta das células. **Normal**, **Largas** e **Estreitas** são conjuntos prontos a usar. **Margens Personalizadas** abre a caixa de diálogo Configurar Página, onde pode escrever cada uma delas e indicar a distância do cabeçalho e do rodapé à margem do papel.
- **Orientação** é **Vertical** (o papel é mais alto do que largo) ou **Horizontal** (mais largo do que alto).
- **Tamanho** é o papel: A4, A5, Ofício, Carta e outros. **Mais Tamanhos de Papel** lista todos.

## Área de impressão

Por predefinição, uma folha imprime tudo o que tem conteúdo: valores, fórmulas, células com limite ou preenchimento visível, e qualquer gráfico, imagem ou forma. Para imprimir apenas parte:

1. Selecione as células que pretende.
2. Escolha **Área de Impressão > Definir Área de Impressão**.

**Adicionar Seleção à Área de Impressão** acrescenta um segundo bloco de células à área de impressão. Cada bloco é impresso em páginas próprias. **Limpar Área de Impressão** volta a imprimir tudo. As linhas ou colunas que inserir ou eliminar dentro da área de impressão alteram-na em conjunto com as células.

## Quebras de página

O local onde uma página termina e a seguinte começa é calculado a partir do papel, das margens e dos tamanhos das linhas e colunas: uma linha ou coluna nunca é cortada ao meio. Para terminar uma página mais cedo, selecione a célula abaixo e à direita de onde a quebra deve ficar e escolha **Quebras > Inserir Quebra de Página**. Com uma linha inteira selecionada, a quebra fica acima dela, e com uma coluna inteira selecionada, fica à sua esquerda. **Remover Quebra de Página** retira a quebra acima ou à esquerda da célula selecionada, e **Repor Todas as Quebras de Página** retira todas as quebras que definiu. As quebras de página que definiu são ignoradas enquanto a folha estiver a ser reduzida para caber.

## A vista Quebras de Página

**Ver > Quebras de Página** (ou o botão no canto inferior direito da janela) mostra a folha dividida nas páginas em que é impressa. As células que são impressas mantêm o seu aspeto, o resto fica sombreado, e cada página tem o seu número sobre as respetivas células. Uma linha tracejada é uma quebra calculada pelo JeffCrunch, e uma linha contínua é uma quebra que definiu.

- **Mover uma quebra.** Arraste uma linha para outra linha ou coluna. Passa a ser uma quebra que definiu nesse local, e as páginas seguintes são novamente divididas.
- **Remover uma quebra.** Arraste uma linha para fora das células impressas, para cima dos números de linha ou das letras de coluna.
- **Alterar a área de impressão.** Arraste a extremidade azul das células impressas. Se não tiver definido uma área de impressão, as células que são impressas por si próprias passam a formar uma, com essa extremidade deslocada.

Cada arrastamento é um passo de Anular. Enquanto a folha estiver a ser reduzida para caber, as suas quebras são ignoradas, pelo que as linhas não podem ser arrastadas. **Normal** repõe as células tal como estavam. A vista em que cada folha foi deixada fica guardada com o livro.

## A vista Esquema de Página

**Ver > Esquema de Página** (ou o botão central no canto inferior direito da janela) mostra a folha como as páginas em que é impressa, uma abaixo da outra, cada uma como uma folha de papel com as respetivas margens e um espaço entre páginas. Há réguas ao longo do topo e à esquerda. O cabeçalho e o rodapé são desenhados onde são impressos, e o mesmo acontece com as imagens que contêm.

- **Trabalhar nas células.** Clique numa célula para a selecionar, arraste para selecionar várias, e use as teclas de seta, a escrita, F2 e duplo clique como faz na vista Normal. O que escrever aparece numa faixa sobre a célula, na respetiva página.
- **Começar pelo separador Inserir.** **Inserir > Cabeçalho e Rodapé** mostra as páginas (se não estiverem visíveis) e abre a parte central do cabeçalho da primeira página, pronta para escrever. Clique noutra parte do cabeçalho ou do rodapé para escrever aí.
- **Escrever um cabeçalho ou rodapé.** Mova o ponteiro sobre a margem superior ou inferior de uma página: as partes esquerda, central e direita acendem-se. Clique numa delas e escreva. Aparece o separador **Cabeçalho e Rodapé**, com botões que inserem o número da página, o número total de páginas, a data, a hora, a pasta e o nome do ficheiro, o nome da folha e uma imagem. As suas opções permitem definir uma primeira página, páginas ímpares e pares com cabeçalhos e rodapés próprios, e indicar se o cabeçalho cresce com as células e se alinha com as margens. **Cabeçalho** e **Rodapé** nesse separador são galerias de formatos prontos, como **Página 1**, **Página 1 de ?**, o nome da folha, o nome do ficheiro e a data. Escolher um deles substitui todo o cabeçalho ou rodapé das páginas em que está a escrever (as palavras que ainda estava a escrever nele não são mantidas). **(nenhum)** esvazia-o. **Ir para o Cabeçalho** e **Ir para o Rodapé** abrem a parte central dessa margem para escrever. Prima **Ctrl+Enter**, ou clique em qualquer outro local, para concluir; **Esc** cancela. Cada cabeçalho ou rodapé concluído é um passo de Anular.
- **Ampliar** com o cursor deslizante, ou mantendo **Ctrl** premido e rodando a roda do rato.

Só são desenhadas as páginas visíveis, pelo que uma folha com milhares de páginas abre de imediato. A vista em que cada folha foi deixada fica guardada com o livro.

## Títulos de impressão

**Títulos de Impressão** repete linhas no topo, e colunas à esquerda, em todas as páginas seguintes àquela em que estão, para que uma linha de cabeçalho apareça em todas as páginas. Escreva as linhas (como **1:3**) e as colunas (como **A:B**) no separador Folha da caixa de diálogo Configurar Página.

## Ajustar tamanho

**Largura** e **Altura** reduzem as células até que a folha não tenha mais do que esse número de páginas de largura ou de altura. **Automático** deixa essa direção ocupar as páginas de que precisar. **Escala** imprime a uma percentagem do tamanho real, de 10 a 400, e funciona enquanto Largura e Altura estiverem ambas em **Automático**. Se a redução para caber tornar as letras muito pequenas, o ecrã de Impressão avisa.

## Opções da folha

**Mostrar Linhas de Grelha** e **Mostrar Cabeçalhos** alteram apenas o ecrã. **Imprimir Linhas de Grelha** coloca no papel as linhas entre células, e **Imprimir Cabeçalhos** coloca no papel os números de linha e as letras de coluna.

**Folha da Direita para a Esquerda** faz com que a folha seja lida a partir da direita. A coluna A passa para a borda direita, e as colunas são contadas para a esquerda. Os números de linha também passam para a borda direita. Use-a para idiomas escritos da direita para a esquerda, como o árabe e o hebraico. Altera apenas o aspeto da folha no ecrã, e a escolha fica guardada com a folha, pelo que volta a aparecer quando abrir o livro. Não é um passo que possa anular: clique novamente para a desativar.

## Fundo

**Fundo** (no grupo **Configurar Página**) coloca uma imagem atrás das células da folha. Escolha uma imagem PNG, JPEG, GIF ou BMP. A imagem é repetida em mosaico para preencher a folha, e desloca-se com as células quando faz deslocamento. Uma célula com cor de preenchimento oculta a imagem, pelo que deve dar preenchimento às células que quer ler com facilidade. A imagem só aparece no ecrã: não é impressa e não entra num PDF.

**Eliminar Fundo** retira a imagem. Ambas as ações são passos que pode anular. A imagem fica guardada com a folha e volta a aparecer quando abrir o livro. Numa cópia .ods, é guardada como imagem de fundo da página, que outros programas poderão imprimir. Um ficheiro de imagem com mais de 8 MB, ou com demasiados pontos, é recusado, e também é recusada uma imagem que faça com que todas as imagens do livro (fundos, cabeçalhos e rodapés) somem, em conjunto, mais de 64 MB. Uma imagem com mais de 2400 pontos num dos lados é, ainda assim, repetida em mosaico à sua dimensão real.

## Mais opções

A pequena seta no canto do grupo **Configurar Página** abre a caixa de diálogo Configurar Página, que tem todas estas opções e mais: [A caixa de diálogo Configurar Página](page-setup).
