# Alterar maiúsculas e minúsculas, ordenar e colar de outras formas
<!-- keywords: botão opções de colagem, recortar e colar inteligente, espaços ao colar, linha horizontal, régua, mostrar formatação, inspetor de estilos, shift+f1, tabulações, posições de tabulação, barra de tabulação, posição de barra, posições de tabulação predefinidas, distância de tabulação, carácter de preenchimento, manter com o seguinte, manter linhas juntas, quebra de página antes, linha isolada, linha órfã, nível de estrutura, quebras de linha e de página, alterar maiúsculas, maiúsculas, minúsculas, maiúscula inicial da frase, maiúscula inicial das palavras, alternar maiúsculas, shift+f3, ordenar, ordenar parágrafos, alfabética, ordem, colar, colar especial, continuar lista, nova lista, unir lista, aninhar tabela, células da folha de cálculo, manter apenas texto, unir formatação, rtf, texto enriquecido, texto formatado, arrastar, texto arrastado, texto sem formatação, imagem, área de transferência, ordenar texto, sort -->

Estes comandos estão no separador **Base**. Atuam sobre o texto que selecionar.

## Alterar as maiúsculas e minúsculas das letras

A **capitalização** de uma letra indica se é uma letra maiúscula (A) ou minúscula (a).

1. Selecione o texto. Se não houver nada selecionado, o JeffWrite altera a palavra onde está o cursor.
2. No grupo **Tipo de Letra**, clique em **Alterar Maiúsculas/Minúsculas**.
3. Escolha uma opção:
   - **Maiúscula inicial da frase** põe em maiúscula a primeira letra de cada frase. As restantes ficam em minúsculas.
   - **minúsculas** põe todas as letras em minúsculas.
   - **MAIÚSCULAS** põe todas as letras em maiúsculas.
   - **Maiúscula Inicial das Palavras** põe em maiúscula a primeira letra de cada palavra.
   - **aLTERNAR mAIÚSCULAS** transforma cada maiúscula em minúscula, e cada minúscula em maiúscula.

Prima **Shift+F3** para percorrer três destas opções por ordem: **Maiúscula Inicial das Palavras**, depois **MAIÚSCULAS**, depois **minúsculas**.

O aspeto das letras mantém-se. O negrito continua negrito. Prima **Ctrl+Z** para anular a alteração de uma só vez. Se o Controlo de Alterações estiver ativado, a alteração aparece como uma eliminação e uma inserção.

## Ordenar parágrafos

Ordenar coloca os parágrafos por ordem, como de A a Z. Use-o para uma lista de nomes ou para linhas de uma tabela feita com tabulações ou vírgulas.

1. Selecione dois ou mais parágrafos.
2. No grupo **Parágrafo**, clique em **Ordenar**.
3. Escolha o critério de ordenação. Cada escolha tem um **campo**, um **tipo** e uma **ordem**.
   - O **campo** é uma parte do parágrafo. O campo 1 é a primeira parte. Considere que as partes estão separadas por vírgulas ou tabulações, na caixa abaixo das chaves. Se deixar em **Parágrafos**, o parágrafo inteiro é o único campo.
   - O **tipo** é **Texto**, **Número** ou **Data**. Um número ordena-se pelo seu valor, pelo que 9 vem antes de 10.
   - A **ordem** é **Crescente** (de A a Z, do menor para o maior, do mais antigo para o mais recente) ou **Decrescente**.
4. Use **Depois por** para desempatar.
5. Marque **O primeiro parágrafo é um cabeçalho** para o deixar no lugar.
6. Clique em **OK**.

Cada parágrafo mantém o seu próprio aspeto. Não é possível ordenar com o Controlo de Alterações ativado, porque uma ordenação não é uma alteração a mostrar. Desative o Controlo de Alterações, ordene e volte a ativá-lo.

## Colar de outras formas

Clique na pequena seta sob **Colar** para ver mais opções:

- Quando o texto vem de outro programa que coloca texto enriquecido (RTF) na área de transferência, **Ctrl+V** mantém o seu aspeto: tipos de letra, tamanhos, cores, negrito, listas, tabelas, ligações e imagens. Se o programa não deu texto enriquecido, o JeffWrite usa a forma de página Web (HTML) e, depois, apenas as palavras. O texto enriquecido com mais de 16 MB não é lido, e a colagem usa a forma seguinte. Arrastar texto formatado de outro programa para a página faz o mesmo.
- **Unir Formatação** cola o texto com o aspeto do local para onde vai. Apenas o negrito, o itálico, o sublinhado e o riscado são mantidos.
- **Manter Apenas Texto** cola as palavras sem qualquer aspeto.
- **Imagem** cola uma imagem da área de transferência. Fica a cinzento quando a área de transferência não contém nenhuma imagem.
- **Colar Especial** (**Ctrl+Alt+V**) lista todas as formas como a área de transferência pode ser colada neste momento, e permite escolher uma, como **Texto formatado (RTF)**, **Texto sem formatação** ou **HTML**.

Uma opção fica a cinzento quando a área de transferência não tem nada que possa usar.

Logo após colar, aparece junto ao cursor um pequeno botão **Opções de Colagem**. Clique nele para escolher **Manter Formatação de Origem**, **Unir Formatação** ou **Manter Apenas Texto**. O JeffWrite anula a colagem e volta a fazê-la dessa forma. A alteração é um só passo: um único **Anular** desfaz a colagem inteira, por mais vezes que mude de ideias. Pelo teclado, prima a tecla **Ctrl** (a tecla **Control** num Mac) depois de colar para abrir o menu do botão e, em seguida, use as teclas de seta e **Enter** para escolher. O botão desaparece assim que escrever, mover o cursor ou premir **Esc**, e não é apresentado num documento que não possa alterar.

**Definir Colagem Predefinida** está na página **Avançadas** de **Opções**. Tem uma caixa para colar dentro do mesmo documento, outra para colar entre documentos e outra para colar de outros programas. Cada uma pode ser **Manter Formatação de Origem** (como era antes), **Unir Formatação** ou **Manter Apenas Texto**, e **Colar** (**{key:Paste}**) faz o que estiver definido para a origem do conteúdo. Exemplo: defina a colagem de outros programas como **Manter Apenas Texto**, e as palavras copiadas de uma página Web ficam com a formatação do local para onde são coladas. Pode continuar a escolher outra forma para uma colagem específica com o botão **Opções de Colagem**.

Duas outras caixas dessa página definem o que **Colar** faz com itens de lista colados junto a uma lista (**Conforme chegam**, **Continuar Lista**, **Nova Lista** ou **Unir Lista**) e com células de folha de cálculo (**Como palavras**, **Tabela**, **Aninhar Tabela**, **Unir Tabela**, **Inserir como Novas Linhas** ou **Substituir Células**). Uma escolha só é usada onde faz sentido: **Aninhar Tabela** com o cursor fora de uma tabela, por exemplo, cola as células como palavras.

Quando a colagem trouxe parágrafos de lista junto a uma lista, o botão oferece também **Continuar Lista** (a numeração prossegue a partir da lista ao lado), **Nova Lista** (a numeração recomeça em 1) e **Unir Lista** (os itens juntam-se à lista ao lado e tomam o seu aspeto). Um simples **Ctrl+V** coloca os itens como vieram, e uma escolha altera esse comportamento. As mesmas opções são comandos no menu Colar do separador Base e em **Colar Especial**, pelo que também podem ser usadas pelo teclado.

Quando a área de transferência contém células de folha de cálculo (copiadas do JeffCrunch ou de outro programa como tabela ou como linhas com tabulações entre as células), um simples **Ctrl+V** cola-as como as palavras que são. Para as colar como células, escolha **Tabela** no botão, no menu Colar ou em **Colar Especial**. Com o cursor dentro de uma tabela, o botão oferece **Aninhar Tabela** (uma tabela dentro da célula), **Unir Tabela** (as células juntam-se à tabela, que ganha as colunas de que precisa), **Inserir como Novas Linhas**, **Inserir como Novas Colunas** (as células vão para novas colunas depois da célula do cursor, a partir da sua linha para baixo; uma tabela pode ter, no máximo, 256 colunas e 50 000 células, e a operação é recusada, com uma mensagem, quando uma célula unida atravessa o local das colunas) e **Substituir Células** (as células substituem as da tabela, a partir da célula do cursor). Uma tabela colada tem, no máximo, 500 linhas e 63 colunas, e uma célula contém, no máximo, 4000 carateres; o que ultrapassar esse limite fica de fora, e uma mensagem informa essa situação. Uma tabela aninhada numa tabela pode ter, no máximo, 8 níveis de profundidade, e uma tabela aninhada cujas colunas ficariam demasiado estreitas para a célula é recusada. Cada colagem é um só passo: **Anular** desfaz tudo, e **Refazer** volta a colocá-lo.

**Recortar e colar inteligente** mantém os espaços corretos ao mover palavras. Se recortar palavras inteiras que estão entre dois espaços, o JeffWrite também retira um desses espaços, para não ficar com dois espaços seguidos. Se colar uma palavra encostada a outra, acrescenta um espaço entre elas. Não altera o interior de uma palavra. Para o desativar, clique em **Ficheiro > Opções > Avançadas** e desmarque **Recortar e colar inteligente**.

## Quebras de linha e de página, e tabulações

Clique no pequeno botão no canto do grupo **Parágrafo** para abrir a caixa **Parágrafo**. Por baixo das opções de espaçamento encontram-se estas:

- **Controlo de linhas isoladas e órfãs** impede que uma linha sozinha de um parágrafo fique isolada no início ou no fim de uma página.
- **Manter com o seguinte** mantém o parágrafo na mesma página que o seguinte. Use-o para um título.
- **Manter linhas juntas** mantém todas as linhas do parágrafo numa só página.
- **Eliminar números de linha** deixa este parágrafo de fora quando as linhas de página são numeradas (veja Esquema > Números de Linha).
- **Quebra de página antes** começa o parágrafo numa nova página.
- **Não hifenizar** impede que as palavras do parágrafo sejam divididas com um hífen no fim de uma linha.
- **Não adicionar espaço entre parágrafos do mesmo estilo** remove o espaço entre dois parágrafos que têm o mesmo estilo.
- **Nível de estrutura** indica onde o parágrafo se encontra na estrutura do documento: **Texto normal**, ou um nível de 1 a 9.

Clique em **Tabulações** na caixa para definir posições de tabulação. Uma posição de tabulação é o sítio para onde o texto salta quando prime **Tab**. Escreva a sua posição, escolha como o texto se alinha nesse ponto (à esquerda, ao centro, à direita ou decimal) e escolha um carácter de preenchimento. Um carácter de preenchimento é a fila de pontos, traços ou uma linha que preenche o espaço antes da posição. Clique em **Definir** para a acrescentar. Para remover uma, selecione-a na lista e clique em **Limpar**. **Limpar Tudo** remove todas.

Escolha **Barra** para definir uma barra. Uma barra não detém o texto. Desenha uma linha fina e reta, de cima a baixo, nesse ponto, em cada linha do parágrafo. É uma forma de pôr um separador entre colunas de texto. Uma barra não tem carácter de preenchimento. A régua não mostra as barras. Um parágrafo pode ter até 64. Ainda não: num parágrafo que se lê da direita para a esquerda, as barras continuam a ser medidas a partir da margem esquerda. Defina-as e limpe-as na caixa **Tabulações**.

**Posições de tabulação predefinidas** é a distância entre as posições que um parágrafo recebe quando não tem posições próprias, e para além da última posição que tiver. A distância habitual é de 1,27 cm (meia polegada). Esta definição aplica-se a todo o documento, e não apenas ao parágrafo onde está. A nova distância aplica-se a todos os parágrafos assim que clicar em **OK**.

## Limites e sombreado para parágrafos

Selecione um ou mais parágrafos. No grupo **Parágrafo**, o botão **Limites** tem as opções rápidas: **Limite Inferior**, **Limite Superior**, **Limite Esquerdo**, **Limite Direito**, **Todos os Limites** e **Sem Limites**. Uma opção rápida desenha uma linha simples, com meio ponto de espessura, a um ponto de distância do texto. Uma opção para um lado deixa os restantes lados como estavam.

**Limites e Sombreado**, no fim dessa lista, abre uma caixa. Marque os lados que devem ter uma linha, ou clique em **Caixa** ou **Nenhum**. Marque **Entre** para pôr uma linha entre os parágrafos selecionados que têm os mesmos limites. Depois escolha o **Estilo** (simples, duplo, grosso, pontilhado, tracejado, traços curtos, traço-ponto ou traço-ponto-ponto), a **Espessura**, a **Cor** e a **Distância do texto**. O **Sombreado** preenche os parágrafos com uma cor. O botão **Sombreado**, ao lado de **Limites**, define apenas o preenchimento, com as mesmas cores de **Cor do Tipo de Letra**.

As linhas e o preenchimento são um só passo que **Anular** desfaz, e são mantidos ao guardar um ficheiro .docx ou .odt. Se um parágrafo veio de um ficheiro com limite ou preenchimento próprios, a sua escolha substitui-os.

## Desenhar uma linha horizontal

Clique em **Linha Horizontal** no grupo **Parágrafo**. O JeffWrite acrescenta um parágrafo próprio com uma linha fina por baixo, depois do parágrafo onde está o cursor. O cursor passa para um parágrafo simples abaixo da linha. Para retirar a linha, selecione o seu parágrafo e elimine-o.

## Ver o que está definido onde está o cursor

Prima **Shift+F1**, ou use **Mostrar Formatação**. Uma caixa lista os estilos, o tipo de letra, o tamanho, o aspeto das letras, a cor e o alinhamento, os avanços e o espaçamento do parágrafo na posição do cursor. Apenas os mostra. Para os alterar, use as caixas **Tipo de Letra** e **Parágrafo**.

## O painel Área de Transferência

O painel Área de Transferência guarda os últimos 24 itens copiados ou recortados nesta janela do programa, para que possa colar um item mais antigo sem o copiar novamente. Clique na pequena seta no canto do grupo **Área de Transferência** no separador **Base** (ou prima **Alt**, depois **H**, depois **FO**) para o abrir. Aparece um painel ao lado da página.

Cada item mostra uma pequena pré-visualização e o tipo de conteúdo: texto, texto formatado, células, uma imagem, formas ou diapositivos. O mais recente fica no topo. Clique num item para o colar onde estiver o cursor ou a seleção. Cola-se como qualquer outra colagem: um passo de **Anular** desfaz-o, e o botão Opções de Colagem aparece como habitualmente.

- **Colar Tudo** cola todos os itens, do mais antigo para o mais recente, para que fiquem na ordem em que os copiou.
- **Eliminar** remove o item escolhido. **Limpar Tudo** remove todos.
- O painel só recolhe itens enquanto estiver aberto. Para continuar a recolher depois de o fechar, marque **Continuar a recolher quando este painel for fechado**. A escolha fica guardada, e a recolha começa quando o programa arranca, antes de abrir o painel.
- Para recolher também o que copia noutros programas, marque **Recolher também o que é copiado noutros programas**. Só são guardados texto simples e imagens, e esta opção volta a desativar-se sempre que abre o programa. Com esta opção ativa, uma cópia feita noutro programa é lida por completo antes de este programa decidir se a guarda; por isso, uma cópia muito grande pode fazer este programa parar por um momento e usar tanta memória quanto o tamanho da cópia. Apenas uma forma dela é lida: o seu texto, quando o tem, caso contrário uma imagem. Com esta opção desativada, nada copiado noutros programas é lido.

Os itens são guardados apenas na memória do programa. Nada é gravado num ficheiro, e a lista é esvaziada quando fechar a última janela do programa. Um item permanece na lista depois de o documento de onde vem ser fechado. Um item maior do que 8 MB não é guardado, e o painel avisa quando isso acontece; uma imagem conta 4 bytes por cada ponto (píxel), pelo que uma imagem com mais de cerca de 2 milhões de píxeis é grande de mais. Todos os itens juntos ficam limitados a 64 MB, e os mais antigos saem primeiro. Qualquer coisa que um gestor de palavras-passe marque como secreta nunca é guardada.
