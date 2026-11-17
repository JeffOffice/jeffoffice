# Criar e editar tabelas
<!-- keywords: tabela, grelha, linhas, colunas, células, selecionar células, selecionar linha, selecionar coluna, várias células, unir células, dividir células, limites, preenchimento de célula, formato de tabela, estilo de tabela, inserir tabela, desenhar tabela, eliminar linha, linhas de grelha, direção do texto, hifenização -->

Uma tabela é uma grelha de linhas e colunas. Cada caixa da grelha é uma célula, e cada célula contém o seu próprio texto.

## Inserir uma tabela

- Clique em **Base > Objetos > Tabela** (ou **Inserir > Tabelas > Tabela**). Desloque o cursor pela grelha para escolher as linhas e colunas e, em seguida, clique.
- Ou escolha **Inserir Tabela…** na parte inferior desse menu. Na caixa de diálogo **Criar Tabela**, defina **Número de linhas**, **Número de colunas** e **Formato de tabela**.
- Ou escolha **Desenhar Tabela** no menu **Tabela** no separador **Base** e arraste um retângulo. Obtém 3 linhas e 3 colunas.

A tabela aparece a meio da página. Arraste a respetiva margem para a mover.

## Colar células como tabela

Copie células do JeffCrunch, ou uma tabela de um documento ou de uma página Web, e prima **Ctrl+V**. As células são colocadas como tabela na zona central superior da página. Cada célula mantém o seu tipo de letra, tamanho, negrito, itálico, sublinhado e cor do texto, a respetiva cor de fundo, os seus limites e a forma como o texto está posicionado. As células unidas continuam unidas, e as palavras são as que as células mostravam, pelo que um número aparece como aparecia. A tabela nunca é mais larga do que a página, e um único **Ctrl+Z** a desfaz.

Se estiver a escrever numa célula de tabela quando colar, as células preenchem a tabela a partir dessa célula. Cada célula assume o aspeto da célula de origem, pelo que uma célula sem cor de fundo ou limite chega sem nenhum, e a que substitui perde os seus. As células unidas e as larguras das colunas não são transferidas para uma tabela que já existe. As células que ficam além da margem da tabela são deixadas de fora, e a barra de estado indica-o. **Manter Formatação de Origem** também cria a tabela. **Manter Apenas Texto** cola apenas as palavras, e **Colar Especial** continua a listar essa opção e as outras formas. Uma página que tenha outros elementos além da tabela é colada como sempre foi. Uma tabela com mais de 1000 linhas, 64 colunas ou 100 000 células, ou com mais de 256 linhas (o máximo que uma tabela suporta), é colada apenas como texto simples, e a barra de estado indica o motivo.

## Escrever numa tabela

Clique numa célula e escreva. Prima Tab para ir para a célula seguinte e Shift+Tab para voltar atrás. Prima Tab na última célula para acrescentar uma linha. Uma linha cresce em altura para caber o respetivo texto.

## Selecionar células, linhas, colunas ou a tabela inteira

- Clique dentro de uma célula para trabalhar nessa célula.
- Para selecionar várias células de uma vez, arraste de uma célula até outra. Ou clique numa célula, mantenha Shift premido e clique noutra. As células no retângulo entre as duas ficam sombreadas, e um novo Shift+clique move o canto oposto. Uma célula unida fica sempre selecionada por inteiro.
- Clique em **Esquema da Tabela > Tabela > Selecionar** para **Selecionar Célula**, **Selecionar Linha**, **Selecionar Coluna** e **Selecionar Tabela**. **Selecionar Célula** seleciona todo o texto da célula em que está. **Selecionar Linha** e **Selecionar Coluna** selecionam todas as células da linha ou coluna em que está. Se já tiver selecionado várias células, selecionam todas as linhas ou colunas que essas células abrangem.
- Clique na margem exterior da tabela, ou prima Esc enquanto escreve, para selecionar a tabela inteira. Prima Esc quando houver células selecionadas para voltar a selecionar a tabela inteira.

Pode fazer tudo isto a partir do teclado. Prima Alt e, em seguida, J e L para abrir **Esquema da Tabela**. Depois prima S e L para abrir o menu **Selecionar**, use as teclas de seta para escolher um comando e prima Enter. Com células selecionadas, Enter ou F2 começa a escrever na primeira delas, e Delete limpa o respetivo texto mas mantém a tabela.

Os preenchimentos, limites, diagonais, alinhamentos, margens das células e a formatação do texto (como tipo de letra, tamanho, negrito, itálico, sublinhado, cor e alinhamento) aplicam-se a todas as células selecionadas, e um único Anular desfaz todas. Aplicam-se à célula em que está se acabou de clicar numa, e a todas as células quando a tabela inteira está selecionada. As diagonais exigem que esteja numa célula ou que existam células selecionadas.

## Adicionar e eliminar linhas e colunas

1. Clique numa célula junto ao local pretendido, ou selecione várias células.
2. Em **Esquema da Tabela > Linhas e Colunas**, clique em **Inserir Acima**, **Inserir Abaixo**, **Inserir à Esquerda** ou **Inserir à Direita**. Com células selecionadas, a nova linha ou coluna fica acima, abaixo, à esquerda ou à direita de todas elas.
3. Para remover linhas ou colunas, clique em **Eliminar** e escolha **Eliminar Linhas**, **Eliminar Colunas** ou **Eliminar Tabela**. **Eliminar Linhas** remove todas as linhas que a célula em que está ou as células selecionadas abrangem, e **Eliminar Colunas** faz o mesmo com as colunas.

Com a tabela inteira selecionada, **Inserir Acima** e **Inserir à Esquerda** acrescentam no início da tabela, e **Inserir Abaixo** e **Inserir à Direita** no fim. Se uma nova linha ou coluna atravessar uma célula unida, a célula unida cresce para a incluir. Para eliminar uma linha ou coluna, clique primeiro nela. Uma tabela mantém pelo menos uma linha e uma coluna, e tem no máximo 256 linhas e 256 colunas.

## Alterar tamanhos

- Escreva um número em **Esquema da Tabela > Tamanho > Altura** ou **Largura** para dimensionar a tabela inteira. Uma nova largura dimensiona todas as colunas proporcionalmente, e uma nova altura dimensiona todas as linhas proporcionalmente. Uma linha nunca fica mais baixa do que o seu texto precisa enquanto **Aumentar para Caber o Texto** estiver selecionado.
- Arraste uma linha entre duas colunas para alterar as respetivas larguras. A tabela mantém a mesma largura.
- Arraste uma linha entre duas linhas para alterar a altura de uma linha.
- Clique em **Esquema da Tabela > Tamanho > Distribuir Linhas** ou **Distribuir Colunas** para as tornar iguais.

## Unir e dividir células

Selecione as células que pretende juntar e escolha **Esquema da Tabela > Unir > Unir Células**. Passam a ser uma só célula, e o respetivo texto combina-se, com um parágrafo de cada célula, da esquerda para a direita e de cima para baixo. Se apenas clicar numa célula, **Unir Células** junta-a à célula à sua direita. **Dividir Células** transforma as células unidas de novo em células separadas.

## Cor e limites

Use o separador **Estrutura da Tabela**.

- **Formatos de Tabela** é uma galeria de aspetos pré-definidos. Um formato define preenchimentos, limites e o texto dos cabeçalhos.
- **Preenchimento de Célula** colore as células selecionadas, a célula em que está ou a tabela inteira.
- Para os limites, escolha primeiro uma **Cor da Linha** e uma **Espessura da Linha**. Depois clique em **Limites** e escolha **Todos os Limites**, **Limites Exteriores**, **Limites Interiores**, **Sem Limites** ou um lado, como **Limite Superior**.
- **Diagonais** divide uma célula com uma linha de canto a canto. Escolha **Dividir para Baixo** ou **Dividir para Cima**. Cada metade tem o seu próprio texto: o texto da célula fica na primeira metade (o canto inferior esquerdo de uma linha descendente, o canto superior esquerdo de uma ascendente), e a segunda metade começa vazia. Clique numa metade para escrever nela, ou prima Tab para passar de uma metade para a outra e, depois, para a célula seguinte. O texto quebra-se dentro da respetiva metade, pelo que as linhas ficam mais curtas em direção ao vértice do triângulo. A linha não cresce por causa disso, por isso aumente a altura da linha se algum texto ficar cortado. Escolher **Dividir para Baixo** ou **Dividir para Cima** numa célula que já está dividida inverte a linha e mantém a respetiva cor e espessura. **Sem Divisão** remove a linha e coloca o texto da segunda metade a seguir ao da primeira, e a linha cresce se o texto combinado precisar de mais espaço.

Para colocar texto numa célula, use os botões em **Esquema da Tabela > Alinhamento**: topo, meio ou fundo, e esquerda, centro, direita ou justificado. **Margens das Células**, também aí, define o espaço entre as margens de uma célula e o respetivo texto: **Nenhuma**, **Estreitas**, **Moderadas** ou **Largas**. Os botões de topo, meio e fundo e **Margens das Células** alteram as células selecionadas, a célula em que está ou todas as células quando a tabela inteira está selecionada.

**Direção do Texto**, também em **Esquema da Tabela > Alinhamento**, roda o texto de uma célula um quarto de volta, para que as linhas se leiam de cima para baixo. Serve bem para um título sobre uma coluna estreita. Clique de novo para repor o texto. A linha não cresce para caber texto rodado, por isso arraste a linha inferior da linha para baixo até o texto caber.

**Hifenização**, ao lado, abre a caixa de diálogo **Hifenização**. Aí pode ativar ou desativar **Hifenizar automaticamente este artigo**, que define se as palavras longas são separadas com hífen no fim de uma linha, e definir a **Zona de hifenização**. As palavras nas células são hifenizadas, salvo se desativar esta opção. Consulte [Ortografia, dicionário de sinónimos, pesquisa e hifenização](proofing).

A Direção do Texto altera as células selecionadas, a célula em que está ou todas as células quando a tabela inteira está selecionada, e cada célula mantém a sua própria direção. A hifenização é diferente: o texto de uma tabela é uma única história, por isso a definição pertence à tabela inteira, sejam quais forem as células selecionadas, e um ficheiro .pub mantém-na assim. **Acrescentar Hífenes Opcionais** na caixa de diálogo atua sobre o texto das células selecionadas, a célula em que está ou todas as células quando a tabela inteira está selecionada.

**Ver Linhas de Grelha** em **Esquema da Tabela > Tabela** mostra linhas pontilhadas à volta de cada célula no ecrã. Não são impressas.

## Formatar Tabela

Clique com o botão direito na tabela e escolha **Formatar Tabela**. Ou clique na pequena seta no canto de **Esquema da Tabela > Alinhamento**, que abre a caixa no separador **Propriedades da Célula**. A caixa tem estes separadores:

- **Cores e Linhas** define o preenchimento por trás das células e a linha à volta da tabela.
- **Tamanho** define a altura, a largura e a rotação da tabela. Marque **Bloquear proporção** para manter as proporções da tabela ao arrastar uma alça de canto.
- **Esquema** define onde a tabela fica na página e a forma como o texto se ajusta à sua volta.
- **Propriedades da Célula** define o alinhamento vertical, as quatro margens e **Rodar texto 90°** (o mesmo que **Direção do Texto**). Aplicam-se às células selecionadas, à célula em que está ou a todas as células quando a tabela inteira está selecionada. Só são aplicadas as definições que alterar.
- **Texto Alternativo** descreve a tabela para pessoas que não a conseguem ver.

## Consulte também

- [Os separadores de Tabela](tab-table)
- [Formatar texto](formatting-text)
- [Marcas, numeração e tabulações](lists-tabs)
