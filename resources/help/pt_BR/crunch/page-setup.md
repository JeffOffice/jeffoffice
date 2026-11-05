# A caixa de diálogo Configurar Página

<!-- keywords: configurar página, caixa de diálogo, cabeçalho, rodapé, números de página, data, primeira página, páginas pares e ímpares, preto e branco, rascunho, anotações, erros, ordem das páginas, centralizar na página, iniciar numeração de páginas, códigos de cabeçalho e rodapé, qualidade de impressão, dpi, pontos por polegada -->

Abra-a pela pequena seta no canto do grupo **Configurar Página** na guia **Layout da Página**. Ela tem quatro guias. **OK** altera a planilha em que você está, como uma única etapa que você pode desfazer.

## Página

- **Orientação do papel**: em retrato ou em paisagem.
- **Como o tamanho das células será impresso**: uma porcentagem do tamanho real, ou **Reduzir até caber**, com o número de páginas na largura e na altura. "Qualquer número de" permite que essa direção use quantas páginas forem necessárias.
- **Papel**: o tamanho do papel.
- **Qualidade de impressão**: quantos pontos por polegada são solicitados à impressora. Mais pontos deixam a impressão mais fina, e algumas impressoras demoram mais. Se uma pasta de trabalho pedir um número que não está na lista, o JeffCrunch o mantém.
- **Iniciar a numeração de páginas em**: o número que recebe a primeira página. Sem essa opção, a primeira página é a 1, e uma pasta de trabalho impressa por inteiro numera as páginas em sequência de uma planilha para a seguinte.

## Margens

Digite o espaço ao redor das células e a distância do cabeçalho e do rodapé até as bordas superior e inferior do papel. As caixas usam centímetros quando o computador está configurado para uma região que usa o sistema métrico, e polegadas nos demais casos. **Horizontalmente** e **Verticalmente** centralizam as células na página.

## Cabeçalho e Rodapé

Um **cabeçalho** é uma linha de texto no topo de cada página, e um **rodapé** é uma linha na parte inferior. Escolha um na lista ou selecione **Editar cabeçalho** ou **Editar rodapé** para escrever o seu próprio. Cada um tem uma parte esquerda, uma central e uma direita, além de botões que inserem o número da página, o número de páginas, a data, a hora, o nome do arquivo, a pasta e o nome da planilha. Esses itens mudam de uma página para outra, por isso são calculados no momento em que a página é impressa. Para colocar uma imagem em uma parte, clique nela e escolha **Imagem…**, depois selecione um arquivo PNG, JPEG, GIF ou BMP. A imagem é reduzida a no máximo 5 cm no seu lado mais longo, e é impressa onde está o código `&G`. **Remover Imagem** a retira. Cada parte comporta uma imagem, e a primeira página e as páginas pares podem ter imagens próprias.

Você também pode definir cabeçalho e rodapé diferentes para a primeira página e para as páginas pares e ímpares, e escolher se as letras aumentam e diminuem com as células e se o cabeçalho e o rodapé se alinham às margens.

Por trás dos botões há códigos curtos que o arquivo guarda, e você pode digitá-los diretamente: **&P** é o número da página, **&N** o número de páginas, **&D** a data, **&T** a hora, **&F** o nome do arquivo, **&A** o nome da planilha, e **&&** é um e comercial. **&B**, **&I** e **&U** ativam e desativam negrito, itálico e sublinhado, **&12** define o tamanho como 12 pontos, e **&"Arial,Negrito"** escolhe uma fonte. **&L**, **&C** e **&R** iniciam as partes esquerda, central e direita.

## Planilha

- **Células a imprimir**: a área de impressão, como intervalos do tipo **A1:D20, F1:G9**. Deixe em branco para imprimir tudo.
- **Linhas a repetir na parte superior** e **Colunas a repetir à esquerda**: os títulos de impressão, como **1:3** e **A:B**.
- **Imprimir também**: as linhas entre as células, os números das linhas e as letras das colunas, somente em preto e branco (sem cores nem preenchimentos), e um rascunho rápido (sem linhas entre as células e sem imagens, gráficos ou formas).
- **Anotações** e **Células que mostram erro**: o que fazer com eles. A configuração para erros altera o que a página mostra: como a célula o exibe, nada, dois traços ou #N/D. Anotações: **Omiti-las** não imprime anotações. **Listá-las após a última página** imprime cada anotação depois da última página da planilha, em páginas próprias que levam os números de página seguintes e o cabeçalho e rodapé da planilha, no formato "Célula A2: o autor: o texto da anotação", na ordem das células. **Mostrá-las onde estão** é guardada no arquivo para outros programas, mas o JeffCrunch ainda não imprime anotações nas células.
- **Qual página vem em seguida**: de cima para baixo e depois da esquerda para a direita, ou da esquerda para a direita e depois de cima para baixo.
