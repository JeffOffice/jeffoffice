# Alterar maiúsculas e minúsculas, ordenar e colar de outras formas
<!-- keywords: botão opções de colagem, recorte e colagem inteligentes, espaços ao colar, linha horizontal, régua, mostrar formatação, inspetor de estilos, shift+f1, tabulações, posições de tabulação, barra de tabulação, posição de barra, posições de tabulação predefinidas, distância de tabulação, carácter de preenchimento, manter com o seguinte, manter linhas juntas, quebra de página antes, linha isolada, linha órfã, nível de contorno, quebras de linha e de página, alterar maiúsculas, maiúsculas, minúsculas, maiúscula inicial, capitalizar, inverter maiúsculas, shift+f3, ordenar, ordenar parágrafos, alfabética, ordem, colar, colar especial, continuar lista, nova lista, unir lista, aninhar tabela, células de folha de cálculo, manter apenas texto, unir formatação, rtf, texto enriquecido, texto formatado, arrastar, arrastar texto, texto sem formatação, imagem, área de transferência -->

Estas ferramentas estão no separador **Base**. Atuam sobre o texto que selecionar.

## Alterar as maiúsculas e minúsculas das letras

A **capitalização** de uma letra indica se ela é maiúscula (A) ou minúscula (a).

1. Selecione o texto. Se não tiver nada selecionado, o JeffWrite altera a palavra onde está o cursor.
2. No grupo **Tipo de Letra**, clique em **Alterar Maiúsculas/Minúsculas**.
3. Escolha uma opção:
   - **Maiúscula inicial da frase** põe em maiúscula a primeira letra de cada frase. As restantes ficam em minúsculas.
   - **minúsculas** põe todas as letras em minúsculas.
   - **MAIÚSCULAS** põe todas as letras em maiúsculas.
   - **Maiúscula Inicial das Palavras** põe em maiúscula a primeira letra de cada palavra.
   - **aLTERNAR mAIÚSCULAS** transforma as maiúsculas em minúsculas e as minúsculas em maiúsculas.

Prima **Shift+F3** para percorrer três destas opções por ordem: **Maiúscula Inicial das Palavras**, depois **MAIÚSCULAS** e, por fim, **minúsculas**.

O aspeto das letras mantém-se. O negrito continua negrito. Prima **Ctrl+Z** para anular a alteração de uma só vez. Se o Controlo de Alterações estiver ativado, a alteração aparece como uma eliminação e uma inserção.

## Ordenar parágrafos

Ordenar coloca os parágrafos por ordem, por exemplo de A a Z. Use esta função para uma lista de nomes ou para linhas de uma tabela feita com tabulações ou vírgulas.

1. Selecione dois ou mais parágrafos.
2. No grupo **Parágrafo**, clique em **Ordenar**.
3. Escolha o critério de ordenação. Cada critério tem um **campo**, um **tipo** e uma **ordem**.
   - O **campo** é uma parte do parágrafo. O campo 1 é a primeira parte. Suponha que as partes estão separadas por vírgulas ou tabulações; isto é indicado na caixa abaixo das chaves. Se deixar **Parágrafos**, o parágrafo inteiro é o único campo.
   - O **tipo** é **Texto**, **Número** ou **Data**. Um número é ordenado pelo seu valor, por isso 9 vem antes de 10.
   - A **ordem** é **Crescente** (de A a Z, do menor para o maior, do mais antigo para o mais recente) ou **Decrescente**.
4. Use **Depois por** para desempatar.
5. Marque **O primeiro parágrafo é um cabeçalho** para o deixar no mesmo lugar.
6. Clique em **OK**.

Cada parágrafo mantém o seu próprio aspeto. Enquanto o Controlo de Alterações estiver ativado, a ordenação é feita como de costume, sem registo, e a barra de estado indica que não foi registada como alteração controlada. Não é possível ordenar as linhas de uma tabela que tenha uma linha inserida ou eliminada: aceite ou rejeite primeiro essas alterações.

## Colar de outras formas

Clique na pequena seta por baixo de **Colar** para ver mais opções:

- Quando o texto vem de outro programa que coloca texto enriquecido (RTF) na área de transferência, **Ctrl+V** mantém o seu aspeto: tipos de letra, tamanhos, cores, negrito, listas, tabelas, ligações e imagens. Se o programa não tiver fornecido texto enriquecido, o JeffWrite usa a forma de página Web (HTML) e, depois, as palavras simples. O texto enriquecido com mais de 16 MB não é lido, e a colagem usa a forma seguinte. Arrastar texto formatado de outro programa para a página faz o mesmo.
- **Unir Formatação** cola o texto com o aspeto do local para onde vai. Só se mantêm o negrito, o itálico, o sublinhado e o riscado.
- **Manter Apenas Texto** cola as palavras sem qualquer aspeto.
- **Imagem** cola uma imagem da área de transferência. Fica a cinzento quando a área de transferência não tem nenhuma imagem.
- **Colar Especial** (**Ctrl+Alt+V**) lista todas as formas como a área de transferência pode ser colada neste momento e deixa-o escolher uma, como **Texto formatado (RTF)**, **Texto sem formatação** ou **HTML**.

Uma opção fica a cinzento quando a área de transferência não tem nada que possa usar.

Logo após colar, aparece um pequeno botão **Opções de Colagem** junto ao cursor. Clique nele para escolher **Manter Formatação de Origem**, **Unir Formatação** ou **Manter Apenas Texto**. O JeffWrite anula a colagem e refaz-na dessa forma. A alteração é um único passo: um **Anular** basta para desfazer toda a colagem, por mais vezes que mude de ideias. A partir do teclado, prima a tecla **Ctrl** (a tecla **Control** num Mac) depois de colar para abrir o menu do botão e use as teclas de seta e **Enter** para escolher. O botão desaparece assim que escrever, mover o cursor ou premir **Esc**, e não é oferecido num documento que não pode ser alterado.

**Definir Colagem Predefinida** está na página **Avançadas** de **Opções**. Tem uma caixa para colar dentro do mesmo documento, outra para colar entre documentos e outra para colar de outros programas. Cada uma pode ser **Manter Formatação de Origem** (como era antes), **Unir Formatação** ou **Manter Apenas Texto**, e **Colar** (**{key:Paste}**) faz o que estiver definido para a origem do conteúdo. Por exemplo: defina a colagem de outros programas como **Manter Apenas Texto** e as palavras copiadas de uma página Web ficam com a formatação do local para onde forem. Pode sempre escolher outra forma para uma colagem específica com o botão **Opções de Colagem**.

Duas outras caixas dessa página definem o que **Colar** faz com itens de lista colados junto a uma lista (**Conforme chegam**, **Continuar Lista**, **Nova Lista** ou **Unir Lista**) e com células de folha de cálculo (**Como palavras**, **Tabela**, **Aninhar Tabela**, **Unir Tabela**, **Inserir como Novas Linhas** ou **Substituir Células**). Uma escolha só é usada quando se aplica: **Aninhar Tabela** com o cursor fora de uma tabela, por exemplo, cola as células como palavras.

Quando a colagem trouxe parágrafos de lista junto a uma lista, o botão também oferece **Continuar Lista** (a numeração continua a partir da lista ao lado), **Nova Lista** (a numeração recomeça em 1) e **Unir Lista** (os itens juntam-se à lista ao lado e tomam o seu aspeto). Um **Ctrl+V** simples coloca os itens como vieram, e uma escolha altera isso. As mesmas opções são comandos no menu Colar do separador Base e em **Colar Especial**, por isso também estão acessíveis pelo teclado.

Quando a área de transferência tem células de folha de cálculo (copiadas do JeffCrunch ou de outro programa como tabela ou como linhas com tabulações entre as células), um **Ctrl+V** simples cola-as como as palavras que são. Para as colar como células, escolha **Tabela** no botão, no menu Colar ou em **Colar Especial**. Com o cursor dentro de uma tabela, o botão oferece **Aninhar Tabela** (uma tabela dentro da célula), **Unir Tabela** (as células juntam-se à tabela, que ganha as colunas de que precisa), **Inserir como Novas Linhas**, **Inserir como Novas Colunas** (as células vão para novas colunas depois da célula do cursor, a partir da sua linha para baixo; uma tabela pode ter no máximo 256 colunas e 50 000 células, e a operação é recusada, com uma mensagem, quando uma célula unida atravessa o local destinado às colunas) e **Substituir Células** (as células substituem as da tabela, a partir da célula do cursor). Uma tabela colada tem no máximo 500 linhas e 63 colunas, e uma célula contém no máximo 4000 carateres; o que ultrapassar isso fica de fora, e uma mensagem informa. Uma tabela dentro de outra pode ter no máximo 8 níveis de profundidade, e uma tabela aninhada cujas colunas ficassem demasiado estreitas para a célula é recusada. Cada colagem é um único passo: **Anular** desfaz tudo e **Refazer** volta a colocar.

O **recorte e colagem inteligentes** mantém os espaços corretos ao mover palavras. Se recortar palavras completas que estão entre dois espaços, o JeffWrite também retira um desses espaços, para não ficar com dois espaços seguidos. Se colar uma palavra encostada a outra, acrescenta um espaço entre elas. Não altera o interior de uma palavra. Para o desativar, clique em **Ficheiro > Opções > Avançadas** e desmarque **Recorte e colagem inteligentes**.

## Quebras de linha e de página, e tabulações

Clique no pequeno botão no canto do grupo **Parágrafo** para abrir a caixa **Parágrafo**. Por baixo das opções de espaçamento estão estas:

- **Controlo de linhas isoladas e órfãs** impede que uma linha sozinha de um parágrafo fique isolada no topo ou na base de uma página.
- **Manter com o seguinte** mantém o parágrafo na mesma página que o seguinte. Use-o para um título.
- **Manter linhas juntas** mantém todas as linhas do parágrafo numa só página.
- **Suprimir números de linha** deixa este parágrafo de fora quando as linhas são numeradas por página (veja Esquema > Números de Linha).
- **Quebra de página antes** começa o parágrafo numa nova página.
- **Não hifenizar** impede que as palavras do parágrafo sejam divididas com um hífen no fim de uma linha.
- **Não adicionar espaço entre parágrafos do mesmo estilo** remove o espaço entre dois parágrafos que têm o mesmo estilo.
- **Nível de contorno** indica onde o parágrafo se encontra na estrutura do documento: **Texto normal**, ou um nível de 1 a 9.

Clique em **Tabulações** na caixa para definir posições de tabulação. Uma posição de tabulação é o sítio para onde o texto salta quando prime **Tab**. Escreva a sua posição, escolha como o texto se alinha ali (à esquerda, ao centro, à direita ou decimal) e escolha um carácter de preenchimento. Um carácter de preenchimento é a linha de pontos, traços ou um traço contínuo que preenche o espaço antes da posição. Clique em **Definir** para a acrescentar. Para remover uma, selecione-a na lista e clique em **Limpar**. **Limpar Tudo** remove todas.

Escolha **Barra** para definir uma barra. Uma barra não interrompe o texto. Desenha uma linha reta fina, de cima a baixo, nesse ponto em cada linha do parágrafo. É uma forma de pôr uma linha separadora entre colunas de texto. Uma barra não tem carácter de preenchimento. A régua não mostra as barras. Um parágrafo mantém até 64 barras. Ainda não está disponível: num parágrafo que se lê da direita para a esquerda, as barras continuam a ser medidas a partir da margem esquerda. Defina e limpe as barras na caixa **Tabulações**.

**Posições de tabulação predefinidas** é a distância entre as posições que um parágrafo recebe quando não tem posições próprias, e para além da última posição que tiver. A distância habitual é de meia polegada (cerca de 1,27 cm). Esta definição aplica-se a todo o documento, não apenas ao parágrafo em que está. A nova distância aplica-se a todos os parágrafos assim que clicar em **OK**.

## Limites e sombreado para parágrafos

Selecione um ou mais parágrafos. No grupo **Parágrafo**, o botão **Limites** tem as opções rápidas: **Limite Inferior**, **Limite Superior**, **Limite Esquerdo**, **Limite Direito**, **Todos os Limites** e **Sem Limites**. Uma opção rápida desenha uma linha simples, com meio ponto de espessura, a um ponto do texto. Uma opção para um só lado deixa os outros lados como estavam.

**Limites e Sombreado**, no fim dessa lista, abre uma caixa. Marque os lados que devem ter uma linha, ou clique em **Caixa** ou **Nenhum**. Marque **Entre** para pôr uma linha entre os parágrafos selecionados que têm os mesmos limites. Depois escolha o **Estilo** (simples, duplo, espesso, pontilhado, tracejado, traços curtos, traço-ponto ou traço-ponto-ponto), a **Espessura**, a **Cor** e a **Distância do texto**. **Sombreado** preenche os parágrafos com uma cor. O botão **Sombreado**, ao lado de **Limites**, define apenas o preenchimento, com as mesmas cores de **Cor do tipo de letra**.

As linhas e o preenchimento são um único passo que **Anular** desfaz, e mantêm-se quando guarda um ficheiro .docx ou .odt. Se um parágrafo veio de um ficheiro com limite ou preenchimento próprios, a sua escolha substitui-os.

## Desenhar uma linha horizontal

Clique em **Linha Horizontal** no grupo **Parágrafo**. O JeffWrite acrescenta um parágrafo próprio com uma linha fina por baixo, depois do parágrafo onde está o cursor. O cursor passa para um parágrafo simples abaixo da linha. Para retirar a linha, selecione o respetivo parágrafo e elimine-o.

## Ver o que está definido onde está o cursor

Prima **Shift+F1**, ou use **Mostrar Formatação**. Uma caixa lista os estilos, o tipo de letra, o tamanho, o aspeto das letras, a cor e o alinhamento, os avanços e o espaçamento do parágrafo no cursor. Apenas os mostra. Para os alterar, use as caixas **Tipo de Letra** e **Parágrafo**.

## O painel Área de Transferência

O painel Área de Transferência memoriza os últimos 24 itens que copiou ou recortou neste programa, na janela atual, para poder colar um item mais antigo sem o copiar de novo. Clique na pequena seta no canto do grupo **Área de Transferência** no separador **Base** (ou prima **Alt**, depois **H** e depois **FO**) para o abrir. Surge um painel ao lado da página.

Cada item mostra uma pequena pré-visualização e o tipo de conteúdo: texto, texto formatado, células, uma imagem, formas ou diapositivos. O mais recente fica no topo. Clique num item para o colar onde estiver o cursor ou a seleção. É colado como qualquer outra colagem: um passo de **Anular** desfaz-o, e o botão Opções de Colagem aparece como de costume.

- **Colar Tudo** cola todos os itens, do mais antigo para o mais recente, para que fiquem na ordem em que os copiou.
- **Eliminar** remove o item escolhido. **Limpar Tudo** remove todos.
- O painel só recolhe itens enquanto estiver aberto. Para continuar a recolher depois de o fechar, marque **Continuar a recolher quando este painel for fechado**. A escolha fica memorizada, e a recolha começa quando o programa arranca, antes de abrir o painel.
- Para recolher também o que copia noutros programas, marque **Recolher também o que é copiado noutros programas**. Só são guardados texto simples e imagens, e esta opção é desativada de novo sempre que abre o programa. Com esta opção ativa, uma cópia feita noutro programa é lida por completo antes de este programa decidir se a guarda; por isso, uma cópia muito grande pode fazer o programa pausar por um momento e usar tanta memória quanto o tamanho da cópia. Só é lida uma forma da cópia: o texto, quando o tem; caso contrário, uma imagem. Com esta opção desativada, nada copiado noutros programas é lido.

Os itens são guardados apenas na memória do programa. Nada é gravado num ficheiro, e a lista é esvaziada quando fecha a última janela do programa. Um item permanece na lista depois de o documento de origem ser fechado. Um item com mais de 8 MB não é guardado, e o painel informa quando isso acontece; uma imagem conta 4 bytes por cada ponto (píxel), por isso uma imagem com mais de cerca de 2 milhões de píxeis é demasiado grande. Todos os itens juntos ficam limitados a 64 MB, e os mais antigos são retirados primeiro. Tudo o que um gestor de palavras-passe marque como secreto nunca é guardado.
