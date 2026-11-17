# Configurar as páginas
<!-- keywords: plano de fundo, imagem atrás das células, mosaico, papel de parede, layout da página, margens, orientação, retrato, paisagem, papel, tamanho, área de impressão, quebra de página, quebras, títulos de impressão, repetir linhas, dimensionar para ajustar, ajustar à página, linhas de grade, títulos, cabeçalhos, opções da planilha, da direita para a esquerda, espelhar -->

A guia **Layout da Página** define como uma planilha é dividida em páginas quando é impressa ou convertida em PDF. Cada planilha mantém as suas próprias escolhas, que são salvas na pasta de trabalho. Cada alteração é uma etapa que você pode desfazer.

## Margens, orientação e tamanho

- **Margens** define o espaço em branco ao redor das células. **Padrão**, **Largas** e **Estreitas** são conjuntos prontos. **Margens Personalizadas** abre a caixa de diálogo Configurar Página, onde você pode digitar cada uma e informar a que distância do limite do papel ficam o cabeçalho e o rodapé.
- **Orientação** é **Retrato** (o papel é mais alto que largo) ou **Paisagem** (mais largo que alto).
- **Tamanho** é o papel: A4, Ofício, Carta e outros. **Mais Tamanhos de Papel** lista todos.

## Área de impressão

Por padrão, uma planilha imprime tudo o que tem algum conteúdo: valores, fórmulas, células com borda ou preenchimento visível e qualquer gráfico, imagem ou forma. Para imprimir apenas parte da planilha:

1. Selecione as células que você deseja.
2. Escolha **Área de Impressão > Definir Área de Impressão**.

**Adicionar Seleção à Área de Impressão** coloca outro bloco de células na área de impressão. Cada bloco é impresso em páginas próprias. **Limpar Área de Impressão** volta a imprimir tudo. Linhas ou colunas inseridas ou excluídas dentro da área de impressão a alteram junto com as células.

## Quebras de página

O ponto em que uma página termina e a próxima começa é calculado a partir do papel, das margens e dos tamanhos das suas linhas e colunas: uma linha ou coluna nunca é cortada ao meio. Para terminar uma página antes, selecione a célula abaixo e à direita de onde a quebra deve ficar e escolha **Quebras > Inserir Quebra de Página**. Com uma linha inteira selecionada, a quebra fica acima dela; com uma coluna inteira selecionada, fica à esquerda dela. **Remover Quebra de Página** elimina a quebra acima ou à esquerda da célula selecionada, e **Redefinir Todas as Quebras de Página** elimina todas as quebras que você definiu. As quebras de página definidas são ignoradas enquanto a planilha é reduzida para caber na página.

## A exibição Quebras de Página

**Exibir > Quebras de Página** (ou o botão no canto inferior direito da janela) mostra a planilha dividida nas páginas em que ela será impressa. As células que serão impressas permanecem como estão, o restante fica sombreado, e cada página tem o seu número sobre as suas células. Uma linha tracejada é uma quebra calculada pelo JeffCrunch, e uma linha contínua é uma quebra que você definiu.

- **Mover uma quebra.** Arraste uma linha para outra linha ou coluna. Ela passa a ser uma quebra definida ali, e as páginas seguintes são recalculadas.
- **Remover uma quebra.** Arraste uma linha para fora das células impressas, sobre os números das linhas ou as letras das colunas.
- **Alterar a área de impressão.** Arraste a borda azul das células impressas. Se você não definiu uma área de impressão, as células que são impressas por conta própria se tornam uma área, com essa borda deslocada.

Cada arrasto é uma etapa de Desfazer. Enquanto a planilha é reduzida para caber na página, as quebras são ignoradas, por isso as linhas não podem ser arrastadas. **Normal** volta a mostrar as células como estavam. A exibição em que cada planilha foi deixada é salva com a pasta de trabalho.

## A exibição Layout da Página

**Exibir > Layout da Página** (ou o botão do meio no canto inferior direito da janela) mostra a planilha como as páginas em que ela será impressa, uma abaixo da outra, cada uma como uma folha de papel com as suas margens e um espaço entre as páginas. Réguas correm pela parte superior e pela esquerda. O cabeçalho e o rodapé são desenhados onde serão impressos, e o mesmo vale para as imagens neles contidas.

- **Trabalhe nas células.** Clique em uma célula para selecioná-la, arraste para selecionar várias e use as setas do teclado, a digitação, F2 e o clique duplo, como na exibição Normal. O que você digitar vai para uma faixa sobre a célula, na sua página.
- **Comece pela guia Inserir.** **Inserir > Cabeçalho e Rodapé** mostra as páginas (quando elas não estão visíveis) e abre a parte central do cabeçalho da primeira página, pronta para digitação. Clique em outra parte do cabeçalho ou do rodapé para digitar ali.
- **Digite um cabeçalho ou rodapé.** Mova o ponteiro sobre a margem superior ou inferior de uma página: as partes esquerda, central e direita dela ficam destacadas. Clique em uma delas e digite. Aparece a guia **Cabeçalho e Rodapé**, com botões que inserem o número da página, o número de páginas, a data, a hora, a pasta e o nome do arquivo, o nome da planilha e uma imagem. As opções dessa guia permitem definir a primeira página, páginas ímpares e pares com cabeçalhos e rodapés próprios, e informar se o cabeçalho cresce com as células e se alinha às margens. **Cabeçalho** e **Rodapé** nessa guia são galerias de modelos prontos, como **Página 1**, **Página 1 de ?**, o nome da planilha, o nome do arquivo e a data. Escolher um modelo substitui todo o cabeçalho ou rodapé das páginas que você está editando (o texto que você ainda estava digitando nele não é mantido). **(nenhum)** esvazia o cabeçalho ou rodapé. **Ir para o Cabeçalho** e **Ir para o Rodapé** abrem a parte central dessa margem para digitação. Pressione **Ctrl+Enter** ou clique em qualquer outro lugar para concluir; **Esc** cancela. Cada cabeçalho ou rodapé concluído é uma etapa de Desfazer.
- **Aplique zoom** com o controle deslizante ou mantenha **Ctrl** pressionado e gire a roda do mouse.

Só as páginas visíveis são desenhadas, por isso uma planilha com milhares de páginas abre de imediato. A exibição em que cada planilha foi deixada é salva com a pasta de trabalho.

## Títulos de impressão

**Títulos de Impressão** repete linhas no topo e colunas à esquerda em cada página depois da página em que elas aparecem, de modo que uma linha de título esteja em todas as páginas. Digite as linhas (como **1:3**) e as colunas (como **A:B**) na guia Planilha da caixa de diálogo Configurar Página.

## Dimensionar para ajustar

**Largura** e **Altura** reduzem as células até que a planilha não fique mais larga, ou mais alta, do que esse número de páginas. **Automático** permite que essa direção use quantas páginas forem necessárias. **Escala** imprime em uma porcentagem do tamanho normal, de 10 a 400, e funciona enquanto Largura e Altura estiverem ambas em **Automático**. Se reduzir para caber deixar as letras muito pequenas, a tela de Impressão avisa.

## Opções da planilha

**Mostrar Linhas de Grade** e **Mostrar Títulos** alteram apenas a tela. **Imprimir Linhas de Grade** coloca no papel as linhas entre as células, e **Imprimir Cabeçalhos** coloca no papel os números das linhas e as letras das colunas.

**Planilha da Direita para a Esquerda** faz a planilha ser lida a partir da direita. A coluna A passa para a borda direita, e as colunas são contadas em direção à esquerda. Os números das linhas também passam para a borda direita. Use para idiomas escritos da direita para a esquerda, como árabe e hebraico. Isso altera apenas a aparência da planilha na tela, e a escolha é salva com a planilha, por isso volta quando você abre a pasta de trabalho. Não é uma etapa que você pode desfazer: clique novamente para desativar.

## Plano de fundo

**Plano de Fundo** (no grupo **Configurar Página**) coloca uma imagem atrás das células da planilha. Escolha uma imagem PNG, JPEG, GIF ou BMP. A imagem é repetida como azulejos para preencher a planilha e se move com as células quando você rola. Uma célula com cor de preenchimento oculta a imagem, por isso dê preenchimento às células que você quer ler com facilidade. A imagem aparece apenas na tela: não é impressa e não vai para o PDF.

**Excluir Plano de Fundo** remove a imagem. Ambas são etapas que você pode desfazer. A imagem é salva com a planilha e volta quando você abre a pasta de trabalho. Em uma cópia .ods, ela é salva como imagem de fundo da página, que outros programas podem imprimir. Um arquivo de imagem maior que 8 MB, ou com pontos demais, é recusado, e também é recusada uma imagem que faria todas as imagens da pasta de trabalho (planos de fundo, cabeçalhos e rodapés) somarem mais de 64 MB. Uma imagem com mais de 2.400 pontos em um dos lados ainda é repetida em mosaico no seu tamanho real.

## Mais opções

A pequena seta no canto do grupo **Configurar Página** abre a caixa de diálogo Configurar Página, que reúne todas essas opções e outras mais: [A caixa de diálogo Configurar Página](page-setup).
