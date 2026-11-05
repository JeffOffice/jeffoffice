# A caixa de diálogo Configurar Página

<!-- keywords: configurar página, caixa de diálogo, cabeçalho, rodapé, números de página, data, primeira página, páginas pares e ímpares, preto e branco, rascunho, notas, erros, ordem das páginas, centrar na página, iniciar números de página, códigos de cabeçalho e rodapé, qualidade de impressão, ppp, pontos por polegada -->

Abra-a a partir da seta pequena no canto do grupo **Configurar Página** do separador **Esquema de Página**. Tem quatro separadores. **OK** altera a folha em que está, como um único passo que pode anular.

## Página

- **Orientação do papel**: vertical ou horizontal.
- **Tamanho de impressão das células**: uma percentagem do tamanho real, ou **Reduzir até caber**, com o número de páginas na horizontal e na vertical. "Qualquer número de" permite que essa direção use as páginas de que precisar.
- **Papel**: o tamanho do papel.
- **Qualidade de impressão**: o número de pontos por polegada que é pedido à impressora. Mais pontos dão uma impressão mais fina, mas algumas impressoras demoram mais. Se um livro pedir um número que não está na lista, o JeffCrunch mantém-no.
- **Iniciar os números de página em**: o número que a primeira página recebe. Sem este valor, a primeira página é a 1, e um livro impresso por inteiro numera as páginas de forma contínua, de uma folha para a seguinte.

## Margens

Escreva o espaço em redor das células e a distância a que o cabeçalho e o rodapé ficam das extremidades superior e inferior do papel. As caixas usam centímetros quando o computador está definido para uma região métrica, e polegadas nos restantes casos. **Horizontalmente** e **Verticalmente** colocam as células no centro da página.

## Cabeçalho e Rodapé

Um **cabeçalho** é uma linha de texto no topo de cada página, e um **rodapé** é uma linha na parte inferior. Escolha um da lista ou escolha **Editar cabeçalho** ou **Editar rodapé** para escrever o seu próprio. Cada um tem uma parte esquerda, uma central e uma direita, e botões que inserem o número de página, o número total de páginas, a data, a hora, o nome do ficheiro, a pasta e o nome da folha. Estes valores mudam de página para página, por isso são calculados quando a página é impressa. Para pôr uma imagem numa parte, clique nessa parte e escolha **Imagem…**, depois escolha um ficheiro PNG, JPEG, GIF ou BMP. A imagem fica com no máximo 5 cm no seu lado mais comprido e é impressa onde está o código `&G`. **Remover Imagem** retira-a. Cada parte tem apenas uma imagem, e a primeira página e as páginas pares podem ter, cada uma, imagens próprias.

Também pode definir um cabeçalho e rodapé diferentes para a primeira página e para as páginas ímpares e pares, e escolher se as letras aumentam e diminuem com as células e se o cabeçalho e o rodapé ficam alinhados com as margens.

Por trás dos botões estão códigos curtos que o ficheiro guarda, e pode escrevê-los você mesmo: **&P** é o número de página, **&N** o número total de páginas, **&D** a data, **&T** a hora, **&F** o nome do ficheiro, **&A** o nome da folha, e **&&** é um e comercial. **&B**, **&I** e **&U** ativam e desativam o negrito, o itálico e o sublinhado, **&12** define o tamanho para 12 pontos, e **&"Arial,Bold"** escolhe um tipo de letra. **&L**, **&C** e **&R** iniciam as partes esquerda, central e direita.

## Folha

- **Células a imprimir**: a área de impressão, como blocos do tipo **A1:D20, F1:G9**. Deixe em branco para imprimir tudo.
- **Repetir estas linhas no topo** e **Repetir estas colunas à esquerda**: os títulos de impressão, como **1:3** e **A:B**.
- **Imprimir também**: as linhas entre células, os números de linha e as letras de coluna, apenas a preto e branco (sem cores nem preenchimentos), e um rascunho rápido (sem linhas entre células, e sem imagens, gráficos nem formas).
- **Notas** e **Células que mostram um erro**: o que fazer com elas. A definição para erros altera o que a página mostra: tal como a célula o mostra, nada, dois traços ou #N/D. Notas: **Omitir** não imprime notas. **Listar após a última página** imprime cada nota depois da última página da folha, em páginas próprias que levam os números de página seguintes e o cabeçalho e rodapé da folha, como "Célula A2: o autor: o texto da nota", pela ordem das células. **Mostrar onde estão** fica guardado no ficheiro para outros programas, mas o JeffCrunch ainda não imprime notas nas células.
- **Qual a página seguinte**: para baixo na folha e depois para a direita, ou para a direita e depois para baixo.
