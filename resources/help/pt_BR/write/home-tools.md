# Mudar maiúsculas e minúsculas, classificar e colar de outras formas
<!-- keywords: botão opções de colagem, recortar e colar inteligente, espaços ao colar, linha horizontal, régua, mostrar formatação, inspetor de estilos, shift+f1, tabulações, paradas de tabulação, tabulação de barra, barra, tabulações padrão, distância de tabulação, preenchimento, manter com o próximo, manter linhas juntas, quebra de página antes, viúva, órfã, nível de estrutura de tópicos, quebras de linha e de página, alterar maiúsculas e minúsculas, maiúsculas, minúsculas, maiúscula inicial, colocar em maiúscula, alternar maiúsculas e minúsculas, shift+f3, classificar, classificar parágrafos, ordem alfabética, ordem, colar, colar especial, continuar lista, nova lista, mesclar lista, aninhar tabela, células de planilha, manter somente texto, mesclar formatação, rtf, texto formatado, soltar, arrastar texto, texto não formatado, imagem, área de transferência -->

Estas ferramentas estão na guia **Página Inicial**. Elas funcionam sobre o texto selecionado.

## Alterar as maiúsculas e minúsculas das letras

A **caixa** de uma letra indica se ela é maiúscula (A) ou minúscula (a).

1. Selecione o texto. Sem nada selecionado, o JeffWrite altera a palavra em que está o cursor.
2. No grupo **Fonte**, clique em **Alterar Maiúsculas/Minúsculas**.
3. Escolha uma opção:
   - **Maiúscula no início da frase** deixa a primeira letra de cada frase em maiúscula. As demais ficam em minúsculas.
   - **minúsculas** deixa todas as letras em minúsculas.
   - **MAIÚSCULAS** deixa todas as letras em maiúsculas.
   - **Iniciais Maiúsculas** deixa a primeira letra de cada palavra em maiúscula.
   - **aLTERNAR mAIÚSC./mINÚSC.** troca cada maiúscula por minúscula, e cada minúscula por maiúscula.

Pressione **Shift+F3** para percorrer três delas em sequência: **Iniciais Maiúsculas**, depois **MAIÚSCULAS** e, por fim, **minúsculas**.

O aspecto das letras se mantém. O negrito continua negrito. Pressione **Ctrl+Z** para desfazer a alteração em uma única etapa. Se o Controlar Alterações estiver ativado, a mudança aparece como uma exclusão e uma inserção.

## Classificar parágrafos

Classificar organiza os parágrafos em ordem, como de A a Z. Use para uma lista de nomes ou para linhas de uma tabela feita com tabulações ou vírgulas.

1. Selecione dois ou mais parágrafos.
2. No grupo **Parágrafo**, clique em **Classificar**.
3. Escolha pelo que classificar. Cada opção tem um **campo**, um **tipo** e uma **ordem**.
   - O **campo** é uma parte do parágrafo. O campo 1 é a primeira parte. Suponha que as partes estejam separadas por vírgulas ou tabulações, na caixa abaixo das chaves. Se você deixar em **Parágrafos**, o parágrafo inteiro é o único campo.
   - O **tipo** é **Texto**, **Número** ou **Data**. Um número é classificado pelo seu valor, portanto 9 vem antes de 10.
   - A **ordem** é **Crescente** (de A a Z, do menor para o maior, do mais antigo para o mais recente) ou **Decrescente**.
4. Use **Em seguida por** para desempatar.
5. Marque **A primeira linha é um cabeçalho** para mantê-la no lugar.
6. Clique em **OK**.

Cada parágrafo mantém a sua própria aparência. Com o Controlar Alterações ativado, a classificação é feita normalmente, sem registro, e a barra de status informa que ela não foi registrada como alteração controlada. Não é possível classificar as linhas de uma tabela que tenha uma linha inserida ou excluída: aceite ou rejeite essas alterações antes.

## Colar de outras formas

Clique na pequena seta abaixo de **Colar** para ver mais opções:

- Quando o texto vem de outro programa que coloca texto formatado (RTF) na área de transferência, **Ctrl+V** mantém a aparência: fontes, tamanhos, cores, negrito, listas, tabelas, links e imagens. Se o programa não forneceu texto formatado, o JeffWrite usa a forma de página da web (HTML) e, depois, apenas as palavras. Texto formatado com mais de 16 MB não é lido, e a colagem usa a forma seguinte. Arrastar texto formatado de outro programa para a página faz o mesmo.
- **Mesclar Formatação** cola o texto com a aparência do lugar para onde ele vai. Apenas negrito, itálico, sublinhado e tachado são mantidos.
- **Manter Somente Texto** cola as palavras sem nenhuma aparência.
- **Imagem** cola uma imagem da área de transferência. Ela fica esmaecida quando a área de transferência não tem imagem.
- **Colar Especial** (**Ctrl+Alt+V**) lista todas as formas como a área de transferência pode ser colada agora, e permite escolher uma, como **Texto formatado (RTF)**, **Texto não formatado** ou **HTML**.

Uma opção fica esmaecida quando a área de transferência não tem nada que ela possa usar.

Logo após colar, um pequeno botão **Opções de Colagem** aparece ao lado do cursor. Clique nele para escolher **Manter formatação original**, **Mesclar formatação** ou **Manter somente texto**. O JeffWrite desfaz a colagem e a refaz do jeito escolhido. A alteração é uma única etapa: um único **Desfazer** desfaz a colagem inteira, por mais vezes que você tenha mudado de ideia. Pelo teclado, toque na tecla **Ctrl** (a tecla **Control**, no Mac) depois de colar para abrir o menu do botão e use as setas e **Enter** para escolher. O botão desaparece assim que você digitar, mover o cursor ou pressionar **Esc**, e não é oferecido em um documento que você não pode alterar.

**Definir Colagem Padrão** fica na página **Avançado** de **Opções**. Ela tem uma caixa para colar dentro do mesmo documento, outra para colar entre documentos e outra para colar de outros programas. Cada uma pode ser **Manter formatação original** (como era antes), **Mesclar formatação** ou **Manter somente texto**, e **Colar** (**{key:Paste}**) faz o que estiver definido para a origem do conteúdo. Exemplo: defina a colagem de outros programas como **Manter somente texto**, e as palavras copiadas de uma página da web ficarão com a formatação do lugar em que forem colocadas. Você ainda pode escolher outra forma para qualquer colagem, com o botão **Opções de Colagem**.

Duas outras caixas nessa página definem o que **Colar** faz com itens de lista colados ao lado de uma lista (**Como estão**, **Continuar Lista**, **Nova Lista** ou **Mesclar Lista**) e com células de planilha (**Como palavras**, **Tabela**, **Aninhar Tabela**, **Mesclar Tabela**, **Inserir como Novas Linhas** ou **Substituir Células**). Uma opção só é usada onde faz sentido: **Aninhar Tabela** com o cursor fora de uma tabela, por exemplo, cola as células como palavras.

Quando a colagem trouxe parágrafos de lista ao lado de uma lista, o botão também oferece **Continuar Lista** (a numeração segue a partir da lista ao lado), **Nova Lista** (a numeração recomeça em 1) e **Mesclar Lista** (os itens se juntam à lista ao lado e assumem a sua aparência). O **Ctrl+V** simples coloca os itens como vieram, e uma opção altera isso. As mesmas opções são comandos no menu Colar da guia Página Inicial e em **Colar Especial**, então o teclado também as alcança.

Quando a área de transferência tem células de planilha (copiadas do JeffCrunch ou de outro programa como tabela ou como linhas com tabulações entre as células), o **Ctrl+V** simples as cola como as palavras que são. Para colá-las como células, escolha **Tabela** no botão, no menu Colar ou em **Colar Especial**. Com o cursor dentro de uma tabela, o botão oferece **Aninhar Tabela** (uma tabela dentro da célula), **Mesclar Tabela** (as células se juntam à tabela, que ganha as colunas de que precisa), **Inserir como Novas Linhas**, **Inserir como Novas Colunas** (as células entram em novas colunas depois da célula do cursor, a partir da sua linha para baixo; uma tabela pode ter no máximo 256 colunas e 50.000 células, e a operação é recusada, com uma mensagem, quando uma célula mesclada ultrapassa o lugar necessário para as colunas) e **Substituir Células** (as células substituem as da própria tabela, a partir da célula do cursor). Uma tabela colada tem no máximo 500 linhas e 63 colunas, e uma célula comporta no máximo 4.000 caracteres; o que passar disso é deixado de fora, e uma mensagem avisa. Uma tabela dentro de outra pode ter no máximo 8 níveis de profundidade, e uma tabela aninhada cujas colunas ficariam estreitas demais para a célula é recusada. Cada colagem é uma única etapa: Desfazer desfaz tudo, e Refazer a restaura.

**Recortar e colar inteligente** mantém os espaços corretos ao mover palavras. Se você recortar palavras inteiras que estão entre dois espaços, o JeffWrite também remove um desses espaços, para que não sobrem dois espaços seguidos. Se você colar uma palavra encostada em outra, ele acrescenta um espaço entre elas. Não altera o meio de uma palavra. Para desativar, clique em **Arquivo > Opções > Avançado** e desmarque **Recortar e colar inteligente**.

## Quebras de linha e de página, e tabulações

Clique no pequeno botão no canto do grupo **Parágrafo** para abrir a caixa **Parágrafo**. Abaixo das opções de espaçamento estão estas:

- **Controle de linhas órfãs/viúvas** impede que uma linha isolada de um parágrafo fique sozinha no alto ou na base de uma página.
- **Manter com o próximo** mantém o parágrafo na mesma página que o seguinte. Use para um título.
- **Manter linhas juntas** mantém todas as linhas do parágrafo em uma mesma página.
- **Suprimir números de linha** deixa este parágrafo de fora quando as linhas são numeradas nas páginas (veja Layout > Números de Linha).
- **Quebra de página antes** inicia o parágrafo em uma nova página.
- **Não hifenizar** impede que as palavras do parágrafo sejam divididas com hífen no fim de uma linha.
- **Não adicionar espaço entre parágrafos do mesmo estilo** remove o espaço entre dois parágrafos que têm o mesmo estilo.
- **Nível de estrutura de tópicos** indica onde o parágrafo está na estrutura do documento: **Texto do corpo**, ou um nível de 1 a 9.

Clique em **Tabulações** na caixa para definir paradas de tabulação. Uma parada de tabulação é um ponto para onde o texto salta quando você pressiona **Tab**. Digite a sua posição, escolha como o texto se alinha ali (à esquerda, centralizado, à direita ou decimal) e escolha um preenchimento. O preenchimento é a linha de pontos, traços ou um traço contínuo que preenche o espaço antes da parada. Clique em **Definir** para adicionar. Para remover uma, selecione-a na lista e clique em **Limpar**. **Limpar Tudo** remove todas.

Escolha **Barra** para definir uma barra. Uma barra não interrompe o texto. Ela desenha uma linha fina e reta, de cima a baixo, naquele ponto de cada linha do parágrafo. É uma forma de pôr um filete entre colunas de texto. A barra não tem preenchimento. A régua não mostra as barras. Um parágrafo comporta até 64 delas. Ainda não: em um parágrafo que é lido da direita para a esquerda, as barras continuam sendo medidas a partir da borda esquerda. Defina e limpe as barras na caixa **Tabulações**.

**Tabulações padrão** é a distância entre as paradas que um parágrafo recebe quando não tem paradas próprias e, depois da última parada que ele tem, a distância que se repete. A distância usual é de 1,27 cm. Esta configuração vale para o documento inteiro, não só para o parágrafo em que você está. A nova distância se aplica a todos os parágrafos assim que você clica em **OK**.

## Bordas e sombreamento para parágrafos

Selecione um ou mais parágrafos. No grupo **Parágrafo**, o botão **Bordas** tem as opções rápidas: **Borda Inferior**, **Borda Superior**, **Borda Esquerda**, **Borda Direita**, **Todas as Bordas** e **Sem Borda**. Uma opção rápida desenha uma linha simples, de meio ponto de espessura, a um ponto das palavras. Uma opção para um lado deixa os outros lados como estavam.

**Bordas e Sombreamento**, no fim dessa lista, abre uma caixa. Marque os lados que recebem uma linha, ou clique em **Caixa** ou **Nenhuma**. Marque **Entre** para pôr uma linha entre os parágrafos selecionados que têm as mesmas bordas. Depois escolha o **Estilo** (simples, duplo, grosso, pontilhado, tracejado, traços curtos, traço-ponto ou traço-ponto-ponto), a **Largura**, a **Cor** e a **Distância do texto**. **Sombreamento** preenche os parágrafos com uma cor. O botão **Sombreamento**, ao lado de **Bordas**, define apenas o preenchimento, com as mesmas cores de **Cor da fonte**.

As linhas e o preenchimento formam uma única etapa que **Desfazer** desfaz, e são mantidos ao salvar um arquivo .docx ou .odt. Se um parágrafo veio de um arquivo com borda ou preenchimento próprios, a sua escolha substitui essa formatação.

## Desenhar uma linha horizontal

Clique em **Linha Horizontal** no grupo **Parágrafo**. O JeffWrite acrescenta um parágrafo próprio com uma linha fina embaixo, depois do parágrafo em que está o cursor. O cursor passa para um parágrafo comum abaixo da linha. Para retirar a linha, selecione o seu parágrafo e exclua-o.

## Ver o que está definido no local do cursor

Pressione **Shift+F1** ou use **Mostrar Formatação**. Uma caixa lista os estilos, a fonte, o tamanho, o aspecto das letras, a cor e o alinhamento, os recuos e o espaçamento do parágrafo no cursor. Ela apenas mostra essas configurações. Para alterá-las, use as caixas **Fonte** e **Parágrafo**.

## O painel da Área de Transferência

O painel Área de Transferência guarda os últimos 24 itens copiados ou recortados nesta janela do programa, para que você possa colar um item mais antigo sem copiá-lo de novo. Clique na pequena seta no canto do grupo **Área de Transferência** na guia **Página Inicial** (ou pressione **Alt**, depois **H** e depois **FO**) para abri-lo. Um painel aparece ao lado da página.

Cada item mostra uma pequena prévia e o tipo de conteúdo: texto, texto formatado, células, uma imagem, formas ou slides. O mais recente fica no topo. Clique em um item para colá-lo onde estão o cursor ou a seleção. Ele é colado como qualquer outra colagem: uma etapa de **Desfazer** o desfaz, e o botão Opções de Colagem aparece normalmente.

- **Colar Tudo** cola todos os itens, do mais antigo para o mais recente, para que apareçam na ordem em que foram copiados.
- **Excluir** remove o item escolhido. **Limpar Tudo** remove todos.
- O painel só coleta enquanto estiver aberto. Para continuar coletando depois de fechá-lo, marque **Continuar coletando quando este painel for fechado**. A escolha fica guardada, e a coleta começa quando o programa é iniciado, antes de você abrir o painel.
- Para coletar também o que você copia em outros programas, marque **Coletar também o que for copiado em outros programas**. Apenas texto simples e imagens são guardados, e essa opção é desativada de novo sempre que você abre o programa. Com ela ativada, uma cópia feita em outro programa é lida por inteiro antes de este programa decidir se a guarda; por isso, uma cópia muito grande pode fazer este programa pausar por um momento e usar tanta memória quanto o tamanho da cópia. Só uma forma dela é lida: o seu texto, quando há texto, ou então uma imagem. Com a opção desativada, nada copiado em outros programas é lido.

Os itens ficam guardados apenas na memória do programa. Nada é salvo em arquivo, e a lista é esvaziada quando você fecha a última janela do programa. Um item permanece na lista mesmo depois de o documento de origem ser fechado. Um item maior que 8 MB não é guardado, e o painel avisa quando isso acontece; uma imagem conta 4 bytes por ponto (pixel), de modo que uma imagem com mais de cerca de 2 milhões de pixels é grande demais. Todos os itens juntos ficam limitados a 64 MB, e os mais antigos saem primeiro. Qualquer coisa que um gerenciador de senhas marque como secreta nunca é guardada.
