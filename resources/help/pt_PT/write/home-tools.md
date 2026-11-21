# Alterar maiúsculas e minúsculas, ordenar e colar de outras formas
<!-- keywords: botão opções de colagem, recortar e colar inteligente, espaços ao colar, linha horizontal, régua, mostrar formatação, painel de estilos, shift+f1, tabulações, posições de tabulação, barra de tabulação, posição de barra, posições de tabulação predefinidas, distância de tabulação, carácter de preenchimento, manter com o seguinte, manter linhas juntas, quebra de página antes, linhas isoladas, órfãs, nível de contorno, quebras de linha e de página, maiúsculas e minúsculas, maiúsculas, minúsculas, maiúscula inicial da frase, pôr em maiúscula, alternar maiúsculas, shift+f3, ordenar, ordenar parágrafos, alfabética, ordem, colar, colar especial, continuar lista, nova lista, unir lista, aninhar tabela, células da folha de cálculo, manter apenas texto, unir formatação, rtf, texto formatado, texto enriquecido, arrastar texto, texto sem formatação, imagem, área de transferência -->

Estes comandos estão no separador **Base**. Atuam sobre o texto que selecionar.

## Alterar as maiúsculas e minúsculas das letras

A **capitalização** de uma letra indica se é uma letra maiúscula (A) ou minúscula (a).

1. Selecione o texto. Sem nada selecionado, o JeffWrite altera a palavra onde está o cursor.
2. No grupo **Tipo de Letra**, clique em **Maiúsculas/Minúsculas**.
3. Escolha uma opção:
   - **Maiúscula inicial da frase** põe em maiúscula a primeira letra de cada frase. As restantes ficam em minúsculas.
   - **minúsculas** coloca todas as letras em minúsculas.
   - **MAIÚSCULAS** coloca todas as letras em maiúsculas.
   - **Maiúscula Inicial das Palavras** põe em maiúscula a primeira letra de cada palavra.
   - **aLTERNAR mAIÚSCULAS** transforma cada maiúscula em minúscula, e cada minúscula em maiúscula.

Prima **Shift+F3** para percorrer três destas opções por ordem: **Maiúscula Inicial das Palavras**, depois **MAIÚSCULAS**, e depois **minúsculas**.

O aspeto das letras mantém-se. O negrito continua negrito. **Ctrl+Z** anula a alteração num só passo. Se o controlo de alterações estiver ativado, a alteração aparece como uma eliminação e uma inserção.

## Ordenar parágrafos

Ordenar coloca os parágrafos por ordem, como de A a Z. Use esta função para uma lista de nomes, ou para linhas de uma tabela feita com tabulações ou vírgulas.

1. Selecione dois ou mais parágrafos.
2. No grupo **Parágrafo**, clique em **Ordenar**.
3. Escolha o critério de ordenação. Cada critério tem um **campo**, um **tipo** e uma **ordem**.
   - O **campo** é uma parte do parágrafo. O campo 1 é a primeira parte. Se as partes estiverem separadas por vírgulas ou tabulações, indique-o na caixa abaixo das chaves. Se deixar **Parágrafos**, o parágrafo inteiro é o único campo.
   - O **tipo** é **Texto**, **Número** ou **Data**. Um número ordena-se pelo seu valor, pelo que 9 vem antes de 10.
   - A **ordem** é **Crescente** (de A a Z, do menor para o maior, do mais antigo para o mais recente) ou **Decrescente**.
4. Use **Depois por** para desempatar.
5. Marque **O primeiro parágrafo é um cabeçalho** para o deixar no mesmo lugar.
6. Clique em **OK**.

Cada parágrafo mantém o seu próprio aspeto. Não é possível ordenar com o controlo de alterações ativado, porque uma ordenação não é uma alteração a mostrar. Desative o controlo de alterações, ordene e volte a ativá-lo.

## Colar de outras formas

Clique na pequena seta abaixo de **Colar** para ver mais opções:

- Quando o texto vem de outro programa que coloca texto formatado (RTF) na área de transferência, **Ctrl+V** mantém o seu aspeto: tipos de letra, tamanhos, cores, negrito, listas, tabelas, ligações e imagens. Se o programa não deu texto formatado, o JeffWrite usa a forma de página Web (HTML) e, depois, apenas as palavras. Texto formatado com mais de 16 MB não é lido, e a colagem passa para a forma seguinte. Arrastar texto formatado de outro programa para a página faz o mesmo.
- **Unir Formatação** cola o texto com o aspeto do local para onde vai. Só se mantêm o negrito, o itálico, o sublinhado e o rasurado.
- **Manter Apenas Texto** cola as palavras sem qualquer aspeto.
- **Imagem** cola uma imagem da área de transferência. Fica a cinzento quando a área de transferência não tem nenhuma imagem.
- **Colar Especial** (**Ctrl+Alt+V**) lista todas as formas como a área de transferência pode ser colada neste momento, e permite escolher uma, como **Texto formatado (RTF)**, **Texto sem formatação** ou **HTML**.

Uma opção fica a cinzento quando a área de transferência não tem nada que possa usar.

Logo após colar, aparece junto ao cursor um pequeno botão **Opções de Colagem**. Clique nele para escolher **Manter Formatação de Origem**, **Unir Formatação** ou **Manter Apenas Texto**. O JeffWrite desfaz a colagem e volta a fazê-la dessa forma. A alteração é um só passo: um único **Anular** desfaz toda a colagem, por mais vezes que mude de ideias. Com o teclado, prima a tecla **Ctrl** (a tecla **Control** num Mac) depois de colar para abrir o menu do botão, e depois use as setas e **Enter** para escolher. O botão desaparece assim que escrever, mover o cursor ou premir **Esc**, e não é oferecido num documento que não possa alterar.

**Predefinir Colagem** está na página **Avançadas** de **Opções**. Tem uma caixa para colar dentro do mesmo documento, outra para colar entre documentos, e outra para colar de outros programas. Cada uma pode ser **Manter Formatação de Origem** (como era antes), **Unir Formatação** ou **Manter Apenas Texto**, e **Colar** (**{key:Paste}**) faz o que estiver definido para a origem do conteúdo. Exemplo: defina colar de outros programas como **Manter Apenas Texto**, e as palavras copiadas de uma página Web ficam com a formatação do local onde são coladas. Pode ainda escolher outra forma para uma colagem específica com o botão **Opções de Colagem**.

Quando a colagem trouxer parágrafos de lista junto de uma lista, o botão oferece também **Continuar Lista** (os números prosseguem a partir da lista ao lado), **Nova Lista** (a numeração recomeça em 1) e **Unir Lista** (os itens juntam-se à lista ao lado e assumem o seu aspeto). **Ctrl+V** simples coloca os itens tal como vieram, e uma escolha altera esse comportamento. As mesmas opções são comandos no menu Colar do separador Base e em **Colar Especial**, para que também possam ser usadas com o teclado.

Quando a área de transferência tiver células de uma folha de cálculo (copiadas do JeffCrunch ou de outro programa como tabela ou como linhas com tabulações entre as células), **Ctrl+V** simples cola-as como as palavras que são. Para as colar como células, escolha **Tabela** no botão, no menu Colar ou em **Colar Especial**. Com o cursor dentro de uma tabela, o botão oferece **Aninhar Tabela** (uma tabela dentro da célula), **Unir Tabela** (as células juntam-se à tabela, que recebe as colunas de que precisa), **Inserir como Novas Linhas**, **Inserir como Novas Colunas** (as células são colocadas em novas colunas depois da célula do cursor, a partir da sua linha para baixo; uma tabela pode ter, no máximo, 256 colunas e 50 000 células, e a operação é recusada, com uma mensagem, quando uma célula unida atravessa o local das colunas) e **Substituir Células** (as células substituem as da tabela, a partir da célula do cursor). Uma tabela colada tem, no máximo, 500 linhas e 63 colunas, e uma célula contém, no máximo, 4000 carateres; o que exceder esse limite é omitido, e uma mensagem informa disso. Uma tabela aninhada numa tabela pode ter, no máximo, 8 níveis de profundidade, e uma tabela aninhada cujas colunas ficassem demasiado estreitas para a célula é recusada. Cada colagem é um só passo: **Anular** desfaz tudo, e **Refazer** volta a colocá-lo.

**Recortar e colar inteligente** mantém os espaços corretos ao mover palavras. Se recortar palavras completas que estejam entre dois espaços, o JeffWrite retira também um desses espaços, para não ficar com dois espaços seguidos. Se colar uma palavra encostada a outra, acrescenta um espaço entre elas. Não altera o interior de uma palavra. Para desativar esta função, clique em **Ficheiro > Opções > Avançadas** e desmarque **Recortar e colar inteligente**.

## Quebras de linha e de página, e tabulações

Clique no pequeno botão no canto do grupo **Parágrafo** para abrir a caixa **Parágrafo**. Por baixo das opções de espaçamento estão estas:

- **Controlo de linhas isoladas e órfãs** impede que uma linha isolada de um parágrafo fique sozinha no topo ou no fundo de uma página.
- **Manter com o seguinte** mantém o parágrafo na mesma página que o seguinte. Use-o para um título.
- **Manter linhas juntas** mantém todas as linhas do parágrafo numa só página.
- **Suprimir números de linha** deixa este parágrafo de fora quando as linhas são numeradas por página (ver Esquema > Números de Linha).
- **Quebra de página antes** começa o parágrafo numa página nova.
- **Não hifenizar** impede que as palavras do parágrafo sejam divididas com um hífen no fim de uma linha.
- **Não adicionar espaço entre parágrafos do mesmo estilo** remove o espaço entre dois parágrafos que têm o mesmo estilo.
- **Nível de contorno** indica o lugar do parágrafo na estrutura do documento: **Texto do corpo**, ou um nível de 1 a 9.

Clique em **Tabulações** na caixa para definir posições de tabulação. Uma posição de tabulação é um lugar para onde o texto salta quando prime **Tab**. Escreva a posição, escolha o alinhamento do texto nesse ponto (à esquerda, ao centro, à direita ou decimal) e escolha um carácter de preenchimento. Um carácter de preenchimento é a linha de pontos, traços ou um risco que preenche o espaço antes da posição. Clique em **Definir** para a acrescentar. Para remover uma, selecione-a na lista e clique em **Limpar**. **Limpar Tudo** remove todas.

Escolha **Barra** para definir uma barra. Uma barra não interrompe o texto. Desenha um traço fino, vertical, nesse ponto em cada linha do parágrafo. É uma forma de colocar um separador entre colunas de texto. Uma barra não tem carácter de preenchimento. A régua não mostra as barras. Um parágrafo mantém até 64 barras. Ainda não está disponível: num parágrafo que se lê da direita para a esquerda, as barras continuam a ser medidas a partir da margem esquerda. Defina-as e elimine-as na caixa **Tabulações**.

**Posições de tabulação predefinidas** é a distância entre as posições que um parágrafo recebe quando não tem posições próprias, e para além da última posição que tiver. A distância habitual é de 1,25 cm. Esta definição aplica-se ao documento inteiro, e não apenas ao parágrafo onde está. A nova distância aplica-se a todos os parágrafos assim que clicar em **OK**.

## Limites e sombreado para parágrafos

Selecione um ou mais parágrafos. No grupo **Parágrafo**, o botão **Limites** tem as escolhas rápidas: **Limite Inferior**, **Limite Superior**, **Limite Esquerdo**, **Limite Direito**, **Todos os Limites** e **Sem Limite**. Uma escolha rápida desenha uma linha simples, com meio ponto de espessura, a um ponto do texto. Uma escolha para um lado deixa os outros lados como estavam.

**Limites e Sombreado**, no fim dessa lista, abre uma caixa. Marque os lados que devem ter uma linha, ou clique em **Caixa** ou **Nenhum**. Marque **Entre** para colocar uma linha entre os parágrafos selecionados que têm os mesmos limites. Depois escolha o **Estilo** (simples, duplo, grosso, pontilhado, tracejado, traços curtos, traço-ponto ou traço-ponto-ponto), a **Espessura**, a **Cor** e a **Distância do texto**. **Sombreado** preenche os parágrafos com uma cor. O botão **Sombreado**, ao lado de **Limites**, define apenas o preenchimento, com as mesmas cores de **Cor do tipo de letra**.

As linhas e o preenchimento são um só passo que **Anular** desfaz, e são mantidos quando guarda um ficheiro .docx ou .odt. Se um parágrafo vier de um ficheiro com limite ou preenchimento próprios, a sua escolha substitui-os.

## Desenhar uma linha horizontal

Clique em **Linha Horizontal** no grupo **Parágrafo**. O JeffWrite acrescenta um parágrafo próprio com uma linha fina por baixo, depois do parágrafo onde está o cursor. O cursor passa para um parágrafo simples abaixo da linha. Para retirar a linha, selecione o seu parágrafo e elimine-o.

## Ver o que está definido onde está o cursor

Prima **Shift+F1**, ou use **Mostrar Formatação**. Uma caixa lista os estilos, o tipo de letra, o tamanho, o aspeto das letras, a cor e o alinhamento, os avanços e o espaçamento do parágrafo na posição do cursor. Apenas os mostra. Para os alterar, use as caixas **Tipo de Letra** e **Parágrafo**.
