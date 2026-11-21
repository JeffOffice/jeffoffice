# Configurar as páginas
<!-- keywords: tema, temas, cores do tema, tipos de letra do tema, cores, tipos de letra, destaque, tipo de letra de título, tipo de letra do corpo, fundo, imagem atrás das células, mosaico, papel de parede, esquema de página, margens, orientação, vertical, horizontal, papel, tamanho, área de impressão, quebra de página, quebras, títulos de impressão, repetir linhas, ajustar à página, linhas de grelha, cabeçalhos, opções da folha, da direita para a esquerda, espelho -->

O separador **Esquema de Página** decide como uma folha é dividida em páginas quando é impressa ou convertida em PDF. Cada folha mantém as suas próprias escolhas, guardadas no livro. Cada alteração é um passo que pode anular.

## Temas, cores e tipos de letra

Um livro tem um **tema**: doze cores e dois tipos de letra (um para títulos e outro para o texto normal), que as suas células, estilos de célula, estilos de tabela, formas e gráficos podem usar em vez de copiar valores. Quando altera o tema, tudo o que o utiliza muda com ele: uma célula preenchida com "Destaque 1, mais claro 40%" passa a usar o novo Destaque 1, mais claro na mesma proporção.

- **Temas** lista temas prontos que definem cores e tipos de letra em conjunto. **Procurar Temas** obtém o tema a partir de um ficheiro de tema, de um livro, de um documento ou de uma apresentação. **Guardar Tema Atual** guarda o tema deste livro num ficheiro, e **Repor para o Tema do Livro de Cálculo** devolve o tema que o livro tinha quando o abriu.
- **Cores** altera apenas as doze cores. **Criar Novas Cores do Tema** permite definir as suas próprias cores e dar-lhes um nome; todas as aplicações do JeffOffice passam então a listá-las.
- **Tipos de Letra** altera apenas os tipos de letra de título e do texto normal. As células que usam os tipos de letra do tema acompanham a alteração; as células com um tipo de letra escolhido pelo nome mantêm-no.

Cada escolha é um passo que pode anular. Os botões de cor (Cor de Preenchimento, Cor do Tipo de Letra, Limites e Cor do Separador) mostram as cores do tema numa linha, com versões mais claras e mais escuras por baixo; uma cor escolhida aí continua a ser uma cor do tema, pelo que muda com o tema. Uma cor de **Cores Padrão** ou de **Mais Cores** mantém-se tal como a escolheu.

Um novo livro começa com o tema próprio do JeffOffice. Uma forma ou um gráfico que cria aqui mantém as cores que lhe foram atribuídas; um que veio no ficheiro e usa uma cor do tema acompanha o tema. Um livro aberto a partir de um ficheiro binário antigo ou de um ficheiro OpenDocument não tem tema a alterar, pelo que estes botões não oferecem nada para ele. Quando guarda um livro como ficheiro OpenDocument, que não tem temas, as cores são gravadas tal como aparecem.

## Margens, orientação e tamanho

- **Margens** define o espaço em branco à volta das células. **Normal**, **Larga** e **Estreita** são conjuntos predefinidos. **Margens Personalizadas** abre a caixa de diálogo Configurar Página, onde pode escrever cada uma delas e indicar a distância do cabeçalho e do rodapé à extremidade do papel.
- **Orientação** é **Vertical** (o papel é mais alto do que largo) ou **Horizontal** (mais largo do que alto).
- **Tamanho** é o papel: A4, A5, Carta, Ofício e outros. **Mais Tamanhos de Papel** lista-os todos.

## Área de impressão

Por predefinição, uma folha imprime tudo o que tem conteúdo: valores, fórmulas, células com limite ou preenchimento visível, e qualquer gráfico, imagem ou forma. Para imprimir apenas uma parte:

1. Selecione as células que pretende.
2. Escolha **Área de Impressão > Definir Área de Impressão**.

**Adicionar Seleção à Área de Impressão** acrescenta à área de impressão um segundo bloco de células. Cada bloco é impresso em páginas próprias. **Limpar Área de Impressão** volta a imprimir tudo. As linhas ou colunas que inserir ou eliminar dentro da área de impressão alteram-na em conjunto com as células.

## Quebras de página

O local onde uma página termina e a seguinte começa é calculado a partir do papel, das margens e dos tamanhos das suas linhas e colunas: uma linha ou coluna nunca é cortada ao meio. Para terminar uma página mais cedo, selecione a célula abaixo e à direita de onde a quebra deve ficar, e escolha **Quebras > Inserir Quebra de Página**. Com uma linha inteira selecionada, a quebra fica acima dela, e com uma coluna inteira selecionada, fica à esquerda. **Remover Quebra de Página** elimina a quebra acima ou à esquerda da célula selecionada, e **Repor Todas as Quebras de Página** elimina todas as quebras que definiu. As quebras de página que definiu são ignoradas enquanto uma folha está a ser reduzida para caber.

## A vista Quebras de Página

**Ver > Quebras de Página** (ou o botão no canto inferior direito da janela) mostra a folha dividida nas páginas em que será impressa. As células que são impressas mantêm o seu aspeto, tudo o resto fica sombreado, e cada página tem o seu número sobre as suas células. Uma linha tracejada é uma quebra calculada pelo JeffCrunch, e uma linha contínua é uma quebra que definiu.

- **Mover uma quebra.** Arraste uma linha para outra linha ou coluna. Ela passa a ser uma quebra definida nesse ponto, e as páginas seguintes são novamente divididas.
- **Remover uma quebra.** Arraste uma linha para fora das células impressas, para os números de linha ou para as letras das colunas.
- **Alterar a área de impressão.** Arraste a extremidade azul das células impressas. Se não tiver definido uma área de impressão, as células que são impressas sozinhas passam a formar uma, com essa extremidade deslocada.

Cada arrastamento é um passo de Anular. Enquanto uma folha está a ser reduzida para caber, as suas quebras são ignoradas, pelo que as linhas não podem ser arrastadas. **Normal** repõe as células tal como estavam. A vista em que cada folha ficou é guardada com o livro.

## A vista Esquema de Página

**Ver > Esquema de Página** (ou o botão do meio no canto inferior direito da janela) mostra a folha como as páginas em que será impressa, uma abaixo da outra, cada uma como uma folha de papel com as suas margens e um espaço entre páginas. Réguas correm ao longo do topo e pela esquerda. O cabeçalho e o rodapé são desenhados onde são impressos, e o mesmo acontece com as imagens neles incluídas.

- **Trabalhe nas células.** Clique numa célula para a selecionar, arraste para selecionar várias, e use as teclas de seta, escrita, F2 e duplo clique como faz na vista Normal. O que escrever aparece numa faixa sobre a célula, na sua página.
- **Comece pelo separador Inserir.** **Inserir > Cabeçalho e Rodapé** mostra as páginas (se não estiverem visíveis) e abre o meio do cabeçalho da primeira página, pronto para escrever. Clique noutra parte do cabeçalho ou do rodapé para escrever aí.
- **Escreva um cabeçalho ou rodapé.** Mova o ponteiro sobre a margem superior ou inferior de uma página: as suas partes esquerda, central e direita ficam realçadas. Clique numa delas e escreva. Aparece o separador **Cabeçalho e Rodapé**, com botões que inserem o número da página, o número de páginas, a data, a hora, a pasta e o nome do ficheiro, o nome da folha e uma imagem; as suas opções permitem definir uma primeira página, e páginas ímpares e pares, cada uma com cabeçalhos e rodapés próprios, e indicar se o cabeçalho cresce com as células e se se alinha com as margens. **Cabeçalho** e **Rodapé** nesse separador são galerias de modelos prontos, como **Página 1**, **Página 1 de ?**, o nome da folha, o nome do ficheiro e a data. Ao escolher um, substitui-se todo o cabeçalho ou rodapé das páginas em que está a escrever (as palavras que ainda estava a escrever não são mantidas). **(nenhum)** esvazia-o. **Ir para o Cabeçalho** e **Ir para o Rodapé** abrem a parte central dessa margem para escrever. Prima **Ctrl+Enter**, ou clique noutro sítio qualquer, para terminar; **Esc** cancela. Cada cabeçalho ou rodapé concluído é um passo de Anular.
- **Ampliar** com o controlo de deslize, ou mantenha **Ctrl** premido e rode a roda do rato.

Só as páginas visíveis são desenhadas, pelo que uma folha com milhares de páginas abre de imediato. A vista em que cada folha ficou é guardada com o livro.

## Títulos de impressão

**Títulos de Impressão** repete linhas no topo, e colunas à esquerda, em todas as páginas a seguir à da sua primeira ocorrência, para que uma linha de cabeçalho apareça em todas as páginas. Escreva as linhas (por exemplo **1:3**) e as colunas (por exemplo **A:B**) no separador Folha da caixa de diálogo Configurar Página.

## Ajustar à página

**Largura** e **Altura** reduzem as células até que a folha não tenha mais do que esse número de páginas de largura, ou de altura. **Automática** deixa essa direção ocupar as páginas de que precisar. **Escala** imprime a uma percentagem do tamanho real, de 10 a 400, e funciona enquanto Largura e Altura estiverem ambas em **Automática**. Se a redução para caber tornar as letras muito pequenas, o ecrã de Impressão avisa.

## Opções da folha

**Mostrar Linhas de Grelha** e **Mostrar Cabeçalhos** alteram apenas o ecrã. **Imprimir Linhas de Grelha** coloca no papel as linhas entre células, e **Imprimir Cabeçalhos** coloca no papel os números de linha e as letras das colunas.

**Folha da Direita para a Esquerda** faz com que a folha seja lida a partir da direita. A coluna A passa para a margem direita, e as colunas contam para a esquerda. Os números de linha também passam para a margem direita. Use esta opção para idiomas escritos da direita para a esquerda, como o árabe e o hebraico. Altera apenas o aspeto da folha no ecrã, e a escolha é guardada com a folha, pelo que volta a aparecer quando abre o livro. Não é um passo que possa anular: clique de novo para a desativar.

## Fundo

**Fundo** (no grupo **Configurar Página**) coloca uma imagem atrás das células da folha. Escolha uma imagem PNG, JPEG, GIF ou BMP. A imagem é repetida em mosaico para preencher a folha, e desloca-se com as células quando percorre a folha. Uma célula com cor de preenchimento oculta a imagem, pelo que deve dar preenchimento às células que quer ler com facilidade. A imagem só aparece no ecrã: não é impressa nem entra num PDF.

**Eliminar Fundo** retira a imagem. Ambas as ações são passos que pode anular. A imagem é guardada com a folha e volta a aparecer quando abre o livro. Numa cópia .ods, é guardada como imagem de fundo da página, que outros programas podem imprimir. Um ficheiro de imagem com mais de 8 MB, ou com dimensões excessivas, é recusado, e o mesmo acontece com uma imagem que faria todas as imagens do livro (fundos, cabeçalhos e rodapés) ultrapassar 64 MB em conjunto. Uma imagem com mais de 2400 pontos num dos lados continua a ser repetida em mosaico, com o seu tamanho real.

## Mais opções

A pequena seta no canto do grupo **Configurar Página** abre a caixa de diálogo Configurar Página, que tem todas estas opções e mais: [A caixa de diálogo Configurar Página](page-setup).
