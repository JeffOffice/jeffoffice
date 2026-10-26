# A caixa de diálogo Configurar Página
<!-- keywords: configurar página, configuração de página, caixa de diálogo, cabeçalho, rodapé, números de página, numeração de páginas, data, primeira página, páginas pares e ímpares, preto e branco, rascunho, anotações, erros, ordem das páginas, centralizar na página, iniciar numeração de páginas, códigos de cabeçalho e rodapé -->

Abra-a pela pequena seta no canto do grupo **Configurar Página** da guia **Layout da Página**. Ela tem quatro guias. **OK** altera a planilha em que você está, como uma única etapa que pode ser desfeita.

## Página

- **Orientação do papel**: retrato ou paisagem.
- **Tamanho de impressão das células**: uma porcentagem do tamanho real, ou **Reduzir até caber**, com quantas páginas de largura e de altura. "Qualquer número de" permite que essa direção use quantas páginas forem necessárias.
- **Papel**: o tamanho do papel.
- **Iniciar a numeração de páginas em**: o número que a primeira página recebe. Sem ele, a primeira página é 1, e uma pasta de trabalho impressa inteira continua a numeração das páginas de uma planilha para a seguinte.

## Margens

Digite o espaço ao redor das células e a distância do cabeçalho e do rodapé até as bordas superior e inferior do papel. As caixas usam centímetros quando o computador está configurado para uma região que usa o sistema métrico, e polegadas nas demais. **Horizontalmente** e **Verticalmente** colocam as células no meio da página.

## Cabeçalho e Rodapé

Um **cabeçalho** é uma linha de texto na parte superior de todas as páginas, e um **rodapé** é uma linha na parte inferior. Escolha um na lista ou escolha **Editar cabeçalho** ou **Editar rodapé** para escrever o seu. Cada um tem uma parte esquerda, uma central e uma direita, e botões que inserem o número da página, o número de páginas, a data, a hora, o nome do arquivo, a pasta e o nome da planilha. Esses itens mudam de uma página para outra, por isso são calculados quando a página é impressa. Para colocar uma imagem em uma parte, clique nessa parte e escolha **Imagem…**, depois escolha um arquivo PNG, JPEG, GIF ou BMP. A imagem fica com no máximo 5 cm no lado maior e é impressa onde está o código `&G`. **Remover Imagem** a retira. Cada parte comporta uma imagem, e a primeira página e as páginas pares podem ter imagens próprias.

Você também pode definir um cabeçalho e um rodapé diferentes para a primeira página e para as páginas pares e ímpares, e escolher se as letras aumentam e diminuem com as células e se o cabeçalho e o rodapé ficam alinhados com as margens.

Por trás dos botões há códigos curtos que o arquivo guarda, e você mesmo pode digitá-los: **&P** é o número da página, **&N** o número de páginas, **&D** a data, **&T** a hora, **&F** o nome do arquivo, **&A** o nome da planilha e **&&** é um E comercial. **&B**, **&I** e **&U** ativam e desativam negrito, itálico e sublinhado, **&12** define o tamanho como 12 pontos e **&"Arial,Bold"** escolhe uma fonte. **&L**, **&C** e **&R** iniciam as partes esquerda, central e direita.

## Planilha

- **Células a imprimir**: a área de impressão, como intervalos do tipo **A1:D20, F1:G9**. Deixe em branco para imprimir tudo.
- **Repetir estas linhas na parte superior** e **Repetir estas colunas à esquerda**: os títulos de impressão, como **1:3** e **A:B**.
- **Imprimir também**: as linhas entre as células, os números de linha e as letras de coluna, somente preto e branco (sem cores nem preenchimentos) e um rascunho rápido (sem linhas entre as células e sem imagens, gráficos ou formas).
- **Anotações** e **Células que mostram um erro**: o que fazer com elas. A configuração de erros altera o que a página mostra: como a célula mostra, nada, dois traços ou #N/D. As anotações são mantidas no arquivo para outros programas, mas o JeffCrunch ainda não as lê nem as imprime.
- **Qual página vem em seguida**: primeiro para baixo na planilha e depois para o lado, ou primeiro para o lado e depois para baixo.
