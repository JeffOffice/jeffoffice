# Configurar as páginas
<!-- keywords: esquema de página, configurar página, margens, orientação, vertical, horizontal, retrato, paisagem, papel, tamanho, área de impressão, quebra de página, quebras, imprimir títulos, títulos, repetir linhas, ajustar à página, ajustar tamanho, escala, linhas de grelha, cabeçalhos, opções da folha -->

O separador **Esquema de Página** determina como uma folha é dividida em páginas quando é impressa ou convertida em PDF. Cada folha mantém as suas próprias opções, que são guardadas no livro. Cada alteração é um passo que pode anular.

## Margens, orientação e tamanho

- **Margens** define o espaço em branco à volta das células. **Normal**, **Largas** e **Estreitas** são conjuntos predefinidos. **Margens Personalizadas** abre a caixa de diálogo Configurar Página, onde pode escrever cada uma delas e indicar a distância do cabeçalho e do rodapé ao limite do papel.
- **Orientação** é **Vertical** (o papel é mais alto do que largo) ou **Horizontal** (mais largo do que alto).
- **Tamanho** é o papel: Letter, Legal, A4 e outros. **Mais Tamanhos de Papel** apresenta a lista completa.

## Área de impressão

Por predefinição, uma folha imprime tudo o que tenha conteúdo: valores, fórmulas, células com um limite ou preenchimento visível, e qualquer gráfico, imagem ou forma. Para imprimir apenas uma parte:

1. Selecione as células pretendidas.
2. Escolha **Área de Impressão > Definir Área de Impressão**.

**Adicionar Seleção à Área de Impressão** coloca um segundo bloco de células na área de impressão. Cada bloco é impresso em páginas próprias. **Limpar Área de Impressão** volta a imprimir tudo. As linhas ou colunas que inserir ou eliminar dentro da área de impressão alteram-na juntamente com as células.

## Quebras de página

O ponto onde uma página termina e a seguinte começa é calculado a partir do papel, das margens e do tamanho das linhas e colunas: uma linha ou coluna nunca é cortada ao meio. Para terminar uma página mais cedo, selecione a célula abaixo e à direita do local onde a quebra deve ficar e escolha **Quebras > Inserir Quebra de Página**. Com uma linha inteira selecionada, a quebra fica acima dela; com uma coluna inteira selecionada, fica à sua esquerda. **Remover Quebra de Página** remove a quebra acima ou à esquerda da célula selecionada, e **Repor Todas as Quebras de Página** remove todas as quebras que definiu. As quebras de página que definiu são ignoradas enquanto a folha estiver a ser reduzida para caber.

## A vista Quebras de Página

**Ver > Quebras de Página** (ou o botão no canto inferior direito da janela) mostra a folha dividida nas páginas em que é impressa. As células impressas mantêm-se como estão, o resto fica sombreado e cada página tem o respetivo número sobre as suas células. Uma linha tracejada é uma quebra calculada pelo JeffCrunch e uma linha sólida é uma quebra que definiu.

- **Mover uma quebra.** Arraste uma linha para outra linha ou coluna. Passa a ser uma quebra definida por si nesse local e as páginas seguintes são divididas novamente.
- **Remover uma quebra.** Arraste uma linha para fora das células impressas, para os números de linha ou as letras de coluna.
- **Alterar a área de impressão.** Arraste o limite azul das células impressas. Se não tiver definido uma área de impressão, as células que são impressas por si só passam a sê-lo, com esse limite deslocado.

Cada arrastamento é um passo de Anular. Enquanto uma folha estiver a ser reduzida para caber, as quebras são ignoradas, pelo que as linhas não podem ser arrastadas. **Normal** repõe as células como estavam. A vista em que cada folha foi deixada é guardada com o livro.

## A vista Esquema de Página

**Ver > Esquema de Página** (ou o botão do meio no canto inferior direito da janela) mostra a folha como as páginas em que é impressa, umas abaixo das outras, cada uma como uma folha de papel com as respetivas margens e um espaço entre páginas. Há réguas ao longo do topo e do lado esquerdo. O cabeçalho e o rodapé são desenhados onde são impressos, tal como as imagens que contêm.

- **Trabalhar nas células.** Clique numa célula para a escolher, arraste para escolher várias e utilize as teclas de seta, a escrita, F2 e o duplo clique tal como na vista Normal. O que escreve aparece numa faixa sobre a célula, na respetiva página.
- **Escrever um cabeçalho ou rodapé.** Mova o ponteiro sobre a margem superior ou inferior de uma página: as partes esquerda, central e direita ficam realçadas. Clique numa delas e escreva. Aparece o separador **Cabeçalho e Rodapé**, com botões que inserem o número da página, o número total de páginas, a data, a hora, a pasta e o nome do ficheiro, o nome da folha e uma imagem; as respetivas opções dão à primeira página, e às páginas pares e ímpares, cabeçalhos e rodapés próprios, e indicam se o cabeçalho é dimensionado com as células e se fica alinhado com as margens. Prima **Ctrl+Enter**, ou clique noutro local, para terminar; **Esc** cancela. Cada cabeçalho ou rodapé concluído é um passo de Anular.
- **Zoom** com o controlo de deslize, ou mantenha premida a tecla **Ctrl** e rode a roda do rato.

Apenas as páginas visíveis são desenhadas, pelo que uma folha com milhares de páginas abre de imediato. A vista em que cada folha foi deixada é guardada com o livro.

## Imprimir títulos

**Imprimir Títulos** repete linhas no topo e colunas à esquerda de todas as páginas após aquela em que se encontram, para que uma linha de cabeçalho apareça em todas as páginas. Escreva as linhas (como **1:3**) e as colunas (como **A:B**) no separador Folha da caixa de diálogo Configurar Página.

## Ajustar à página

**Largura** e **Altura** reduzem as células até a folha não ter mais páginas de largura, ou de altura, do que esse número. **Automático** permite que essa direção ocupe as páginas de que precisar. **Escala** imprime numa percentagem do tamanho real, de 10 a 400, e funciona quando Largura e Altura estão ambas em **Automático**. Se a redução para caber tornar as letras muito pequenas, o ecrã Imprimir avisa.

## Opções da folha

**Mostrar Linhas de Grelha** e **Mostrar Cabeçalhos** alteram apenas o ecrã. **Imprimir Linhas de Grelha** imprime as linhas entre células e **Imprimir Cabeçalhos** imprime os números de linha e as letras de coluna.

## Mais opções

A pequena seta no canto do grupo **Configurar Página** abre a caixa de diálogo Configurar Página, que tem todas estas opções e mais algumas: [A caixa de diálogo Configurar Página](page-setup).
