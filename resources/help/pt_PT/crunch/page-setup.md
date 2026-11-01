# A caixa de diálogo Configurar Página
<!-- keywords: configurar página, configuração de página, caixa de diálogo, cabeçalho, rodapé, números de página, numeração, data, primeira página, páginas pares e ímpares, preto e branco, rascunho, notas, erros, ordem das páginas, centrar na página, número da primeira página, códigos de cabeçalho e rodapé, imprimir -->

Abra-a a partir da pequena seta no canto do grupo **Configurar Página** do separador **Esquema de Página**. Tem quatro separadores. **OK** altera a folha em que se encontra, num único passo que pode anular.

## Página

- **Orientação do papel**: vertical ou horizontal.
- **Tamanho de impressão das células**: uma percentagem do tamanho real, ou **Reduzir até caber**, com o número de páginas na horizontal e na vertical. "Qualquer número de" permite que essa direção ocupe as páginas de que precisar.
- **Papel**: o tamanho do papel.
- **Iniciar os números de página em**: o número atribuído à primeira página. Sem este valor, a primeira página é a 1 e, quando imprime o livro inteiro, a numeração das páginas continua de uma folha para a seguinte.

## Margens

Escreva o espaço à volta das células e a distância a que o cabeçalho e o rodapé ficam dos limites superior e inferior do papel. As caixas utilizam centímetros se o computador estiver configurado para uma região métrica, e polegadas nos restantes casos. **Horizontalmente** e **Verticalmente** colocam as células no meio da página.

## Cabeçalho e Rodapé

Um **cabeçalho** é uma linha de texto no topo de cada página e um **rodapé** é uma linha no fundo. Escolha um da lista ou escolha **Editar cabeçalho** ou **Editar rodapé** para escrever o seu. Cada um tem uma parte esquerda, uma central e uma direita, e botões que inserem o número da página, o número total de páginas, a data, a hora, o nome do ficheiro, a pasta e o nome da folha. Estes valores mudam de página para página, pelo que são calculados no momento da impressão. Para inserir uma imagem numa parte, clique nessa parte e escolha **Imagem…** e, em seguida, escolha um ficheiro PNG, JPEG, GIF ou BMP. A imagem fica com, no máximo, 5 cm no lado maior e é impressa no local onde se encontra o código `&G`. **Remover Imagem** retira-a. Cada parte pode conter apenas uma imagem, e a primeira página e as páginas pares podem ter imagens próprias.

Também pode definir um cabeçalho e um rodapé diferentes para a primeira página e para as páginas ímpares e pares, e escolher se as letras aumentam e diminuem com as células e se o cabeçalho e o rodapé ficam alinhados com as margens.

Por trás dos botões estão códigos curtos que o ficheiro guarda, e pode escrevê-los diretamente: **&P** é o número da página, **&N** o número total de páginas, **&D** a data, **&T** a hora, **&F** o nome do ficheiro, **&A** o nome da folha e **&&** é um "e" comercial. **&B**, **&I** e **&U** ativam e desativam o negrito, o itálico e o sublinhado, **&12** define o tamanho como 12 pontos e **&"Arial,Bold"** escolhe um tipo de letra. **&L**, **&C** e **&R** iniciam as partes esquerda, central e direita.

## Folha

- **Células a imprimir**: a área de impressão, em blocos como **A1:D20, F1:G9**. Deixe em branco para imprimir tudo.
- **Repetir estas linhas no topo** e **Repetir estas colunas à esquerda**: os títulos de impressão, como **1:3** e **A:B**.
- **Imprimir também**: as linhas entre células, os números de linha e as letras de coluna, apenas a preto e branco (sem cores nem preenchimentos) e um rascunho rápido (sem linhas entre células e sem imagens, gráficos ou formas).
- **Notas** e **Células que mostram um erro**: o que fazer com elas. A definição para erros altera o que a página mostra: tal como a célula o apresenta, nada, dois traços ou #N/D. Notas: **Omitir** não imprime notas. **Listar após a última página** imprime cada nota depois da última página da folha, em páginas próprias que recebem os números de página seguintes e o cabeçalho e o rodapé da folha, no formato "Célula A2: o autor: o texto da nota", pela ordem das células. **Mostrar onde se encontram** é mantido no ficheiro para outros programas, mas o JeffCrunch ainda não imprime as notas sobre as células.
- **Qual a página seguinte**: para baixo na folha e depois para o lado, ou para o lado e depois para baixo.
