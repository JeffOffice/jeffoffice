# Configurar as páginas
<!-- keywords: tema, temas, cores do tema, fontes do tema, cores, fontes, ênfase, fonte de título, fonte do corpo, plano de fundo, imagem atrás das células, ladrilhar, papel de parede, layout da página, margens, orientação, retrato, paisagem, papel, tamanho, área de impressão, quebra de página, quebras, títulos de impressão, repetir linhas, dimensionar para ajustar, ajustar à página, linhas de grade, títulos de linha e coluna, opções da planilha, da direita para a esquerda, espelhar, configurar página, layout da página -->

A guia **Layout da Página** define como uma planilha é dividida em páginas quando é impressa ou convertida em PDF. Cada planilha mantém as próprias escolhas, que são salvas na pasta de trabalho. Cada alteração é uma etapa que você pode desfazer.

## Temas, cores e fontes

Uma pasta de trabalho tem um **tema**: doze cores e duas fontes (uma para títulos e outra para o texto do corpo) que suas células, estilos de célula, estilos de tabela, formas e gráficos podem usar em vez de copiá-las. Quando você altera o tema, tudo o que se refere a ele muda junto: uma célula preenchida com "Ênfase 1, mais clara 40%" passa a usar a nova Ênfase 1, mais clara na mesma proporção.

- **Temas** lista temas prontos que definem cores e fontes juntas. **Procurar Temas** obtém o tema de um arquivo de tema, de uma pasta de trabalho, de um documento ou de uma apresentação. **Salvar Tema Atual** guarda o tema desta pasta de trabalho em um arquivo, e **Redefinir para o Tema da Pasta de Trabalho** restaura o tema que a pasta tinha quando você a abriu.
- **Cores** altera apenas as doze cores. **Criar Novas Cores do Tema** permite definir e nomear suas próprias cores; todos os aplicativos do JeffOffice passam a listá-las.
- **Fontes** altera apenas as fontes de título e do corpo. As células que usam as fontes do tema acompanham a mudança; as células com uma fonte escolhida pelo nome a mantêm.

Cada escolha é uma etapa que você pode desfazer. Os botões de cor (Cor de Preenchimento, Cor da Fonte, Bordas e Cor da Guia) mostram as cores do tema em uma linha, com versões mais claras e mais escuras abaixo de cada uma. Uma cor escolhida ali continua sendo uma cor do tema e muda com ele. Uma cor de **Cores Padrão** ou de **Mais Cores** permanece como você a escolheu.

Uma nova pasta de trabalho começa com o tema próprio do JeffOffice. Uma forma ou gráfico criado aqui mantém as cores que recebeu; um que veio no arquivo e usa uma cor do tema acompanha o tema. Uma pasta de trabalho aberta a partir de um arquivo binário antigo ou de um arquivo OpenDocument não tem tema para alterar, por isso esses botões não oferecem nada para ela. Quando você salva uma pasta de trabalho como arquivo OpenDocument, que não tem temas, as cores são gravadas como aparecem.

## Margens, orientação e tamanho

- **Margens** define o espaço em branco ao redor das células. **Padrão**, **Largas** e **Estreitas** são conjuntos prontos. **Margens Personalizadas** abre a caixa de diálogo Configurar Página, em que você pode digitar cada uma delas e indicar a distância do cabeçalho e do rodapé até a borda do papel.
- **Orientação** é **Retrato** (o papel é mais alto do que largo) ou **Paisagem** (mais largo do que alto).
- **Tamanho** é o papel: A4, Ofício, Carta e outros. **Mais Tamanhos de Papel** lista todos eles.

## Área de impressão

Por padrão, uma planilha imprime tudo o que tem algum conteúdo: valores, fórmulas, células com borda ou preenchimento visível, além de qualquer gráfico, imagem ou forma. Para imprimir apenas uma parte:

1. Selecione as células que deseja.
2. Escolha **Área de Impressão > Definir Área de Impressão**.

**Adicionar Seleção à Área de Impressão** inclui um segundo bloco de células na área de impressão. Cada bloco é impresso em páginas próprias. **Limpar Área de Impressão** volta a imprimir tudo. Linhas ou colunas inseridas ou excluídas dentro da área de impressão a alteram junto com as células.

## Quebras de página

O ponto em que uma página termina e a próxima começa é calculado a partir do papel, das margens e do tamanho das suas linhas e colunas: uma linha ou coluna nunca é cortada ao meio. Para terminar uma página antes, selecione a célula abaixo e à direita de onde a quebra deve ficar e escolha **Quebras > Inserir Quebra de Página**. Com uma linha inteira selecionada, a quebra fica acima dela; com uma coluna inteira selecionada, fica à esquerda dela. **Remover Quebra de Página** elimina a quebra acima ou à esquerda da célula selecionada, e **Redefinir Todas as Quebras de Página** elimina todas as quebras que você definiu. As quebras de página definidas por você são ignoradas enquanto a planilha estiver sendo reduzida para caber na página.

## A exibição Quebras de Página

**Exibir > Quebras de Página** (ou o botão no canto inferior direito da janela) mostra a planilha dividida nas páginas em que ela será impressa. As células que serão impressas continuam como estão, o restante fica sombreado, e cada página tem seu número sobre as células. Uma linha tracejada é uma quebra calculada pelo JeffCrunch, e uma linha contínua é uma quebra definida por você.

- **Mover uma quebra.** Arraste uma linha para outra linha ou coluna. Ela passa a ser uma quebra definida ali, e as páginas seguintes são recalculadas.
- **Remover uma quebra.** Arraste uma linha para fora das células impressas, até os números das linhas ou as letras das colunas.
- **Alterar a área de impressão.** Arraste a borda azul das células impressas. Se você não definiu uma área de impressão, as células que são impressas por conta própria passam a formar uma, com essa borda deslocada.

Cada arrastar é uma etapa do Desfazer. Enquanto a planilha estiver sendo reduzida para caber na página, as quebras são ignoradas e as linhas não podem ser arrastadas. **Normal** restaura as células como estavam. A exibição em que cada planilha foi deixada é salva com a pasta de trabalho.

## A exibição Layout da Página

**Exibir > Layout da Página** (ou o botão do meio no canto inferior direito da janela) mostra a planilha como as páginas em que será impressa, uma abaixo da outra, cada uma como uma folha de papel com suas margens e um espaço entre as páginas. Réguas correm pela parte superior e pela esquerda. O cabeçalho e o rodapé são desenhados onde serão impressos, assim como as imagens neles.

- **Trabalhe nas células.** Clique em uma célula para selecioná-la, arraste para selecionar várias e use as setas, a digitação, F2 e o clique duplo como faria na exibição Normal. O que você digitar aparece em uma faixa sobre a célula, na página dela.
- **Comece pela guia Inserir.** **Inserir > Cabeçalho e Rodapé** mostra as páginas (quando não estiverem exibidas) e abre a parte central do cabeçalho da primeira página, pronta para digitação. Clique em outra parte do cabeçalho ou do rodapé para digitar ali.
- **Digite um cabeçalho ou rodapé.** Mova o ponteiro sobre a margem superior ou inferior de uma página: as partes esquerda, central e direita dela ficam destacadas. Clique em uma delas e digite. A guia **Cabeçalho e Rodapé** aparece, com botões que inserem o número da página, o total de páginas, a data, a hora, a pasta e o nome do arquivo, o nome da planilha e uma imagem; suas opções definem uma primeira página, páginas pares e ímpares com cabeçalhos e rodapés próprios, e indicam se o cabeçalho cresce com as células e se alinha às margens. **Cabeçalho** e **Rodapé** nessa guia são galerias de modelos prontos, como Página 1, Página 1 de ?, o nome da planilha, o nome do arquivo e a data. Escolher um deles substitui todo o cabeçalho ou rodapé das páginas em que você está digitando (as palavras que você ainda estava digitando nele não são mantidas). **(nenhum)** o esvazia. **Ir para o Cabeçalho** e **Ir para o Rodapé** abrem a parte central dessa margem para digitação. Pressione **Ctrl+Enter** ou clique em qualquer outro lugar para concluir; **Esc** descarta a alteração. Cada cabeçalho ou rodapé concluído é uma etapa do Desfazer.
- **Amplie ou reduza** com o controle deslizante ou mantendo **Ctrl** pressionado enquanto gira a roda do mouse.

Somente as páginas visíveis são desenhadas, por isso uma planilha com milhares de páginas abre de uma vez. A exibição em que cada planilha foi deixada é salva com a pasta de trabalho.

## Títulos de impressão

**Títulos de Impressão** repete linhas no topo e colunas à esquerda em todas as páginas depois daquela em que elas aparecem, de modo que uma linha de cabeçalho esteja em todas as páginas. Digite as linhas (como **1:3**) e as colunas (como **A:B**) na guia Planilha da caixa de diálogo Configurar Página.

## Dimensionar para ajustar

**Largura** e **Altura** reduzem as células até que a planilha não tenha mais do que esse número de páginas de largura ou de altura. **Automática** permite que essa direção use quantas páginas forem necessárias. **Escala** imprime em uma porcentagem do tamanho normal, de 10 a 400, e funciona enquanto Largura e Altura estiverem ambas em Automática. Se a redução para caber deixar as letras muito pequenas, a tela de Impressão avisa.

## Opções da planilha

**Mostrar Linhas de Grade** e **Mostrar Títulos de Linha e Coluna** alteram apenas a tela. **Imprimir Linhas de Grade** coloca no papel as linhas entre as células, e **Imprimir Títulos de Linha e Coluna** coloca no papel os números das linhas e as letras das colunas.

**Planilha da Direita para a Esquerda** faz a planilha ser lida a partir da direita. A coluna A vai para a borda direita, e as colunas são contadas para a esquerda. Os números das linhas também passam para a borda direita. Use essa opção para idiomas escritos da direita para a esquerda, como árabe e hebraico. Ela altera apenas a aparência da planilha na tela, e a escolha é salva com a planilha, de modo que volta quando você abre a pasta de trabalho. Não é uma etapa que pode ser desfeita: clique de novo para desativá-la.

## Plano de fundo

**Plano de Fundo** (no grupo **Configurar Página**) coloca uma imagem atrás das células da planilha. Escolha uma imagem PNG, JPEG, GIF ou BMP. A imagem é repetida como ladrilhos para preencher a planilha e se move com as células quando você rola a tela. Uma célula com cor de preenchimento oculta a imagem, por isso dê preenchimento às células que você quer ler com facilidade. A imagem aparece somente na tela: não é impressa e não entra em um PDF.

**Excluir Plano de Fundo** remove a imagem. Ambas as ações podem ser desfeitas. A imagem é salva com a planilha e volta quando você abre a pasta de trabalho. Em uma cópia .ods, ela é salva como a imagem de fundo da página, que outros programas podem imprimir. Um arquivo de imagem maior que 8 MB, ou com pontos demais, é recusado, e também é recusada uma imagem que faria todas as imagens da pasta de trabalho (planos de fundo, cabeçalhos e rodapés) ultrapassarem 64 MB juntas. Uma imagem com mais de 2.400 pontos de lado ainda é repetida em seu tamanho real.

## Mais opções

A pequena seta no canto do grupo **Configurar Página** abre a caixa de diálogo Configurar Página, que reúne todas essas opções e outras mais: [A caixa de diálogo Configurar Página](page-setup).
