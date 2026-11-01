# Configurar as páginas
<!-- keywords: layout da página, configurar página, margens, orientação, retrato, paisagem, papel, tamanho do papel, área de impressão, quebra de página, quebras, imprimir títulos, títulos, repetir linhas, dimensionar para ajustar, ajustar à página, linhas de grade, títulos de linha e coluna, opções de planilha -->

A guia **Layout da Página** define como uma planilha é dividida em páginas quando é impressa ou convertida em PDF. Cada planilha mantém as suas próprias opções, que são salvas na pasta de trabalho. Cada alteração é uma etapa que você pode desfazer.

## Margens, orientação e tamanho

- **Margens** define o espaço em branco ao redor das células. **Padrão**, **Larga** e **Estreita** são conjuntos prontos. **Margens Personalizadas** abre a caixa de diálogo Configurar Página, onde você pode digitar cada margem e informar a distância do cabeçalho e do rodapé até a borda do papel.
- **Orientação** pode ser **Retrato** (o papel é mais alto do que largo) ou **Paisagem** (mais largo do que alto).
- **Tamanho** é o papel: A4, Ofício, Carta e outros. **Mais Tamanhos de Papel** lista todos eles.

## Área de impressão

Por padrão, uma planilha imprime tudo o que tem algum conteúdo: valores, fórmulas, células com borda ou preenchimento visível e qualquer gráfico, imagem ou forma. Para imprimir apenas uma parte:

1. Selecione as células desejadas.
2. Escolha **Área de Impressão > Definir Área de Impressão**.

**Adicionar Seleção à Área de Impressão** coloca um segundo bloco de células na área de impressão. Cada bloco é impresso em páginas próprias. **Limpar Área de Impressão** volta a imprimir tudo. Linhas ou colunas inseridas ou excluídas dentro da área de impressão alteram a área junto com as células.

## Quebras de página

O ponto onde uma página termina e a próxima começa é calculado a partir do papel, das margens e do tamanho das linhas e colunas: uma linha ou coluna nunca é cortada ao meio. Para terminar uma página antes, selecione a célula abaixo e à direita do ponto onde a quebra deve ficar e escolha **Quebras > Inserir Quebra de Página**. Com uma linha inteira selecionada, a quebra fica acima dela; com uma coluna inteira selecionada, fica à esquerda dela. **Remover Quebra de Página** remove a quebra acima ou à esquerda da célula selecionada, e **Redefinir Todas as Quebras de Página** remove todas as quebras que você definiu. As quebras de página que você define são ignoradas enquanto a planilha está sendo reduzida para ajustar.

## O modo de exibição Quebras de Página

**Exibir > Quebras de Página** (ou o botão no canto inferior direito da janela) mostra a planilha dividida nas páginas em que será impressa. As células impressas permanecem como estão, todo o resto fica sombreado, e cada página mostra o seu número sobre as células. Uma linha tracejada é uma quebra calculada pelo JeffCrunch, e uma linha sólida é uma quebra definida por você.

- **Mover uma quebra.** Arraste uma linha para outra linha ou coluna. Ela se torna uma quebra definida por você nesse local, e as páginas seguintes são divididas novamente.
- **Remover uma quebra.** Arraste a linha para fora das células impressas, sobre os números de linha ou as letras de coluna.
- **Alterar a área de impressão.** Arraste a borda azul das células impressas. Se você não tiver definido uma área de impressão, as células impressas por padrão se tornam uma, com essa borda deslocada.

Cada ação de arrastar é uma etapa de Desfazer. Enquanto a planilha está sendo reduzida para ajustar, as quebras são ignoradas, por isso as linhas não podem ser arrastadas. **Normal** volta a mostrar as células como antes. O modo de exibição em que cada planilha ficou é salvo com a pasta de trabalho.

## O modo de exibição Layout da Página

**Exibir > Layout da Página** (ou o botão do meio no canto inferior direito da janela) mostra a planilha como as páginas em que será impressa, uma abaixo da outra, cada uma como uma folha de papel com as suas margens e um espaço entre as páginas. As réguas ficam na parte superior e à esquerda. O cabeçalho e o rodapé aparecem onde serão impressos, assim como as imagens que eles contêm.

- **Trabalhar nas células.** Clique em uma célula para selecioná-la, arraste para selecionar várias e use as teclas de direção, a digitação, F2 e o clique duplo como no modo de exibição Normal. O que você digita aparece em uma faixa sobre a célula, na página dela.
- **Começar pela guia Inserir.** **Inserir > Cabeçalho e Rodapé** mostra as páginas (quando não estão sendo mostradas) e abre a parte central do cabeçalho da primeira página, pronta para digitação. Clique em outra parte do cabeçalho ou do rodapé para digitar nela.
- **Digitar um cabeçalho ou rodapé.** Mova o ponteiro sobre a margem superior ou inferior de uma página: as partes esquerda, central e direita ficam realçadas. Clique em uma delas e digite. A guia **Cabeçalho e Rodapé** aparece, com botões que inserem o número da página, o número de páginas, a data, a hora, a pasta e o nome do arquivo, o nome da planilha e uma imagem; as opções dela permitem que a primeira página e as páginas pares e ímpares tenham cabeçalhos e rodapés próprios, e definem se o cabeçalho é dimensionado com as células e se fica alinhado com as margens. Pressione **Ctrl+Enter**, ou clique em qualquer outro lugar, para concluir; **Esc** descarta as alterações. Cada cabeçalho ou rodapé concluído é uma etapa de Desfazer.
- **Zoom**: use o controle deslizante ou mantenha **Ctrl** pressionada e gire a roda do mouse.

Somente as páginas visíveis são desenhadas, por isso uma planilha com milhares de páginas abre imediatamente. O modo de exibição em que cada planilha ficou é salvo com a pasta de trabalho.

## Imprimir títulos

**Imprimir Títulos** repete linhas na parte superior e colunas à esquerda de todas as páginas posteriores àquela em que estão, para que uma linha de títulos apareça em todas as páginas. Digite as linhas (como **1:3**) e as colunas (como **A:B**) na guia Planilha da caixa de diálogo Configurar Página.

## Dimensionar para ajustar

**Largura** e **Altura** reduzem as células até que a planilha não ultrapasse essa quantidade de páginas na largura ou na altura. **Automático** permite que essa direção ocupe quantas páginas forem necessárias. **Escala** imprime em uma porcentagem do tamanho real, de 10 a 400, e funciona quando Largura e Altura estão ambas em **Automático**. Se a redução para ajustar deixar as letras muito pequenas, a tela Imprimir avisa.

## Opções de planilha

**Mostrar Linhas de Grade** e **Mostrar Títulos de Linha e Coluna** alteram somente a tela. **Imprimir Linhas de Grade** imprime as linhas entre as células, e **Imprimir Títulos de Linha e Coluna** imprime os números de linha e as letras de coluna.

## Mais opções

A pequena seta no canto do grupo **Configurar Página** abre a caixa de diálogo Configurar Página, que tem todas essas opções e outras: [A caixa de diálogo Configurar Página](page-setup).
