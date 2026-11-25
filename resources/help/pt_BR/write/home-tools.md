# Alterar maiúsculas e minúsculas, classificar e colar de outras formas

<!-- keywords: botão opções de colagem, recortar e colar inteligente, espaços ao colar, linha horizontal, régua, revelar formatação, inspetor de estilos, shift+f1, tabulações, paradas de tabulação, barra de tabulação, barra de parada, paradas de tabulação padrão, distância de tabulação, preenchimento, manter com o próximo, manter linhas juntas, quebra de página antes, viúva, órfã, nível de estrutura de tópicos, quebras de linha e de página, alterar maiúsculas e minúsculas, maiúsculas, minúsculas, maiúscula no início da frase, colocar em maiúscula, alternar maiúsculas e minúsculas, shift+f3, classificar, classificar parágrafos, ordem alfabética, ordem, colar, colar especial, continuar lista, nova lista, mesclar lista, aninhar tabela, células de planilha, manter somente texto, mesclar formatação, rtf, texto formatado, arrastar, texto sem formatação, imagem, área de transferência, bordas, sombreamento, linha horizontal, separador -->

Estas ferramentas estão na guia **Página Inicial**. Elas trabalham sobre o texto que você seleciona.

## Alterar as maiúsculas e minúsculas das letras

A **caixa** de uma letra indica se ela é maiúscula (A) ou minúscula (a).

1. Selecione o texto. Se nada estiver selecionado, o JeffWrite altera a palavra em que está o cursor.
2. No grupo **Fonte**, clique em **Alterar Maiúsculas/Minúsculas**.
3. Escolha uma opção:
   - **Maiúscula no início da frase** deixa a primeira letra de cada frase maiúscula. As demais ficam minúsculas.
   - **minúsculas** deixa todas as letras minúsculas.
   - **MAIÚSCULAS** deixa todas as letras maiúsculas.
   - **Iniciais Maiúsculas** deixa a primeira letra de cada palavra maiúscula.
   - **aLTERNAR mAIÚSC./mINÚSC.** transforma cada maiúscula em minúscula, e cada minúscula em maiúscula.

Pressione **Shift+F3** para percorrer três delas em sequência: **Iniciais Maiúsculas**, depois **MAIÚSCULAS**, depois **minúsculas**.

O aspecto das letras permanece o mesmo. O negrito continua negrito. **Ctrl+Z** desfaz a alteração em um único passo. Se o Controlar Alterações estiver ativado, a alteração aparece como uma exclusão e uma inserção.

## Classificar parágrafos

Classificar coloca os parágrafos em ordem, como de A a Z. Use para uma lista de nomes ou para linhas de uma tabela feita com tabulações ou vírgulas.

1. Selecione dois ou mais parágrafos.
2. No grupo **Parágrafo**, clique em **Classificar**.
3. Escolha pelo que classificar. Cada critério tem um **campo**, um **tipo** e uma **ordem**.
   - O **campo** é uma parte do parágrafo. O campo 1 é a primeira parte. Considere que as partes são separadas por vírgulas ou tabulações, na caixa abaixo das chaves. Se você deixar **Parágrafos**, o parágrafo inteiro é o único campo.
   - O **tipo** é **Texto**, **Número** ou **Data**. Um número é classificado pelo seu valor, então 9 vem antes de 10.
   - A **ordem** é **Crescente** (de A a Z, do menor para o maior, da data mais antiga para a mais recente) ou **Decrescente**.
4. Use **Em seguida por** para desempatar.
5. Marque **O primeiro parágrafo é um cabeçalho** para deixá-lo onde está.
6. Clique em **OK**.

Cada parágrafo mantém a sua própria aparência. A classificação não pode ser usada enquanto o Controlar Alterações estiver ativado, porque uma classificação não é uma alteração a ser mostrada. Desative o Controlar Alterações, classifique e ative-o novamente.

## Colar de outras formas

Clique na pequena seta abaixo de **Colar** para ver mais opções:

- Quando o texto vem de outro programa que coloca texto formatado (RTF) na área de transferência, **Ctrl+V** mantém a aparência: fontes, tamanhos, cores, negrito, listas, tabelas, links e imagens. Se o programa não forneceu texto formatado, o JeffWrite usa a forma de página da web dele (HTML) e, depois, apenas as palavras. Texto formatado com mais de 16 MB não é lido, e a colagem usa a forma seguinte. Arrastar texto formatado de outro programa para a página faz o mesmo.
- **Mesclar Formatação** cola o texto com a aparência do lugar para onde ele vai. Somente negrito, itálico, sublinhado e tachado são mantidos.
- **Manter Somente Texto** cola as palavras sem nenhuma aparência.
- **Imagem** cola uma imagem da área de transferência. Ela fica esmaecida quando a área de transferência não tem imagem.
- **Colar Especial** (**Ctrl+Alt+V**) lista todas as formas como a área de transferência pode ser colada agora, e permite escolher uma, como **Texto formatado (RTF)**, **Texto sem formatação** ou **HTML**.

Uma opção fica esmaecida quando a área de transferência não tem nada que ela possa usar.

Logo após colar, um pequeno botão **Opções de Colagem** aparece junto ao cursor. Clique nele para escolher **Manter Formatação de Origem**, **Mesclar Formatação** ou **Manter Somente Texto**. O JeffWrite desfaz a colagem e a refaz desse jeito. A alteração é um único passo: um **Desfazer** basta para desfazer a colagem inteira, por mais vezes que você tenha mudado de ideia. Pelo teclado, toque na tecla **Ctrl** (a tecla **Control** em um Mac) depois de colar para abrir o menu do botão e use as setas e **Enter** para escolher. O botão desaparece assim que você digitar, mover o cursor ou pressionar **Esc**, e não é oferecido em um documento que você não pode alterar.

**Definir Colagem Padrão** fica na página **Avançado** de **Opções**. Ela tem uma caixa para colar dentro do mesmo documento, outra para colar entre documentos e outra para colar de outros programas. Cada uma pode ser **Manter Formatação de Origem** (como era antes), **Mesclar Formatação** ou **Manter Somente Texto**, e **Colar** (**{key:Paste}**) faz o que estiver definido para a origem do conteúdo. Exemplo: defina colar de outros programas como **Manter Somente Texto**, e as palavras copiadas de uma página da web passam a ter a formatação do lugar onde são coladas. Você ainda pode escolher outra forma para uma colagem específica com o botão **Opções de Colagem**.

Quando a colagem trouxe parágrafos de lista ao lado de uma lista, o botão também oferece **Continuar Lista** (a numeração segue a partir da lista ao lado), **Nova Lista** (a numeração recomeça em 1) e **Mesclar Lista** (os itens se juntam à lista ao lado e assumem a aparência dela). **Ctrl+V** comum coloca os itens como estavam, e uma escolha altera isso. As mesmas opções são comandos no menu Colar da guia **Página Inicial** e em **Colar Especial**, então o teclado também chega a elas.

Quando a área de transferência contém células de planilha (copiadas do JeffCrunch ou de outro programa como tabela ou como linhas com tabulações entre as células), **Ctrl+V** comum as cola como as palavras que são. Para colá-las como células, escolha **Tabela** no botão, no menu Colar ou em **Colar Especial**. Com o cursor dentro de uma tabela, o botão oferece **Aninhar Tabela** (uma tabela dentro da célula), **Mesclar Tabela** (as células se juntam à tabela, que ganha as colunas de que precisa), **Inserir como Novas Linhas**, **Inserir como Novas Colunas** (as células entram em novas colunas depois da célula do cursor, a partir da linha dele para baixo; uma tabela pode ter no máximo 256 colunas e 50.000 células, e a operação é recusada, com uma mensagem, quando uma célula mesclada atravessa o local das colunas) e **Substituir Células** (as células substituem as da própria tabela, a partir da célula do cursor). Uma tabela colada tem no máximo 500 linhas e 63 colunas, e cada célula contém no máximo 4.000 caracteres; o que passar disso é deixado de fora, e uma mensagem avisa. Uma tabela aninhada em outra pode ter no máximo 8 níveis de profundidade, e uma tabela aninhada cujas colunas ficariam estreitas demais para a célula é recusada. Cada colagem é um único passo: **Desfazer** desfaz tudo, e **Refazer** a coloca de volta.

**Recortar e colar inteligente** mantém os espaços corretos ao mover palavras. Se você recortar palavras inteiras entre dois espaços, o JeffWrite leva também um desses espaços, para que não fique um espaço duplo. Se você colar uma palavra logo ao lado de outra, ele acrescenta um espaço entre elas. Não altera o meio de uma palavra. Para desativar, clique em **Arquivo > Opções > Avançado** e desmarque **Recortar e colar inteligente**.

## Quebras de linha e de página, e tabulações

Clique no pequeno botão no canto do grupo **Parágrafo** para abrir a caixa **Parágrafo**. Abaixo das opções de espaçamento estão estas:

- **Controle de viúvas e órfãs** impede que uma linha isolada de um parágrafo fique sozinha no início ou no fim de uma página.
- **Manter com o próximo** mantém o parágrafo na mesma página que o seguinte. Use para um título.
- **Manter linhas juntas** mantém todas as linhas do parágrafo em uma só página.
- **Suprimir números de linha** deixa este parágrafo de fora quando as linhas são numeradas nas páginas (veja Layout > Numeração de Linhas).
- **Quebra de página antes** inicia o parágrafo em uma nova página.
- **Não hifenizar** impede que as palavras do parágrafo sejam divididas com hífen no fim de uma linha.
- **Não adicionar espaço entre parágrafos do mesmo estilo** remove o espaço entre dois parágrafos que têm o mesmo estilo.
- **Nível de estrutura de tópicos** indica onde o parágrafo está na estrutura do documento: **Texto do corpo**, ou um nível de 1 a 9.

Clique em **Tabulações** na caixa para definir paradas de tabulação. Uma parada de tabulação é um ponto para onde o texto salta quando você pressiona **Tab**. Digite a posição, escolha como o texto se alinha ali (à esquerda, centralizado, à direita ou decimal) e escolha um preenchimento. Um preenchimento é a linha de pontos, traços ou um traço contínuo que ocupa o espaço antes da parada. Clique em **Definir** para adicioná-la. Para remover uma, selecione-a na lista e clique em **Limpar**. **Limpar Tudo** remove todas.

Escolha **Barra** para definir uma barra. Uma barra não interrompe o texto. Ela desenha uma linha fina e reta, de cima a baixo, naquele ponto em cada linha do parágrafo. É uma forma de colocar um traço entre colunas de texto. A barra não tem preenchimento. A régua não mostra as barras. Um parágrafo comporta até 64 delas. Ainda não é possível: em um parágrafo que é lido da direita para a esquerda, as barras continuam sendo medidas a partir da borda esquerda. Defina e limpe-as na caixa **Tabulações**.

**Paradas de tabulação padrão** é a distância entre as paradas que um parágrafo recebe quando não tem paradas próprias, e além da última parada que ele tem. A distância usual é de 1,27 cm. Esta opção vale para o documento inteiro, e não apenas para o parágrafo em que você está. A nova distância se aplica a todos os parágrafos assim que você clicar em **OK**.

## Bordas e sombreamento para parágrafos

Selecione um ou mais parágrafos. No grupo **Parágrafo**, o botão **Bordas** tem as opções rápidas: **Borda Inferior**, **Borda Superior**, **Borda Esquerda**, **Borda Direita**, **Todas as Bordas** e **Sem Borda**. Uma opção rápida desenha uma única linha, de meio ponto de espessura, a um ponto de distância das palavras. Uma opção para um só lado mantém os outros lados como estavam.

**Bordas e Sombreamento**, no fim dessa lista, abre uma caixa. Marque os lados que recebem uma linha, ou clique em **Caixa** ou **Nenhuma**. Marque **Entre** para colocar uma linha entre os parágrafos selecionados que têm as mesmas bordas. Depois escolha o **Estilo** (simples, duplo, grosso, pontilhado, tracejado, traços curtos, traço-ponto ou traço-ponto-ponto), a **Largura**, a **Cor** e a **Distância do texto**. **Sombreamento** preenche os parágrafos com uma cor. O botão **Sombreamento**, ao lado de **Bordas**, define apenas o preenchimento, com as mesmas cores de **Cor da fonte**.

As linhas e o preenchimento formam um único passo que **Desfazer** desfaz, e são mantidos ao salvar um arquivo .docx ou .odt. Se um parágrafo veio de um arquivo com borda ou preenchimento próprios, a sua escolha toma o lugar deles.

## Desenhar uma linha horizontal

Clique em **Linha Horizontal** no grupo **Parágrafo**. O JeffWrite adiciona um parágrafo próprio com uma linha fina embaixo, depois do parágrafo em que está o cursor. O cursor passa para um parágrafo comum abaixo da linha. Para remover a linha, selecione o parágrafo dela e exclua-o.

## Ver o que está definido onde está o cursor

Pressione **Shift+F1** ou use **Revelar Formatação**. Uma caixa lista os estilos, a fonte, o tamanho, o aspecto das letras, a cor e o alinhamento, os recuos e o espaçamento do parágrafo no cursor. Ela apenas mostra essas informações. Para alterá-las, use as caixas **Fonte** e **Parágrafo**.

## O painel Área de Transferência

O painel Área de Transferência guarda os últimos 24 itens que você copiou ou recortou neste programa, na janela atual, para que você possa colar um item mais antigo sem copiá-lo de novo. Clique na pequena seta no canto do grupo **Área de Transferência** da guia **Página Inicial** (ou pressione **Alt**, depois **H**, depois **FO**) para abri-lo. Um painel aparece ao lado da página.

Cada item mostra uma pequena prévia e o tipo de conteúdo: texto, texto formatado, células, imagem, formas ou slides. O mais recente fica no topo. Clique em um item para colá-lo onde estão o cursor ou a seleção. Ele é colado como qualquer outra colagem: um passo de **Desfazer** o desfaz, e o botão Opções de Colagem aparece normalmente.

- **Colar Tudo** cola todos os itens, do mais antigo para o mais recente, para que fiquem na ordem em que você os copiou.
- **Excluir** remove o item escolhido. **Limpar Tudo** remove todos.
- O painel só coleta enquanto estiver aberto. Para continuar coletando depois de fechá-lo, marque **Continuar coletando quando este painel for fechado**. A escolha fica guardada, e a coleta começa quando o programa é iniciado, antes de você abrir o painel.
- Para coletar também o que você copia em outros programas, marque **Coletar também o que for copiado em outros programas**. Somente texto simples e imagens são guardados, e a opção se desativa de novo toda vez que você abre o programa. Com ela ativada, uma cópia feita em outro programa é lida por inteiro antes de este programa decidir se a guarda; por isso, uma cópia muito grande pode fazer este programa pausar por um momento e usar tanta memória quanto o tamanho da cópia. Só uma forma dela é lida: o texto, quando há texto, ou então uma imagem. Com a opção desligada, nada copiado em outros programas é lido.

Os itens ficam guardados apenas na memória do programa. Nada é salvo em arquivo, e a lista é esvaziada quando você fecha a última janela do programa. Um item continua na lista depois que o documento de onde veio é fechado. Um item maior que 8 MB não é guardado, e o painel avisa quando isso acontece; uma imagem conta 4 bytes por ponto (pixel), então uma imagem com mais de cerca de 2 milhões de pixels é grande demais. Todos os itens juntos são limitados a 64 MB, e os mais antigos saem primeiro. Qualquer coisa que um gerenciador de senhas marque como secreta nunca é guardada.
