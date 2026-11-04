# Configurar as páginas
<!-- keywords: esquema de página, margens, orientação, vertical, horizontal, papel, tamanho, área de impressão, quebra de página, quebras, títulos de impressão, títulos, repetir linhas, ajustar à página, ajustar ao tamanho, linhas de grelha, cabeçalhos, opções de folha, da direita para a esquerda, espelhar, configurar página, margens espelhadas -->

O separador **Esquema de Página** define a forma como uma folha é dividida em páginas quando é impressa ou convertida em PDF. Cada folha mantém as suas próprias escolhas, que ficam guardadas no livro. Cada alteração é um passo que pode ser anulado.

## Margens, orientação e tamanho

- **Margens** define o espaço em branco à volta das células. **Normal**, **Larga** e **Estreita** são conjuntos predefinidos. **Margens Personalizadas** abre a caixa de diálogo Configurar Página, onde pode escrever cada uma delas e indicar a distância do cabeçalho e do rodapé à extremidade do papel.
- **Orientação** é **Vertical** (o papel é mais alto do que largo) ou **Horizontal** (mais largo do que alto).
- **Tamanho** é o papel: A4, A5, Legal e outros. **Mais Tamanhos de Papel** lista todos.

## Área de impressão

Por predefinição, uma folha imprime tudo o que tem conteúdo: valores, fórmulas, células com limite ou preenchimento visível, e qualquer gráfico, imagem ou forma. Para imprimir apenas parte:

1. Selecione as células que pretende.
2. Escolha **Área de Impressão > Definir Área de Impressão**.

**Adicionar Seleção à Área de Impressão** acrescenta um segundo bloco de células à área de impressão. Cada bloco é impresso em páginas próprias. **Limpar Área de Impressão** volta a imprimir tudo. As linhas ou colunas que inserir ou eliminar dentro da área de impressão alteram-na juntamente com as células.

## Quebras de página

O ponto onde uma página termina e a seguinte começa é calculado a partir do papel, das margens e dos tamanhos das suas linhas e colunas: uma linha ou coluna nunca é cortada ao meio. Para terminar uma página mais cedo, selecione a célula abaixo e à direita de onde a quebra deve ficar e escolha **Quebras > Inserir Quebra de Página**. Se selecionar uma linha inteira, a quebra fica acima dela, e se selecionar uma coluna inteira, fica à sua esquerda. **Remover Quebra de Página** elimina a quebra acima ou à esquerda da célula selecionada, e **Repor Todas as Quebras de Página** elimina todas as quebras que definiu. As quebras de página que definiu são ignoradas enquanto a folha estiver a ser reduzida para caber.

## A vista Quebras de Página

**Ver > Quebras de Página** (ou o botão no canto inferior direito da janela) mostra a folha dividida nas páginas em que será impressa. As células que se imprimem mantêm o aspeto, o restante fica sombreado, e cada página tem o seu número por cima das células. Uma linha tracejada é uma quebra que o JeffCrunch calculou, e uma linha contínua é uma quebra que definiu.

- **Mover uma quebra.** Arraste uma linha para outra linha ou coluna. Passa a ser uma quebra definida nesse ponto, e as páginas seguintes são novamente divididas.
- **Remover uma quebra.** Arraste uma linha para fora das células impressas, para os números de linha ou as letras de coluna.
- **Alterar a área de impressão.** Arraste a extremidade azul das células impressas. Se não tiver definido uma área de impressão, as células que se imprimem por si próprias passam a formar uma, com essa extremidade movida.

Cada arrastamento é um passo de Anular. Enquanto a folha estiver a ser reduzida para caber, as suas quebras são ignoradas, pelo que as linhas não podem ser arrastadas. **Normal** repõe as células como estavam. A vista em que cada folha ficou é guardada com o livro.

## A vista Esquema de Página

**Ver > Esquema de Página** (ou o botão central no canto inferior direito da janela) mostra a folha como as páginas em que será impressa, umas por baixo das outras, cada uma como uma folha de papel com as suas margens e um espaço entre páginas. Réguas correm ao longo do topo e pela esquerda. O cabeçalho e o rodapé são desenhados onde serão impressos, e o mesmo acontece com as imagens que neles existam.

- **Trabalhar nas células.** Clique numa célula para a selecionar, arraste para selecionar várias, e use as teclas de seta, a escrita, F2 e duplo clique como na vista Normal. O que escrever aparece numa faixa sobre a célula na sua página.
- **Começar pelo separador Inserir.** **Inserir > Cabeçalho e Rodapé** mostra as páginas (quando não estiverem visíveis) e abre o meio do cabeçalho da primeira página, pronto para escrever. Clique noutra parte do cabeçalho ou do rodapé para escrever aí.
- **Escrever um cabeçalho ou rodapé.** Mova o ponteiro sobre a margem superior ou inferior de uma página: as suas partes esquerda, central e direita ficam realçadas. Clique numa delas e escreva. Aparece o separador **Cabeçalho e Rodapé**, com botões que inserem o número da página, o número total de páginas, a data, a hora, a pasta e o nome do ficheiro, o nome da folha e uma imagem. As suas opções permitem definir uma primeira página, páginas pares e ímpares com cabeçalhos e rodapés próprios, e indicar se o cabeçalho aumenta com as células e se fica alinhado com as margens. **Cabeçalho** e **Rodapé** nesse separador são galerias de formatos prontos, como **Página 1**, **Página 1 de ?**, o nome da folha, o nome do ficheiro e a data. Escolher um deles substitui todo o cabeçalho ou rodapé das páginas em que está a escrever (as palavras que ainda estava a escrever nele não são mantidas). **(nenhum)** esvazia-o. **Ir para o Cabeçalho** e **Ir para o Rodapé** abrem a parte central dessa margem para escrever. Prima **Ctrl+Enter**, ou clique em qualquer outro lugar, para terminar; **Esc** desiste. Cada cabeçalho ou rodapé concluído é um passo de Anular.
- **Aumentar ou reduzir** com o cursor de zoom, ou mantenha **Ctrl** premida e rode a roda do rato.

Só são desenhadas as páginas visíveis, por isso uma folha com milhares de páginas abre de imediato. A vista em que cada folha ficou é guardada com o livro.

## Títulos de impressão

**Títulos de Impressão** repete linhas no topo, e colunas à esquerda, em todas as páginas seguintes à da sua posição, para que uma linha de títulos apareça em todas as páginas. Escreva as linhas (como **1:3**) e as colunas (como **A:B**) no separador Folha da caixa de diálogo Configurar Página.

## Ajustar à página

**Largura** e **Altura** reduzem as células até que a folha não tenha mais do que esse número de páginas de largura ou de altura. **Automático** permite que essa direção ocupe as páginas de que precisar. **Escala** imprime a uma percentagem do tamanho real, entre 10 e 400, e funciona enquanto Largura e Altura estiverem ambas em **Automático**. Se reduzir para caber tornar as letras muito pequenas, o ecrã de Impressão avisa.

## Opções de folha

**Mostrar Linhas de Grelha** e **Mostrar Cabeçalhos** alteram apenas o ecrã. **Imprimir Linhas de Grelha** coloca no papel as linhas entre as células, e **Imprimir Cabeçalhos** coloca no papel os números de linha e as letras de coluna.

**Folha da Direita para a Esquerda** faz com que a folha seja lida a partir da direita. A coluna A passa para a extremidade direita, e as colunas contam para a esquerda. Os números de linha também passam para a extremidade direita. Use esta opção para idiomas escritos da direita para a esquerda, como o árabe e o hebraico. Altera a forma como a folha aparece no ecrã, e a escolha fica guardada com a folha, pelo que volta a estar ativa ao abrir o livro. Não é um passo que possa ser anulado: clique novamente para a desativar.

## Mais opções

A pequena seta no canto do grupo **Configurar Página** abre a caixa de diálogo Configurar Página, que tem todas estas opções e mais: [A caixa de diálogo Configurar Página](page-setup).
